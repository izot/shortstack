/*
 * ShortStack for Raspberry Pi Example of runtime interface selection.
 *
 * This example demonstrates an IzoT ShortShort application with runtime
 * interface selection. The application's interface,
 * in this context, is the sum of network-facing items such as datapoints,
 * blocks and properties, their attributes and their relations.
 *
 * An application with runtime interface selection is one which implements
 * multiple interfaces, selecting exactly one to be active at any one time.
 * Each interface in itself is static, that is, it cannot change at runtime.
 * Dynamic interfaces are those which support runtime addition or removal
 * of blocks and datapoints. Dynamic interfaces require an advanced protocol
 * stack and are not supported by IzoT ShortStack.
 *
 * Runtime interface selection is an advanced technique and not required by
 * most IzoT ShortStack applications.
 *
 * For an example use-case, consider a device which is sold in two variants,
 * a single-channel "regular" CO2 sensor device, and a "deluxe" model with
 * four CO2 sensors. Runtime interface selection allow you to create and
 * manage one and the same application for both models, unlocking the
 * premium features at production time, for example using a sealed hardware
 * jumper, a secret configuration message or a license file.
 *
 * See the doc within this application example's folder for a more
 * detailed discussion of this approach.
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
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "ShortStackDev.h"
#include "ShortStackApi.h"
#include "ldv.h"

#define TITLE       "Shortstack Goes Raspberry Pi, Runtime Interface Selection with III"
#define COPYRIGHT   "Copyright (C) 2015 Echelon Corporation"
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

extern void SelectApplicationInterface(int id);

/*
 * RetrieveInterfaceSelection
 *
 * This function determines the currently selected interface.
 * This example provides a trivial implementation, reading the
 * product license as a simple unencrypted number from a file.
 *
 * Better implementations of this method could, for example,
 * determine the selected interface by sampling an input (a
 * configuration jumper, for example), or could read and
 * validate a license file which contains a digested form of
 * the local device's MAC-ID or Micro Server's Neuron ID.
 */
static LonApiError RetrieveInterfaceSelection(void)
{
	LonApiError result = LonApiInitializationFailure;

	int fd = open(".license", O_RDONLY);

	if (fd != -1) {
		int interface_type = 0;
		int bytes = read(fd, &interface_type, sizeof(interface_type));
		if (bytes == sizeof(interface_type)) {
			SelectApplicationInterface(interface_type);
			result = LonApiNoError;
		}
		close(fd);
	} else {
		/* For this application, it is OK if a license file doesn't
		 * exist. We interpret this as the smallest feature set.
		 */
		SelectApplicationInterface(0);
		result = LonApiNoError;
	}
	return result;
}	/* RetrieveInterfaceSelection */

/*
 * SaveInterfaceSelection
 *
 * For demonstration purposes, this example implementation allows to
 * change the selected interface with a console command at runtime.
 * This function saves the current selection.
 *
 * See the RetrieveInterfaceSelection() for related considerations.
 */
static LonApiError SaveInterfaceSelection(int id)
{
	LonApiError result = LonApiInitializationFailure;

	int fd = open(".license", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	if (fd != -1) {
		int bytes = write(fd, &id, sizeof(id));
		if (bytes == sizeof(id)) {
			result = LonApiNoError;
		}
		close(fd);
	}
	return result;

}	/* SaveInterfaceSelection */

int main(int argc, char* argv[], char* env[])
{
    LonApiError sts = Parse(argc, argv) == 0 ? LonApiNoError : LonApiInitializationFailure;

    if (sts == LonApiNoError) {
		printf("%s %s\n%s\n\n", TITLE, VERSION, COPYRIGHT);
		printf("Enter 'x' to exit or ? for help.\n");

		if (tracefile.name) {
			tracefile.fp = fopen(tracefile.name, "w");
		}

		sts = RetrieveInterfaceSelection();
    }
    if (sts == LonApiNoError) {
        sts = LonInit(&ctrl);
        ConioCtrl(TRUE);

        while (sts == LonApiNoError) {
            LonEventHandler();

            if (ConioPending()) {
                switch(ConioGetc()) {
                case '0':
                {
                	SelectApplicationInterface(0);
                	if (LonReinit() == LonApiNoError) {
                		Execute("SaveInterfaceSelection()", SaveInterfaceSelection(0));
                	}
                }
                break;
                case '1':
                {
                	SelectApplicationInterface(1);
                	if (LonReinit() == LonApiNoError) {
                		Execute("SaveInterfaceSelection()", SaveInterfaceSelection(1));
                	}
                }
                break;
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
                    	"0: switch to the regular interface\n"
                    	"1: switch to the deluxe interface\n"
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

