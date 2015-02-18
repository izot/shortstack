/*
 * Filename: ShortStackIsiApi.c
 *
 * Description: This file contains function implementations for the Interoperable
 * Self-Installation (ISI) part of the IzoT ShortStack LonTalk Compact API.
 *
 * Copyright (c) 2008-2015 Echelon Corporation.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

#include <string.h>
#include "ShortStackDev.h"

#if	LON_ISI_ENABLED
#include "ShortStackIsiApi.h"

/*
 * Forward declarations for functions used internally by the ShortStack ISI API.
 * These functions are implemented in ShortStackIsiInternal.c.
 */
extern LonApiError SendDownlinkRpc(IsiDownlinkRpcCode code, LonByte param1, LonByte param2, void* pData, unsigned len);

/*
 * Function: IsiStop
 * Stops the ISI engine.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use the <IsiStart> function to restart the ISI engine.
 * This function has no forwarder. Calling this function when the ISI engine
 * is already stopped has no action.
 */
const LonApiError IsiStop(void)
{
    return SendDownlinkRpc(IsiRpcStop, 0, 0, NULL, 0);
}

/*
 * Function: IsiStart
 * Starts the ISI engine.
 *
 * Parameters:
 * type - type of the ISI engine.
 * flags - option flags when starting the engine.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * After this function is called, the ISI engine sends and receives ISI
 * messages, and manages the network configuration of your device. Typically,
 * you start the ISI engine in your reset task when self-installation is
 * enabled, and you stop the ISI engine when self-installation is disabled.
 */
const LonApiError IsiStart(IsiType type, IsiStartFlags flags)
{
    return SendDownlinkRpc(IsiRpcStart, type, flags, NULL, 0);
}

/*
 * Function: IsiReturnToFactoryDefaults
 * Restores the device�s self-installation data to factory defaults.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function restores the device�s self-installation data to factory defaults,
 * causing the immediate and unrecoverable loss of all connection information.
 * This function has the same functionality regardless of whether the ISI engine
 * is running or not. The engine is stopped and the device resets to complete the
 * process. Because of the reset, this function never returns to the caller. Any
 * changes related to returning to factory defaults, such as resetting of
 * device-specific configuration properties to their initial values, must occur
 * prior to calling this function.
 */
const LonApiError IsiReturnToFactoryDefaults(void)
{
    return SendDownlinkRpc(IsiRpcReturnToFactoryDefaults, 0, 0, NULL, 0);
}

/*
 * Function: IsiAcquireDomain
 * Starts or restarts the domain ID acquisition process in an ISI-DA device.
 *
 * Parameters:
 * sharedServicePin - boolean indicating whether the service pin is shared.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function is also used for the confirmation step in the domain ID
 * acquisition and domain ID sniffing procedures.
 * The ISI engine must be running for this function to have any effect.
 * This function has unpredictable effect on an ISI-S device and therefore
 * should only be used with an ISI-DA device.
 */
const LonApiError IsiAcquireDomain(LonBool sharedServicePin)
{
    return SendDownlinkRpc(IsiRpcAcquireDomain, sharedServicePin, 0, NULL, 0);
}

/*
 * Function: IsiStartDeviceAcquisition
 * Starts or retriggers device acquisition mode on a domain address server.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function has no effect on an ISI-S or ISI-DA device, or if the ISI
 * engine is stopped. No forwarder is provided for this function.
 */
const LonApiError IsiStartDeviceAcquisition(void)
{
    return SendDownlinkRpc(IsiRpcStartDeviceAcquisition, 0, 0, NULL, 0);
}

/*
 * Function: IsiOpenEnrollment
 * Opens manual enrollment for the specified assembly.
 *
 * Parameters:
 * assembly - assembly for which the enrollment should be opened.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This operation turns the device into a connection host for this connection,
 * and sends a CSMO manual connection invitation to all devices in the network.
 * No forwarder is provided for this function. The ISI engine must be running
 * and in the idle state.
 */
const LonApiError IsiOpenEnrollment(unsigned assembly)
{
    return SendDownlinkRpc(IsiRpcOpenEnrollment, assembly, 0, NULL, 0);
}

/*
 * Function: IsiCreateEnrollment
 * Accepts a connection invitation.
 *
 * Parameters:
 * assembly - assembly for which the enrollment should be created.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function is called after the application has received and approved a
 * CSMO open enrollment message. The connection is created anew, replacing
 * any previously existing enrollment information associated with this assembly.
 * On a connection host that has received at least one CSME enrollment acceptance
 * message, this command completes the enrollment and implements the connection
 * as new, replacing any previously existing enrollment information associated
 * with this assembly.
 * The ISI engine must be running and in the correct state for this function to
 * have any effect. For a connection host, the ISI engine must be in the
 * approved state. Other devices must be in the pending state.
 */
const LonApiError IsiCreateEnrollment(unsigned assembly)
{
    return SendDownlinkRpc(IsiRpcCreateEnrollment, assembly, 0, NULL, 0);
}

/*
 * Function: IsiExtendEnrollment
 * Extends an enrollment invitation.
 *
 * Parameters:
 * assembly - assembly for which the enrollment should be extended.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function is called after the application has received and approved a
 * CSMO open enrollment message. The connection is added to any previously
 * existing connections. If no previous connection exists for assembly, a new
 * connection is created.
 * On a connection host that has received at least one CSME enrollment acceptance
 * message, this command completes the enrollment and extends any existing
 * connections. If no previous connection exists for assembly, a new connection
 * is created.
 * The ISI engine must be running and in the correct state for this function to
 * have any effect. For a connection host, the ISI engine must be in the approved
 * state. Other devices must be in the pending state.
 */
const LonApiError IsiExtendEnrollment(unsigned assembly)
{
    return SendDownlinkRpc(IsiRpcExtendEnrollment, assembly, 0, NULL, 0);
}

/*
 * Function: IsiCancelEnrollment
 * Cancels an open (pending or approved) enrollment.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * When used on a connection host, a CSMX connection cancellation message is
 * issued to cancel enrollment on the connection members. When used on a device
 * that has accepted, but not yet implemented, an open enrollment, this function
 * causes the device to opt out of the enrollment locally.
 * The function has no effect unless the ISI engine is running and in the pending
 * or approved state.
 */
const LonApiError IsiCancelEnrollment(void)
{
    return SendDownlinkRpc(IsiRpcCancelEnrollment, 0, 0, NULL, 0);
}

/*
 * Function: IsiLeaveEnrollment
 * Removes the specified assembly from all enrolled connections as a local
 * operation only.
 *
 * Parameters:
 * assembly - assembly for which the enrollment should be removed.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * When used on the connection host, the function is automatically
 * interpreted as <IsiDeleteEnrollment>.
 * No forwarder is provided for this function. This function has no effect if
 * the ISI engine is stopped.
 */
const LonApiError IsiLeaveEnrollment(unsigned assembly)
{
    return SendDownlinkRpc(IsiRpcLeaveEnrollment, assembly, 0, NULL, 0);
}

/*
 * Function: IsiDeleteEnrollment
 * Removes the specified assembly from all enrolled connections.
 *
 * Parameters:
 * assembly - assembly for which the enrollment should be deleted.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function removes the specified assembly from all enrolled connections,
 * and sends a CSMD connection deletion message to all other devices in the
 * connection to remove them from the connection as well.
 * This function has no effect if the ISI engine is stopped.
 */
const LonApiError IsiDeleteEnrollment(unsigned assembly)
{
    return SendDownlinkRpc(IsiRpcDeleteEnrollment, assembly, 0, NULL, 0);
}

/*
 * Function: IsiInitiateAutoEnrollment
 * Starts automatic enrollment.
 *
 * Parameters:
 * pCsma - data for automatic enrollment.
 * assembly - assembly for which the enrollment should be initiated.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to start automatic enrollment. The local device becomes the
 * host for the automatic connection. Automatic enrollment can replace previous
 * connections, if any. No forwarder exists for this function.
 * This function cannot be called before the *isiWarm* event has been signaled
 * in the <IsiUpdateUserInterface> callback.
 * This function does nothing when the ISI engine is stopped.
 */
const LonApiError IsiInitiateAutoEnrollment(const IsiCsmoData* pCsma, unsigned Assembly)
{
    return SendDownlinkRpc(IsiRpcInitiateAutoEnrollment, Assembly, 0, (void*) pCsma, sizeof(IsiCsmoData));
}

/*
 * Function: IsiQueryIsConnected
 * Queries the connection status of an assembly.
 *
 * Parameters:
 * assembly - assembly for which the connection status needs to be queried.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Successful invocation of this function results in the callback
 * <IsiIsConnectedReceived>. The callback returns TRUE if the specified assembly
 * is enrolled in a connection. No forwarder is provided for this function.
 * The function operates whether the ISI engine is running or not. The callback
 * returns FALSE if the ISI engine is stopped.
 */
const LonApiError IsiQueryIsConnected(unsigned assembly)
{
    return SendDownlinkRpc(IsiRpcIsConnected, assembly, 0, NULL, 0);
}

/*
 * Function: IsiQueryImplementationVersion
 * Queries the version number of the ISI implementation.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Successful invocation of this function results in the callback
 * <IsiImplementationVersionReceived>. The callback returns the version number
 * of the ISI implementation in the Micro Server. No forwarder is provided for
 * this function. This function operates in any state of the ISI engine.
 */
const LonApiError IsiQueryImplementationVersion(void)
{
    return SendDownlinkRpc(IsiRpcImplementationVersion, 0, 0, NULL, 0);
}

/*
 * Function: IsiQueryProtocolVersion
 * Queries the protocol version of the ISI implementation.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Successful invocation of this function results in the callback
 * <IsiProtocolVersionReceived>. The callback returns the version of the ISI
 * protocol supported by the ISI engine. The number indicates the maximum
 * protocol version supported.
 * The ISI engine also supports protocol versions less than the number
 * returned unless explicitly indicated. No forwarder is provided for this
 * function. The function operates in either state of the ISI engine.
 */
const LonApiError IsiQueryProtocolVersion(void)
{
    return SendDownlinkRpc(IsiRpcProtocolVersion, 0, 0, NULL, 0);
}

/*
 * Function: IsiQueryIsRunning
 * Queries the state of the ISI engine.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Successful invocation of this function results in the callback
 * <IsiIsRunningReceived>. The callback returns TRUE if the ISI engine is
 * running and FALSE if the ISI engine is stopped. Non-zero values are
 * considered TRUE. No forwarder is provided for this function. The function
 * operates in all states of the ISI engine.
 */
const LonApiError IsiQueryIsRunning(void)
{
    return SendDownlinkRpc(IsiRpcIsRunning, 0, 0, NULL, 0);
}

/*
 * Function: IsiQueryIsBecomingHost
 * Queries whether the specified assembly is a host for an enrollment.
 *
 * Parameters:
 * assembly - assembly which needs to be queried.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Successful invocation of this function results in the callback
 * <IsiIsBecomingHostReceived>. The callback returns TRUE if <IsiOpenEnrollment>
 * has been called for the specified assembly and the enrollment has not yet
 * timed out, been cancelled, or confirmed. The function returns FALSE otherwise.
 * You can also override the <IsiUpdateUserInterface> callback function and
 * track the state of the assembly. The callack returns TRUE if that state is
 * *IsiPendingHost* or *IsiApprovedHost*.
 * No forwarder is provided for this function. The function operates whether the
 * ISI engine is running or not.
 */
const LonApiError IsiQueryIsBecomingHost(unsigned assembly)
{
    return SendDownlinkRpc(IsiRpcIsBecomingHost, assembly, 0, NULL, 0);
}

/*
 * Function: IsiCancelAcquisition
 * Cancels both device and domain acquisition.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Following the completion of this function call, <IsiUpdateUserInterface> is
 * called with the *IsiNormal* event.
 * This function has no effect unless the ISI engine is running and the device is
 * either in device or domain acquisition mode.
 */
const LonApiError IsiCancelAcquisition(void)
{
    return SendDownlinkRpc(IsiRpcCancelAcquisition, 0, 0, NULL, 0);
}

/*
 * Function: IsiFetchDevice
 * Fetches a device by assigning a domain to the device from a domain address
 * server (DAS).
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function must not be called from a device that is not a domain address
 * server. This operation does not require application code on the remote device.
 * The remote device remains unaware of the change to its primary domain.
 * An alternate method to assign a domain to a device is for the device to use the
 * <IsiAcquireDomain> function. This function provides more immediate recovery
 * from network addressing conflicts and more immediate maintenance of automatic
 * connections. The disadvantage of using the <IsiAcquireDomain> function is that
 * it typically requires more code on the device, and it requires that the device
 * support ISI-DA. The <IsiFetchDevice> function can be used with any device. A
 * DAS must support both methods.
 * The ISI engine must be running for this function to have any effect, and this
 * function only operates on a domain address server.
 */
const LonApiError IsiFetchDevice(void)
{
    return SendDownlinkRpc(IsiRpcFetchDevice, 0, 0, NULL, 0);
}

/*
 * Function: IsiFetchDomain
 * Starts or restarts the fetch domain process in a domain address server (DAS).
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function must not be called from a device that is not a domain address
 * server. The ISI engine must be running for this function to have any effect,
 * and this function only operates on a domain address server.
 */
const LonApiError IsiFetchDomain(void)
{
    return SendDownlinkRpc(IsiRpcFetchDomain, 0, 0, NULL, 0);
}

/*
 * Function: IsiIssueHeartbeat
 * Sends an update for the specified bound output network variable and its
 * aliases, using group addressing.
 *
 * Parameters:
 * index - network variable index for which the heart beat needs to be issued.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function sends a network variable update for the specified network
 * variable index and its aliases, as long as the network variable is a bound
 * output network variable using group addressing.
 * This function uses the address table for addressing information, but always
 * uses unacknowledged service with one repeat when issuing the heartbeat. It
 * only sends a network variable if it uses group addressing (the default ISI
 * addressing mode) � it skips updates that use a different addressing mode.
 * This function is typically called in an <IsiQueryHeartbeat> callback. If the
 * function is called outside of this callback to create a custom heartbeat
 * scheme, the application must only call this function for bound output network
 * variables.
 * This function requires that the ISI engine has been started with the
 * *IsiFlagHeartbeat* flag.
 */
const LonApiError IsiIssueHeartbeat(unsigned index)
{
    return SendDownlinkRpc(IsiRpcIssueHeartbeat, index, 0, NULL, 0);
}

#endif	//	LON_ISI_ENABLED
