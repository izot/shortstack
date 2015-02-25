/*
 * Filename: ShortStackIsiHandlers.c
 *
 * Description: This file contains the implementation for the
 * callback handlers invoked by the ISI engine through an ISI-aware
 * Micro Server.
 *
 * Copyright (c) Echelon Corporation 2008-2015.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

#include "ShortStackDev.h"

#if	LON_ISI_ENABLED
#include "ShortStackIsiApi.h"

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
 *
 * Define ISICREATEPERIODICMSG_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISICREATEPERIODICMSG_HANDLED
LonBool IsiCreatePeriodicMsg(void)
{
    /*
     * TO DO
     */
    return FALSE;
}
#endif
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
 *
 * Define ISIUPDATEUSERINTERFACE_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIUPDATEUSERINTERFACE_HANDLED
void IsiUpdateUserInterface(IsiEvent event, unsigned parameter)
{
    /*
     * TO DO
     */
}
#endif
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
 *
 * Define ISICREATECSMO_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISICREATECSMO_HANDLED
void IsiCreateCsmo(unsigned assembly, IsiCsmoData* pCsmo)
{
    /*
     * TO DO
     */
    pCsmo = NULL;
}
#endif
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
 *
 * Define ISIGETPRIMARYGROUP_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETPRIMARYGROUP_HANDLED
unsigned IsiGetPrimaryGroup(unsigned assembly)
{
    /*
     * TO DO
     */
    return ISI_DEFAULT_GROUP;
}
#endif
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
 *
 * Define ISIGETASSEMBLY_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETASSEMBLY_HANDLED
unsigned IsiGetAssembly(const IsiCsmoData* pCsmo, LonBool automatic)
{
    /*
     * TO DO
     */
    return ISI_NO_ASSEMBLY;
}
#endif	/*	ISIGETASSEMBLY_HANDLED */
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
 *
 * Define ISIGETNEXTASSEMBLY_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETNEXTASSEMBLY_HANDLED
unsigned IsiGetNextAssembly(const IsiCsmoData* pCsmo, LonBool automatic, unsigned assembly)
{
    /*
     * TO DO
     */
    return ISI_NO_ASSEMBLY;
}
#endif	/*	ISIGETNEXTASSEMBLY_HANDLED	*/
#endif

#ifdef ISI_HOST_GETNVINDEX
/*
 * Callback:   IsiGetNvIndex
 * Returns the network variable index 0 to 254 of the network variable at the
 * specified offset within the specified assembly, or ISI_NO_INDEX if no such
 * network variable exists.
 *
 * Remarks:
 * This function must return at least one valid network variable index for each
 * assembly number returned by <IsiGetAssembly> and <IsiGetNextAssembly>. The
 * *offset* parameter is zero-based and relates to the selector number offset that
 * is used with the enrollment of this assembly.
 * The function operates whether the ISI engine is running or not.
 *
 * Define ISIGETNVINDEX_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETNVINDEX_HANDLED
unsigned IsiGetNvIndex(unsigned assembly, unsigned offset)
{
    /*
     * TO DO
     */
    return ISI_NO_INDEX;
}
#endif	/*	ISIGETNVINDEX_HANDLED	*/
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
 *
 * Define ISIGETNEXTNVINDEX_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETNEXTNVINDEX_HANDLED
unsigned IsiGetNextNvIndex(unsigned assembly, unsigned offset, unsigned previous)
{
    /*
     * TO DO
     */
    return ISI_NO_INDEX;
}
#endif	/* ISIGETNEXTNVINDEX_HANDLED */
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
 *
 * The ISI domain is specific to your network, and typically assigned a quasi-
 * unique 6 byte domain ID value from your ISI Domain Address Server (DAS).
 * Many ISI networks do not require an ISI Domain Address Server, however, but
 * use a domain ID assigned by you instead. This is implemented with the this
 * callback. ISI uses a default 3-byte domain ID of "IS\0".
 *
 * Define ISIGETPRIMARYDID_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETPRIMARYDID_HANDLED
const unsigned* IsiGetPrimaryDid(unsigned* pLength)
{
    /*
     * TO DO: Adjust, if necessary, to support your network's unique domain ID.
     */
    *pLength = 3;
    return "IS\0";
}
#endif	/* ISIGETPRIMARYDID_HANDLED */
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
 *
 * Define ISIGETWIDTH_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETWIDTH_HANDLED
unsigned IsiGetWidth(unsigned assembly)
{
    /*
     * TO DO
     */
    return 0;
}
#endif	/* ISIGETWIDTH_HANDLED */
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
 *
 * Define ISICONNECTIONTABLE_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISICONNECTIONTABLE_HANDLED
unsigned IsiGetConnectionTableSize(void)
{
    /*
     * TO DO
     */
    return 0;
}

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
 *
 * Define ISICONNECTIONTABLE_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
IsiConnection* IsiGetConnection(unsigned index)
{
    /*
     * TO DO
     */
    return NULL;
}

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
 *
 * Define ISICONNECTIONTABLE_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
void IsiSetConnection(IsiConnection* pConnection, unsigned index)
{
    /*
     * TO DO
     */
}
#endif	/* ISICONNECTIONTABLE_HANDLED */
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
 *
 * Define ISIGETREPEATCOUNT_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETREPEATCOUNT_HANDLED
const unsigned IsiGetRepeatCount(void)
{
    /*
     * TO DO
     */
    return 0;
}
#endif	/* ISIGETREPEATCOUNT_HANDLED */
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
 *
 * Define ISIQUERYHEARTBEAT_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIQUERYHEARTBEAT_HANDLED
LonBool IsiQueryHeartbeat(unsigned index)
{
    /*
     * TO DO
     */
    return FALSE;
}
#endif	/* ISIQUERYHEARTBEAT_HANDLED */
#endif

/*
 * Callback:   IsiGetNvValue
 * Returns the current value of the specified network variable.
 *
 * Remarks:
 * This callback returns the value of the network variable specified by the index
 * nvIndex.
 *
 * Define ISIGETNVVALUE_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIGETNVVALUE_HANDLED
const unsigned* IsiGetNvValue(unsigned nvIndex, LonByte* pLength)
{
    const LonNvDescription* const nvTable =
    		(const LonNvDescription* const)LonGetNvTable();

    *pLength = nvTable[nvIndex].DeclaredSize;
    return (void*) nvTable[nvIndex].pData;
}
#endif	/* ISIGETNVVALUE_HANDLED */

/*
 * Callback:   IsiUserCommand
 * Informs the host application about events happening in the Micro Server.
 *
 * Remarks:
 * Some custom Micro Servers may need to inform the host application of events
 * known to the custom Micro Server alone. This callback can be invoked by such
 * Micro Servers for that purpose. If the host application can't process such
 * an event, it returns a NACK to the Micro Server.
 *
 * Define ISIUSERCOMMAND_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIUSERCOMMAND_HANDLED
unsigned IsiUserCommand(unsigned param1, unsigned param2, const void* pData, unsigned length)
{
    /*
     * TO DO
     */
    return 0xFF; /* This will result in the API sending a NACK back to the Micro Server */
}
#endif /* ISIUSERCOMMAND_HANDLED */

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
 *
 * Define ISIISCONNECTEDRECEIVED_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIISCONNECTEDRECEIVED_HANDLED
void IsiIsConnectedReceived(unsigned assembly, LonBool isConnected)
{
    /*
     * TO DO
     */
}
#endif /* ISIISCONNECTEDRECEIVED_HANDLED */

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
 *
 * Define ISIIMPLEMENTATIONVERSIONRECEIVED_HANDLED in your project settings
 * to indicate that you provide this callback outside this module.
 */
#ifndef ISIIMPLEMENTATIONVERSIONRECEIVED_HANDLED
void IsiImplementationVersionReceived(unsigned version)
{
    /*
     * TO DO
     */
}
#endif /* ISIIMPLEMENTATIONVERSIONRECEIVED_HANDLED */

/*
 * Callback:   IsiProtocolVersionReceived
 * Occurs when the response to <IsiQueryProtocolVersion> is received.
 *
 * Parameters:
 * version - version number of the ISI protocol being implemented.
 *
 * Remarks:
 * This callback contains the results of the <IsiQueryProtocolVersion> function.
 *
 * Define ISIPROTOCOLVERSIONRECEIVED_HANDLED in your project settings
 * to indicate that you provide this callback outside this module.
 */
#ifndef ISIPROTOCOLVERSIONRECEIVED_HANDLED
void IsiProtocolVersionReceived(unsigned version)
{
    /*
     * TO DO
     */
}
#endif /* ISIPROTOCOLVERSIONRECEIVED_HANDLED */

/*
 * Callback:   IsiIsRunningReceived
 * Occurs when the response to <IsiQueryIsRunning> is received.
 *
 * Parameters:
 * isRunning - boolean indicating whether the ISI engine is running or not.
 *
 * Remarks:
 * This callback contains the results of the <IsiQueryIsRunning> function.
 *
 * Define ISIISRUNNINGRECEIVED_HANDLED in your project settings to
 * indicate that you provide this callback outside this module.
 */
#ifndef ISIISRUNNINGRECEIVED_HANDLED
void IsiIsRunningReceived(LonBool isRunning)
{
    /*
     * TO DO
     */
}
#endif	/* ISIISRUNNINGRECEIVED_HANDLED */

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
 *
 * Define ISIISBECOMINGHOSTRECEIVED_HANDLED in your project settings to
 * indicate that you provide this callback outside this module.
 */
#ifndef ISIISBECOMINGHOSTRECEIVED_HANDLED
void IsiIsBecomingHostReceived(unsigned assembly, LonBool isBecomingHost)
{
    /*
     * TO DO
     */
}
#endif /* ISIISBECOMINGHOSTRECEIVED_HANDLED */

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
 *
 * Define ISIAPICOMPLETE_HANDLED in your project settings to indicate that
 * you provide this callback outside this module.
 */
#ifndef ISIAPICOMPLETE_HANDLED
void IsiApiComplete(IsiDownlinkRpcCode code, LonByte sequence, LonBool success)
{
    /*
     * TO DO
     */
}
#endif /* ISIAPICOMPLETE_HANDLED */
#endif	// LON_ISI_ENABLED
