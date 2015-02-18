//
// MicroServer.h
//
// Description:
// This files defines preferences used and features included,
// or excluded, with a custom ShortStack Micro Server firmware.
// This file is included with the MicroServer.nc file.
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

#ifndef    MICRO_SERVER_H
#define    MICRO_SERVER_H

// The following section contains various preferences that can be set for
// each Micro Server. The default values used here (and listed in source
// code comments) equal those used to construct the FT 3120 @ 20 MHz
// standard Micro Server.
// ISI-enabled Micro Servers require at least two application output
// buffers; see SS_SUPPORT_ISI, below, for control about ISI support
// with this Micro Server.

// These directives control the application/network buffer counts.
#pragma app_buf_out_count   2           // default for FT3120@20MHz: 2
#pragma app_buf_out_priority_count 1    // default for FT3120@20MHz: 1
#pragma app_buf_in_count    5           // default for FT3120@20MHz: 5
#pragma net_buf_out_count   2           // default for FT3120@20MHz: 2
#pragma net_buf_out_priority_count 1    // default for FT3120@20MHz: 1
#pragma net_buf_in_count    2           // default for FT3120@20MHz: 2

// The following directives control the size of the application and
// network buffers.
//
// A minimum application buffer size of 50 bytes and a minimum network
// buffer size of 66 bytes is recommended for most interoperable
// applications.
// ISI-enabled Micro Servers require a minimum buffer size of 66 bytes;
// see SS_SUPPORT_ISI, below, for control about ISI support with this
// Micro Server.
//
#pragma app_buf_out_size    66          // default for FT3120@20MHz: 66
#pragma app_buf_in_size     66          // default for FT3120@20MHz: 66
#pragma net_buf_out_size    66          // default for FT3120@20MHz: 66
#pragma net_buf_in_size     66          // default for FT3120@20MHz: 66

// Use the set_netvar_count directive to control the maximum number of
// network variables supported by your Micro Server; the actual number
// of network variables (less or equal to this maximum) is a function
// of the host application in use, and will be adjusted during initialization
// at runtime.
// Valid values range from 0 to 62 when using system firmware prior to
// version 16, and range from 0 to 254 with version 16 firmware (or better),
// available hardware resources permitting.
#pragma set_netvar_count    254         // default for FT3120@20MHz: 254

// This directive controls the number of alias table entries. Valid values
// range from 0 to 62 when using system firmware prior to version 16, and
// range from 0 to 127 with version 16 firmware (or better).
// Note that a large alias table may decrease the device's overall
// performance in busy network traffic conditions.
#pragma num_alias_table_entries 127     // default for FT3120@20MHz: 127

// Define the Micro Server key, an unsigned 16-bit value. For a custom Micro
// Server, raise the most signifficant bit (0x8000ul), and allocate the
// key value according to your application needs. Standard Micro Server
// keys have the MSB cleared, and follow the following interpretation
// in pseudo-code (in big endian ordering, but ignoring byte boundaries)
//
// typedef struct {
//    unsigned custom  : 1;   // always 0 for standard Micro Server
//    unsigned version : 4;   //
//    unsigned chip    : 4;   // 0:ft3120, 1:ft3150, 2:pl3120, 3:pl3150 4:pl3170
//                            // 5:ft5000, 6:neuron5000, 7:series 6000 chip
//    unsigned clock   : 3;   // 0:5Mhz, 1:10MHz, 2:20MHz, etc
//    unsigned isi     : 1;   // 1:ISI supported, 0 else
//    unsigned xcvr    : 3;   // 0:tp/ft, 1:pl-20C/N. Use 7 for any other transceiver
// } std_micro_server_key;
//
// Custom micro servers are free to choose a different scheme, but must
// raise the MSB. The Micro Server firmware core makes no assumption about this
// value, but forwards it to your host application, which may use this value to
// identify the Micro Server and its capabilities.
#define  MICRO_SERVER_KEY    0x8000ul   // default for custom firmware: 0x8000ul

// Define values for the three symbols SS_VERSION_MAJOR, SS_VERSION_MINOR and
// SS_VERSION_BUILD to reflect this Micro Server's current version.
// Each of the three numbers ranges 0..255. Note this is the Micro Server version;
// the version of the Micro Server's core library (e.g. ShortStack400.lib) is
// reported through its own version number triplet.
// To read the version numbers at runtime, the utility functions must be enabled when
// creating the application framework for the host application. The host application
// can then use the LonQueryVersion() API and LonVersionReceived() callback
// to determine the version for both the core library and this application.
// Note versioning is supported as part of the utility functions set, which may not
// be available with all Micro Servers and core libraries.
#define SS_VERSION_MAJOR    4
#define SS_VERSION_MINOR    30
#define SS_VERSION_BUILD    0

// Enable Direct Memory Files. To disable the feature, outcomment the following
// directive. You must do so if your Micro Server uses version 15 system firmware,
// or an earlier version. If you use version 16 (or better) system firmware, it
// is recommended that you leave DMF enabled, as the API and application framework
// code automatically handle all related requests.
#pragma enable_dmf                      // default for FT3120@20MHz: enabled

// Define the SS_SUPPORT_ISI macro to include ISI support with the
// Micro Server, or comment out the definition of this macro. When supporting ISI,
// you must also configure the following four macros, and you must link the Micro
// Server Neuron C application with an ISI-enabled version of the ShortStack core
// firmware library, and a suitable ISI library. Creating Micro Server firmware that
// supports ISI typically requires an FT 3150 or PL 3150-based Micro Server.
//#define SS_SUPPORT_ISI        1       // default for FT3120@20MHz: disabled

// The following two macros define the types of ISI networks this Micro Server firmware
// supports, in addition to the minimum ISI requirement. The minimum ISI requirement is
// ISI-S; optional versions are ISI-DA, and ISI-DAS. It is recommended to support ISI-DA.
// Due to its higher demands in processing power, buffers and application data storage
// (off-chip RAM), supporting ISI-DAS should be enabled only if required.
// If you do support ISI-DAS, however, you must also support ISI-DA. Supporting ISI-DA
// requires support for ISI-S.
// This setting is effective only if SS_SUPPORT_ISI is defined.

//#   define SS_SUPPORT_ISI_S     1
//#   define SS_SUPPORT_ISI_DA    1     // default for FT3120@20MHz: disabled
//#   define SS_SUPPORT_ISI_DAS   0     // default for FT3120@20MHz: disabled

// Define SS_CONTROLLED_ENROLLMENT to enable built-in support for ISI controlled enrollment
// procedures. Controlled enrollment is an optional part of the ISI protocol. You may chose
// to exclude support for ISI controlled enrollment for a more compact implementation of your
// custom Micro Server.
// This setting is effective only if SS_SUPPORT_ISI is defined.
#define SS_CONTROLLED_ENROLLMENT    1

// The following three symbols help to describe your custom Micro Server's hardware:
// SS_ISI_IN_SYSTEM_IMAGE, SS_COMPACT, SS_5000 and SS_6050.
//
// Define SS_ISI_IN_SYSTEM_IMAGE if your Micro Server firmware supports ISI as part
// of the Smart Transceiver's system image, as is the case with the PL 3170 Smart
// Transceiver. This macro is used to suppress certain functions in MicroServer.nc.
// This macro is independent from the SS_SUPPORT_ISI macro, and is relevant even if
// ISI support is not configured.
//
// Define SS_COMPACT when creating a compact Micro Server. A compact Micro Server
// uses the MicroServer core from ShortStack211CptIsi.lib, which is feature-restricted
// as discussed in the product documentation. This feature is normally only used with
// the PL 3170 Smart Transceiver.
//
// Define SS_5000 when creating a Micro Server that runs on a series 5000 Neuron Chip
// or Smart Transceiver.
// Define SS_6050 when creating a Micro Server for a Series 6000 Neuron Chip or Smart
// Transceiver.

//#define SS_ISI_IN_SYSTEM_IMAGE 0      // default for FT3120@20MHz: disabled
//#define SS_COMPACT             0      // default for FT3120@20MHz: disabled
//#define SS_5000                0      // default for FT3120@20MHz: disabled
//#define SS_6050                0      // default for FT3120@20MHz: disabled

// Following a a few validations and fix-ups of selections made above. None of the following
// code should need editing.

// ISI-DAS builds on ISI-DA, and ISI-DA builds on ISI-S
#ifdef SS_SUPPORT_ISI
#   ifdef SS_SUPPORT_ISI_DAS
#       ifndef SS_SUPPORT_ISI_DA
#           define SS_SUPPORT_ISI_DA
#           warning "ISI-DAS requires ISI-DA, which has been automatically enabled"
#       endif  // SS_SUPPORT_DA
#   endif  // SS_SUPPORT_DAS
#   ifdef SS_SUPPORT_ISI_DA
#       ifndef SS_SUPPORT_ISI_S
#           define SS_SUPPORT_ISI_S
#           warning "ISI-DA requires ISI-S, which has been automatically enabled"
#       endif  // SS_SUPPORT_S
#   endif  // SS_SUPPORT_DA
#endif  // SS_SUPPORT_ISI

#endif  // MICRO_SERVER_H
