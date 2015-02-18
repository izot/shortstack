/*
 * IzoT ShortStack for Raspberry Pi Simple Example
 *
 * ldvq.c implements a simple protected queue for use with the example driver
 * for Raspberry Pi. See ldvq.h for general comments, and see below for
 * specific comments about this implementation.
 *
 * It is important to note that the ShortStack API does not access the Ldvq*
 * functions provided by this module; only the example driver does. When you
 * implement your ShortStack serial link-layer driver, you are free to
 * implement a different strategy.
 *
 * Any ShortStack link-layer driver needs at least one, preferably two or
 * more, frame buffers for each direction; uplink (from the Micro Server)
 * and downlink (to the Micro Server). Asynchronous driver implementations
 * (those using interrupts or concurrent processing methods such as threads)
 * will generally also need synchronization tools such as semaphores or
 * mutexes to provided protected access to these buffers.
 *
 * This example implementation provides protected queues. Protection uses
 * POSIX mutexes, queues are implemented as a simple linked list. This
 * implementation allocates frame buffers off the heap and therefore has
 * virtually unlimited buffers in either direction.
 *
 * Some implementations may wish to use the heap but limit the number of
 * frame buffers to a configured maximum. This example supports such a
 * configuration with the MAX_FRAMES definition, below.
 *
 * A common alternative is the implementation of a statically allocated
 * pool of buffers, from which buffers are allocated and to which buffers
 * are released, and a statically allocated ringbuffer of finite size to
 * implement the queues. Such an implementation would not require dynamic
 * memory allocation and a heap, but an application may run out of
 * available buffer space adn may need to re-attempt the sending of output
 * values or application messages later in time.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ldvq.h"

/*
 * Macro: MAX_FRAMES
 *
 * Set to 0 for unlimited use of the heap, set to N to limit the number of
 * frame buffers per queue to N.
 * When all configured buffers are in use, further API invocations which
 * require a buffer fail with an appropriate error code.
 * Typical driver implementations require two queues, one for each direction.
 */
#define MAX_FRAMES  16

/*
 * QItem holds one element within a queue.
 */
typedef struct QItem {
    struct QItem* next;
    LonSmipMsg* data;
} QItem;

/*
 * QCtrl is the queue control data. The queue handle obtained from LdvqOpen
 * is a pointer to this structure, cast to a suitable scalar base type
 * (<LdvqHandle>) to isolate this definition from the calling layers.
 */
typedef struct {
    QItem* head;
    QItem* tail;
    pthread_mutex_t mutex;
#if MAX_FRAMES
    unsigned allocated;
#endif
} QCtrl;

/*
 * To use this simple queue, create one with LdvqOpen() and keep the handle
 * returned. LdvqOpen() returns 0 for failure.
 */
LdvqHandle LdvqOpen(void)
{
    QCtrl* q = (QCtrl*) malloc(sizeof(QCtrl));

    if (q) {
        memset(q, 0, sizeof(QCtrl));
        pthread_mutex_init(&q->mutex, NULL);
    }

    return (LdvqHandle) q;
}   // LdvqOpen

/*
 * When done, call LdvqClose(). The handle may not be used after this.
 * Because the function destroys the queue (and any remaining data),
 * it is assumed that all participating threads or interrupts are
 * terminated by the time of this call.
 */
void LdvqClose(LdvqHandle handle)
{
    QCtrl* q = (QCtrl*) handle;

    if (q) {
        QItem* current = q->head;

        while (current) {
            QItem* next = current->next;
            free(current->data);
            free(current);
            current = next;
        }

        pthread_mutex_destroy(&q->mutex);
    }

    free(q);
}   // LdvqClose

/*
 * LdvqPush() pushes data to the end of the queue. This must be allocated
 * with <LdvqAlloc>. Also see <LdvqCopy>.
 */
LonApiError LdvqPush(LdvqHandle handle, LonSmipMsg* data)
{
    LonApiError result = LonApiNoError;
    QCtrl* q = (QCtrl*) handle;

    if (q) {
        QItem* item = (QItem*) malloc(sizeof(QItem));

        if (item) {

            pthread_mutex_lock(&q->mutex);

            item->data = data;
            item->next = NULL;

            if (q->tail) {
                q->tail->next = item;
            }

            q->tail = item;

            if (q->head == NULL) {
                q->head = item;
            }

            pthread_mutex_unlock(&q->mutex);
        } else {
            result = LonApiTxBufIsFull;
        }
    } else {
        result = LonApiQueueNotOpen;
    }

    return result;
}   // LdvqPush

/*
 * LdvqCopy() pushes a copy of data (size bytes) to the end of the queue.
 * Also see LdvqPush().
 */
LonApiError LdvqCopy(LdvqHandle handle, LonSmipMsg* data)
{
    LonSmipMsg* duplicate = NULL;
    LonApiError result = LdvqAlloc(handle, &duplicate);

    if (result == LonApiNoError) {
        memcpy(duplicate, data, sizeof(LonSmipMsg));
        result = LdvqPush(handle, duplicate);
    }

    return result;
}   // LdvqCopy

/*
 * Use LdvqPop() to pop the head of the queue. May return NULL.
 */
LonSmipMsg* LdvqPop(LdvqHandle handle)
{
    LonSmipMsg* result = NULL;
    QCtrl* q = (QCtrl*) handle;

    if (q) {
        QItem* item = NULL;

        pthread_mutex_lock(&q->mutex);
        item = q->head;

        if (item) {
            q->head = item->next;

            if (q->tail == item) {
                q->tail = NULL;
            }

            result = item->data;
            free(item);
        }

        pthread_mutex_unlock(&q->mutex);
    }

    return result;
}   // LdvqPop

/*
 * Use LdvqEmpty to determine whether the queue is empty.
 */
int LdvqEmpty(LdvqHandle handle)
{
    int result = 0;
    QCtrl* q = (QCtrl*) handle;

    if (q) {
        pthread_mutex_lock(&q->mutex);
        result = q->head != NULL;
        pthread_mutex_unlock(&q->mutex);
    }

    return result;
}   // LdvqEmpty

LonApiError LdvqAlloc(LdvqHandle handle, LonSmipMsg** frame_pointer)
{
    static uint16_t frame_number = 0;
    LonApiError result = LonApiNoError;

#if MAX_FRAMES
    LonSmipMsg* new_frame = NULL;
    QCtrl* q = (QCtrl*) handle;

    if (q) {
        pthread_mutex_lock(&q->mutex);

        if (q->allocated < MAX_FRAMES) {
            new_frame = (LonSmipMsg*) malloc(sizeof(LonSmipMsg));
        }

        if (new_frame) {
            q->allocated += 1;
        }

        pthread_mutex_unlock(&q->mutex);
    }

#else
    LonSmipMsg* new_frame = (LonSmipMsg*)malloc(sizeof(LonSmipMsg));
#endif  // MAX_FRAMES

    if (new_frame) {
        *frame_pointer = new_frame;
        memset(new_frame, 0, sizeof(LonSmipMsg));
        new_frame->Id = ++frame_number;
    } else {
        result = LonApiTxBufIsFull;
    }

    return result;
}

LonApiError LdvqFree(LdvqHandle handle, LonSmipMsg* frame)
{
    LonApiError result = LonApiNoError;

    if (frame) {
#if MAX_FRAMES
        QCtrl* q = (QCtrl*) handle;

        if (q) {
            pthread_mutex_lock(&q->mutex);

            if (q->allocated) {
                /*
                 * This should be balanced but we cannot allow an
                 * underflow in any circumstances.
                 */
                q->allocated -= 1;
            }

            pthread_mutex_unlock(&q->mutex);

            free(frame);
        }

#else
        free(frame);
#endif  //  MAX_FRAMES
    }

    return result;
}

LonApiError LdvqClear(LdvqHandle q)
{
    LonApiError result = LonApiNoError;
    LonSmipMsg* frame = LdvqPop(q);

    while (frame) {
        LdvqFree(q, frame);
        frame = LdvqPop(q);
    }

    return result;
}
