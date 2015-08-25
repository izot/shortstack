/*
 * ShortStack for Raspberry Pi Simple Example
 * This example application implements a pair of one input and one
 * output datapoints (also known as network variables). The
 * LonNvUpdateOccurred callback executes When the input is updated with a
 * new value from the network, assigning and propagating three plus the
 * input value to the output.
 *
 * main.c
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

/*
 * See here for instructions to configure Eclipse for compilation and
 * cross-debugging with a Raspberry Pi:
 * http://www.gurucoding.com/en/raspberry_pi_eclipse/index.php
 *
 * For detailed instructions and considerations about the Eclipse
 * project configuration, see *Eclipse Configuration* in the docs
 * folder of the IzoT ShortStack repository.
 */
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>

#include "ShortStackDev.h"
#include "ShortStackApi.h"
#include "ldv.h"

/*
 * Some global instructions for the IzoT Interface Interpreter follow.
 *
 * The IzoT Interface Interpreter parses this source file and interprets
 * the annotations contains in C comments, which must start with the
 * "//@IzoT" tag (case insensitive).
 *
 * The IzoT Interface Interpreter generates output in ShortStackDev.c
 * and ShortStackDev.h source files.
 */

//@IzoT Option target("shortstack-classic")
//@IzoT Option programId("9F:FF:FF:08:16:01:04:00")
//@IzoT Option server("../../../microserver/standard/SS430_FT6050_SYS20000kHz")

/*
 * The application's interface definition follows.
 *
 * This simple application consists of two simple actuator blocks named
 * 'driver', which implement the standard closed loop actuator profile
 * using a standard network variable type SNVT_volt for their mandatory
 * datapoints. This simple application's algorithm is trivial and
 * implemented in the onDriverUpdate event handler, defined below.
 *
 * Because this application implements more than one block, it must also
 * implement the standard node object profile and provide simple code to
 * supply housekeeping and device management functions. This is provided
 * with the onNviRequest event handler, also defined below.
 *
 * The node object must be declared first.
 */
SFPTnodeObject(node) nodeObject;    //@IzoT block external("nodeObject"), \
//@IzoT onUpdate(nviRequest, onNviRequest),\
//@IzoT implement(nciDevMajVer, init=1), implement(nciDevMinVer, init=0)

SFPTclosedLoopActuator(volt, SNVT_volt) driver[2];  //@izot block external("volts"), \
//@IzoT onUpdate(nviValue, onDriverUpdate), \
//@IzoT implement(nciLocation, init="room 101")

/*
 * onNviRequest
 *
 * The onNviRequest event handler executes when the node object receives a
 * request through its mandatory 'nviRequest' input datapoint.
 */
void onNviRequest(const unsigned index, const LonReceiveAddress* const pSourceAddress)
{
    uint32_t flags = LON_GET_UNSIGNED_DOUBLEWORD(
                         nodeObject.nvoStatus.data.flags
                     );
    uint16_t object_id = LON_GET_UNSIGNED_WORD(
                             nodeObject.nviRequest.data.object_id
                         );

    // The following implements a minimum functionality Node Object block
    // with the following characteristics: state and alarm conditions are
    // not preserved over power cycle or reset, and all blocks are expected
    // to support the same set of features (all blocks report the same mask).
    // The Node Object assumes the status output datapoint is polled, and
    // never actively propagates the status output.

    // Most operations apply to all blocks if the object_id is 0, or to one
    // specific block with the given object_id. The object_id equals a block's
    // global_index. This event handler executes these operations on a range
    // from 'first' to 'last'.
#define FIRST_BLOCK_AFFECTED    max(1, object_id)
#define LAST_BLOCK_AFFECTED     min(object_id, LON_FB_COUNT-1)

    // Clear all transient flags.
    // More comprehensive implementations of a node object could implement
    // persistent modes and related flags, for example those in relation to
    // error or alarm conditions. Those longer lived flags would need
    // preserving here.
    flags &= ~(ST_REPORT_MASK | ST_INVALID_ID | ST_INVALID_REQUEST);

    if (object_id >= LON_FB_COUNT) {
        object_id = 0;
        flags |= ST_INVALID_ID;
    } else if (nodeObject.nviRequest.data.object_request == RQ_REPORT_MASK) {
        // This assumes that all blocks support the same features.
        flags = ST_REPORT_MASK | ST_INVALID_ID | ST_INVALID_REQUEST;
    } else if (nodeObject.nviRequest.data.object_request == RQ_NORMAL) {

        // TODO: Return your blocks in the FIRST_BLOCK_AFFECTED...
        // LAST_BLOCK_AFFECTED range to the normal state (not disabled,
        // not overridden).
        // Then report the combined (OR'ed) status flags of all affected
        // blocks in the status output (here: always 0).



        flags = 0;
    } else if (nodeObject.nviRequest.data.object_request == RQ_UPDATE_STATUS) {

        // TODO: Combine the current status of each of the blocks in the
        // FIRST_BLOCK_AFFECTED...LAST_BLOCK_AFFECTED range, then update the
        // output status datapoint with that value.
        // The present implementation always reports a status of 0, because
        // the minimal node object supports no block-specific states.



        flags = 0;
    } else if (nodeObject.nviRequest.data.object_request == RQ_CLEAR_STATUS) {
        flags = 0;


        // TODO: Support other request codes where necessary.
        // Common (but not mandatory) requests include RQ_ENABLE / RQ_DISABLED and
        // RQ_OVERRIDE / RQ_RMV_OVERRIDE, requests to support alarm conditions and
        // alarm cancellation or self-test functions.



    } else {
        flags = ST_INVALID_REQUEST;
    }

    LON_SET_UNSIGNED_WORD(nodeObject.nvoStatus.data.object_id, object_id);
    LON_SET_UNSIGNED_DOUBLEWORD(nodeObject.nvoStatus.data.flags, flags);
}

/*
 * onDriverUpdate
 *
 * The onDriverUpdate event handler executes when either of the members of the
 * 'driver' array of blocks receive an updated value in their mandatory input
 * datapoint 'nviValue'.
 *
 * This simple example simulates a hypothetical device which assigns the value
 * received by the input, plus 3, to the output. While of little practical use,
 * this simple algorithm is sufficient to confirm correct device operation
 * during early prototyping steps. A typical "real" application might us the
 * input value to drive a physical output, such as a voltage generator's
 * setpoint, sample the physical equipment's actually generated voltage using
 * some other appropriate circuitry, and report the actual reading back through
 * the 'nvoValueFb' output datapoints.
 */
void onDriverUpdate(const unsigned index, const LonReceiveAddress* const pSourceAddress)
{
    for (int i = 0; i < sizeof(driver) / sizeof(driver[0]); ++i) {
        if (index == driver[i].nviValue.global_index) {
            LON_SET_UNSIGNED_WORD(
                driver[i].nvoValueFb.data,
                3 + LON_GET_UNSIGNED_WORD(driver[i].nviValue.data)
            );
            LonPropagateNv(driver[i].nvoValueFb.global_index);
            break;
        }
    }
}

#define TITLE       "Shortstack Goes Raspberry Pi Sample Application for III"
#define COPYRIGHT   "Copyright (C) 2014-2015 Echelon Corporation"
#define VERSION     "1.10.00"

#ifdef _DEBUG
static int debug = 1;
#else
static int debug = 0;
#endif  // _DEBUG

static struct {
    char* name;
    FILE* fp;
    pthread_mutex_t mutex;
} tracefile = { "trace.log", NULL, PTHREAD_MUTEX_INITIALIZER };

static int Trace(const char* fmt, ...);

static LdvCtrl ctrl = {
    LDVCTRL_DEFAULT_DEVICE,
    LDVCTRL_DEFAULT_BITRATE,
    {
        LDVCTRL_DEFAULT_GPIO_RTS,
        LDVCTRL_DEFAULT_GPIO_CTS,
        LDVCTRL_DEFAULT_GPIO_HRDY
    },
    Trace
};

/*
 * ConioCtrl disables console input canonical mode if the direct flag
 * is true, and restores the normal mode if not. This is used to allow
 * for a kbhit()-style call (a simple non-blocking keyboard API, defined
 * below as conio_pending().
 */
static void ConioCtrl(int direct)
{
    struct termios tio;

    /*
     * Note that the switch to non-canonical mode doesn't seem to happen
     * when used through Eclipse's built-in remote GDB debugging console.
     * (Must hit Enter, still get an echo.)
     * A regular console or SSH terminal behaves as expected.
     */
    tcgetattr(STDIN_FILENO, &tio);

    if (direct) {
        tio.c_lflag &= ~(ICANON | ECHO);
    } else {
        tio.c_lflag |= ICANON | ECHO;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &tio);
}

/*
 * ConioPending() provides a simple non-blocking API to query availability
 * of keyboard input. The tool returns a true value if keyboard input is
 * pending.
 */
static int ConioPending()
{
    struct timeval timeout = { 0, 0 };  // no timeout, return immediately
    fd_set  read_fds;

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
    return FD_ISSET(STDIN_FILENO, &read_fds);
}

/*
 * ConioGetc() provides getc() functionality.
 */
static char ConioGetc()
{
    char c = '\0';
    (void)read(STDIN_FILENO, &c, 1);
    return c;
}

/*
 * Trace() provides a thread-safe version of fprintf for the tracefile,
 * if one is open. This can be called by low-level driver functions to
 * produce link-layer and other trace output.
 */
static int Trace(const char* fmt, ...)
{
    int result = -1;

    if (tracefile.fp) {
        pthread_mutex_lock(&tracefile.mutex);
        {
            va_list args;
            va_start(args, fmt);
            result = vfprintf(tracefile.fp, fmt, args);

            if (debug) {
                (void)vprintf(fmt, args);
            }

            va_end(args);
        }
        pthread_mutex_unlock(&tracefile.mutex);
    }

    return result;
}

static void Execute(const char* name, LonApiError result)
{
    if (result == LonApiNoError) {
        Trace("%s completed with success\n", name);
    } else {
        Trace("%s completed with error %d\n", name, result);
    }
}

static int SetPort(int* const port, char id, int* i, int argc, char* argv[])
{
    int errors = 0;

    if ((*i + 1) < argc) {
        *i += 1;
        *port = atoi(argv[*i]);
    } else {
        ++errors;
        fprintf(stderr, "Missing argument: -%c port\n", id);
    }

    return errors;
}

static int Parse(int argc, char* argv[])
{
    int i = 1;
    int errors = 0;
    int showUsage = FALSE;

    while (i < argc) {
        const char* const arg = argv[i];

        if (arg[0] == '-') {
            switch (arg[1]) {
            case 'c':
                errors += SetPort(&ctrl.gpio.cts, 'c', &i, argc, argv);
                break;

            case 'd':
                if ((i + 1) < argc) {
                    ctrl.device = argv[++i];
                } else {
                    ++errors;
                    fprintf(stderr, "Missing argument: -d device\n");
                }

                break;

            case 'h':
                errors += SetPort(&ctrl.gpio.hrdy, 'h', &i, argc, argv);
                break;

            case 'r':
                errors += SetPort(&ctrl.gpio.rts, 'r', &i, argc, argv);
                break;

            case 's':
                debug = FALSE;
                break;

            case 'v':
                debug = TRUE;
                break;

            case '?':
                showUsage = TRUE;
                break;

            default:
                ++errors;
                showUsage = TRUE;
            }
        } else {
            ++errors;
            showUsage = TRUE;
        }

        ++i;
    }

    if (showUsage) {
        fprintf(
            stderr, "Usage: %s [options]\n"
            "Options are \n"
            "-c port      select the GPIO port# for ~CTS\n"
            "-d device    specify the serial device (%s)\n"
            "-h port      select the GPIO port# for ~HRDY\n"
            "-r port      select the GPIO port# for ~RTS\n"
            "-s           enable silent mode\n"
            "-v           enable verbose mode\n", argv[0], ctrl.device);
    }

    return errors;
}

int main(int argc, char* argv[], char* env[])
{
    LonApiError sts = Parse(argc, argv) == 0 ? LonApiNoError : LonApiInitializationFailure;

    if (sts == LonApiNoError) {
		printf("%s %s\n%s\n\n", TITLE, VERSION, COPYRIGHT);
		printf("Enter 'x' to exit or ? for help.\n");

		if (tracefile.name) {
			tracefile.fp = fopen(tracefile.name, "w");
		}

        sts = LonInit(&ctrl);
        ConioCtrl(TRUE);

        while (sts == LonApiNoError) {
            LonEventHandler();

            if (ConioPending()) {
                switch(ConioGetc()) {
                case 'p':
                    Execute("LonSendServicePin", LonSendServicePin());
                    break;

                case 'r':
                    Execute("LonResume", LonResume());
                    break;

                case 's':
                    Execute("LonSuspend(SYNCHED)", LonSuspend(LDV_SUSPEND_SYNCHED, 5));
                    break;

                case 'x':
                    Trace("Shutting down...\n");
                    goto shutdown;
                    break;

                case '?':
                    printf(
                        "r: resume a suspended driver\n"
                        "s: suspend the driver\n"
                        "p: send service pin message\n"
                        "x: exit\n"
                    );
                    break;
                }
            }

            // Do something else.



        }

shutdown:
        ConioCtrl(FALSE);

        if (sts == LonApiNoError) {
            sts = LonExit();
        }
    }

    if (tracefile.fp) {
        fclose(tracefile.fp);
    }

    return sts != LonApiNoError;
}
