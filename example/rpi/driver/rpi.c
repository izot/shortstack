/*
 * ShortStack for Raspberry Pi Simple Example
 *
 * rpi.c implements the driver API defined in ldv.h for the Raspberry
 * Pi and a standard Raspian Wheezy operating system.
 *
 * See ldv.h for general comments about the API required from this driver.
 * See ldvg.h for a discussion of the buffering and queuing methods used
 * by this driver.
 * See the comments throughout this module for implementation details and
 * rationale.
 *
 * This driver implements an asynchronous event-driver thread. The thread
 * handler calls an uplink and a downlink state engine whenever necessary.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>

#include "ShortStackDev.h"
#include "ShortStackApi.h"
#include "ldvq.h"
#include "ldv.h"

#include "io.h"

/*
 * Some notes about the implementation of a ShortStack SCI link-layer driver
 * on Raspberry Pi (Raspbian Linux).
 *
 * The RxD0/TxD0 functionality is supported with the kernel-mode driver and
 * available through /dev/ttyAMA0. The standard 38400bd rate is supported
 * both by this driver and by the ShortStack Micro Server. This driver only
 * supports standard bitrates because non-standard bitrates depend on
 * specific implementations and are not generally portable.
 *
 * Note that your Raspberry Pi is typically configured to use the UART0
 * serial device for boot mode debug messages and login shells.
 * This must be disabled.
 *
 * To disable the login shell, edit /etc/inittab (as superuser), and use
 * a # character to comment-out all lines with reference to tty/AMA0, e.g.
 *
 * #T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100
 *
 * To disable kernel debug and boot messages, edit /boot/cmdline.txt (as
 * superuser) and remove the console declaration for tty/AMA0. You will
 * normally find a console and a kgdboc directive in reference to this
 * device; both need removing.
 *
 * A reboot is required after editing /etc/inittab and /boot/cmdline.txt,
 * but these changes only need to be made once.
 *
 * The Linux serial device driver does not support the half-duplex hardware
 * handshake protocol required by the ShortStack Micro Server. It is possible
 * to enable and control the RTS0 and CTS0 signals in accordance with the
 * ShortStack Micro Server's expectations, however, this approach has
 * several disadvantages and no benefit in comparison to using any other
 * freely available GPIO.
 *
 * Disadvantages of using RTS0/CTS0:
 *
 * First, these signals are available only with the "alternative 3" I/O
 * configuration. This configuration must be enabled using code (or compiled
 * utilities) which require execution as superuser, and the configuration
 * is not persistent (it must be repeated after every reboot).
 *
 * Second, the RTS0/CTS0 (GPIO17, GPIO16) signals are not readily available
 * on all versions of the Raspberry Pi boards; some soldering is required
 * on some boards, and very early editions do not expose these signals at all.
 *
 * Third, the CTS0 pin can be polled through the /dev/ttyAMA0 device, but it
 * can not signal an event when the triggering edge occurs.
 *
 * RTS/CTS via GPIO:
 *
 * Instead of using RTS0/CTS0, this implementation uses GPIO on configurable
 * pins for the RTS and CTS functionality, and uses the standard kernel mode
 * sysfs driver to control the RTS output signal and monitor the CTS input
 * for falling and rising edges.
 *
 * This implementation also supports the optional HRDY signal in the same
 * way as RTS and CTS.
 *
 * The example driver configures the RTS pin as an output and drives it as
 * necessary, and configures the CTS pin as an input with a trigger on both
 * rising and falling edge.
 *
 * Using the sysfs device does not require executing as root, but requires
 * that the executing user is member of the 'gpio' group.
 * This only needs to be done once.
 *
 * Use the 'groups' command to determine current group membership.
 * To add a user to a group, use 'usermod -a -G <groupname> <username>'.
 *
 * GPIO access through sysfs is sometimes dismissed as "too slow." With
 * this application, using a standard Raspberry Pi board in its default
 * clocking configuration can produce a square wave > 150kHz with a sequence
 * of SetHrdy(FALSE), SetHrdy(TRUE) calls, and we measure a duration of
 * 60..90 us between the falling CTS~ edge (an interrupt-trigger) and the
 * corresponding RTS~ de-assertion. These timings are more than enough
 * for a ShortStack link layer driver.
 *
 * Finally, comprehensive and easy-to-follow instructions for setting up
 * Eclipse and a compiler tool chain for cross-compilation and cross-
 * debugging for a Raspberry Pi can be found here:
 *
 * http://www.gurucoding.com/en/raspberry_pi_eclipse/index.php
 *
 * The instructions aim at a Windows development host, but are easily
 * transferred to a Linux desktop environment.
 */

/*
 * Define SUPPORT_SUSPEND to zero to disable support for the LdvSuspend() and
 * LdvResume() API. When disabled, code will be smaller and the affected API
 * return an error (LonApiNotSupported).
 */
#define SUPPORT_SUSPEND 1   /* 1 to enable, 0 to disable */

/*
 * Timeout values are configured in the unit of ticks, which is defined
 * within the driver thread's worker function.
 * This implementation uses a ten millisecond tick, so all of the following
 * timeout values are multiples of 10ms. Because the timers are not
 * synchronized, the smallest value should be 2. The resulting timeout
 * would be in the 10..20 ms interval.
 */
#define TICKS_PER_SECOND    100
#define TIMEOUT_IN_MICROSECONDS 10000l

/*
 * Macro: TIMEOUT_CTS_DEASSERT
 *
 * This timeout limits the duration for which the downlink driver waits
 * until the Micro Server de-asserts the clear-to-send (CTS) signal
 * before the next request to send is made. The Micro Server deasserts
 * CTS when a downlink segment has been fully received; expiry of this
 * timeout indicates a non-responsive Micro Server. Because the Micro
 * Server itself implements a 840ms watchdog timer, which governs the
 * duration of each downlink segment, a serial error (such as a missing
 * start-bit) will generally reset the Micro Server before this value
 * here expires.
 *
 * A value of 2 seconds is recommended.
 */
#define TIMEOUT_CTS_DEASSERT    (2*TICKS_PER_SECOND)

/*
 * Macro: TIMEOUT_CTS_ASSERT
 *
 * This timeout governs the time from asserting the request-to-send (RTS)
 * signal to the assertion of the clear-to-send (CTS) response. An idle
 * Micro Server will be fast to assert CTS, but a non-idle Micro Server
 * may need to wait for available output buffers until it can accept the
 * downlink transfer of a payload segment.
 * This can take a considerable amount of time. The protocol supports up
 * to 16 transmission attempts for acknowledged service or request
 * messages (15 repeats) with a maximum transmit timer of 3072ms. The
 * theoretical maximum is, therefore, 16 * 3072ms = 49152ms, plus an
 * allowance for processing time, assuming that such an exceptional
 * configuration exists, and that the related transaction fails (exhausts
 * all retries).
 * Repeated service can also be configured for up to 16 * 3072 = 49152ms.
 *
 * These durations will be much lower in typical configurations. For
 * example, four attempts, 192ms apart, would amount to 768ms. When you
 * reduce this timeout, however, keep in mind that the device is not
 * normally in control of those parameters, as they are configured by the
 * network tool. Also keep in mind that the Micro Server itself can take
 * considerable time to configure itself during reset-processing, especially
 * when a new host application registers with the Micro Server.
 *
 * A minimum value of 60s is recommended.
 *
 * This implementation uses a 90s timeout, accounting for the fact that
 * it is designed to operate not on a real time operating system. The
 * extra amount is an allowance for a possible non-deterministic delay in
 * detection and reporting of the CTS assertion by the operating system.
 */
#define TIMEOUT_CTS_ASSERT  (90*TICKS_PER_SECOND)

/*
 * Macro: TIMEOUT_UPLINK_DATA
 *
 * This timeout governs the time from receipt of a portion of an uplink
 * frame to receipt of the next portion. In effect, this timeout detects
 * incomplete uplink transfers.
 *
 * A value of 50 ms is recommended.
 */
#if TICKS_PER_SECOND != 100
#   error   Adjust the definition of TIMEOUT_UPLINK_DATA
#endif
#define TIMEOUT_UPLINK_DATA 5   // 50ms

/*
 * Macro: TIMEOUT_UPLINK_ENQUEUE
 *
 * This timeout governs the time from complete receipt of an uplink frame
 * to successful submission of that frame to the uplink frame queue.
 * This duration is determined by the availability of buffer space (the
 * pool might be finite and full), and by the time required to gain access
 * to the protected queue.
 *
 * A value of 5s is recommended.
 */
#define TIMEOUT_UPLINK_ENQUEUE  (5*TICKS_PER_SECOND)

/*
 * Transmit states
 */
typedef enum {
    TXS_Idle = 0, TXS_AwaitCtsDeassert = 1, TXS_AwaitCtsAssert = 2
} TransmitState;

/*
 * This driver uses the following values for LonSmipMsg.Ctrl.Data.
 * These values are automatically managed by the transmitter's
 * state engine and are used to determine the next segment to transmit.
 */
#define LDV_CTRL_UP         0xFF    /* Used when logging uplink frames */
#define LDV_CTRL_HEADER     0       /* Must be zero */
#define LDV_CTRL_EXTHDR     1       /* Next segment is the extended header */
#define LDV_CTRL_PAYLOAD    2       /* Next segment is the payload */

/*
 * LinkLayerFrame is an overlay of the LonSmipMsg structure used by the API
 * implementation with a raw set of bytes. The raw data is used by portions
 * of the driver code (the uplink receiver writes data to the raw overlay so
 * that it can easily cross the boundary between the header and payload
 * sections of LonSmipMsg).
 */
typedef LON_UNION_BEGIN(LinkLayerFrame)
{
    LonSmipMsg smip;
    uint8_t raw[LON_APP_OUTPUT_BUFSIZE];
} LON_UNION_END(LinkLayerFrame);

/*
 * RpiHandle is the structure used for LdvHandle
 */
typedef struct {
    /*
     * File descriptors
     */
    struct {
        int sio;    // serial i/o
        int rts;    // rts via sysfs
        int cts;    // cts via sysfs
        int hrdy;   // hrdy via sysfs
        int epo;    // event pipe output (thread end)
        int epi;    // event pipe input (control end)
#if SUPPORT_SUSPEND
        int spo;    // suspend feedback pipe (control end)
        int spi;    // suspend feedback pipe (thread end)
#endif
    } fd;

    /*
     * GPIO ports
     */
    struct {
        struct {
            int rts;
            int cts;
            int hrdy;
        } port;
        struct {
            int cts;
        } state;
    } gpio;

    /*
     * Thread handle and mutex. The mutex is used to hold the driver's thread
     * in suspension and is only required when suspend/resume operations are
     * supported.
     */
    struct {
        pthread_t sio;
#if SUPPORT_SUSPEND
        pthread_mutex_t mutex;
#endif  //  SUPPORT_SUSPEND
    } thread;

    struct {
        LdvqHandle queue; /* Incoming from the Micro Server */
        unsigned long timeouts;
        LinkLayerFrame frame; /* Buffer to compile an uplink frame */
        unsigned buffered; /* Number of bytes in buffer */
        unsigned expected; /* Number of bytes still expected */
        unsigned timer; /* Timeout timer */
        uint16_t id; /* uplink frame Id */
#if SUPPORT_SUSPEND
#   define  LDV_SUSPEND_UL_MASK 0x0F
#   define  IS_SUSPEND_UL_IMMEDIATE(v)  ((v) && (v) == (LDV_SUSPEND_UL_MASK & LDV_SUSPEND_IMMEDIATE))
#   define  IS_SUSPEND_UL_SYNCHED(v)    ((v) && (v) == (LDV_SUSPEND_UL_MASK & LDV_SUSPEND_SYNCHED))

        unsigned suspend;
        unsigned suspended;
#endif  //  SUPPORT_SUSPEND
    } uplink;

    struct {
        LdvqHandle queue; /* outgoing to the Micro Server */
        unsigned long timeouts;
        LinkLayerFrame* frame; /* The work-in-progress frame */
        TransmitState state;
        unsigned timer; /* Timeout timer */
#if SUPPORT_SUSPEND
#   define  LDV_SUSPEND_DL_MASK 0xF0
#   define  IS_SUSPEND_DL_IMMEDIATE(v)  ((v) && (v) == (LDV_SUSPEND_DL_MASK & LDV_SUSPEND_IMMEDIATE))
#   define  IS_SUSPEND_DL_SYNCHED(v)    ((v) && (v) == (LDV_SUSPEND_DL_MASK & LDV_SUSPEND_SYNCHED))
        unsigned suspend;
        unsigned suspended;
#endif  //  SUPPORT_SUSPEND
    } downlink;

    int (*trace)(const char* fmt, ...); // trace function from LdvCtrl
} RpiHandle;

#define RPI_TRACE(trace, ...) if (trace) trace(__VA_ARGS__)

#if !defined(TRUE)
#   define TRUE 1
#endif
#if !defined(FALSE)
#   define  FALSE   0
#endif
#if !defined(max)
#   define  max(a, b)   (a) > (b) ? (a) : (b)
#endif
#if !defined(min)
#   define  min(a, b)   (a) < (b) ? (a) : (b)
#endif
#define ssizeof(x)  (ssize_t)sizeof(x)

/*
 * PipeEvent are values sent to the SIO thread through the control pipe.
 */
typedef enum {
    PEV_None = 0, /* Not an event, used as initializer */
    /*
     * Normal pipe events use negative codes. This reserves the entire positive
     * value range for suspension codes. These are defined in ldv.h as
     * LDV_SUSPEND_*
     * Consider positive values reserved for suspend requests.
     */
    PEV_Terminate = -1, /* Terminate the SIO thread */
    PEV_Wakeup = -2, /* Wake up and transmit */
    PEV_Resume = -3, /* resume if suspended */
    PEV_Reset = -4 /* immediate driver request */
} PipeEvent;

/*
 * ThreadEvent is used to tell the SIO thread's sub-routines why they
 * were called.
 */
typedef enum {
    TEV_None = 0, /* Not an event, just a placeholder */
    TEV_Data = 1, /* Incoming data is available */
    TEV_Tick = 2, /* A new tick occurred */
    TEV_CTS = 3, /* A change occurred on CTS */
    TEV_Wakeup = 4, /* A new transmit request arrived */
    TEV_Reset = 5 /* Abort current frame immediately, if any */
} ThreadEvent;

/*
 * EncodeBitrate() transcodes the numeric bit rate requested in the control
 * data structure into the encoded value used by the kernel's driver.
 */
static speed_t EncodeBitrate(const LdvCtrl* ctrl)
{
    static const struct {
        unsigned plain;
        speed_t encoded;
    } bitrate_table[] = { { 4800, B4800 }, { 9600, B9600 },
        { 19200, B19200 }, { 38400, B38400 }
    };
    speed_t result = 0;

    for (int i = 0; i < sizeof(bitrate_table) / sizeof(bitrate_table[0]); ++i) {
        if (ctrl->bitrate == bitrate_table[i].plain) {
            result = bitrate_table[i].encoded;
            break;
        }
    }

    if (result == 0) {
        RPI_TRACE(ctrl->trace, "Cannot support %u bps\n", ctrl->bitrate);
    }

    return result;
}

/*
 * GetCts() returns the logical state of CTS, i.e. TRUE if CTS is
 * asserted (physical level low). Note the call returns cached state
 * information and does not actually query the physical line (because
 * the physical input will be configured to report edge conditions
 * only, not state).
 */
static int GetCts(RpiHandle* rpi)
{
    return rpi->gpio.state.cts;
}

/*
 * SetRts sets or clears the RTS signal. The call expects the logical
 * state, i.e. TRUE to assert RTS (which yields a physical low output level).
 */
static void SetRts(RpiHandle* rpi, int state)
{
    char buffer = state ? '0' : '1';
    write(rpi->fd.rts, &buffer, 1);
}

/*
 * SetHrdy sets or clears the HRDY signal. The call expects the logical
 * state, i.e. TRUE to assert HRDY (which yields a physical low output level).
 */
static void SetHrdy(RpiHandle* rpi, int state)
{
    if (rpi->fd.hrdy > 0) {
        char buffer = state ? '0' : '1';
        write(rpi->fd.hrdy, &buffer, 1);
    }
}

/*
 * LogFrame() reports a packet log. 'bytes' can be zero; will add the segment sizes
 * in that case.
 */
static void LogFrame(RpiHandle *rpi, const char* preamble,
                     LonSmipMsg* frame, uint8_t ctrl)
{
    /* A local buffer for printing the frame contents.
     * 3 bytes per byte ("FE.") plus segment frame (" X:0x") and '\0'
     */
    if (rpi->trace) {
        char buffer[LON_APP_OUTPUT_BUFSIZE * 3 + 3 * 6 + 10 + 1];

        size_t pos = sprintf(
                         buffer, "H:0x%02x.%02x",
                         (unsigned) frame->Header.Length,
                         (unsigned) frame->Header.Command
                     );

        if (frame->ExtHdr.Index
        && (ctrl == LDV_CTRL_EXTHDR || ctrl == LDV_CTRL_PAYLOAD)) {
            pos += sprintf(
                       buffer + pos, " X:0x%02x.%02x",
                       (unsigned) frame->ExtHdr.Index,
                       (unsigned) frame->ExtHdr.Reserved
                   );
        }

        if (frame->Header.Length
        && (ctrl == LDV_CTRL_UP || ctrl == LDV_CTRL_PAYLOAD)) {
            pos += sprintf(buffer + pos, " P:0x");

            for (int i = 0; i < frame->Header.Length; ++i) {
                pos += sprintf(
                           buffer + pos, "%02x.",
                           (unsigned) frame->Payload[i]
                       );
            }

            /* Remove the trailing '.' */
            buffer[pos - 1] = '\0';
        }

        RPI_TRACE(rpi->trace, "%s.%05u %s\n", preamble, frame->Id, buffer);
    }
}

/*
 * Downlink() is called from the SIO thread and handles everything regarding the
 * downlink transfer of a single frame by implementing an asynchronous state
 * engine.
 */
static void Downlink(RpiHandle* rpi, ThreadEvent tev)
{
    TransmitState new_state = rpi->downlink.state;

#if SUPPORT_SUSPEND

    if (tev == TEV_Reset
    || IS_SUSPEND_DL_IMMEDIATE(rpi->downlink.suspend)
    || (IS_SUSPEND_DL_SYNCHED(rpi->downlink.suspend)
        && rpi->downlink.frame == NULL)
    ) {
#else

    if (tev == TEV_Reset) {
#endif  // SUPPORT_SUSPEND

        /*
         * Take care of immediate suspension requests.
         */
        SetRts(rpi, FALSE);

        LdvqFree(rpi->downlink.queue, &rpi->downlink.frame->smip);
        rpi->downlink.frame = NULL;
        rpi->downlink.state = TXS_Idle;
        rpi->downlink.timer = 0;

#if SUPPORT_SUSPEND
        rpi->downlink.suspended = rpi->downlink.suspend;
#endif  //  SUPPORT_SUSPEND
    } else {
        /*
         * Normal downlink state engine processing.
         */
        if (tev == TEV_Tick) {
            if (rpi->downlink.timer) {
                rpi->downlink.timer -= 1;

                if (rpi->downlink.timer == 0) {
                    /* Timeout. */
                    SetRts(rpi, FALSE);

                    LdvqFree(rpi->downlink.queue, &rpi->downlink.frame->smip);
                    rpi->downlink.frame = NULL;
                    rpi->downlink.state = TXS_Idle;
                    rpi->downlink.timeouts += 1;

                    RPI_TRACE(rpi->trace, "Downlink timeout\n");
                }
            }
        }

        /*
         * Loop until the state no longer changes. Note that error conditions
         * or synchronized suspension may execute 'break' within this loop
         * (in order to support structured code and avoid a mid-function
         * 'return'), so be careful when adding additional inner loops.
         */
        do {
            rpi->downlink.state = new_state;

            if (rpi->downlink.state == TXS_Idle) {
                rpi->downlink.frame = (LinkLayerFrame*)LdvqPop(rpi->downlink.queue);

                if (rpi->downlink.frame) {
                    if (GetCts(rpi)) {
                        /* Must wait for CTS to be cleared before proceeding. */
                        rpi->downlink.timer = TIMEOUT_CTS_DEASSERT;
                        new_state = TXS_AwaitCtsDeassert;
                    } else {
                        /* Can assert RTS and wait for CTS response. */
                        SetRts(rpi, TRUE);
                        rpi->downlink.timer = TIMEOUT_CTS_ASSERT;
                        new_state = TXS_AwaitCtsAssert;
                    }
                }
            }   // state TXS_Idle

            if (rpi->downlink.state == TXS_AwaitCtsDeassert) {
                if (!GetCts(rpi)) {
                    /* Can assert RTS and wait for CTS response. */
                    SetRts(rpi, TRUE);
                    rpi->downlink.timer = TIMEOUT_CTS_ASSERT;
                    new_state = TXS_AwaitCtsAssert;
                }
            }   // state TXS_AwaitCtsDeassert

            if (rpi->downlink.state == TXS_AwaitCtsAssert) {
                if (GetCts(rpi)) {
                    /*
                     * Transmit the next segment.
                     * Determine its data pointer and size.
                     */
                    size_t size = sizeof(LonSmipHdr);
                    uint8_t* data = (uint8_t*) &rpi->downlink.frame->smip.Header;

                    if (rpi->downlink.frame->smip.Ctrl.Data == LDV_CTRL_EXTHDR) {
                        size = sizeof(LonSmipExtHdr);
                        data = (uint8_t*) &rpi->downlink.frame->smip.ExtHdr;
                    } else if (rpi->downlink.frame->smip.Ctrl.Data == LDV_CTRL_PAYLOAD) {
                        size = rpi->downlink.frame->smip.Header.Length;
                        data = (uint8_t*) &rpi->downlink.frame->smip.Payload;
                    }

                    SetRts(rpi, FALSE);

                    if (write(rpi->fd.sio, data, size) == size) {
                        /*
                         * The write succeeded. It is unlikely to fail on a Linux
                         * host (the kernel typically maintains a 4096 byte buffer),
                         * but if it does fail, we do nothing and let the timeout
                         * run. We'll automatically try again with the next tick.
                         *
                         * But here, it did succeed. See if there is more to transfer.
                         * Otherwise, be done.
                         */
                        LogFrame(
                            rpi, "DN",
                            &rpi->downlink.frame->smip,
                            rpi->downlink.frame->smip.Ctrl.Data
                        );

                        /*
                         * Determine what's next.
                         */
                        if (rpi->downlink.frame->smip.Ctrl.Data == LDV_CTRL_HEADER) {
                            if (rpi->downlink.frame->smip.ExtHdr.Index) {
                                rpi->downlink.frame->smip.Ctrl.Data = LDV_CTRL_EXTHDR;
                            } else if (rpi->downlink.frame->smip.Header.Length) {
                                rpi->downlink.frame->smip.Ctrl.Data = LDV_CTRL_PAYLOAD;
                            }
                        } else if (rpi->downlink.frame->smip.Ctrl.Data == LDV_CTRL_EXTHDR) {
                            rpi->downlink.frame->smip.Ctrl.Data = LDV_CTRL_PAYLOAD;
                        } else {
                            rpi->downlink.frame->smip.Ctrl.Data = LDV_CTRL_HEADER;
                        }

                        if (rpi->downlink.frame->smip.Ctrl.Data != LDV_CTRL_HEADER) {
                            /*
                             * There is at least one more segment to work on.
                             */
                            new_state = TXS_AwaitCtsDeassert;
                        } else {
                            /*
                             * Nothing else to do for this frame. Discard it:
                             */
                            LdvqFree(rpi->downlink.queue, (LonSmipMsg*) rpi->downlink.frame);
                            rpi->downlink.frame = NULL;
                            rpi->downlink.timer = 0;
                            new_state = TXS_Idle;
#if SUPPORT_SUSPEND

                            /*
                             * Signal completion of suspension requests.
                             */
                            if (IS_SUSPEND_DL_SYNCHED(rpi->downlink.suspend)) {
                                rpi->downlink.suspended = rpi->downlink.suspend;
                                rpi->downlink.state = TXS_Idle;
                                break;
                            }

#endif  //  SUPPORT_SUSPEND
                        }
                    }
                }
            }   // state TXS_AwaitCtsAssert
        } while (rpi->downlink.state != new_state);
    }   // if not immediate suspension or reset
}

/*
 * Uplink() is called from the SIO thread. The function retrieves incoming
 * ('uplink') data and handles timeout conditions. The function enqueues
 * an incoming frame (once successfully received) in the uplink queue.
 * The host API retrieves it from there with the standard Ldv* API functions.
 */
static void Uplink(RpiHandle* rpi, ThreadEvent tev)
{
#if SUPPORT_SUSPEND

    if (tev == TEV_Reset
    || IS_SUSPEND_UL_IMMEDIATE(rpi->uplink.suspend)
    || (IS_SUSPEND_UL_SYNCHED(rpi->uplink.suspend)
        && rpi->uplink.buffered == 0)
    ) {
#else

    if (tev == TEV_Reset) {
#endif  //  SUPPORT_SUSPEND
        /*
         * Take care of immediate suspension requests.
         */
        rpi->uplink.timer = rpi->uplink.buffered = rpi->uplink.expected = 0;
#if SUPPORT_SUSPEND
        rpi->uplink.suspended = rpi->uplink.suspend;
#endif  //  SUPPORT_SUSPEND
    } else {
        if (tev == TEV_Tick) {
            if (rpi->uplink.timer) {
                rpi->uplink.timer -= 1;

                if (rpi->uplink.timer == 0) {
                    /* Timeout. */
                    rpi->uplink.timeouts += 1;
                    rpi->uplink.buffered = rpi->uplink.expected = 0;
                    RPI_TRACE(rpi->trace, "Uplink timeout\n");
                }
            }
        } else if (tev == TEV_Data) {
            int available = 0;

            if (ioctl(rpi->fd.sio, FIONREAD, &available) == -1) {
                available = 0; /* to be safe in case ioctl modified it */
            }

            if (available) {
                /*
                 * Kill the timeout right away (we may need to arm it again
                 * later).
                 */
                rpi->uplink.timer = 0;

                if (rpi->uplink.buffered == 0) {
                    /*
                     * Start a new frame
                     */
                    int accepted = read(
                                       rpi->fd.sio,
                                       rpi->uplink.frame.raw,
                                       min(sizeof(LonSmipHdr), available)
                                   );

                    if (accepted > 0) {
                        rpi->uplink.frame.smip.Id = ++rpi->uplink.id;
                        /* What we have. */
                        rpi->uplink.buffered = accepted;
                        /* What is yet to come. */
                        /* Assumes the Length is in the *first* header byte */
                        rpi->uplink.expected =
                            rpi->uplink.frame.smip.Header.Length
                            + sizeof(LonSmipHdr)
                            - rpi->uplink.buffered;
                        /* What is left in the OS buffer. */
                        available -= accepted;
                    }
                }
            }

            if (available) {
                /*
                 * More data available.
                 */
                if (rpi->uplink.expected > 0) {
                    /*
                     * Add more incoming data to the temporary buffer
                     */
                    int accepted = read(
                                       rpi->fd.sio,
                                       rpi->uplink.frame.raw + rpi->uplink.buffered,
                                       min(rpi->uplink.expected, available)
                                   );

                    if (accepted > 0) {
                        rpi->uplink.buffered += accepted;
                        rpi->uplink.expected -= accepted;
                    }
                }
            }

            if (rpi->uplink.buffered > 0) {

                if (rpi->uplink.buffered >= sizeof(LonSmipHdr)
                && rpi->uplink.frame.smip.Header.Command == LonNiReset) {
                    /*
                     * The Micro Server reports a reset. The driver must handle
                     * this immediately by canceling any in-progress downlink
                     * transfer in order to preserve link layer integrity.
                     *
                     * The IzoT ShortStack Micro Server (version 4.30) introduces
                     * a configurable post-reset pause, but if this pause is too
                     * short, disabled, or not supported with an older Micro
                     * Server, this receiver code here must take action
                     * immediately.
                     */
                    Downlink(rpi, TEV_Reset);
                }

                if (rpi->uplink.expected > 0) {
                    /*
                     * More bytes are expected. Arm the timeout.
                     */
                    rpi->uplink.timer = TIMEOUT_UPLINK_DATA;
                } else {
                    /*
                     * We have a complete frame. Set a very long timeout to
                     * monitor the time it takes to enqueue this. This should
                     * not normally take any time at all, but the queue may be
                     * finite and we may need to wait until space becomes
                     * available.
                     */
                    rpi->uplink.timer = TIMEOUT_UPLINK_ENQUEUE;
                }
            }
        }

        if (rpi->uplink.expected == 0
        && rpi->uplink.buffered >= sizeof(LonSmipHdr)) {
            /*
             * The frame is complete, so let's enqueue it and get ready for the
             * next one.
             */
            if (LdvqCopy(rpi->uplink.queue, &rpi->uplink.frame.smip) == LonApiNoError) {
                /*
                 * OK, a copy of this frame is now in the queue, from where the
                 * API will fetch it with LdvGetMsg().
                 *
                 * If the LdvqCopy call fails, maybe because the queue has finite
                 * capacity and is full, we try again with the next interrupt or
                 * tick. In this case, a large timeout is already armed. This uses
                 * a large value, because the most likely reason for the enqueue
                 * operation to fail is that the application is too busy with
                 * higher priority tasks and can't serve our queue right now.
                 *
                 * Here, it succeeded.
                 */
                LogFrame(rpi, "UP", &rpi->uplink.frame.smip, LDV_CTRL_UP);

                rpi->uplink.buffered = rpi->uplink.expected = 0;
                rpi->uplink.timer = 0;
#if SUPPORT_SUSPEND

                /*
                 * Signal completion of suspension requests.
                 */
                if (IS_SUSPEND_UL_SYNCHED(rpi->uplink.suspend)) {
                    rpi->uplink.suspended = rpi->uplink.suspend;
                }

#endif  //  SUPPORT_SUSPEND
            }
        }
    }
}

/*
 * sio_thread is the serial I/O thread.
 */
static void* SioThread(void* arg)
{
    RpiHandle* rpi = (RpiHandle*) arg;
    int running = TRUE;
    fd_set read_fds, write_fds, interrupt_fds;
    struct timeval timeout = { 0, TIMEOUT_IN_MICROSECONDS };
    int selected = 0;
    int nfds = -1;

    while (running) {
#if SUPPORT_SUSPEND
        /*
         * Grab the suspend_mutex. When the thread is suspended, the suspender
         * holds that mutex, and the following lock() call does not return
         * until the suspended resumes the thread.
         * This is done to minimize CPU activity while the driver is suspended,
         * which is sometimes needed during time-critical operations.
         *
         * This mutex is only used to hold the driver thread in suspension, so
         * we can unlock it as soon as we acquire it. This reduces the risk of
         * failing to unlock it later.
         */
        pthread_mutex_lock(&rpi->thread.mutex);
        pthread_mutex_unlock(&rpi->thread.mutex);
#endif  //  SUPPORT_SUSPEND

        /*
         * Prepare the file descriptor arrays and timeout structure for
         * select(). Remember that select() modifies the file descriptor
         * arguments, and some implementations (Linux in particular)
         * modify the timeout argument, so these must be initialized
         * before each select() call.
         *
         * We use a timeout of 10ms. This provides a base tick for the
         * uplink and downlink engines to implement their various actual
         * timeout values.
         */

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&interrupt_fds);
        nfds = 0;

        FD_SET(rpi->fd.sio, &read_fds);
        nfds = max(nfds, rpi->fd.sio);

        FD_SET(rpi->fd.epo, &read_fds);
        nfds = max(nfds, rpi->fd.epo);

        FD_SET(rpi->fd.cts, &interrupt_fds);
        nfds = max(nfds, rpi->fd.cts);

        timeout.tv_sec = 0;
        timeout.tv_usec = TIMEOUT_IN_MICROSECONDS;

        selected = select(nfds + 1, &read_fds, &write_fds, &interrupt_fds, &timeout);

        if (selected == 0) {
            /*
             * Timeout. We use this to implement various timeouts based on this
             * 10 ms ticker. Arguably this time base is not reliable, because a
             * very busy device may rarely see a select() call timing out.
             * However, a device busy sending and receiving data will not be
             * suffering from timeout conditions, so this consideration is more
             * of an academic nature.
             */
            Uplink(rpi, TEV_Tick);
            Downlink(rpi, TEV_Tick);
        } else if (selected > 0) {
            /*
             * We've got work to do.
             */
            if (FD_ISSET(rpi->fd.epo, &read_fds)) {
                /*
                 * A Pipe event wants to be read
                 */
                PipeEvent event = PEV_None;

                if (read(rpi->fd.epo, &event, sizeof(event)) == sizeof(event)) {
                    if (event == PEV_Terminate) {
                        running = FALSE;
                    } else if (event == PEV_Wakeup) {
                        Downlink(rpi, TEV_Wakeup);
                    } else if (event == PEV_Reset) {
                        Uplink(rpi, TEV_Reset);
                        Downlink(rpi, TEV_Reset);
#if SUPPORT_SUSPEND
                    } else if (event == PEV_Resume) {
                        rpi->uplink.suspended = rpi->downlink.suspended = 0;
                        Uplink(rpi, TEV_Wakeup);
                        Downlink(rpi, TEV_Wakeup);
                    } else if (event > 0) {
                        /*
                         * A suspend request. Completion could take some time; this
                         * is examined and reported back to the suspender later
                         * (below, in this function).
                         */
                        rpi->uplink.suspend = event & LDV_SUSPEND_UL_MASK;
                        rpi->downlink.suspend = event & LDV_SUSPEND_DL_MASK;
                        Uplink(rpi, TEV_Wakeup);
                        Downlink(rpi, TEV_Wakeup);
#endif  // SUPPORT_SUSPEND
                    }
                }
            }

            if (FD_ISSET(rpi->fd.sio, &read_fds)) {
                /*
                 * Uplink data waiting to be read
                 */
                Uplink(rpi, TEV_Data);
            }

            if (FD_ISSET(rpi->fd.cts, &interrupt_fds)) {
                /*
                 * A CTS edge occurred. Read its current state:
                 */
                if (lseek(rpi->fd.cts, 0, SEEK_SET) != -1) {
                    char buffer;
                    ssize_t bytes = read(rpi->fd.cts, &buffer, sizeof(buffer));

                    if (bytes == ssizeof(buffer)) {
                        rpi->gpio.state.cts = buffer == '0';
                    }
                }

                Downlink(rpi, TEV_CTS);
            }
        }

#if SUPPORT_SUSPEND

        /*
         * Check if suspend requests have been met.
         */
        if (rpi->uplink.suspend
        && rpi->uplink.suspend == rpi->uplink.suspended
        && rpi->downlink.suspend
        && rpi->downlink.suspend == rpi->downlink.suspended) {
            /*
             * Clear the request and report success.
             */
            LonApiError result = LonApiNoError;
            rpi->uplink.suspend = rpi->downlink.suspend = 0;
            write(rpi->fd.spi, &result, sizeof(result));
        }

#endif  //  SUPPORT_SUSPEND

    }   // while running

    return NULL;
}

LonApiError LdvOpen(const LdvCtrl* ctrl, LdvHandle* handle)
{
    int fds[2] = { -1, -1 };
    RpiHandle* rpi = (RpiHandle*) malloc(sizeof(RpiHandle));
    LonApiError result = LonApiNoError;

    memset(rpi, 0, sizeof(RpiHandle));
    rpi->trace = ctrl->trace;

    rpi->gpio.port.rts = ctrl->gpio.rts;
    rpi->gpio.port.cts = ctrl->gpio.cts;
    rpi->gpio.port.hrdy = ctrl->gpio.hrdy;

    if (ctrl->gpio.hrdy) {
        /*
         *  The HRDY signal is optional, but if we have it, we should
         *  deassert it at once.
         */
        rpi->fd.hrdy = GpioOpen(ctrl->gpio.hrdy, O_WRONLY, "high", NULL);
    } else {
        rpi->fd.hrdy = 0;
    }

    rpi->fd.rts = GpioOpen(ctrl->gpio.rts, O_WRONLY, "high", NULL);
    rpi->fd.cts = GpioOpen(ctrl->gpio.cts, O_RDONLY, "in", "both");

    rpi->gpio.state.cts = FALSE;    // not asserted

    rpi->fd.sio = open(ctrl->device, O_RDWR | O_NOCTTY | O_NDELAY);

    if (pipe(fds) == -1) {
        rpi->fd.epi = rpi->fd.epo = -1;
    } else {
        rpi->fd.epo = fds[0];
        rpi->fd.epi = fds[1];
    }

#if SUPPORT_SUSPEND

    if (pipe(fds) == -1) {
        rpi->fd.spi = rpi->fd.spo = -1;
    } else {
        rpi->fd.spo = fds[0];
        rpi->fd.spi = fds[1];
    }

#endif  // SUPPORT_SUSPEND

    if (rpi->fd.sio == -1) {
        RPI_TRACE(rpi->trace, "Can't connect to %s\n", ctrl->device);
        result = LonApiInitializationFailure;
        LdvClose((LdvHandle) rpi);
    } else if (rpi->fd.rts == -1) {
        RPI_TRACE(rpi->trace, "Can't connect to RTS\n");
        result = LonApiInitializationFailure;
        LdvClose((LdvHandle) rpi);
    } else if (rpi->fd.cts == -1) {
        RPI_TRACE(rpi->trace, "Can't connect to CTS\n");
        result = LonApiInitializationFailure;
        LdvClose((LdvHandle) rpi);
    } else if (rpi->fd.epi == -1 || rpi->fd.epo == -1) {
        RPI_TRACE(rpi->trace, "Can't create the SIO thread control pipe\n");
        result = LonApiInitializationFailure;
        LdvClose((LdvHandle) rpi);
    }

    if (result == LonApiNoError) {
        struct termios tio;

        // Configure serial communications
        tcgetattr(rpi->fd.sio, &tio);

        tio.c_cflag = CS8 | CLOCAL | CREAD | EncodeBitrate(ctrl);
        tio.c_iflag = IGNPAR;
        tio.c_oflag = 0;
        tio.c_lflag = 0;
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 0;

        tcsetattr(rpi->fd.sio, TCSAFLUSH, &tio);

        rpi->uplink.queue = LdvqOpen();
        rpi->downlink.queue = LdvqOpen();

#if SUPPORT_SUSPEND
        pthread_mutex_init(&rpi->thread.mutex, NULL);
#endif  //  SUPPORT_SUSPEND

        if (pthread_create(&rpi->thread.sio, NULL, SioThread, rpi)) {
            RPI_TRACE(rpi->trace, "Can't create the SIO thread");
            result = LonApiInitializationFailure;
            LdvClose((LdvHandle) rpi);
        } else {
            SetHrdy(rpi, TRUE);
            *handle = (LdvHandle) rpi;
            RPI_TRACE(
                rpi->trace,
                "Connected to %s,CTS~: GPIO%d, RTS~: GPIO%d, HRDY~: GPIO%d\n",
                ctrl->device,
                ctrl->gpio.cts,
                ctrl->gpio.rts,
                ctrl->gpio.hrdy
            );
        }
    }

    return result;
}

/*
 * LdvClose() closes the handle.
 * The function is also called when LdvOpen fails, so not all handles
 * may be valid when this function executes.
 */
LonApiError LdvClose(LdvHandle handle)
{
    RpiHandle* rpi = (RpiHandle*) handle;

    SetHrdy(rpi, FALSE);

    /*
     * Shut down the tread: send a terminate event, call join().
     */
    if (rpi->thread.sio != -1) {
        const PipeEvent terminate = PEV_Terminate;

        write(rpi->fd.epi, &terminate, sizeof(terminate));
        pthread_join(rpi->thread.sio, NULL);
#if SUPPORT_SUSPEND
        pthread_mutex_destroy(&rpi->thread.mutex);
#endif  //  SUPPORT_SUSPEND
    }

    if (rpi->fd.rts != -1) {
        SetRts(rpi, FALSE);
    }

    /*
     * Now that the SIO thread is dead, let's close the open files:
     */
    if (rpi->fd.sio != -1) {
        tcflush(rpi->fd.sio, TCIOFLUSH);
        close(rpi->fd.sio);
        rpi->fd.sio = -1;
    }

    if (rpi->fd.epi != -1) {
        close(rpi->fd.epi);
        close(rpi->fd.epo);
        rpi->fd.epi = rpi->fd.epo = -1;
    }

#if SUPPORT_SUSPEND

    if (rpi->fd.spi != -1) {
        close(rpi->fd.spi);
        close(rpi->fd.spo);
        rpi->fd.spi = rpi->fd.spo = -1;
    }

#endif  // SUPPORT_SUSPEND

    if (rpi->fd.cts != -1) {
        GpioClose(rpi->gpio.port.cts, rpi->fd.cts);
        rpi->fd.cts = -1;
    }

    if (rpi->fd.rts != -1) {
        GpioClose(rpi->gpio.port.rts, rpi->fd.rts);
        rpi->fd.rts = -1;
    }

    if (rpi->fd.hrdy != -1) {
        GpioClose(rpi->gpio.port.hrdy, rpi->fd.hrdy);
        rpi->fd.hrdy = -1;
    }

    if (rpi->uplink.queue) {
        LdvqClose(rpi->uplink.queue);
        rpi->uplink.queue = 0;
    }

    if (rpi->downlink.queue) {
        LdvqClose(rpi->downlink.queue);
        rpi->downlink.queue = 0;
    }

    free(rpi);

    return LonApiNoError;
}

/*
 * LdvAllocateMsg() allocates a transmit buffer.
 */
LonApiError LdvAllocateMsg(LdvHandle handle, LonSmipMsg** pFrame)
{
    RpiHandle* rpi = (RpiHandle*) handle;
    return LdvqAlloc(rpi->downlink.queue, pFrame);
}

/*
 * LdvAllocateMsgWait() is a time-limited blocking version of
 * LdvAllocateMsg(). Since this implementation uses the heap and is designed
 * for Raspberry Pi (or similar) boards, we consider the pool of available
 * buffers to be infinite for the purpose of application initialization.
 *
 * This implementation of LdvAllocateMsgWait() is nothing but an alias to
 * LdvAllocateMsg(), therefore.
 *
 * Implementations which use a finite buffer pool may need to wait until a
 * timeout occurs or a buffer becomes available, whichever comes first.
 *
 * The Shortstack API calls this function only during initialization. The
 * Micro Server is in quiet mode during that phase; incoming network messages
 * are not expected at that time.
 */
LonApiError LdvAllocateMsgWait(LdvHandle handle, LonSmipMsg** pFrame)
{
    return LdvAllocateMsg(handle, pFrame);
}

/*
 * LdvPutMsg() submits a message for downlink transfer.
 */
LonApiError LdvPutMsg(LdvHandle handle, LonSmipMsg* pFrame)
{
    RpiHandle* rpi = (RpiHandle*) handle;
    LonApiError result = LonApiNoError;

    result = LdvqPush(rpi->downlink.queue, pFrame);

    if (result == LonApiNoError) {
        PipeEvent event = PEV_Wakeup;

        if (write(rpi->fd.epi, &event, sizeof(event)) != sizeof(event)) {
            result = LonApiDriverCtrl;
        }
    }

    return result;
}

/*
 * LdvGetMsg() retrieves an incoming message (if any).
 */
LonApiError LdvGetMsg(LdvHandle handle, LonSmipMsg** pFrame)
{
    RpiHandle* rpi = (RpiHandle*) handle;
    LonApiError result = LonApiNoError;

    *pFrame = LdvqPop(rpi->uplink.queue);

    if (*pFrame == NULL) {
        result = LonApiRxMsgNotAvailable;
    }

    return result;
}

/*
 * LdvReleaseMsg() releases the message buffer after processing
 * an incoming message is complete.
 */
LonApiError LdvReleaseMsg(LdvHandle handle, LonSmipMsg* pFrame)
{
    RpiHandle* rpi = (RpiHandle*) handle;
    return LdvqFree(rpi->uplink.queue, pFrame);
}

/*
 * LdvReset resets the driver. Note that the function returns
 * as soon as the driver reset request has been submitted and not
 * when it has been executed.
 *
 * Implementations which require the reset to be completed before
 * this API completes might use the synchronization pipe to signal
 * completion, and wait through a select() call here until this
 * is indicated.
 */
LonApiError LdvReset(LdvHandle handle)
{
    RpiHandle* rpi = (RpiHandle*) handle;
    const PipeEvent reset = PEV_Reset;

    write(rpi->fd.epi, &reset, sizeof(reset));
    return LonApiNoError;
}

/*
 * LdvSuspend() for a graceful wind down or temporary suspension. Use
 * LdvResume() to resume.
 */
LonApiError LdvSuspend(LdvHandle handle, unsigned mode, unsigned timeout)
{
#if SUPPORT_SUSPEND
    RpiHandle* rpi = (RpiHandle*) handle;
    LonApiError result = LonApiNoError;
    PipeEvent command = 0;

    if (rpi->uplink.suspended == 0 && rpi->downlink.suspended == 0) {
        /* Not suspended right now. Suspend: */
        if (mode & (LDV_SUSPEND_UL_MASK | LDV_SUSPEND_DL_MASK)) {
            command = (PipeEvent) mode;

            if (write(rpi->fd.epi, &command, sizeof(command)) != sizeof(command)) {
                result = LonApiDriverCtrl;
            } else {
                /*
                 * Now wait for a success signal or timeout.
                 */
                fd_set read_fds;
                struct timeval timeout_ctrl = { timeout };
                int selected = 0;

                FD_ZERO(&read_fds);
                FD_SET(rpi->fd.spo, &read_fds);

                selected = select(rpi->fd.spo + 1, &read_fds, NULL, NULL, &timeout_ctrl);

                if (!selected) {
                    result = LonApiTimeout;
                } else {
                    /*
                     * Now grab the thread's mutex. This ensures that the thread
                     * is truly suspended, not just ticking away doing nothing.
                     * Read the thread's response, deactivate HRDY and return.
                     */
                    pthread_mutex_lock(&rpi->thread.mutex);
                    read(rpi->fd.spo, &result, sizeof(result));
                    SetHrdy(rpi, FALSE);
                }
            }
        }
    }

    return result;
#else
    return LonApiNotSupported;
#endif  //  SUPPORT_SUSPEND
}

/*
 * LdvResume() resumes a previously suspended driver.
 */
LonApiError LdvResume(LdvHandle handle)
{
#if SUPPORT_SUSPEND
    RpiHandle* rpi = (RpiHandle*) handle;
    LonApiError result = LonApiNoError;
    PipeEvent command = PEV_Resume;

    if (rpi->uplink.suspended || rpi->downlink.suspended) {
        /*
         * Wake-up the thread and unlock the suspension mutex.
         */

        if (write(rpi->fd.epi, &command, sizeof(command)) != sizeof(command)) {
            result = LonApiDriverCtrl;
        }

        pthread_mutex_unlock(&rpi->thread.mutex);
        SetHrdy(rpi, TRUE);
    }

    return result;
#else
    return LonApiNotSupported;
#endif  //  SUPPORT_SUSPEND
}

