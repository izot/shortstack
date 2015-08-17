/*
 * IzoT ShortStack for Raspberry Pi Example code.
 *
 * gpio.c provides a few routines for general-purpose I/O with the
 * Raspberry Pi and a standard Raspian Wheezy operating system, or
 * compatible platforms.
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

/*
 * Openf() implements open for sprintf-style filenames, useful when dealing
 * with sysfs files.
 */
static int Openf(mode_t mode, const char* fmt, ...)
{
    char filename[256];
    va_list list;

    va_start(list, fmt);
    vsprintf(filename, fmt, list);
    va_end(list);

    /*
     * After exporting GPIO pins into sysfs, the system takes a moment until
     * these are accessible with the intended group and access permissions.
     * This delay is caused by the time required for the udev daemon to
     * activate the corresponding rules.
     * Here, we wait until write permission is granted or until a timeout
     * occurs. When the time out occurs, we proceed trying to open anyway,
     * because this yields the appropriate system error code for us when it
     * fails.
     *
     * Note that this code assumes that we'll be granted write access through
     * group permissions.
     */
    if (mode & O_WRONLY) {
        int timeout = 100;
        struct stat status = {0};

        do {
			/*
			 * We always begin with a small delay. It seems this is almost
			 * always required before we can even stat the file, because
			 * execute permission to the directory is required to stat()
			 * the file, and the file must exist.
			 */
			struct timespec wait = {0, 5000000};    // 5ms
			nanosleep(&wait, NULL);
			--timeout;

			if (stat(filename, &status)) {
				/* stat() failed. This implementation assumes that the
				 * failure is transient, either because the GPIO sysfs
				 * value or direction file does not yet exist, or
				 * because the folder containing that file does not yet
				 * exist or is not yet searchable (executable), which
				 * is required by stat(). Therefore, this implementation
				 * ignores the error and tries again, replying on eventual
				 * success or failure by timeout.
				 */
				memset(&status, 0, sizeof(status));
			}
        } while (timeout && !(status.st_mode & S_IWGRP));
    }

    return open(filename, mode);
}

/*
 * GpioOpen opens one GPIO pin in the given direction and mode, with an
 * optional edge trigger condition.
 *
 * Arguments:
 *
 * port     1 for GPIO1, etc
 * mode     O_WRONLY or O_RDONLY
 * dir      one of "in", "out", "high" or "low"
 * trigger  one of "none", "rising", "falling" or "both". Can be NULL.
 *
 * See here for a full documentation of GPIO through sysfs:
 * https://www.kernel.org/doc/Documentation/gpio/sysfs.txt
 */
int GpioOpen(
    int port, mode_t mode, const char* dir, const char* trigger
)
{
    int fd = open("/sys/class/gpio/export", O_WRONLY);

    if (fd) {
        char buffer[8];
        sprintf(buffer, "%d", port);
        write(fd, buffer, strlen(buffer));
        close(fd);

        fd = Openf(O_WRONLY, "/sys/class/gpio/gpio%d/direction", port);

        if (fd) {
            write(fd, dir, strlen(dir));
            close(fd);

            if (trigger) {
                fd = Openf(O_WRONLY, "/sys/class/gpio/gpio%d/edge", port);

                if (fd) {
                    write(fd, trigger, strlen(trigger));
                    close(fd);
                }
            }
        }
    }

    return Openf(mode, "/sys/class/gpio/gpio%d/value", port);
}

/*
 * GpioClose closes a GPIO port and unexports it from sysfs.
 */
int GpioClose(int port, int handle)
{
    int fd = -1;

    close(handle);

    fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (fd) {
        char buffer[8];
        sprintf(buffer, "%d", port);
        write(fd, buffer, strlen(buffer));
        close(fd);
    }

    return 0;
}


