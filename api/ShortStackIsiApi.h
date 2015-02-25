/*
 * Filename: ShortStackIsiApi.h
 *
 * Description: This file contains function prototypes for the Interoperable
 * Self-Installation (ISI) part of the ShortStack LonTalk Compact API.
 *
 * Copyright (c) 2008-2015 Echelon Corporation.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

#ifndef SHORTSTACK_ISI_API_H
#define SHORTSTACK_ISI_API_H

#if	LON_ISI_ENABLED

/*
 * *****************************************************************************
 * TITLE: SHORTSTACK ISI API
 * *****************************************************************************
 *
 * Function prototypes for the Interoperable Self-Installation (ISI) portion of
 * the ShortStack LonTalk Compact API.
 */

/*
 * Note this file is best included through ShortStackDev.h, which is generated
 * by the LonTalk Interface Developer.
 */
#ifndef _LON_PLATFORM_H
#   error You must include LonPlatform.h first (prefer including ShortStackDev.h)
#endif  /* _LON_PLATFORM_H */

#ifndef DEFINED_SHORTSTACKDEV_H
#   error You must include ShortStackDev.h first
#endif  /* DEFINED_SHORTSTACKDEV_H */

#include "ShortStackIsiTypes.h"
#include "ShortStackIsiHandlers.h"

/*
 * ******************************************************************************
 * SECTION: ISI API FUNCTIONS
 * ******************************************************************************
 *
 * This section details the ShortStack ISI API functions.
 */

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
extern const LonApiError IsiStop(void);

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
extern const LonApiError IsiStart(IsiType type, IsiStartFlags flags);

/*
 * Function: IsiReturnToFactoryDefaults
 * Restores the device's self-installation data to factory defaults.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function restores the device's self-installation data to factory defaults,
 * causing the immediate and unrecoverable loss of all connection information.
 * This function has the same functionality regardless of whether the ISI engine
 * is running or not. The engine is stopped and the device resets to complete the
 * process. Because of the reset, this function never returns to the caller. Any
 * changes related to returning to factory defaults, such as resetting of
 * device-specific configuration properties to their initial values, must occur
 * prior to calling this function.
 */
extern const LonApiError IsiReturnToFactoryDefaults(void);

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
extern const LonApiError IsiAcquireDomain(LonBool sharedServicePin);

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
extern const LonApiError IsiStartDeviceAcquisition(void);

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
extern const LonApiError IsiOpenEnrollment(unsigned assembly);

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
extern const LonApiError IsiCreateEnrollment(unsigned assembly);

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
extern const LonApiError IsiExtendEnrollment(unsigned assembly);

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
extern const LonApiError IsiCancelEnrollment(void);

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
extern const LonApiError IsiLeaveEnrollment(unsigned assembly);

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
extern const LonApiError IsiDeleteEnrollment(unsigned assembly);

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
extern const LonApiError IsiInitiateAutoEnrollment(const IsiCsmoData* pCsma, unsigned assembly);

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
extern const LonApiError IsiQueryIsConnected(unsigned assembly);

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
extern const LonApiError IsiQueryImplementationVersion(void);

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
extern const LonApiError IsiQueryProtocolVersion(void);

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
extern const LonApiError IsiQueryIsRunning(void);

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
extern const LonApiError IsiQueryIsBecomingHost(unsigned assembly);

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
extern const LonApiError IsiCancelAcquisition(void);

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
extern const LonApiError IsiFetchDevice(void);

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
extern const LonApiError IsiFetchDomain(void);

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
extern const LonApiError IsiIssueHeartbeat(unsigned index);

/*
 * ******************************************************************************
 * SECTION: ISI UPLINK DATA CALLBACK PROTOTYPES
 * ******************************************************************************
 *
 * Following are the prototypes for the functions which serve as a means for
 * returning data requested by certain ISI API calls.
 * Skeletons for these callback functions are provided in ShortStackIsiHandlers.c.
 */

/*
 * Callback:   IsiIsConnectedReceived
 * Occurs when the response to <IsiQueryIsConnected> is received.
 *
 * Parameters:
 * assembly - assembly for which the query was made.
 * isConnected - boolean indicating the connection status of the assembly.
 *
 * Remarks:
 * This callback contains the results of the <IsiQueryIsConnected> function.
 */
extern void IsiIsConnectedReceived(unsigned assembly, LonBool isConnected);

/*
 * Callback:   IsiImplementationVersionReceived
 * Occurs when the response to <IsiQueryImplementationVersion> is received.
 *
 * Parameters:
 * version - version number of the ISI implementation.
 *
 * Remarks:
 * This callback contains the results of the <IsiQueryImplementationVersion>
 * function.
 */
extern void IsiImplementationVersionReceived(unsigned version);

/*
 * Callback:   IsiProtocolVersionReceived
 * Occurs when the response to <IsiQueryProtocolVersion> is received.
 *
 * Parameters:
 * version - version number of the ISI protocol being implemented.
 *
 * Remarks:
 * This callback contains the results of the <IsiQueryProtocolVersion> function.
 */
extern void IsiProtocolVersionReceived(unsigned version);

/*
 * Callback:   IsiIsRunningReceived
 * Occurs when the response to <IsiQueryIsRunning> is received.
 *
 * Parameters:
 * isRunning - boolean indicating whether the ISI engine is running or not.
 *
 * Remarks:
 * This callback contains the results of the <IsiQueryIsRunning> function.
 */
extern void IsiIsRunningReceived(LonBool isRunning);

/*
 * Callback:   IsiIsBecomingHostReceived
 * Occurs when the response to <IsiQueryIsBecomingHost> is received.
 *
 * Parameters:
 * assembly - the assembly for which the query was made.
 * isBecomingHost - boolean indicating whether the assembly in question is
 * becoming a host.
 *
 * Remarks:
 * This callback contains the results of the <IsiQueryIsBecomingHost> function.
 */
extern void IsiIsBecomingHostReceived(unsigned assembly, LonBool isBecomingHost);

/*
 * Callback:   IsiApiComplete
 * Indicates that the API function is complete and the result has been received.
 *
 * Parameters:
 * code - downlink function that just completed.
 * sequence - sequence number of the downlink message.
 * success - boolean indicating whether the function completed successfully.
 *
 * Remarks:
 * This function is called when an API function completes.  You generally
 * should not call an ISI API until the previous one completes.  In some cases
 * if you violate this rule, the ShortStack Micro Server could return a NACK,
 * and this callback will be invoked with a value of FALSE for the "success"
 * parameter.  Note that the Micro Server is not guaranteed to return ACKs and
 * NACKs in order.
 */
extern void IsiApiComplete(IsiDownlinkRpcCode code, LonByte sequence, LonBool success);

/*
 * ******************************************************************************
 * SECTION: ISI CALLBACK HANDLER PROTOTYPES
 * ******************************************************************************
 *
 * Following are the prototypes for the ShortStack ISI host API callback handler
 * functions that are called by the ISI engine through the Micro Server.
 * Skeletons for these callback functions are provided in ShortStackIsiHandlers.c.
 */

#ifdef ISI_HOST_CREATEPERIODICMSG
/*
 * Callback:   IsiCreatePeriodicMsg
 * Specifies whether the application has any messages for the ISI engine to send
 * using the periodic broadcast scheduler.
 *
 * Remarks:
 * Because the ISI engine sends periodic outgoing messages at regular intervals,
 * this function allows an application to send a message in one of the periodic
 * message slots. If the application has no message to send, then this function
 * should return FALSE. If it does have a message to send, then this function
 * should return TRUE.
 * To use this function, you must enable application-specific periodic messages
 * using the *IsiFlagApplicationPeriodic* flag when you call the <IsiStart> function.
 * The default implementation of <IsiCreatePeriodicMsg> does nothing but return
 * FALSE. You can override this function by providing an application-specific
 * implementation of <IsiCreatePeriodicMsg>.
 * Do not send any messages, start other network transactions, or call other ISI
 * API functions while the <IsiCreatePeriodicMsg> callback is running.
 */
extern LonBool IsiCreatePeriodicMsg(void);
#endif

#ifdef ISI_HOST_UPDATEUSERINTERFACE
/*
 * Callback:   IsiUpdateUserInterface
 * Provides status feedback from the ISI engine.
 *
 * Remarks:
 * These events are useful for synchronizing the device's user interface with
 * the ISI engine. To receive notification of ISI status events, override the
 * <IsiUpdateUserInterface> callback function. The default implementation of
 * <IsiUpdateUserInterface> does nothing.
 * The ISI engine calls this function with the event parameter set to one of the
 * values defined for the <IsiEvent> enumeration. Some of these events carry a
 * meaningful value in the parameter argument, as detailed in the <IsiEvent>
 * definition. You can use the event parameter passed to the
 * <IsiUpdateUserInterface> callback function to track the state of the ISI
 * engine. This is a simple way to determine which ISI function calls make sense
 * at any time, and which ones don't. The function operates whether the ISI
 * engine is running or not.
 */
extern void IsiUpdateUserInterface(IsiEvent event, unsigned parameter);
#endif

#ifdef ISI_HOST_CREATECSMO
/*
 * Callback:   IsiCreateCsmo
 * Constructs the IsiCsmoData portion of a CSMO Message.
 *
 * Remarks:
 * The pCsmo parameter is a pointer to an *IsiCsmoData* structure that is filled
 * by this function call. This function is called by the ISI engine prior to
 * sending a CSMO message.
 * This function has the same effect if the ISI engine is running or not.
 */
extern void IsiCreateCsmo(unsigned assembly, IsiCsmoData* pCsmo);
#endif

#ifdef ISI_HOST_GETPRIMARYGROUP
/*
 * Callback:   IsiGetPrimaryGroup
 * Returns the group ID for the specified assembly.
 *
 * Remarks:
 * The default implementation returns ISI_DEFAULT_GROUP (128). This function is
 * only required if the default implementation, or the forwardee, of
 * <IsiCreateCsmo> is in use.
 * The function operates whether the ISI engine is running or not.
 */
extern unsigned IsiGetPrimaryGroup(unsigned assembly);
#endif

#ifdef ISI_HOST_GETASSEMBLY
/*
 * Callback:   IsiGetAssembly
 * Returns the number of the first assembly that can join the enrollment
 * characterized by pCsmo.
 *
 * Remarks:
 * The function returns ISI_NO_ASSEMBLY (0xFF) if no such assembly exists, or
 * an application-defined assembly number (0 to 254). The automatic parameter
 * specifies a manually initiated enrollment (automatic = FALSE) or an automatically
 * initiated enrollment (automatic = TRUE). The automatic flag is TRUE both for
 * initial automatic enrollment messages (CSMA) or reminders that relate to a
 * possibly new connection (CSMR). Automatic enrollment reminder messages that
 * relate to existing connections on the local device are not passed to the
 * application. The pointer provided with the *pCsmo* parameter is only valid
 * for the duration of this function call.
 * The function operates whether the ISI engine is running or not.
 */
extern unsigned IsiGetAssembly(const IsiCsmoData* pCsmo, LonBool automatic);
#endif

#ifdef ISI_HOST_GETNEXTASSEMBLY
/*
 * Callback:   IsiGetNextAssembly
 * Returns the next applicable assembly following the one indicated with the
 * assembly argument for an incoming CSMO.
 *
 * Remarks:
 * The function returns ISI_NO_ASSEMBLY (0xFF) if there are no such assemblies,
 * or an application-specific assembly number (1 to 254). This function is called
 * after calling the <IsiGetAssembly> function, unless <IsiGetAssembly> returned
 * ISI_NO_ASSEMBLY. The automatic parameter specifies a manually initiated
 * enrollment (automatic = FALSE) or an automatically initiated enrollment
 * (automatic = TRUE). The automatic flag is TRUE both for initial automatic
 * enrollment messages (CSMA) or reminders that relate to a possibly new
 * connection (CSMR). Automatic enrollment reminder messages that relate to
 * existing connections on the local device are not passed to the application.
 * The pointer provided with the *pCsmo* parameter is only valid for the
 * duration of this function call. The function operates whether the ISI engine
 * is running or not.
 */
extern unsigned IsiGetNextAssembly(const IsiCsmoData* pCsmo, LonBool automatic, unsigned assembly);
#endif

#ifdef ISI_HOST_GETNVINDEX
/*
 * Callback:   IsiGetNvIndex
 * Returns the network variable index 0 - 254 of the network variable at the
 * specified offset within the specified assembly, or ISI_NO_INDEX if no such
 * network variable exists.
 *
 * Remarks:
 * This function must return at least one valid network variable index for each
 * assembly number returned by <IsiGetAssembly> and <IsiGetNextAssembly>. The
 * *offset* parameter is zero-based and relates to the selector number offset that
 * is used with the enrollment of this assembly.
 * The function operates whether the ISI engine is running or not.
 */
extern unsigned IsiGetNvIndex(unsigned assembly, unsigned offset);
#endif

#ifdef ISI_HOST_GETNEXTNVINDEX
/*
 * Callback:   IsiGetNextNvIndex
 * Returns the network variable index of the network variable at the specified
 * offset within the specified assembly, following the network variable specified
 * by the previous index.
 *
 * Remarks:
 * Returns ISI_NO_INDEX if there are no more network variables, or a valid
 * network variable index (1 to 254) otherwise. The *offset* parameter is zero-based
 * and relates to the selector number offset within the assembly that is
 * used with the enrollment of this assembly.
 * The function operates whether the ISI engine is running or not.
 */
extern unsigned IsiGetNextNvIndex(unsigned assembly, unsigned offset, unsigned previous);
#endif

#ifdef ISI_HOST_GETPRIMARYDID
/*
 * Callback:   IsiGetPrimaryDid
 * Returns a pointer to the default primary domain ID for the device.
 *
 * Remarks:
 * The function also provides the domain ID length in the location provided by
 * the *pLength* parameter. Domain IDs can be 1, 3, or 6 bytes long; the 0-length
 * domain ID cannot be used for the primary domain. The number of bytes provided
 * through the *pLength* output parameter must be valid in the returned pointer.
 * You can override this function to override the ISI standard domain ID value.
 * This function is only used to create a non-ISI system.
 * Both length and value of the domain ID provided are considered constant when
 * the ISI engine is running. To change the primary domain ID at runtime using
 * the <IsiGetPrimaryDid> callback, stop and re-start the ISI engine.
 */
extern const unsigned* IsiGetPrimaryDid(unsigned* pLength);
#endif

#ifdef ISI_HOST_GETWIDTH
/*
 * Callback:   IsiGetWidth
 * Returns the width in the specified assembly.
 *
 * Remarks:
 * The width is equal to the number of network variable selectors associated
 * with the assembly.
 * Applications must override the <IsiGetWidth> function to support compound
 * assemblies. This function operates whether the ISI engine is running or not.
 */
extern unsigned IsiGetWidth(unsigned assembly);
#endif

#ifdef ISI_HOST_CONNECTIONTABLE
/*
 * Callback:   IsiGetConnectionTableSize
 * Returns the number of entries in the connection table.
 *
 * Remarks:
 * The default implementation returns the number of entries in the built-in
 * connection table. You can override this function to support an
 * application-specific implementation of the ISI connection table. You can use
 * this function to provide a larger connection table or to store the connection
 * table outside of the Smart Transceiver address space.
 * The ISI library supports connection tables with 0 to 254 entries. The
 * connection table size is considered constant following a call to <IsiStart>;
 * you must first stop, then re-start the ISI engine if the connection table size
 * changes dynamically.
 */
extern unsigned IsiGetConnectionTableSize(void);

/*
 * Callback:   IsiGetConnection
 * Returns a pointer to an entry in the connection table.
 *
 * Remarks:
 * The default implementation returns a pointer to a built-in connection table.
 * You can override this function to provide an application-specific means of
 * accessing the connection table, or to provide an application table of a
 * different size. The ISI engine requests only one connection table entry at a
 * time, and makes no assumption about the pointer value.
 * This function is called frequently, and should return as soon as possible.
 */
extern IsiConnection* IsiGetConnection(unsigned index);

/*
 * Callback:   IsiSetConnection
 * Updates an entry in the connection table.
 *
 * Remarks:
 * The default implementation updates an entry in the built-in connection table.
 * You can override this function to provide an application-specific means of
 * accessing the connection table, or to provide an application table of a
 * different size.
 * The ISI engine requests only one connection table entry at a time, and makes
 * no assumption about the pointer value.
 * This function is called frequently, and should return as soon as possible.
 */
extern void IsiSetConnection(IsiConnection* pConnection, unsigned index);
#endif

#ifdef ISI_HOST_GETREPEATCOUNT
/*
 * Callback:   IsiGetRepeatCount
 * Specifies the repeat count used with all network variable connections, where
 * all connections share the same repeat counter.
 *
 * Remarks:
 * The repeat counter value is considered constant for the lifetime of the
 * application, and will only be queried: when the device powers up the first
 * time after a new application image has been loaded, and every time
 * <IsiReturnToFactoryDefaults> runs. Only repeat counts of 1, 2 or 3 are
 * supported. This function has no affect on ISI messages.
 * This function operates whether the ISI engine is running or not.
 */
extern const unsigned IsiGetRepeatCount(void);
#endif

#ifdef ISI_HOST_QUERYHEARTBEAT
/*
 * Callback:   IsiQueryHeartbeat
 * Returns TRUE if a heartbeat for the network variable with the global index
 * nvIndex has been sent, and returns FALSE otherwise.
 *
 * Remarks:
 * When network variable heartbeat processing is enabled and the ISI engine
 * is running, the engine queries bound output network variables using this
 * callback (including any alias connections) whenever the heartbeat is due.
 */
extern LonBool IsiQueryHeartbeat(unsigned nvIndex);
#endif

/*
 * Callback:   IsiGetNvValue
 * Returns the current value of the specified network variable.
 *
 * Remarks:
 * This callback returns the value of the network variable specified by the index
 * nvIndex.
 */
extern const unsigned* IsiGetNvValue(unsigned nvIndex, LonByte* pLength);

/*
 * Callback:   IsiUserCommand
 * Informs the host application about events happening in the Micro Server.
 *
 * Remarks:
 * Some custom Micro Servers may need to inform the host application of events
 * known to the custom Micro Server alone. This callback can be invoked by such
 * Micro Servers for that purpose. If the host application can't process such
 * an event, it returns a NACK to the Micro Server.
 */
extern unsigned IsiUserCommand(unsigned param1, unsigned param2, const void* pData, unsigned length);

#endif	// LON_ISI_ENABLED
#endif /* SHORTSTACK_ISI_API_H */
