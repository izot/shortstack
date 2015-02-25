/*
 * ShortStack for Raspberry Pi Simple ISI Example
 *
 * This example application implements a clock and a clock display.
 * The example is designed for use with generic hardware
 * and simulates all physical I/O with simple functions and console input
 * and output.
 *
 * This example demonstrates fundamentals of the Interoperable Self-
 * Installation API (ISI) with IzoT ShortStack.
 * The application manages the ISI engine's life cycle and can offer and
 * accept connections between the application's input datapoints (representing
 * the sunblind actuator, or driver) and output datapoints  (representing the
 * sunblind controller). Such a connection can be established
 * between two or more like devices, but this application is also able to
 * connect to itself (a 'turnaround connection') and thus can function with
 * one device only.
 *
 * (Note that ISI can only create turnaround connections if the host assembly
 * contains at least one output datapoint. This is the case in this application
 * example.)
 *
 * In addition to the general project settings discussed in the Eclipse
 * Configuration document, this example requires that you define the following
 * symbols in your project settings or makefile for all configurations:
 *
 * ISIGETWIDTH_HANDLED
 * ISIGETNVINDEX_HANDLED
 * ISIGETASSEMBLY_HANDLED
 * ISICREATECSMO_HANDLED
 * ISIUPDATEUSERINTERFACE_HANDLED
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
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>

#include "ShortStackDev.h"
#include "ShortStackApi.h"
#include "ShortStackIsiApi.h"
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
//@IzoT Option programId("9F:FF:FF:08:00:04:04:00")
//@IzoT Option isi(On)
//@IzoT Option server("../../../microserver/standard/SS430_FT6050ISI_SYS20000kHz")

//@IzoT Event onReset(onReset), onWink(onWink)

/*
 * The application's interface definition follows.
 *
 * Because this application implements more than one block, it must also
 * implement the standard node object profile and provide simple code to
 * supply housekeeping and device management functions. This is provided
 * with the onNviRequest event handler, also defined below.
 *
 * The node object must be declared first.
 */
SFPTnodeObject(node) nodeObject;    //@IzoT block external("nodeObject"), \
//@IzoT onUpdate(nviRequest, onNviRequest), \
//@IzoT implement(nciLocation), \
//@IzoT implement (nciNetConfig, flags=Reset, init=CFG_LOCAL), \
//@IzoT onUpdate(nciNetConfig, onNetConfigUpdate), \
//@IzoT implement(nciDevMajVer, init=1), implement(nciDevMinVer, init=0)

/*
 * Real Time Keeper
 *
 * This example application implements the standard real time keeper profile.
 * This provides date and time through an output datapoint. The application
 * automatically re-sends the current date and time at configurable intervals,
 * which start with a default value assigned to the nciUpdateDate property.
 */
SFPTrealTimeKeeper(rtc) rtc; 	//@IzoT block external("clock"), \
//@IzoT implement(nvoTimeDate.nciUpdateRate, init=60) /* 60.0 seconds */

/*
 * Clock Display
 *
 * This example application implements a simple generic open loop actuator
 * profile as a date and time display.
 * The open loop actuator profile does not stipulate a particular datapoint
 * type; this implementation uses SNVT_time_stamp for compatibility with the
 * real time keeper block.
 */
SFPTopenLoopActuator(display, SNVT_time_stamp) display; //@IzoT block \
//@IzoT external("clockDisplay"), onUpdate(nviValue, onClockDisplay)

static int Trace(const char* fmt, ...);

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

void onNetConfigUpdate(
   const unsigned index,
   const LonReceiveAddress* const pSourceAddress
) {
	if (*nodeObject.nciNetConfig == CFG_LOCAL) {
        /* The device is returned to self-installation.
         * Clear old configuration data and start again.
         * Note that this task can take a significant
         * amount of time, after which the Micro Server
         * resets. */
    	Trace("Resetting ISI engine and data\n");
        IsiReturnToFactoryDefaults();
     }
}

void onClockDisplay(
   const unsigned index,
   const LonReceiveAddress* const pSourceAddress
) {
	static const char* months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
    printf(
        "%d-%s-%d %02d:%02d:%02d\n",
        display.nviValue.data.day,
		months[display.nviValue.data.month],
		LON_GET_UNSIGNED_WORD(display.nviValue.data.year),
		display.nviValue.data.hour,
		display.nviValue.data.minute,
		display.nviValue.data.second
	);
}

void onReset(
	const LonResetNotification* const pResetNotification
) {
    if (*nodeObject.nciNetConfig == CFG_LOCAL) {
        /* Start the ISI engine */
    	Trace("Starting the ISI engine\n");
        IsiStart(IsiTypeS, IsiFlagExtended);
    }
}

void onWink(void)
{
	printf("Wink, wink, wink\n");
}

#define TITLE       "Shortstack Goes Raspberry Pi Simple ISI Demonstration"
#define COPYRIGHT   "Copyright (C) 2015 Echelon Corporation"
#define VERSION     "1.00.00"

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

#define RTC_ASSEMBLY		0
#define DISPLAY_ASSEMBLY	1

#define RTC_PROFILE_KEY		3300

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
            	/*
            	 * This application example is designed for use with generic
            	 * hardware, and does not require specific I/O (other than
            	 * the required connection to the ShortStack Micro Server).
            	 *
            	 * While most ISI devices implement their ISI user interface
            	 * with LEDs and buttons, this application uses the console.
            	 *
            	 * To establish a connection, enter 'o' to create a new
            	 * enrollment, using the RTC block as the host. This invites
            	 * other devices to join, including other portions of this
            	 * device. In this application, the display block signals its
            	 * eligibility (See IsiGetAssembly, below).
            	 *
            	 * Now enter 'C' to have the display join the enrollment,
            	 * then enter 'c'to confirm the enrollment with the host.
            	 *
            	 * This completes the enrollment and establishes the
            	 * connection. The display begins to receive datapoint updates
            	 * in the onClockDisplay handler. Note that the default update
            	 * rate is configured for 60 seconds.
            	 *
            	 * To delete the enrollment, enter 'd'.
            	 */
                switch(ConioGetc()) {
                case 'c':
					IsiCreateEnrollment(RTC_ASSEMBLY);
                	break;
                case 'C':
					IsiCreateEnrollment(DISPLAY_ASSEMBLY);
                	break;
                case 'd':
                	IsiDeleteEnrollment(RTC_ASSEMBLY);
                	break;
                case 'l':
                	IsiLeaveEnrollment(DISPLAY_ASSEMBLY);
                	break;
                case 'o':
                	IsiOpenEnrollment(RTC_ASSEMBLY);
                	break;
                case 'p':
                    Execute("LonSendServicePin", LonSendServicePin());
                    break;

                case 'r':
                    Execute("LonResume", LonResume());
                    break;

                case 's':
                    Execute(
                    	"LonSuspend(SYNCHED)",
						LonSuspend(LDV_SUSPEND_SYNCHED, 5)
					);
                    break;

                case 'x':
                    Trace("Shutting down...\n");
                    goto shutdown;
                    break;

                case 'z':
                	IsiCancelEnrollment();
                	break;

                case '?':
                    printf(
                    	"c: create enrollment for the RTC\n"
                    	"C: create enrollment for the display\n"
                    	"d: delete the enrollment\n"
                    	"l: leave the enrollment (display)\n"
                    	"o: open enrollment for the RTC\n"
                        "p: send service pin message\n"
                        "r: resume a suspended driver\n"
                        "s: suspend the driver\n"
                    	"z: cancel the pending enrollment (zap)\n"
                        "x: exit\n"
                    );
                    break;
                }
            }

            // Update the realtime keeper:
            {
            	static time_t then = 0;
            	static time_t countdown = 1;

            	time_t now = time(NULL);

            	/*
            	 * Act once per second.
            	 */
            	if (now != then) {
            		struct tm current = *localtime(&now);

            		then = now;
            		rtc.nvoTimeDate.data.second = current.tm_sec;
            		rtc.nvoTimeDate.data.minute = current.tm_min;
            		rtc.nvoTimeDate.data.hour = current.tm_hour;
            		rtc.nvoTimeDate.data.day = current.tm_mday;
            		rtc.nvoTimeDate.data.month = current.tm_mon;
            		LON_SET_UNSIGNED_WORD(
            			rtc.nvoTimeDate.data.year, current.tm_year + 1900
					);

            		/*
            		 * Propagate the new value to the network according to
            		 * the nciUpdateRate property. The property supports a
            		 * resolution of 0.1 s, but this clock changes only once
            		 * every second. The SFPTrealTimeKeeper profile requires
            		 * a minumum setting of 60s and recommends only settings
            		 * of 1 minute or 1 hour or 1 day. This implementation
            		 * rounds fractions of a second up to the next second.
            		 */
            		--countdown;

            		if (countdown <= 0) {
            			const unsigned update_rate = LON_GET_UNSIGNED_WORD(
            				*rtc.nvoTimeDate.nciUpdateRate
						);

            			LonPropagateNv(rtc.nvoTimeDate.global_index);

            			/*
            			 * Compute the new countdown value, rounding up
            			 * all tenths of a second to the next whole second.
            			 */
            			countdown = update_rate / 10;
            			if (update_rate % 10) {
            				++countdown;
            			}
            		}
            	}
            }




        }

shutdown:
        ConioCtrl(FALSE);

        if (sts == LonApiNoError) {
        	IsiStop();
            sts = LonExit();
        }
    }

    if (tracefile.fp) {
        fclose(tracefile.fp);
    }

    return sts != LonApiNoError;
}

/*
 * ISI callbacks. See ShortStackIsiHandlers.c for detailed discussion of
 * these handlers.
 * This application example implements the application-specific ISI callback
 * functions in this module, leaving the default ShortStackIsiHandlers.c
 * as a re-usable, not application-specific module.
 */

/*
 * IsiUpdateUserInterface receives status information from the ISI engine.
 *
 * This callback is used to drive the user interface, typically an LED.
 */
void IsiUpdateUserInterface(IsiEvent event, unsigned parameter) {
	const char* role = "member";

	switch(event) {
	case IsiRun:
		printf("ISI is running\n");
		break;
	case IsiPendingHost:
		role = "host";
		// fall through
	case IsiPending:
		printf("Enrollment pending for %s assembly %d\n", role, parameter);
		break;
	case IsiApprovedHost:
		role = "host";
		// fall through
	case IsiApproved:
		printf("Enrollment approved for %s assembly %d\n", role, parameter);
		break;
	case IsiImplemented:
		printf("Enrollment implemented for assembly %d\n", parameter);
		break;
	case IsiCancelled:
		printf("Enrollment cancelled for assembly %d\n", parameter);
		break;
	case IsiDeleted:
		printf("Enrollment deleted for assembly %d\n", parameter);
		break;
	case IsiAborted:
		printf("Enrollment aborted for assembly %d\n", parameter);
		break;
	}
}

/*
 * Create a CSMO for assembly 0, the only assembly in this application
 * which can open enrollment. This announces the availability of the
 * RTC block:
 */
static const IsiCsmoData rtcCsmo = {
	ISI_DEFAULT_GROUP,
	IsiDirectionOutput << ISI_CSMO_DIR_SHIFT || 1 << ISI_CSMO_WIDTH_SHIFT,
	{ RTC_PROFILE_KEY / 256, RTC_PROFILE_KEY % 256 }
	/* all other fields are zero */
};

/*
 * IsiCreateCsmo supplies the ISI engine with CSMO data for the given
 * assembly. This function must support all assemblies which might
 * become enrollment hosts. In this application example, only assembly
 * 0 (the realtime keeper) can become host.
 */
void IsiCreateCsmo(unsigned assembly, IsiCsmoData* pCsmo)
{
	*pCsmo = rtcCsmo;
}

/*
 * IsiGetWidth advises the ISI engine on the number of network variable
 * selectors required by a given assembly. This is also known as the
 * assembly's width.
 * Both assemblies in this application have a width of 1.
 */
unsigned IsiGetWidth(unsigned assembly) {
	return 1;
}


/*
 * IsiGetAssembly informs the ISI engine whether at least one of the
 * local assemblies could accept this enrollment. Additional assemblies
 * can be advised with the IsiGetNextAssembly callback, which is not
 * used in this application example.
 */
unsigned IsiGetAssembly(const IsiCsmoData* pCsmo, LonBool automatic)
{
	const unsigned key = LON_GET_UNSIGNED_WORD(pCsmo->Profile);
	unsigned result = ISI_NO_ASSEMBLY;

	if (!automatic && key == RTC_PROFILE_KEY) {
		result = DISPLAY_ASSEMBLY;
	}
	return result;
}

/*
 * IsiGetNvIndex informs the ISI engine how the network variables map
 * to a given assembly. The IsiGetNvIndex callback handles the first
 * network variable, the IsiGetNextNvIndex callback, which is not
 * used in this application example, handles additional network variables.
 *
 * The offset can be used to share one network variable selector among
 * multiple network variables. This is an advanced feature not used in
 * this application.
 */
unsigned IsiGetNvIndex(unsigned assembly, unsigned offset)
{
	unsigned result = ISI_NO_INDEX;

	switch(assembly) {
	case RTC_ASSEMBLY:
		if (offset == 0) {
			result = rtc.nvoTimeDate.global_index;
		}
		break;
	case DISPLAY_ASSEMBLY:
		if (offset == 0) {
			result = display.nviValue.global_index;
		}
		break;
	}

	return result;
}

