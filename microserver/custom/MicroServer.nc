// ---------------------------------------------------------------------
//
// File: MicroServer.nc
//
// Description:
// IzoT ShortStack MicroServer Neuron C Application.
// See MicroServer.h for configuration options, and link with a suitable
// IzoT ShortStack core library such as ShortStack430.lib or
// ShortStack430Isi.lib). See documentation for details.
//
// Copyright (c) 2001-2015 Echelon Corporation.  All rights reserved.
//
// This file is Example Software as defined in the Software
// License Agreement that governs its use.
//
// ECHELON MAKES NO REPRESENTATION, WARRANTY, OR CONDITION OF
// ANY KIND, EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE OR IN
// ANY COMMUNICATION WITH YOU, INCLUDING, BUT NOT LIMITED TO,
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY, SATISFACTORY
// QUALITY, FITNESS FOR ANY PARTICULAR PURPOSE,
// NONINFRINGEMENT, AND THEIR EQUIVALENTS.
//
// ---------------------------------------------------------------------

#ifdef SS_6050
#   pragma resident enabled
#endif
//
// The following directives must not be changed. While disabling code optimization
// only impacts performance and memory use, the micro_interface directive must be
// given when creating a ShortStack Micro Server.
//
#pragma micro_interface
#pragma optimization all

//
// Include the MicroServer.h header file that contains configuration details for
// this Micro Server.
//
#ifndef STD_MICRO_SERVER
//  Construct a regular custom Micro Server
#   include "MicroServer.h"
#else
//  Construct a standard Micro Server (Echelon only). Note the StdMicroServer.h
//  file is not supplied with the product; you should use MicroServer.h instead.
#   include "StdMicroServer.h"
#endif  // std_micro_server

#include <control.h>
#include <addrdefs.h>
#include <access.h>

//
// Support for ISI is optional, and can be selected in MicroServer.h. If selected,
// we need to include some ISI-related definitions. If you don't have the latest
// ISI implementation, or wish to learn more about the Interoperable Self-installation
// protocol, visit http://www.echelon.com/isi.
//
#ifdef SS_SUPPORT_ISI
    // The following macro prevents the ISI header file from adjusting the
    // buffer configuration - we've done that already in MicroServer.h
#   define BUFFER_SIZES_SET

    //
    // Include the standard ISI header file, and the ShortStack Micro Server-
    // specific file that controls the ISI callback overrides.
    //
#   include <isi.h>
#   include "MicroServerIsiHandlers.c"
#endif // SS_SUPPORT_ISI

// This directive sets the number of address table entries. Classic
// Neuron Chips and Smart Transceivers support up to 15 address table
// entries, and it is strongly recommended that you enable the maximum
// of 15 address table entries for those types of Micro Servers.
// However, the Series 6000 chips introduce support for an extended
// address table with up to 254 records (subject to available resources).
// For Micro Servers based on chips which support an extended address
// table, an address table smaller than the maximum size is generally
// recommended. Most devices will require much less than 254 address
// table entries, leaving a large proportion of the address table unused
// at all times. However, the entire address table is subject to frequent
// search operations. An oversized and largely unused address table can
// unnecessarily affect system performance, therefore.
#ifndef ADDRESS_TABLE_SIZE_SET
#	pragma num_addr_table_entries 15
#endif

// ---------------------------------------------------------------------
// The following section includes several directives that should not
// be changed.
// ---------------------------------------------------------------------

// See chapter 4 in the Neuron C Programmer's Guide for a detailed
// discussion of idempotent versus non-idempotent transactions.
// Do NOT edit this directive.
#pragma idempotent_duplicate_on

// This directive sets the number of receive transaction blocks.
#pragma receive_trans_count 16

// Explicit addressing must be enabled for the Micro Server.
// The ShortStack host application may enable or disable explicit
// addressing support at runtime, but the feature is always required
// when constructing a Micro Server.
// Do NOT edit this directive.
#pragma explicit_addressing_on

// Network variable processing must be on with a ShortStack Micro Server.
// Do NOT edit this directive.
#pragma netvar_processing_on

// This directive sets the number of domain table entries. One or two
// domain table entries are supported, but two domain table entries are
// typically required for interoperability. The ISI self-installation
// engine also requires that two domain table entries are provided.
#pragma num_domain_entries 2

// ---------------------------------------------------------------------
// The following section declares the I/O usage by the Micro Server
// DO NOT CHANGE ANY ASPECT OF THE I/O DECLARATION - the ShortStack library
// (e.g. ShortStack400.lib) relies on these declarations, and changing the
// following declarations is likely to result in a defunct Micro Server.
//
// Note that the I/O 1 and I/O 10 signals change direction subject to the
// chosen link layer mode, selected with the I/O 3 SPI/SCI~ signal. The
// definitions below define both I/O 1 and I/O 10 as input so that the
// Micro Server starts out with high impedance on these pins until, very
// soon into the MicroServer() routine, the ShortStack Micro Server
// samples the SPI/SCI~ signal and adjusts the direction of I/O 1 and 10
// accordingly. Prior to ShortStack 4.30, the I/O 1 and I/O 10 pines were
// declared outputs by default. [SS-55122, SS-59615]
// ---------------------------------------------------------------------
#ifndef SS_5000
#ifndef SS_6050
#   pragma enable_io_pullups
#endif  // SS_6050
#endif  // SS_5000

IO_0 output     bit CTS = 1;                    // _CTS (SCI); R/W_ (SPI)
IO_1 input      bit SLCK;                       // _HRDY (SCI); SCLK (SPI)
#pragma ignore_notused CTS
#pragma ignore_notused SLCK

#ifndef SS_COMPACT
IO_2 output     bit ss = 1;                     // NC (SCI); SS (SPI)
IO_3 input      bit interface_type;             // 0 for SCI and 1 for SPI.
#pragma ignore_notused ss
#pragma ignore_notused interface_type
#endif  // SS_COMPACT

IO_4 input      bit RTS;                        // RTS (SCI); TREQ (SPI)
#pragma ignore_notused RTS

#ifndef SS_COMPACT
IO_5 input      bit sbr0;                       // Serial baud rate LSB
IO_6 input      bit sbr1;                       // Serial baud rate
IO_7 output     bit MOSI;                       // NC (SCI); MOSI (SPI)
#pragma ignore_notused sbr0
#pragma ignore_notused sbr1
#pragma ignore_notused MOSI
#endif  //  SS_COMPACT

IO_8 input      serial fast_in;                 // RXD (SCI); MISO (SPI)
IO_9 input      bit rsvd;                       // Reserved
IO_10 output    serial fast_out;                // TXD (SCI); _HRDY (SPI)
IO_10 input     bit default_direction_10;
#pragma ignore_notused fast_in
#pragma ignore_notused rsvd
#pragma ignore_notused fast_out
#pragma ignore_notused default_direction_10

// ---------------------------------------------------------------------
// The following section contains the executable portion of the Micro
// Server application. Custom Micro Server applications may add additional
// initialization and processing prior to calling the MicroServer()
// function, if needed.
// ---------------------------------------------------------------------

// ShortStack Micro Server firmware core entry point:
extern void _RESIDENT MicroServerInit(void);
extern void _RESIDENT MicroServer(void);

// Declare the Service Pin timer.
// This is used to track how long the service pin has been held.
// The timer is served by ShortStack Micro Server firmware.
//
// *** servicePinTimer MUST BE THE FIRST TIMER IN ORDER OF TIMER DECLARATION ***
// *** You may declare other timers within a custom Micro Server if needed,  ***
// *** but make sure to declare any other timers AFTER THIS DECLARATION.     ***
//
stimer  servicePinTimer = 0;

//
// Declare the Micro Server Key as a global constant. This constant is used by
// the ShortStack library when constructing the uplink reset notification.
//
const unsigned long key = MICRO_SERVER_KEY;

#pragma ignore_notused servicePinTimer
#pragma ignore_notused key

// ---------------------------------------------------------------------
// when(reset)
// This task executes when the Micro Server resets. The task performs some
// mandatory initialization, then enters the Micro Server core by calling
// the MicroServer() function. This function never returns.
// ---------------------------------------------------------------------
when (reset)
{
#ifdef   SS_SUPPORT_ISI
    // Call IsiPreStart() before anything else. This call is vital for chips that
    // hold the core ISI processing engine in ROM (such as the PL 3170 Smart
    // Transceiver). On other chips, the call does nothing and returns immediately,
    // and is safe to make.
    IsiPreStart();

    // ISI devices are required to hesitate during reset processing. This allows
    // the network tool in a managed network to reliably take control over a self-
    // installed (ISI) device. The ISI documentation recommends an interval of
    // 800..1,500ms. We use 250ms here, because the duration of the Micro Server's
    // own reset processing can be added to this artificial hesitation.
    msec_delay(250u);
#endif

#ifndef SS_5000
#ifndef SS_6050
    // Perform pre-activation initialization steps. Note these steps are not required
    // with a Micro Server running on a series 5000 Neuron Chip or Smart Transceiver.
    MicroServerInit();
#endif 	// SS_6050
#endif  //  SS_5000

#ifndef SS_COMPACT
    // Activate SPI slave
    io_out(ss,0);
#endif  //  SS_COMPACT

    // ShortStack Micro Server firmware core entry point.
    // This call never returns!
    MicroServer();
}

//
// The following definition holds the version number of this Micro Server, as defined
// in MicroServer.h. Standard Micro Servers are built by Echelon and obtain the same
// information through an automated process
#ifndef STD_MICRO_SERVER
const unsigned appVersion[] = {
   SS_VERSION_MAJOR, SS_VERSION_MINOR, SS_VERSION_BUILD
};
#pragma ignore_notused appVersion
#endif //  STD_MICRO_SERVER


//
// Following are some ISI-related utilities.
// You do not normally need to edit this code, but you will need to review the preferences
// in ShortStackIsiHandlers.h, and possibly the code in MicroServerIsiHandlers.c.
// See ShortStackIsiHandlers.h for details.
//
// If the StortStack Micro Server core is enabled to support ISI (e.g. ShortStack400Isi.lib),
// the core will make calls to certain utilities, which are defined here.
// In addition to these utilities, a small number of standard ISI API functions are
// overridden for ShortStack-specific implementations in the following code.
//
#ifdef  SS_SUPPORT_ISI
    //
    // The Micro Server needs to keep track of the ISI type currently active.
    // The ShortStackIsiStart() function, defined below within this source file, initializes
    // this variable to the current type.
    IsiType far isiType;

    //
    // ISI needs a periodic timer at 250ms.
    // Do not change the definition of this timer.
    //
    mtimer repeating isiTick;

    //
    // In addition to the regular ISI timer, the ShortStack ISI implementation also
    // requires a timeout to monitor the remote procedure calls when invoking
    // ISI callbacks in the host application. This timer is defined here, and
    // accessed from the ShortStack Micro Server core through the IsiStartRpcTimeout
    // and IsiIsRpcTimeout functions, which are also implemented in this file.
    mtimer isiRpcTimeout;
#endif // SS_SUPPORT_ISI

#ifdef SS_SUPPORT_ISI
    //
    // Function: ShortStackIsiStart
    // ShortStack-specific superset of standard IsiStart function.
    //
    // The ShortStackIsiStart() API function is implemented here. When the host calls
    // the IsiStart API through the RPC indirection, the call activates the
    // ShortStackIsiStart function.
    //
    // This function is only provided with ISI-enabled Micro Servers.
    //
    _RESIDENT void ShortStackIsiStart(IsiType type, IsiFlags flags) {
        isiType = type;
        isiTick = 250u;

#   ifdef SS_SUPPORT_ISI_DAS
        if (type == isiTypeDas)
            IsiStartDas(flags);
        else
#   endif   //  SS_SUPPORT_ISI_DAS
#   ifdef SS_SUPPORT_ISI_DA
        if (type == isiTypeDa)
            IsiStartDa(flags);
        else
#   endif  //  SS_SUPPORT_DA
        if (type == isiTypeS)
            IsiStartS(flags);
    }
#   pragma ignore_notused  ShortStackIsiStart

    _RESIDENT void ShortStackIsiStop(void) {
        isiTick = 0;
        IsiStop();
    }
#   pragma ignore_notused ShortStackIsiStop

    // The IsiCancelAcquisition and IsiCancelAcquisitionDas functions use
    // the ShortStackIsiCancelAcquisition wrapper function, which makes the
    // correct calls into the ISI API according to the current ISI network
    // type.
    _RESIDENT void ShortStackIsiCancelAcquisition(void) {
#   ifdef  SS_SUPPORT_ISI_DAS
        if (isiType == isiTypeDas)
            IsiCancelAcquisitionDas();
        else
#   endif  // SS_SUPPORT_ISI_DAS
#   ifdef  SS_SUPPORT_ISI_DA
            IsiCancelAcquisition();
#   endif   // SS_SUPPORT_ISI_DA
    }
#   pragma ignore_notused ShortStackIsiCancelAcquisition

    // The IsiStaertDeviceAcquisition, IsiFetchDevice and IsiFetchDomain functions also
    // have a ShortStack-specific wrapper function, allowing for selective inclusion
    // (or exclusion) of support for ISI-DAS
#   ifndef SS_COMPACT
    _RESIDENT void ShortStackIsiStartDeviceAcquisition(void) {
#   ifdef  SS_SUPPORT_ISI_DAS
        IsiStartDeviceAcquisition();
#   endif   // SS_SUPPORT_ISI_DAS
    }
#   pragma ignore_notused ShortStackIsiStartDeviceAcquisition
#   endif  //  SS_COMPACT

#   ifndef SS_COMPACT
    _RESIDENT void ShortStackIsiFetchDevice(void) {
#   ifdef  SS_SUPPORT_ISI_DAS
        IsiFetchDevice();
#   endif   // SS_SUPPORT_ISI_DAS
    }
#   pragma ignore_notused ShortStackIsiFetchDevice
#   endif  //  SS_COMPACT

#   ifndef SS_COMPACT
    _RESIDENT void ShortStackIsiFetchDomain(void) {
#   ifdef  SS_SUPPORT_ISI_DAS
        IsiFetchDomain();
#   endif   // SS_SUPPORT_ISI_DAS
    }
#   pragma ignore_notused ShortStackIsiFetchDomain
#   endif  // SS_COMPACT

    //
    // IsiBackground is a utility function called by the Micro Server core.
    // For an ISI-aware Micro Server, the IsiBackground utility serves the
    // periodic ISI API (IsiTick). Similar to the IsiStart implementation
    // above, this implementation takes advantage of conditional compilation
    // for minimum memory footprint.
    //
    _RESIDENT void IsiBackground(void) {
        if (timer_expires(isiTick)) {
#   ifdef  SS_SUPPORT_ISI_DAS
            if (isiType == isiTypeDas)
                IsiTickDas();
            else
#   endif  // SS_SUPPORT_ISI_DAS
#   ifdef  SS_SUPPORT_ISI_DA
            if (isiType == isiTypeDa)
                IsiTickDa();
            else
#   endif  // SS_SUPPORT_ISI_DA
                IsiTickS();
        }
    }
#   pragma ignore_notused IsiBackground

    //
    // IsiMsgHandler is a utility function used by the ShortStack Micro Server
    // core to identify and process ISI messages.
    // This function returns true if the message was handled by this function.
    //
    // When in preemption mode, the message handler drops all incoming ISI messages.
    // Within the Micro Server, we cannot handle incoming ISI messages when in
    // preemption mode. Preemption mode is transient and ISI messages are all repeated,
    // so we can afford losing one. Will process the next one.
    //
    // 2.11:    The ShortStack core library calls the IsiMsgHandler dispatcher with
    //          the preemptionMode argument; explicit calls to shortStackInPreempt() are
    //          no longer required.
    //
#   ifdef SS_CONTROLLED_ENROLLMENT
        extern _RESIDENT boolean isiControlledEnrollment(void);   // provided with ShortStack core library
#       define PROCESS_DAS   (preemptionMode || isiControlledEnrollment() || !IsiProcessMsgDas())
#       define PROCESS_DA    (preemptionMode || isiControlledEnrollment() || !IsiProcessMsgDa())
#       define PROCESS_S     (preemptionMode || isiControlledEnrollment() || !IsiProcessMsgS())
#   else
#       define PROCESS_DAS   (preemptionMode || !IsiProcessMsgDas())
#       define PROCESS_DA    (preemptionMode || !IsiProcessMsgDa())
#       define PROCESS_S     (preemptionMode || !IsiProcessMsgS())
#   endif  // SS_CONTROLLED_ENROLLMENT

    _RESIDENT boolean IsiMsgHandler(boolean preemptionMode)
    {
#   ifdef SS_SUPPORT_ISI_DAS
        if (isiType == isiTypeDas)
            return IsiApproveMsgDas() && PROCESS_DAS;
        else
#   endif  //  SS_SUPPORT_ISI_DAS
#   ifdef SS_SUPPORT_ISI_DA
        if (isiType == isiTypeDa)
            return IsiApproveMsg() && PROCESS_DA;
#   endif   // SS_SUPPORT_ISI_DA
        return IsiApproveMsg() && PROCESS_S;
    }
#   pragma ignore_notused IsiMsgHandler

#   ifndef SS_COMPACT
    //
    // IsiRespHandler is a utility function called by the ShortStack Micro Server
    // core. It is the equivalent of a Neuron C when(resp_arrives) task, and returns
    // TRUE if the response has been processed by this handler. Returning FALSe from
    // IsiRespHandler will pass the response to the host.
    // The Micro Server firmware core will only call this function if ISI support is
    // enabled, and if the ISI engine is running. It will call this function for all
    // ISI modes, but the handler does nothing unless the active mode is ISI-DAS.
    // You do not normally need to edit this function.
    _RESIDENT boolean IsiRespHandler(void)
    {
#   ifdef  SS_SUPPORT_ISI_DAS
        return isiType == isiTypeDas && !IsiProcessResponse();
#   else
        return FALSE;
#   endif  // SS_SUPPORT_ISI_DAS
    }
    #pragma ignore_notused IsiRespHandler
#   endif  //  SS_COMPACT

    //
    // IsiStartRpcTimeout is called from the ShortStack Micro Server core
    // to start a timeout guard when waiting for the host application to
    // reply to a remote procedure call (a callback invocation on the host).
    // The function is not defined unless ISI support has been enabled for
    // this Micro Server.
    _RESIDENT void IsiStartRpcTimeout(unsigned long value) {
        isiRpcTimeout = value;
    }
#   pragma ignore_notused IsiStartRpcTimeout

    //
    // IsiIsRpcTimeout is called from the ShortStack Micro Server core
    // to test for expiry of the a remote procedure call timeout guard.
    // The function is not defined unless ISI support has been enabled for
    // this Micro Server.
    _RESIDENT boolean IsiIsRpcTimeout(void) {
        return timer_expires(isiRpcTimeout);
    }
#   pragma ignore_notused IsiIsRpcTimeout

    //
    // isiBufferAlloc is the routine in control of ISI message buffer allocation.
    // The ShortStack Micro Server provides its own implementation of this
    // routine in order to correctly implement preemption mode for ShortStack.
    // This routine is not necessary with PL 3170-based (firmware 17) Micro Servers.
    //
    // DO NOT CHANGE OR REMOVE THIS FUNCTION.
    //
#ifndef SS_COMPACT
    void _RESIDENT isiBufferAlloc(unsigned service) {
        extern _RESIDENT void shortStackBufferAlloc(unsigned service);
        shortStackBufferAlloc(service);
    }
#   pragma ignore_notused isiBufferAlloc
#endif  //      SS_COMPACT

#endif  //  SS_SUPPORT_ISI


