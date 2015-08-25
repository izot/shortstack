/*
 * ShortStack for Raspberry Pi Example of applications with runtime
 * interface selection.
 *
 * See the rpi-ris.c file for more about this application example.
 * This file implements the regular interface for this device, which
 * includes nothing but a single simple CO2 sensor block.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

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
#include "regularDev.h"
#include "ShortStackApi.h"


//@IzoT Option target("shortstack-classic")
//@IzoT Option programId("9F:FF:FF:0A:46:04:04:00")
//@IzoT Option server("../../../microserver/standard/SS430_FT6050_SYS20000kHz")
//@IzoT Option namespace("regular")
//@IzoT Option output("regularDev")

/*
 * The application's interface definition follows.
 *
 * This file implements the regular interface for this device, which
 * includes nothing but a single simple CO2 sensor block. Because this
 * application implements only a single block, an implementation of a
 * node object is not required.
 *
 * Note that the implementation of the SFPTco2Sensor profile is incomplete
 * and not interoperable.
 * This example application demonstrates how to implement blocks satisfying
 * the profile's interface requirements, but does not implement the behavior
 * as required by the profile.
 */
SFPTco2Sensor(regular)	regularCO2;	//@IzoT block external("CO2")

