/*
 * ShortStack for Raspberry Pi Example of applications with runtime
 * interface selection.
 *
 * See the rpi-ris.c file for more about this application example.
 * This file implements the deluxe interface for this device, which
 * includes four CO2 sensor blocks and a node object. The deluxe
 * implementation also supports the optional nciCO2Offset property.
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
#include "ShortStackDev.h"
#include "ShortStackApi.h"

//@IzoT Option target("shortstack-classic")
//@IzoT Option programId("9F:FF:FF:0A:46:04:04:01")
//@IzoT Option server("../../../microserver/standard/SS430_FT6050_SYS20000kHz")
//@IzoT Option namespace("deluxe")

/*
 * The application's interface definition follows.
 *
 * The node object must be declared first.
 */
SFPTnodeObject(node) nodeObject;    //@IzoT block external("nodeObject"), \
//@IzoT onUpdate(nviRequest, onNviRequest),\
//@IzoT implement(nciLocation), implement (nciNetConfig), \
//@IzoT implement(nciDevMajVer, init=1), implement(nciDevMinVer, init=0)

SFPTco2Sensor(deluxe) deluxeCO2[4];		//@IzoT block external("CO2"), \
//@IzoT implement(nvoCO2ppm.nciCO2Offset)

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

