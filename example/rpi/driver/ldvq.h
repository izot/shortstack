/*
 * IzoT ShortStack for Raspberry Pi Simple Example
 *
 * ldvq.h defines a simple API for a protected queue implemented in ldvq.c.
 *
 * The ShortStack API does not access these functions directly; instead, the
 * ShortStack API uses functions such as <LdvAllocateMsg>, <LdvReleaseMsg>,
 * <LdvGetMsg> and <LdvPutMsg>, which your driver implements elsewhere.
 *
 * Your implementation of the ShortStack serial link-layer driver is not
 * required to use the queueing interface defined here; this is just an
 * example. See the comments in the corresponding implementation file,
 * ldvq.c, for more discussion.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#if !defined(IZOT_SHORTSTACK_LDVQ_H)
#   define IZOT_SHORTSTACK_LDVQ_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "ShortStackDev.h"
#include "ShortStackApi.h"

/*
 * Typedef: LdvqHandle
 *
 * This type is used to identify a given queue. It is returned from <LdvqOpen>
 * and used when calling all other Ldvq* API.
 */
typedef unsigned long LdvqHandle;

/*
 * Function: LdvqOpen
 *
 * To use this simple queue, create one with LdvqOpen() and keep the handle
 * returned. LdvqOpen() returns 0 for failure.
 *
 * Parameters:
 * n/a
 *
 * Result:
 * <LdvqHandle>.
 */
extern LdvqHandle LdvqOpen(void);

/*
 * Function: LdvqClose
 *
 * When done, call LdvqClose(). The handle may not be used after this.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 */
extern void LdvqClose(LdvqHandle q);

/*
 * Function: LdvqPush
 *
 * LdvqPush() pushes a frame to the end of the queue. The frame buffer
 * must be allocated with <LdvqAlloc>. Also see <LdvqCopy>.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 * data - the frame pointer to enqueue
 *
 * Returns:
 * <LonApiError>
 */
extern LonApiError LdvqPush(LdvqHandle q, LonSmipMsg* data);

/*
 * Function: LdvqCopy
 *
 * LdvqCopy() pushes a copy of a frame buffer to the end of the queue.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 * data - the frame pointer to enqueue
 *
 * Returns:
 * <LonApiError>
 */
extern LonApiError LdvqCopy(LdvqHandle q, LonSmipMsg* data);

/*
 * Function: LdvqPop()
 *
 * Use LdvqPop() to retrieve the head of the queue. May return NULL.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 *
 * Returns:
 * A <LonSmipMsg> pointer or NULL.
 */
extern LonSmipMsg* LdvqPop(LdvqHandle q);

/*
 * Function: LdvqEmpty
 *
 * Use LdvqEmpty() to determine whether the queue is empty.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 *
 * Returns:
 * A true or false value, true if the queue is empty.
 */
extern int LdvqEmpty(LdvqHandle q);

/*
 * Function: LdvqAlloc
 *
 * LdvqAlloc() allocates and returns a frame buffer. It is set to
 * all zeroes. The function can fail.
 * Use <LdvqFree> to return it.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 * ppMsg - output parameter, pointer to a frame pointer variable.
 *
 * Returns:
 * <LonApiError>.
 */
extern LonApiError LdvqAlloc(LdvqHandle q, LonSmipMsg** ppMsg);

/*
 * Function: LdvqFree
 *
 * Use LdvqFree to release a frame allocated with <LdvqAlloc>.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 * pMsg - frame pointer.
 *
 * Returns:
 * <LonApiError>.
 */
extern LonApiError LdvqFree(LdvqHandle q, LonSmipMsg* pMsg);

/*
 * Function: LdvqClear
 *
 * Use LdvqClear to empty the queue. Note that this is not an atomic
 * operation but merely a sequence of pop/free calls. Each of those
 * is thread-safe though.
 *
 * Parameters:
 * handle - queue handle, as obtained from <LdvqOpen>.
 *
 * Returns:
 * <LonApiError>.
 */
extern LonApiError LdvqClear(LdvqHandle q);

#endif  // IZOT_SHORTSTACK_LDVQ_H
