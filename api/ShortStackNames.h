/*
 * ShortStackNames.h
 *
 * This file contains definitions that map names for global types, functions,
 * and preprocessor macros as used with ShortStack 2.10 to the names used with
 * earlier versions of the product. This file may be used as a cross-reference,
 * and as a migration aid when transferring an application to the ShortStack 2.10
 * framework and API.
 * You should use this file during the migration process only. Future versions of
 * ShortStack may not include updates to this file.
 *
 * Note this file provides backwards-compatible names for global types, functions,
 * and macros. Backwards-compatible names for members of structures, unions, or
 * enumerations are not provided with this file. Also note that the legacy names
 * are not generally unique within the namespace of your application.
 *
 * IzoT ShortStack applications should not depend on these name translations.
 * The ShortStackNames.h file and the name translations provided therein are
 * deprecated and may be discontinued with a future release of ShortStack.
 *
 * Copyright (c) 2007-2015 Echelon Corporation.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#ifndef LONNAMES_H
#define LONNAMES_H

/*
 *  from lonaccss.h:
 */
#define AUTH_KEY_LEN             LON_AUTHENTICATION_KEY_LEN
#define DOMAIN_ID_LEN            LON_DOMAIN_ID_LENGTH
#define ID_STR_LEN               LON_PROGRAM_ID_LENGTH
#define LOCATION_LEN             LON_LOCATION_LENGTH
#define NEURON_ID_LEN            LON_NEURON_ID_LENGTH
#define NUM_COMM_PARAMS          LON_NUM_COMM_PARAMS

#define nid_struct               LonNeuronId

#ifdef  SS_API_EXT
#   define domain_struct         LonDomain
#   define direct_param_struct   LonDirectModeTransceiver
#   define xcvr_status_struct    LonTransceiverStatus
#   define config_data_struct    LonConfigData
#   define status_struct         LonDeviceStatus
#endif   // ss_api_ext

/*
 *  from lonstate.h:
 */
#define nm_node_state            LonNodeState

/*
 *  from lonerr.h:
 */
#define LonNmError              LonSystemError
#define LonApiErrorCode         LonApiError
/*
 * from lonmodel.h:
 */
#define  nm_model_code           LonNeuronModel

#endif  // lonnames_h 
