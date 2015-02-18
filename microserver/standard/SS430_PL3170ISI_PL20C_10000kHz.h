/*
 * ShortStackIsiHandlers.h
 *
 * This file defines the location of ISI callbacks. See below
 * for an exhaustive discussion.
 *
 * Copyright (c) 2008-2009 Echelon Corporation.  All rights reserved.
 *
 * This file is ShortStack API Software as defined in the Software
 * License Agreement that governs its use.
 *
 * ECHELON MAKES NO REPRESENTATION, WARRANTY, OR CONDITION OF
 * ANY KIND, EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE OR IN
 * ANY COMMUNICATION WITH YOU, INCLUDING, BUT NOT LIMITED TO,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, SATISFACTORY
 * QUALITY, FITNESS FOR ANY PARTICULAR PURPOSE, 
 * NONINFRINGEMENT, AND THEIR EQUIVALENTS.
 */

#ifndef ISI_HANDLERS_H
#define ISI_HANDLERS_H

/*
 * Each ISI callback can be implemented in three different ways:
 * 
 * (a) an ISI callback may be implemented in the host application.
 * This is known as a host-side ISI callback override. If this option
 * is chosen, the Micro Server supplies the remote procedure call 
 * that invokes the callback within the host application, and returns
 * possible result data to ISI. You will write code that implements 
 * the host-side ISI callback override with your embedded development
 * tool. You will use the ShortStackIsiHandlers.c file to provide these
 * implementations. The LonTalk Interface Developer tool provides a 
 * skeleton version of the ShortStackIsiHandlers.c file within your
 * host application framework project folder. 
 * 
 * (b) an ISI callback may be implemented local to the Micro Server.
 * This is known as a Micro Server-side ISI callback override. If this
 * option is chosen, you can implement the respective callback override
 * in the MicroServerIsiHandlers.c file, using the Neuron C language. 
 * This option allows to create custom ShortStack Micro Servers with ISI 
 * callback implementations that are tailored to your application's requirements,
 * but do not consume code space within your host processor. Micro Server-
 * side callbacks complete faster than host-side callbacks, allowing for 
 * a more responsive overall application.
 * 
 * (c) an ISI callback may be implemented using the callback's default 
 * implementation, which is provided with ISI itself. This option requires 
 * no code at all. Default callbacks are a good choice for generic callbacks
 * such as those providing access to the ISI connection table, or those 
 * concerned with rarely used optional features. 
 * For application-specific callbacks, a host-side or Micro Server-side,
 * application-specific callback implementation is recommended.
 * Default callbacks are always implemented as Micro Server-side ISI callbacks.
 * 
 * You use this file to determine the location of each ISI callback between
 * the three possibilities discussed above. You define -or comment out- 
 * Neuron C proprocessor symbols ("defines," or "macros") to select exactly 
 * one location for each callback, using symbols that follow a simple naming 
 * scheme:
 * The symbol name starts with ISI_, followed by DEFAULT_, HOST_ or SERVER_,
 * followed by the callback identifier. The callback identifier equals the 
 * callback name all in capitals, and minus the "Isi" prefix. For example,
 * ISI_DEFAULT_GETNVINDEX or ISI_SERVER_GETASSEMBLY are valid names that 
 * follow this convention. 
 * All supported symbols are provided below, so you don't have to create 
 * these names (but you must understand them in order to make the 
 * associated decisions). 
 * 
 * Exceptions:
 *
 * (1) The callbacks responsible for providing access to the ISI connection 
 * table (IsiGetConnectionTableSize, IsiSetConnection and IsiGetConnection) 
 * must be thought of as an atomic unit, and must always be implemented 
 * in the same location. To meet this requirement, a single symbol defines 
 * the location of all three callbacks that are related to the connection 
 * table: ISI_*_CONNECTIONTABLE.
 *
 * (2) The IsiGetNvValue callback _must_ be implemented on the host. The 
 * location of this callback is not configurable with this scheme, therefore. 
 *
 * (3) You cannot implement both the connection table and the IsiCreateCsmo 
 * callback on the host.
 */



/*
 *  Callback: IsiCreatePeriodicMsg
 *  Standard location: default
 *
 *  The IsiCreatePeriodicMsg callback enabled an optional and advanced feature,
 *  through which the application can claim a slot in the ISI broadcast scheduler.
 *  This callback is rarely overridden.
 */
#define ISI_DEFAULT_CREATEPERIODICMSG
/*#define ISI_SERVER_CREATEPERIODICMSG  */
/*#define ISI_HOST_CREATEPERIODICMSG    */



/*
 *  Callback: IsiUpdateUserInterface
 *  Standard location: host
 *
 *  The IsiUpdateUserInterface callback informs your application of important
 *  state changes, and allows keeping a user interface in synch with ISI. 
 *  This callback will almost always be overridden on the host application.
 */
/*#define ISI_DEFAULT_UPDATEUSERINTERFACE   */
/*#define ISI_SERVER_UPDATEUSERINTERFACE    */
#define ISI_HOST_UPDATEUSERINTERFACE



/*
 *  Callback: IsiCreateCsmo
 *  Standard location: host
 *
 *  The IsiCreateCsmo callback is used to supply ISI with connection information
 *  on devices that make calls to the IsiOpenEnrollment API. This callback is 
 *  typically overridden on the host processor.
 *
 *  You cannot implement both the connection table and the IsiCreateCsmo callback 
 *  on the host.
 *
 *  Note that using the default implementation of the IsiCreateCsmo callback is
 *  not supported. You may implement this callback within a custom Micro Server
 *  or within your host application, but you cannot use the default implementation
 *  that is incuded with the ISI library: The default implementation of the 
 *  IsiCreateCsmo callback required access to the device's self-identification and
 *  self-documentation data, which is located within the host application.
 */
/*#define ISI_SERVER_CREATECSMO         */
#define ISI_HOST_CREATECSMO



/*
 *  Callback: IsiGetPrimaryGroup
 *  Standard location: host
 *
 *  The IsiGetPrimaryGroup callback supplies ISI with the identifier of this device's
 *  primary ISI device group identifier, as defined by LonMark International. This
 *  callback is typically overwritten on the host or the Micro Server.
 */
/*#define ISI_DEFAULT_GETPRIMARYGROUP   */
/*#define ISI_SERVER_GETPRIMARYGROUP    */
#define ISI_HOST_GETPRIMARYGROUP



/*
 *  Callback: IsiGetAssembly
 *  Standard location: host
 *
 *  The IsiGetAssembly callback is used to determine local assembly numbers that might
 *  accept an new enrollment. This callback is typically overridden on the host processor,
 *  but a Micro Server-side override might lead to a more responsive device.
 */
/*#define ISI_DEFAULT_GETASSEMBLY       */
/*#define ISI_SERVER_GETASSEMBLY        */
#define ISI_HOST_GETASSEMBLY



/*
 *  Callback: IsiGetNextAssembly
 *  Standard location: host
 *
 *  The IsiGetNextAssembly callback is used in conjunction with the IsiGetAssembly callback,
 *  and is typically, but not necessarily, overridden in the same location. See IsiGetAssembly.
 */
/*#define ISI_DEFAULT_GETNEXTASSEMBLY   */
/*#define ISI_SERVER_GETNEXTASSEMBLY    */
#define ISI_HOST_GETNEXTASSEMBLY



/*
 *  Callback: IsiGetNvIndex
 *  Standard location: host
 *
 *  The IsiGetNvIndex callback is used to determine local network variables that are
 *  members of a given assembly number. This callback is typically overridden on the host 
 *  processor, but a Micro Server-side override might lead to a more responsive device.
 */
/*#define ISI_DEFAULT_GETNVINDEX        */
/*#define ISI_SERVER_GETNVINDEX         */
#define ISI_HOST_GETNVINDEX



/*
 *  Callback: IsiGetNextNvIndex
 *  Standard location: host
 *
 *  The IsiGetNextNvIndex callback is used in conjunction with the IsiGetNvIndex callback,
 *  and is typically, but not necessarily, overridden in the same location. 
 *  See IsiGetNvIndex.
 */
/*#define ISI_DEFAULT_GETNEXTNVINDEX    */  
/*#define ISI_SERVER_GETNEXTNVINDEX     */
#define ISI_HOST_GETNEXTNVINDEX



/*
 *  Callback: IsiGetPrimaryDid
 *  Standard location: default
 *
 *  The IsiGetPrimaryDid callback is used to override the default domain Id ("ISI" by default). 
 *  A device's final domain Id is typically assigned in the field by using an ISI domain address
 *  server (DAS). The IsiGetPrimaryDid() callback is typically not overridden for ISI-S and 
 *  ISI-DA devices.
 */
#define ISI_DEFAULT_GETPRIMARYDID
/*#define ISI_SERVER_GETPRIMARYDID      */
/*#define ISI_HOST_GETPRIMARYDID        */



/*
 *  Callback: IsiGetWidth
 *  Standard location: host
 *
 *  The IsiGetWidth callback is used to inform ISI about the number of network variable selectors
 *  that are required to implement a given local assembly. This callback is typically, but not 
 *  necessarily, implemented in the same location as the IsiGetAssembly callback. See there.
 */
/*#define ISI_DEFAULT_GETWIDTH          */
/*#define ISI_SERVER_GETWIDTH           */
#define ISI_HOST_GETWIDTH



/*
 *  Callback: IsiGetConnectionTableSize, IsiGetConnection, IsiSetConnection
 *  Standard location: server
 *
 *  The ISI connection table and the three related callbacks are typically overridden. 
 *  On severely resource-constrained devices, overrides allow for a connection table with 
 *  fewer than eight records (the default). Typcially, the connection table is overridden 
 *  to allow for a larger ISI connection table, thus allowing for a more flexibility when 
 *  integrating the device in the network. 
 *  Since the ISI connection table is frequently searched for particular records, 
 *  overriding the ISI connection table on the Micro Server itself is recommended, as this 
 *  leads to better performance compared to implementing the table on the host processor.
 *
 *  You cannot implement both the connection table and the IsiCreateCsmo callback 
 *  on the host.
 */
/*#define ISI_DEFAULT_CONNECTIONTABLE   */
#define ISI_SERVER_CONNECTIONTABLE  
/*#define ISI_HOST_CONNECTIONTABLE      */

/*
 *  Check that we did not implement both the IsiCreateCsmo callback _and_ the connection table
 *  on the host. You would typically implement the connection table local to the Micro Server
 *  (ISI_SERVER_CONNECTIONTABLE) for best performance and in order to facilitate unused Flash
 *  memory on the Micro Server. Some special cases require a host-based connection table, however.
 *  These cases include very resource-limited Micro Servers, or a requirement for very large 
 *  ISI connection tables. You may implement a Micro Server that expects the connection table
 *  implemented on the host processor, but must implement the IsiCreateCsmo callback on local
 *  to the Micro Server in this case. You can also implement both the IsiCreateCsmo callback 
 *  and the connection table local to the Micro Server, but you cannot implement both on the 
 *  host.
 */
#ifdef ISI_HOST_CREATECSMO
#   ifdef ISI_HOST_CONNECTIONTABLE
#       error "You cannot implement both the connection table and the IsiCreateCsmo callback on the host"
#   endif
#endif

/*
 *  Callback: IsiGetRepeatCount
 *  Standard location: default
 *
 *  The IsiGetRepeatCount callback is used to change the default repeat count for network variable
 *  connections from three to one. This is a rarely used optional feature; this callback is rarely 
 *  overwritten, therefore.
 */
#define ISI_DEFAULT_GETREPEATCOUNT
/*#define ISI_SERVER_GETREPEATCOUNT     */
/*#define ISI_HOST_GETREPEATCOUNT       */



/*
 *  Callback: IsiQueryHeartbeat
 *  Standard location: host
 *
 *  The IsiQueryHeartbeart function is used to select those bound output network variables that are 
 *  subject to heartbeat messages (periodic re-transmission of their most recent values). This call-
 *  back is typically overridden in the location of the IsiGetNvIndex callback.
 */
/*#define ISI_DEFAULT_QUERYHEARTBEAT    */
/*#define ISI_SERVER_QUERYHEARTBEAT     */
#define ISI_HOST_QUERYHEARTBEAT



#endif   /*  ISI_HANDLERS_H */

