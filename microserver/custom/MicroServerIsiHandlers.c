//
// MicroServerIsiHandlers.c
//
// This file contains the Micro Server C code to handle ISI callbacks,
// according to your preferences from the ShortStackIsiHandlers.h and
// MicroServer.h files.
// See ShortStackIsiHandlers.h for exhaustive comments.
// For the implementation of host-side ISI callback overrides, see the
// ShortStackIsiHandlers.c file located in you application framework
// project folder.
//
// Copyright (c) 2008-2009 Echelon Corporation.  All rights reserved.
//
// This file is ShortStack API Software as defined in the Software
// License Agreement that governs its use.
//
// Within this file, you may define the behavior of ISI callbacks
// that are executed local to the Micro Server. Overriding an ISI
// callback in this way creates a Micro Server that may be specific
// to a single application (as the Micro Server-local callbacks
// are closely coupled with the host-side application). On the other
// hand, executing ISI callbacks local to the Micro Server is faster
// than redirecting the callback to the host processor through a
// remote procedure call.
//
// See the ShortStackIsiHandlers.h file for the definition of the
// location of each of these callback overrides.
//
// ECHELON MAKES NO REPRESENTATION, WARRANTY, OR CONDITION OF
// ANY KIND, EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE OR IN
// ANY COMMUNICATION WITH YOU, INCLUDING, BUT NOT LIMITED TO,
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY, SATISFACTORY
// QUALITY, FITNESS FOR ANY PARTICULAR PURPOSE,
// NONINFRINGEMENT, AND THEIR EQUIVALENTS.
//
#include <stddef.h>
#include "ShortStackIsiHandlers.h"

//
// These are the callback identifiers for uplink API calls, used with the IsiRpc()
// function that executes these callbacks as remote procedure calls on the host.
// Lic stands for Lon Isi Callback.
// The LicIsiNoAck flag indicates an unacknowledged callback.
//
// DO NOT CHANGE THE DEFINITIONS OF THIS FLAG AND THESE CALLBACK IDENTIFIERS.
//
#define LicIsiNoAck                     ((signed)0x80)

typedef enum {
    LicIsiCreatePeriodicMsg             = 0,
    LicIsiUpdateUserInterface           = 1  | LicIsiNoAck,
    LicIsiCreateCsmo                    = 2,
    LicIsiGetPrimaryGroup               = 3,
    LicIsiGetAssembly                   = 4,
    LicIsiGetNextAssembly               = 5,
    LicIsiGetNvIndex                    = 6,
    LicIsiGetNextNvIndex                = 7,
    LicIsiGetPrimaryDid                 = 8,
    LicIsiGetWidth                      = 9,
    LicIsiGetNvValue                    = 10,
    LicIsiGetConnTabSize                = 11,
    LicIsiGetConnection                 = 12,
    LicIsiSetConnection                 = 13 | LicIsiNoAck,
    LicIsiQueryHeartbeat                = 14,
    LicIsiGetRepeatCount                = 15,
    LicIsiUserCommand                   = 64
} LonIsiCallback;

//
// The code in this file uses the IsiRpc() function to remote callbacks
// to the host processor. Possible results are supplied with the two
// variables isiRpcReturnValue and isiRpcReturnData.
// All calls to IsiRpc, and all uses of the isiRpcReturnValue and
// isiRpcReturnData variables, are provided below. You do not need to
// write code which employs these items.
//
extern unsigned _RESIDENT IsiRpc(LonIsiCallback code, unsigned a, unsigned b, const void* pData, unsigned length);

extern unsigned char isiRpcReturnValue;
extern unsigned char isiRpcReturnData[31];

//
// Following are the implementations of the various ISI callbacks. According to
// your preferences made in ShortStackIsiHandlers.h, the code here will support
// implementing a callback override local to the Micro Server -this code could
// be supplied in this file-, a callback located in the host application -the
// code in this file will manage the related remote procedure calls-, or do
// nothing. In the latter case, ISI's default callback will be used, and
// executed local to the Micro Server.
//


// ----------------------------------------------------------------
//  Callback:   IsiCreatePeriodicMsg
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_CREATEPERIODICMSG
boolean _RESIDENT IsiCreatePeriodicMsg(void) {
#ifdef  ISI_SERVER_CREATEPERIODICMSG
    //
    // TO DO: add code for your implementation of this Micro Server-side
    // callback override here.
    //
    // When in doubt, return FALSE, indicating that your application
    // does not claim a slot from the ISI broadcast scheduler.
    //

#else
#ifdef  ISI_HOST_CREATEPERIODICMSG
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiCreatePeriodicMsg, 0, 0, NULL, 0);
#endif  //  ISI_HOST_CREATEPERIODICMSG
#endif  //  ISI_SERVER_CREATEPERIODICMSG
}   // IsiCreatePeriodicMsg
#pragma ignore_notused  IsiCreatePeriodicMsg
#endif  //  ISI_DEFAULT_CREATEPERIODICMSG



// ----------------------------------------------------------------
//  Callback: IsiUpdateUserInterface
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_UPDATEUSERINTERFACE
_RESIDENT void IsiUpdateUserInterface(IsiEvent event, unsigned parameter) {
#ifdef  ISI_SERVER_UPDATEUSERINTERFACE
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //



#else
#ifdef  ISI_HOST_UPDATEUSERINTERFACE
    // DO NOT MODIFY - This code redirects the callback to the host
    (void)IsiRpc(LicIsiUpdateUserInterface, (unsigned)event, parameter, NULL, 0);
#endif  //  ISI_HOST_UPDATEUSERINTERFACE
#endif  //  ISI_SERVER_UPDATEUSERINTERFACE
}   //  IsiUpdateUserInterface
#pragma ignore_notused  IsiUpdateUserInterface
#endif  //  ISI_DEFAULT_UPDATEUSERINTERFACE



// ----------------------------------------------------------------
//  Callback: IsiCreateCsmo
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_CREATECSMO
_RESIDENT void IsiCreateCsmo(unsigned assembly, IsiCsmoData* pCsmoData) {
#ifdef  ISI_SERVER_CREATECSMO
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //



#else
#ifdef  ISI_HOST_CREATECSMO
    // DO NOT MODIFY - This code redirects the callback to the host
    (void)IsiRpc(LicIsiCreateCsmo, assembly, 0, NULL, 0);
    memcpy(pCsmoData, isiRpcReturnData, sizeof(IsiCsmoData));
#endif  //  ISI_HOST_CREATECSMO
#endif  //  ISI_SERVER_CREATECSMO
}   //  IsiCreateCsmo
#pragma ignore_notused  IsiCreateCsmo
#endif  //  ISI_DEFAULT_CREATECSMO



// ----------------------------------------------------------------
//  Callback: IsiGetPrimaryGroup
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETPRIMARYGROUP
_RESIDENT unsigned IsiGetPrimaryGroup(unsigned assembly) {
#ifdef  ISI_SERVER_GETPRIMARYGROUP
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //
    // The standard group identifiers and usage category numbers
    // are defined in the ISI Protocol Specification.
    //



#else
#ifdef  ISI_HOST_GETPRIMARYGROUP
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetPrimaryGroup, assembly, 0, NULL, 0);
#endif  //  ISI_HOST_GETPRIMARYGROUP
#endif  //  ISI_SERVER_GETPRIMARYGROUP
}   //  IsiGetPrimaryGroup
#pragma ignore_notused  IsiGetPrimaryGroup
#endif  //  ISI_DEFAULT_GETPRIMARYGROUP



// ----------------------------------------------------------------
//  Callback: IsiGetAssembly
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETASSEMBLY
_RESIDENT unsigned IsiGetAssembly(const IsiCsmoData* pCsmoData, boolean automatic) {
#ifdef  ISI_SERVER_GETASSEMBLY
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //
    // If no assembly is applicable to this enrollment invitation, return
    // 0xFF (ISI_NO_ASSEMBLY)
    //



#else
#ifdef  ISI_HOST_GETASSEMBLY
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetAssembly, automatic, 0, pCsmoData, sizeof(IsiCsmoData));
#endif  //  ISI_HOST_GETASSEMBLY
#endif  //  ISI_SERVER_GETASSEMBLY
}   //  IsiGetAssembly
#pragma ignore_notused  IsiGetAssembly
#endif  //  ISI_DEFAULT_GETASSEMBLY



// ----------------------------------------------------------------
//  Callback: IsiGetNextAssembly
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETNEXTASSEMBLY
_RESIDENT unsigned IsiGetNextAssembly(const IsiCsmoData* pCsmoData, boolean automatic, unsigned assembly) {
#ifdef  ISI_SERVER_GETNEXTASSEMBLY
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //
    // If no further assembly is applicable to this enrollment invitation,
    // return 0xFF (ISI_NO_ASSEMBLY)
    //



#else
#ifdef  ISI_HOST_GETNEXTASSEMBLY
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetNextAssembly, automatic, assembly, pCsmoData, sizeof(IsiCsmoData));
#endif  //  ISI_HOST_GETNEXTASSEMBLY
#endif  //  ISI_SERVER_GETNEXTASSEMBLY
}   //  IsiGetNextAssembly
#pragma ignore_notused  IsiGetNextAssembly
#endif  //  ISI_DEFAULT_GETNEXTASSEMBLY



// ----------------------------------------------------------------
//  Callback: IsiGetNvIndex
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETNVINDEX
_RESIDENT unsigned IsiGetNvIndex(unsigned assembly, unsigned offset) {
#ifdef  ISI_SERVER_GETNVINDEX
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //
    // If no network variable is implemented for this offset within
    // the given local assembly, return 0xFF (ISI_NO_INDEX)



#else
#ifdef  ISI_HOST_GETNVINDEX
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetNvIndex, assembly, offset, NULL, 0);
#endif  //  ISI_HOST_GETNVINDEX
#endif  //  ISI_SERVER_GETNVINDEX
}   //  IsiGetNvIndex
#pragma ignore_notused  IsiGetNvIndex
#endif  //  ISI_DEFAULT_GETNVINDEX



// ----------------------------------------------------------------
//  Callback: IsiGetNextNvIndex
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETNEXTNVINDEX
_RESIDENT unsigned IsiGetNextNvIndex(unsigned assembly, unsigned offset, unsigned previous) {
#ifdef  ISI_SERVER_GETNEXTNVINDEX
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //
    // Note the IsiGetNextNvIndex callback is used to map more than one
    // local network variable to a single selector, and should be overridden
    // with great care, only.
    //
    // If no further network variable is implemented for this offset within
    // the given local assembly, return 0xFF (ISI_NO_INDEX).



#else
#ifdef  ISI_HOST_GETNEXTNVINDEX
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetNextNvIndex, assembly, offset, &previous, 1);
#endif  //  ISI_HOST_GETNEXTNVINDEX
#endif  //  ISI_SERVER_GETNEXTNVINDEX
}   //  IsiGetNextNvIndex
#pragma ignore_notused  IsiGetNextNvIndex
#endif  //  ISI_DEFAULT_GETNEXTNVINDEX



// ----------------------------------------------------------------
//  Callback: IsiGetPrimaryDid
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETPRIMARYDID
_RESIDENT const unsigned* IsiGetPrimaryDid(unsigned* pLength) {
#ifdef  ISI_SERVER_GETPRIMARYDID
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //



#else
#ifdef  ISI_HOST_GETPRIMARYDID
    // DO NOT MODIFY - This code redirects the callback to the host
    memset(isiRpcReturnData, 0, DOMAIN_ID_LENGTH);
    *pLength = IsiRpc(LicIsiGetPrimaryDid, 0, 0, NULL, 0);
    return (const unsigned *)isiRpcReturnData;
#endif  //  ISI_HOST_GETPRIMARYDID
#endif  //  ISI_SERVER_GETPRIMARYDID
}   //  IsiGetPrimaryDid
#pragma ignore_notused  IsiGetPrimaryDid
#endif  //  ISI_DEFAULT_GETPRIMARYDID



// ----------------------------------------------------------------
//  Callback: IsiGetWidth
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETWIDTH
_RESIDENT const unsigned IsiGetWidth(unsigned assembly) {
#ifdef  ISI_SERVER_GETWIDTH
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //



#else
#ifdef  ISI_HOST_GETWIDTH
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetWidth, assembly, 0, NULL, 0);
#endif  //  ISI_HOST_GETWIDTH
#endif  //  ISI_SERVER_GETWIDTH
}   //  IsiGetWidth
#pragma ignore_notused  IsiGetWidth
#endif  //  ISI_DEFAULT_GETWIDTH



// ----------------------------------------------------------------
// Callback: IsiGetConnectionTableSize, IsiGetConnection, IsiSetConnection
//
// A default, Micro Server-side implementation is provided for the ISI
// connection table related functions, allowing for easy adjustments of the
// ISI connection table size while implementing this table on the Micro Server
// for optimum performance.
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_CONNECTIONTABLE
#ifdef  ISI_SERVER_CONNECTIONTABLE

#ifndef MY_CONNECTION_TABLE_SIZE
#   define  MY_CONNECTION_TABLE_SIZE    32
#endif  // MY_CONNECTION_TABLE_SIZE

static eeprom far IsiConnection myConnectionTable[MY_CONNECTION_TABLE_SIZE];

_RESIDENT const unsigned IsiGetConnectionTableSize(void) {
    return MY_CONNECTION_TABLE_SIZE;
}   //  IsiGetConnectionTableSize

_RESIDENT const IsiConnection* IsiGetConnection(unsigned index) {
    return myConnectionTable + index;
}   //  IsiGetConnection

_RESIDENT void IsiSetConnection(const IsiConnection* pConnection, unsigned index) {
#pragma relaxed_casting_on
#pragma warnings_off
    eeprom_memcpy((IsiConnection*)(myConnectionTable + index), pConnection, (unsigned)sizeof(IsiConnection));
#pragma relaxed_casting_off
#pragma warnings_on
}   //  IsiSetConnection
#else
#ifdef  ISI_HOST_CONNECTIONTABLE
_RESIDENT const unsigned IsiGetConnectionTableSize(void) {
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetConnTabSize, 0, 0, NULL, 0);
}   //  IsiGetConnectionTableSize

_RESIDENT const IsiConnection* IsiGetConnection(unsigned index) {
    // DO NOT MODIFY - This code redirects the callback to the host
    (void)IsiRpc(LicIsiGetConnection, index, 0, NULL, 0);
    return (const IsiConnection*) isiRpcReturnData;
}   //  IsiGetConnection

_RESIDENT void IsiSetConnection(const IsiConnection* pConnection, unsigned index) {
    // DO NOT MODIFY - This code redirects the callback to the host
    (void)IsiRpc(LicIsiSetConnection, index, 0, pConnection, sizeof(IsiConnection));
}   //  IsiSetConnection
#endif  //  ISI_HOST_CONNECTIONTABLE
#endif  //  ISI_SERVER_CONNECTIONTABLE
#pragma ignore_notused  IsiGetConnectionTableSize
#pragma ignore_notused  IsiGetConnection
#pragma ignore_notused  IsiSetConnection
#endif  //  ISI_DEFAULT_CONNECTIONTABLE



// ----------------------------------------------------------------
//  Callback: IsiGetRepeatCount
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_GETREPEATCOUNT
_RESIDENT const unsigned IsiGetRepeatCount(void) {
#ifdef  ISI_SERVER_GETREPEATCOUNT
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //



#else
#ifdef  ISI_HOST_GETREPEATCOUNT
    // DO NOT MODIFY - This code redirects the callback to the host
    return IsiRpc(LicIsiGetRepeatCount, 0, 0, NULL, 0);
#endif  //  ISI_HOST_GETREPEATCOUNT
#endif  //  ISI_SERVER_GETREPEATCOUNT
}   //  IsiGetRepeatCount
#pragma ignore_notused  IsiGetRepeatCount
#endif  //  ISI_DEFAULT_GETREPEATCOUNT



// ----------------------------------------------------------------
//  Callback: IsiQueryHeartbeat
// ----------------------------------------------------------------
#ifndef ISI_DEFAULT_QUERYHEARTBEAT
_RESIDENT boolean IsiQueryHeartbeat(unsigned index) {
#ifdef  ISI_SERVER_QUERYHEARTBEAT
    //
    // TO DO: add code for your implementation of this
    // Micro Server-side callback override here.
    //



#else
#ifdef  ISI_HOST_QUERYHEARTBEAT
    // DO NOT MODIFY - This code redirects the callback to the host
    return (boolean)IsiRpc(LicIsiQueryHeartbeat, index, 0, NULL, 0);
#endif  //  ISI_HOST_QUERYHEARTBEAT
#endif  //  ISI_SERVER_QUERYHEARTBEAT
}   //  IsiQueryHeartbeat
#pragma ignore_notused  IsiQueryHeartbeat
#endif  //  ISI_DEFAULT_QUERYHEARTBEAT



// ----------------------------------------------------------------
//  Callback: IsiGetNvValue
//
// This callback _must_ be implemented on the host.
// ----------------------------------------------------------------
_RESIDENT const unsigned* IsiGetNvValue(unsigned index, unsigned* pLength) {
    // DO NOT MODIFY - This code redirects the callback to the host
    *pLength = IsiRpc(LicIsiGetNvValue, index, 0, NULL, 0);
    return (const unsigned*)isiRpcReturnData;
}   //  IsiGetNvValue
#pragma ignore_notused  IsiGetNvValue


