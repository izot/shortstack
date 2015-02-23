/*
 * IzoT ShortStack 4.30 Link Layer driver API.
 *
 * ldv.h defines the driver API expected by the IzoT ShortStack host API.
 * Earlier versions of this API have been defined within ShortStackApi.h.
 *
 * The driver API has been separated out into Ldv.h to clearly identify
 * the API your driver for the serial ShortStack link-layer must implement.
 *
 * The IzoT ShortStack LDV API has been enhanced from earlier versions of
 * the same API by providing improved support for use with an embedded
 * operating system while still supporting bare-metal systems. See the
 * comments provided within this file for details and considerations.
 *
 * Note that the ShortStack 4.30 is not backwards-compatible. Key
 * improvements are:
 *
 * 1. LdvInit() is no longer supported. Use LdvOpen() and, where applicable,
 *    LdvClose() instead. LdvOpen supports an optional control data structure
 *    which your application may use to supply the driver with additional
 *    details, such as the name of a serial port. This control data structure
 *    is defined by you in LdvTypes.h, and used by your application and
 *    your driver. The ShortStack API passes this through but makes no
 *    assumptions on its type.
 *
 *    When not required, define this as a simple type (e.g. int) and assign
 *    a NULL-pointer.
 *
 * 2. All Ldv* API (with the exception of LdvOpen()) require a handle
 *    as the first argument. This handle is returned by LdvOpen(). The
 *    handle is introduced to better support targets with an operating
 *    system rather than bare-metal host designs.
 *
 *    The ShortStack API makes no assumptions on the value or nature of the
 *    driver handle. Driver implementations which do not require such a
 *    handle may use a simple non-zero integer value.
 *
 *    Note that ShortStack 4.30 does not require the use of an operating
 *    system.
 *
 * 3. The LdvPutMsgBlocking() API is obsolete and no longer used.
 *
 * 4. The LdvFlushMsgs() API is obsolete and no longer used. Link
 *    layer drivers are expected to be either blocking or asynchronous
 *    based on interrupts or concurrent processing (threads, fibers).
 *
 *    However, for implementions with limited resources, the blocking
 *    LdvAllocateMsgWait() API has been added. The ShortStack API only
 *    uses the LdvAllocateMsgWait() API during initialization.
 *
 * 5. An optional LdvSuspend() and LdvResume() API has been added.
 *    Applications may use this to temporarily suspend the driver and
 *    minimize processing activity (to the extend possible).
 *
 *    This API is implemented in your driver, and available options are
 *    defined by your driver and used by your application. The ShortStack
 *    API does not require or use this API, but provides a pass-through
 *    API with <LonResume> and <LonSuspend> functions.
 *
 * 6. A new LdvClose() API has been added to support application termination
 *    and orderly shutdown.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#if !defined(IZOT_SHORTSTACK_LDV_H)
#   define IZOT_SHORTSTACK_LDV_H

#include <stdlib.h>
#include <stdint.h>

#include "ldvTypes.h"

/*
 * Function: LdvOpen
 *
 * LdvOpen() succeeds the LdvInit() function. It prepares the IzoT ShortStack
 * link layer driver and returns a valid handle or 0 through the handle output
 * parameter.
 *
 * Parameters:
 * ctrl - a pointer to the driver-specific <LdvCtrl> control data structure.
 * handle - output parameter, pointer to <LdvHandle>.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvOpen(const LdvCtrl* ctrl, LdvHandle* handle);

/*
 * Function: LdvClose
 *
 * LdvClose() completes all pending downlink traffic and closes the driver.
 * The handle is no longer valid when LdvClose() returns.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvClose(LdvHandle handle);

/*
 * Function: LdvAllocateMsg
 *
 * LdvAllocateMsg() allocates a transmit buffer.
 * A successfully allocated buffer is always cleared to zeroes, and has
 * a quasi-unique Id assigned. The Id number may be helpful when logging
 * frame data for debugging purposes, but is otherwise irrelevant. The
 * ShortStack API imposes no meaning on this number.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 * pFrame - output parameter, pointer to a LonSmipMsg* pointer variable.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvAllocateMsg(LdvHandle handle, LonSmipMsg** pFrame);

/*
 * Function: LdvAllocateMsgWait
 *
 * LdvAllocateMsgWait() allocates a transmit buffer, and waits a suitable
 * time, if necessary, until one becomes available.
 * This API is only used during initialization of the application.
 *
 * Note this API may still fail, for example with LonApiTimeout. When
 * successful, this function returns a frame conditioned in the same way
 * as with <LdvAllocateMsg>.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 * pFrame - output parameter, pointer to a LonSmipMsg* pointer variable.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvAllocateMsgWait(LdvHandle handle, LonSmipMsg** pFrame);

/*
 * Function: LdvPutMsg
 *
 * LdvPutMsg() submits a message for downlink transfer. The driver returns
 * the frame buffer to the pool once all data has been transmitted.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 * pFrame - pointer to the frame buffer.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvPutMsg(LdvHandle handle, LonSmipMsg* pFrame);

/*
 * Function: LdvGetMsg
 *
 * LdvGetMsg() retrieves an incoming message (if any). When successful, the
 * caller is responsible for returing the frame buffer to the pool with
 * <LdvReleaseMsg>.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 * pFrame - output parameter, pointer to a frame buffer pointer.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvGetMsg(LdvHandle handle, LonSmipMsg** pFrame);

/*
 * Function: LdvReleaseMsg
 *
 * LdvReleaseMsg() releases the message buffer after processing
 * is complete.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 * pFrame - pointer to the frame buffer.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvReleaseMsg(LdvHandle handle, LonSmipMsg* pFrame);

/*
 * Function: LdvReset
 *
 * LdvReset() resets the driver in a brute force operation, abandoning
 * and dismissing any partial transfers.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvReset(LdvHandle handle);

/*
 * Function: LdvSuspend
 *
 * LdvSuspend() is an optional feature; drivers not supporting this
 * operation may do nothing but return LonApiNotSupported. When the
 * call succeeds, the caller is responsible for calling <LdvResume>.
 *
 * Implementations which support <LdvSuspend> MUST also support
 * <LdvResume>.
 *
 * The ShortStack API does not use this API but provides a simple
 * pass-through <LonSuspend> API.
 *
 * The driver implementation also defines the modes supported and the
 * timeout units. This example implementation supports a timeout in
 * seconds and two suspend modes:
 *
 * IMMEDIATE:   Suspend, and discard all work in progress.
 * SYNCHED:     Suspend after pending frame is complete.
 *
 * The HRDY signal will be asserted when the uplink path is suspended,
 * if a HRDY GPIO pin assignment was provided with LdvCtrl.
 *
 * The operation can time out after 'timeout' seconds. A timeout
 * value of zero can block indefinitely.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 * mode -   mode flags, defined by the driver.
 *
 * Result:
 * <LonApiError>.
 */
#define LDV_SUSPEND_IMMEDIATE   0x11
#define LDV_SUSPEND_SYNCHED     0x22

extern LonApiError LdvSuspend(LdvHandle handle, unsigned mode, unsigned timeout);

/*
 * Function: LdvResume
 *
 * LdvResume() resumes a previously suspended driver.
 *
 * This is an optional feature; drivers not supporting this
 * operation may do nothing but return LonApiNotSupported.
 *
 * Implementations should always support both <LdvSuspend> and <LdvResume>
 * or neither.
 *
 * The ShortStack API does not use this API but provides a simple
 * pass-through <LonResume> API.
 *
 * Parameters:
 * handle - the driver handle obtained from <LdvOpen>.
 *
 * Result:
 * <LonApiError>.
 */
extern LonApiError LdvResume(LdvHandle handle);

#endif  /*  IZOT_SHORTSTACK_LDV_H */
