/*
 * IzoT ShortStack 4.30 Link Layer driver API Types.
 *
 * ldvTypes.h defines some implementation-specific datatypes used to
 * control the serial driver from the application.
 *
 * The remainder of the Link Layer driver API is defined in ldv.h
 *
 * Copyright (c) 2015 Echelon Corporation.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#if !defined(IZOT_SHORTSTACK_LDVTYPES_H)
#   define IZOT_SHORTSTACK_LDVTYPES_H

#include <stdint.h>

/*
 * Typedef: LdvCtrl
 *
 * The LdvCtrl structure is new in IzoT ShortStack 4.30. It provides a
 * simple block of configuration data used by the link layer driver.
 *
 * A pointer to the <LdvCtrl> structure is passed to LdvOpen().
 *
 * The ShortStack API imposes no meaning and makes no assumptions about
 * the type of <LdvCtrl>, it merely passes the <LdvCtrl> pointer through
 * from your application into your driver.
 *
 * The following example illustrates a possible use of the <LdvCtrl>
 * structure. Implementations which do not require such a control data
 * block should define this type as a simple data type (e.g. int),
 * and use a NULL pointer.
 */
typedef struct {
    /*
     * 'device' points to a string containing the device name.
     * Example: "/dev/ttyAMA0"
     */
#define LDVCTRL_DEFAULT_DEVICE  "/dev/ttyAMA0"
    const char* device;
    /*
     * 'bitrate' is the desired bitrate, e.g. 38400.
     */
#define LDVCTRL_DEFAULT_BITRATE 38400
    uint32_t bitrate;

    /*
     * gpio.rts and gpio.cts are the GPIO port numbers to use for RTS and
     * CTS signals via sysfs. Defaults are GPIO 10 (RTS), GPIO 9 (CTS) and
     * GPIO 11 (HRDY).
     */
#define LDVCTRL_DEFAULT_GPIO_RTS    10
#define LDVCTRL_DEFAULT_GPIO_CTS    9
#define LDVCTRL_DEFAULT_GPIO_HRDY   11

    struct {
        int rts;
        int cts;
        int hrdy;
    } gpio;

    /*
     * 'trace' is an optional pointer to a printf-style function supplied
     * by the caller. The pointer may be NULL if no trace logging is
     * desired, but a non-NULL trace function pointer must be valid for
     * the lifetime of the driver (from <LdvOpen> to <LdvClose>).
     */
    int (*trace)(const char* fmt, ...);
} LdvCtrl;

/*
 * Typedef: LdvHandle
 *
 * LdvHandle is an output parameter of LdvOpen, and an input to all other
 * Ldv* functions. LdvHandle is typically a pointer to a data structure
 * defined and managed by the driver implementation, the details of which
 * are hidden from the calling layers.
 *
 * Driver implementations not using such data should define this as a
 * simple type, e.g. int, and <LdvOpen> should return a compatible non-zero
 * value in this case (for example, one).
 */
typedef unsigned long LdvHandle;

#endif  //  IZOT_SHORTSTACK_LDVTYPES_H
