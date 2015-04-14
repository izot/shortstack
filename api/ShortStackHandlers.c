/*
 * Filename: ShortStackHandlers.c
 *
 * Description: This file contains callback handler functions that must be
 * implemented by the application.
 *
 * The IzoT Interface Interpreter always generates callback functions for
 * the most commonly used events. The IzoT Markup Language provides build-in
 * support for those, and supports dispatching into user-defined event
 * handlers. Those callback functions are excluded from this file using
 * conditional compilation based upon the LON_FRAMEWORK_TYPE_III symbol.
 *
 * For all else, you must implement the required callback handler either
 * in this file, or by defining a compatible handler elsewhere, and declaring
 * a matching xxx_HANDLED symbol in your project.
 *
 * For example, you can implement the functionality of the LonMsgArrived
 * callback function in this file (an empty body of this callback function
 * is defined below). Alternatively, you can implement the LonMsgArrived
 * callback, using the same name and the correct prototype, elsewhere in
 * your code and declare this fact by defining the LONMSGARRIVED_HANDLED
 * symbol in your project.
 *
 * This symbol must be defined at the time of compiling ShortStackHandlers.c
 * The symbol does not require a value.
 *
 * Callbacks which are governed by the IzoT Interface Interpreter cannot
 * be overridden in this manner.
 *
 * Copyright (c) Echelon Corporation 2002-2015.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */
#include "ShortStackDev.h"
#include "ShortStackApi.h"

#if LON_DMF_ENABLED
#include "string.h"         /* Required for memcpy */
#endif /* LON_DMF_ENABLED */

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * Framework functions
 */
extern void* LonGetNvTable();
extern LonUbits32 LonGetSignature();
extern unsigned LonGetNvCount();

/*
 * This example implementation uses the standard open(), close(), read() and
 * write() API for simple file I/O to maintain values of persistent network
 * variables. This is implemented with the LonNvdDeserializeNvs() and
 * LonNvdSerializeNvs() API, defined below in this module.
 * This example implementation is configured with two preprocessor symbols.
 * The LON_NVD_FILEIO macro is either defined or undefined. When defined,
 * the example implementation of the file system based value storage is
 * enabled.
 * The LON_NVD_FILEIO macro defines the name of the file to be used.
 *
 * The recommended Eclipse configuration defines the LON_NVD_FILEIO symbol
 * with the project settings, allowing the sharing of this source code
 * across several applications.
 */
/* #define LON_NVD_FILEIO "myApp.nvd" */
#if defined(LON_NVD_FILEIO)
/* Add a stringification macro definition */
#	define STRINGIFY(name)	STRING(name)
#	define STRING(name)		#name
#	define LON_NVD_FILENAME	STRINGIFY(LON_NVD_FILEIO)
#endif

/*
 * Callback:   LonResetOccurred
 * Occurs when the Micro Server has completed a reset.
 *
 * Parameters:
 * pResetNotification - <LonResetNotification> structure with capabilities and
 * identifying data.
 *
 * Remarks:
 * The pointer to <LonResetNotification> is only valid for the duration of
 * this callback, but the API buffers the notification details. Use the
 * <LonGetLastResetNotification> function to fetch a pointer to that buffer when
 * needed.
 *
 * The reset message contains the link-layer protocol version number implemented
 * by the Micro Server. The application should make sure that it can handle that
 * link-layer protocol by comparing it with the API's link-layer protocol
 * version number defined by LON_LINK_LAYER_PROTOCOL_VERSION.
 *
 * The serial Link Layer driver and the ShortStack API reset and resyncronize
 * the link layer in the event of an unexpected Micro Server reset before this
 * callback is invoked.
 *
 * The ShortStack API also filters the LON_RESET_INITIALIZED attribute and
 * automatically (re-)initializes the Micro Server when necessary. As a result,
 * this handler does only executes with the LON_RESET_INITIALIZED attribute
 * set (the Micro Server is initialized).
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonResetOccurred(const LonResetNotification* const pResetNotification)
{
	/*
	 * TO DO: implement application-specific reset processing.
	 */
}
#endif  /* LON_FRAMEWORK_TYPE_III */

/*
 * Callback: LonWink
 * Occurs when the Micro Server has received a WINK command.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonWink(void)
{
	/*
	 * TO DO
	 */
}
#endif  /*  LON_FRAMEWORK_TYPE_III */

/*
 * Callback: LonOffline
 * Occurs when the Micro Server has received a command to enter the offline state.
 *
 * Remarks:
 * The Micro Server does not actually go offline until the <LonOffline> callback
 * function returns and the ShortStack LonTalk Compact API sends a confirmation
 * message to the Micro Server. Thus, the host application should assume that
 * the Micro Server is offline only after this callback completes processing.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonOffline(void)
{
	/*
	 * TO DO
	 */
}
#endif  /* LON_FRAMEWORK_TYPE_III */

/*
 * Callback: LonOnline
 * Occurs when the Micro Server has received a command to enter the online state.
 *
 * Remarks:
 * The Micro Server does not actually go online until the <LonOnline> callback
 * function returns and the ShortStack LonTalk Compact API sends a confirmation
 * message to the Micro Server. Thus, the host application should assume that
 * the Micro Server is online only after this callback completes processing.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonOnline(void)
{
	/*
	 * TO DO
	 */
}
#endif  /* LON_FRAMEWORK_TYPE_III */

/*
 * Callback: LonServicePinPressed
 * Occurs when the service pin has been activated.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonServicePinPressed(void)
{
	/*
	 * TO DO
	 */
}
#endif  /* LON_FRAMEWORK_TYPE_III */

/*
 * Callback: LonServicePinHeld
 * Occurs when the service pin has been continuously activated for a
 * configurable time.
 *
 * Remarks:
 * Use the LonTalk Interface Developer to enable this feature and to specify
 * the duration for which the service pin must be activated to trigger this
 * callback.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonServicePinHeld(void)
{
	/*
	 * TO DO
	 */
}
#endif  /* LON_FRAMEWORK_TYPE_III */

/*
 * Callback: LonNvUpdateOccurred
 * Occurs when new input network variable data has arrived.
 *
 * Parameters:
 * index - global index (local to the device) of the network variable
 * pSourceaddress - pointer to source address description
 *
 * Remarks:
 * The network variable with local index given in this callback has been
 * updated with a new value. The new value is already stored in the network
 * variable's location; access the value through the global variable
 * representing the network variable, or obtain the pointer to the network
 * variable's value from the network variable table.
 * The pSourceAddress pointer is only valid for the duration of this callback.
 *
 * For an element of a network variable array, the index is the global network
 * variable index plus the array-element index. For example, if nviVolt[0] has
 * global network variable index 4, then nviVolt[1] has global network variable
 * index 5.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonNvUpdateOccurred(
		const unsigned index,
		const LonReceiveAddress* const pSourceAddress
)
{
}
#endif  /*  LON_FRAMEWORK_TYPE_III */

/*
 * Callback:   LonNvUpdateCompleted
 * Signals completion of a network variable update
 *
 * Parameters:
 * index - global index (local to the device) of the network variable
 * success - indicates whether the update was successful or unsuccessful
 *
 * Remarks:
 * This callback completes the transaction that was started by calling the
 * <LonPropagateNv> or <LonPollNv> API functions.  The index parameter
 * delivered with this callback matches the one from the API invocation.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
void LonNvUpdateCompleted(const unsigned index, const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif  /* LON_FRAMEWORK_TYPE_III */

/*
 * Callback: LonGetCurrentNvSize
 * Gets the current size of a network variable.
 *
 * Parameters:
 * index - the local index of the network variable
 *
 * Returns:    Current size of the network variable.
 *             Zero if the network variable index doesn't exist.
 *
 * Remarks:
 * If the network variable size is fixed, this function should return
 * <LonGetDeclaredNvSize>. If the network variable size is changeable, the
 * current size should be returned. The default implementation for changeable
 * type network variables returns 0, and must be updated by the application
 * developer.
 *
 * The Micro Server will not propagate a network variable with size 0, nor will
 * it generate an update event if a network variable update is received from the
 * network when the current network variable size is 0.
 *
 * Note that even though this is a callback function, it *is* legal for the
 * application to call <LonGetDeclaredNvSize> from this callback.
 *
 * For applications based on the IzoT Interface Interpreter (III), this
 * callback is implemented in the III-generated ShortStackDev.c, from where
 * the associated application-specific event handlers are dispatched.
 */
#ifndef LON_FRAMEWORK_TYPE_III
const unsigned LonGetCurrentNvSize(const unsigned nvIndex)
{
	unsigned size = 0;
	const LonNvDescription* const pNvTable =
	(const LonNvDescription* const )LonGetNvTable();

	if (LON_GET_ATTRIBUTE(pNvTable[nvIndex], LON_NVDESC_CHANGEABLE)) {
		/*
		 * TO DO: Supply the current size of this network variable.
		 * The current size can typically be found in the associated
		 * SCPTnvType configuration property.
		 * Note the NvTable holds the network variable's declared
		 * size, which equals its maximum size, but not necessarily
		 * its current size.
		 */

	} else {
		/*
		 * For not changeable network variables, the declared
		 * size held in the table also equals the current size.
		 */
		size = LonGetDeclaredNvSize(nvIndex);
	}

	return size;
}
#endif  /*  LON_FRAMEWORK_TYPE_III */

#if !defined(LONNVDSERIALIZENVS_HANDLED) || !defined(LONNVDDESERIALIZENVS_HANDLED)
/*
 * Function: TransactionControl
 * Updates the transaction control file with a new value and
 * returns the previous value.
 *
 * Parameters:
 * value - new transaction control value or -2 (do not write)
 *
 * Returns:
 * previous transaction control value or -2 (none found)
 *
 * Remarks:
 * The LonNvdSerializeNvs function sets the transaction control value to 1 (one)
 * before beginning to write data to persistent data storage. When finished, the
 * function sets the transaction control value to 0 (zero). This indicates that
 * the serialization of persistent items to persistent data storage has been
 * completed.
 * The LonNvdDeserializeNvs function reads the transaction control value prior
 * to reading data from the persistent data storage. When the transaction control
 * value is anything but 0 (zero), persistent data is considered invalid.
 * This would be the case if a power cycle or physical reset occurs before the
 * writing of persistent data completes.
 *
 * The transaction control data fetch-and-write operation should be atomic, safe
 * and fast, but for targets which do not support suitable facilities (such as
 * on-chip EEPROM memory with dedicated machine instructions), a small file on
 * the file system can be used.
 *
 */
typedef signed TransactionControlType;
#define TX_CONTROL_NIL	(-2)	/* don't change transaction control value */
#define TX_CONTROL_NONE (-1)	/* no transaction control file */
#define TX_CONTROL_IDLE (0)		/* transaction is idle */
#define TX_CONTROL_BUSY (1)		/* transaction in progress */
#define LON_NVD_TXNAME	STRINGIFY(LON_NVD_FILEIO.tx)

static TransactionControlType TransactionControl(TransactionControlType value)
{
	TransactionControlType result = TX_CONTROL_NIL;
	int fd = open(
		LON_NVD_TXNAME,
		// ###TODO: Consider including O_DIRECT if your target supports it.
		// O_DIRECT minimizes the effects of a file I/O cache.
        O_RDWR | O_CREAT, S_IRUSR | S_IWUSR
    );

	if (fd != -1) {
		if (read(fd, &result, sizeof(result)) != sizeof(result)) {
			result = TX_CONTROL_NIL;
		}
		if (value != TX_CONTROL_NIL) {
			lseek(fd, 0, SEEK_SET);
			write(fd, &value, sizeof(value));
			close(fd);
		}
	} else {
		result = TX_CONTROL_NONE;
	}

	return result;
}
#endif

/*
 * Callback: LonNvdSerializeNvs
 * Updates the persistent network variable values in non-volatile storage.
 *
 * Remarks:
 * This callback is called by the API whenever the device receives a new value
 * for a persistent network variable from the network. The event is not fired
 * when your application makes local assignments, such as assignments to
 * properties flagged with the device_specific and manufacture attributes.
 *
 * See <LonNvdDeserializeNvs> for the complimentary API.
 *
 * Declare LONNVDSERIALIZENVS_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONNVDSERIALIZENVS_HANDLED
const LonApiError LonNvdSerializeNvs(void) {
#if LON_PERSISTENT_NVS
#ifdef LON_NVD_FILEIO

	LonApiError result = LonApiNoError;
	TransactionControl(TX_CONTROL_BUSY);
	{
		int fd = open(
			LON_NVD_FILENAME,
			O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR
		);

		if (fd == -1) {
			result = LonApiNvdFileError;
		} else {
			int index = 0;
			const LonNvDescription* pNvDescription =
					(const LonNvDescription*) LonGetNvTable();

			/*
			 * Write the application signature first. This is read and
			 * validated by LonNvdDeserializeNvd to ensure that the data
			 * kept on file matches the exact set of this interface.
			 */
			const LonUbits32 signature = LonGetSignature();
			const unsigned nvCount = LonGetNvCount();

			if (write(fd, &signature, sizeof(signature)) != sizeof(signature)) {
				result = LonApiNvdFailure;
			}

			while (index < nvCount && result == LonApiNoError) {
				if (pNvDescription->Attributes & LON_NVDESC_PERSISTENT_MASK) {
					unsigned length = LonGetCurrentNvSize(index);

					if (length && length != (unsigned) -1) {
						if (write(fd, (void*) pNvDescription->pData, length) != length) {
							result = LonApiNvdFailure;
						}
					} else {
						result = LonApiNvdSizeNotSupported;
					}
				}
				++index;
				++pNvDescription;
			}

			if (result == LonApiNoError) {
				/* Write the modifiable value file, if any. */
				unsigned size = 0;
				void* file_data = LonGetFile(LON_DMF_FILEINDEX_MODIFIABLE, &size);

				if (file_data) {
					if (write(fd, file_data, size) != size) {
						result = LonApiNvdFailure;
					}
				}
			}

			if (result == LonApiNoError) {
				/* Write the read-only value file, if any.
				 * Note the "read only value file" is "read only" from the network
				 * tool's standpoint of view. While this file often contains
				 * constant data such as version numbers, device-specific properties
				 * may also exist within the read-only file, and may be written by
				 * the application under appropriate conditions.
				 */
				unsigned size = 0;
				void* file_data = LonGetFile(LON_DMF_FILEINDEX_READONLY, &size);

				if (file_data) {
					if (write(fd, file_data, size) != size) {
						result = LonApiNvdFailure;
					}
				}
			}

			/* Let's try to close the file even if an error occurred, but
			 * preserve the original error if the close call also fails.
			 */
			fsync(fd);
			if (close(fd) == -1) {
				if (result == LonApiNoError) {
					result = LonApiNvdFailure;
				}
			}
		}
	}
	TransactionControl(TX_CONTROL_IDLE);
#else
	LonApiError result = LonApiInitializationFailure;
#endif  /* LON_NVD_FILEIO */

	return result;
#else
	/*
	 * This application implements no eeprom network variables or
	 * configuration network variables.
	 * No change to this code is needed
	 */
	return LonApiNoError;
#endif  /* LON_PERSISTENT_NVS */
}
#endif /* LONNVDSERIALIZENVS_HANDLED */

/*
 * Callback: LonNvdDeserializeNvs
 * Initializes the network variable values from non-volatile storage.
 *
 * Remarks:
 * This callback is called by the API's initialization routine <LonInit>.
 * If the device contains any network variable that requires persistent storage,
 * such as an eeprom network variable or a configuration property network variable
 * (CPNV), the application should provide a mechanism to store and retrieve its
 * data from persistent memory.
 *
 * Declare LONNVDDESERIALIZENVS_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONNVDDESERIALIZENVS_HANDLED
const LonApiError LonNvdDeserializeNvs(void) {
#if LON_PERSISTENT_NVS
#if defined(LON_NVD_FILEIO)
	LonApiError result = LonApiNoError;
	TransactionControlType txControl = TransactionControl(TX_CONTROL_NIL);

	if (txControl == TX_CONTROL_BUSY) {
		result = LonApiNvdFailure;
	} else if (txControl != TX_CONTROL_NONE) {
		int fd = open(LON_NVD_FILENAME, O_RDONLY);

		if (fd == -1) {
			if (errno != ENOENT) {
				/*
				 * Failure to open the NVD file is OK in case of ENOENT (the file
				 * does not exist). Other open failure are a genuine error for
				 * this application.
				 */
				result = LonApiNvdFileError;
			}
		} else {
			int index = 0;
			const LonNvDescription* pNvDescription =
					(const LonNvDescription*) LonGetNvTable();

			/*
			 * Write the application signature first. This is read and validated
			 * by LonNvdDeserializeNvd to ensure that the data kept on file matches
			 * the exact set of this interface. The framework generator tools
			 * (LID, III) generate a 16-bit signature at this time. This code here
			 * is prepared for a possible future expansion to a 32-bit signature
			 * value.
			 */
			LonUbits32 signature = 0;
			const unsigned nvCount = LonGetNvCount();

			if (read(fd, &signature, sizeof(signature)) != sizeof(signature)) {
				result = LonApiNvdFailure;
			} else if (signature != LonGetSignature()) {
				/* The file applies to a different application. We accept this as
				 * not an error (the application may have rightfully changed),
				 * but we must ignore the file's content.
				 */
				result = LonApiNoError;
			} else {
				while (index < nvCount && result == LonApiNoError) {
					if (pNvDescription->Attributes & LON_NVDESC_PERSISTENT_MASK) {
						unsigned length = LonGetCurrentNvSize(index);

						if (length && length != (unsigned) -1) {
							if (read(fd, (void*) pNvDescription->pData, length) != length) {
								result = LonApiNvdFailure;
							}
						} else {
							result = LonApiNvdSizeNotSupported;
						}
					}
					++index;
					++pNvDescription;
				}

				if (result == LonApiNoError) {
					/* Update the modifiable value file, if any. */
					unsigned size = 0;
					void* file_data = LonGetFile(LON_DMF_FILEINDEX_MODIFIABLE, &size);

					if (file_data) {
						if (read(fd, file_data, size) != size) {
							result = LonApiNvdFailure;
						}
					}
				}

				if (result == LonApiNoError) {
					/* Update the read-only value file, if any.
					 * See LonNvdSerializeNvs() for a comment discussing the nature
					 * of the "read only" value file.
					 */
					unsigned size = 0;
					void* file_data = LonGetFile(LON_DMF_FILEINDEX_READONLY, &size);

					if (file_data) {
						if (read(fd, file_data, size) != size) {
							result = LonApiNvdFailure;
						}
					}
				}
			}

			/* Let's try to close the file even if an error occurred, but
			 * preserve the original error if the close call also fails.
			 */
			if (close(fd) == -1) {
				if (result == LonApiNoError) {
					result = LonApiNvdFailure;
				}
			}
		}
	}

#else
	LonApiError result = LonApiInitializationFailure;
#endif  /* LON_STD_FILEIO */

	return result;
#else
	/*
	 * This application implements no eeprom network variables or
	 * configuration network variables.
	 * No change to this code is needed
	 */
	return LonApiNoError;
#endif  /* LON_PERSISTENT_NVS */
}
#endif	/*	LONNVDDESERIALIZENVS_HANDLED	*/

#if LON_APPLICATION_MESSAGES

/*
 * Callback: LonMsgArrived
 * Occurs when an application message arrives.
 *
 * Parameters:
 * pAddress - source and destination address (see <LonReceiveAddress>)
 * correlator - correlator; preserve and use with <LonSendResponse>
 * priority - indicates whether its a priority message
 * serviceType - indicates the service type of the message
 * authenticated - TRUE if the message was (successfully) authenticated
 * code - message code
 * pData - pointer to message data bytes, might be NULL if dataLength is zero
 * dataLength - length of bytes pointed to by pData
 *
 * Remarks:
 * This callback reports the arrival of a message that is neither a network
 * variable message, nor a non-Nv message that is otherwise processed by the
 * Micro Server. Typically, this callback handler function is used with
 * application message codes in the value range indicated by the
 * <LonApplicationMessageCode> enumeration.
 * All pointers are only valid for the duration of this callback.
 *
 * Declare LONMSGARRIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONMSGARRIVED_HANDLED
void LonMsgArrived(
		const LonReceiveAddress* const pAddress,
		const LonCorrelator correlator,
		const LonBool priority,
		const LonServiceType serviceType,
		const LonBool authenticated,
		const LonByte code,
		const LonByte* const pData,
		const unsigned dataLength
)
{
	/*
	 * TO DO
	 */
}
#endif /* LONMSGARRIVED_HANDLED */

/*
 * Callback: LonResponseArrived
 * Occurs when a response arrived.
 *
 * Parameters:
 * pAddress - source and destination address used for response (see <LonResponseAddress>)
 * tag - tag to match the response to the request
 * code - response code
 * pData - pointer to response data, might by NULL if dataLength is zero
 * dataLength = number of bytes available through pData.
 *
 * Remarks:
 * This callback occurs when a message arrives in response to an earlier request,
 * sent with the <LonSendMsg> API.
 *
 * Declare LONRESPONSEARRIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONRESPONSEARRIVED_HANDLED
void LonResponseArrived(
		const LonResponseAddress* const pAddress,
		const unsigned tag,
		const LonByte code,
		const LonByte* const pData,
		const unsigned dataLength
)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONRESPONSEARRIVED_HANDLED	*/

/*
 * Callback: LonMsgCompleted
 * Occurs when a message transaction has completed.  See <LonSendMsg>.
 *
 * Parameters:
 * tag - use to correlate the event with the message sent
 * success - TRUE for successful completion, otherwise FALSE
 *
 * Remarks:
 * For unacknowledged or repeated messages, the transaction is complete when
 * the message has been sent with the configured number of retries. For
 * acknowledged messages, the Micro Server calls <LonMsgCompleted> with
 * *success* set to TRUE after receiving acknowledgments from all of the
 * destination devices, and calls <LonMsgCompleted> with *success* set to
 * FALSE if the transaction timeout period expires before receiving
 * acknowledgements from all destinations.
 * Likewise, for request messages, the transaction is considered successful
 * when the Micro Server receives a response from each of the destination
 * devices, and unsuccessful if the transaction timeout expires before
 * responses have been received from all destinations devices.
 *
 * Declare LONMSGCOMPLETED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONMSGCOMPLETED_HANDLED
void LonMsgCompleted(const unsigned tag, const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONMSGCOMPLETED_HANDLED */

#endif    /* LON_APPLICATION_MESSAGES    */

#if LON_NM_QUERY_FUNCTIONS    /* used to be extended API callbacks */

/*
 * Callback: LonDomainConfigReceived
 * Callback following call to <LonQueryDomainConfig>.
 *
 * Parameters:
 * pDomain - pointer to the <LonDomain> structure iff success is TRUE
 * success - boolean success indicator
 *
 * Remarks:
 * The pDomain pointer to the <LonDomain> structure is only valid for the
 * duration of this callback, and only if the success parameter indicates
 * success. Applications that need to preserve this information beyond the
 * duration of this callback must make a copy of the <LonDomain> data.
 * This callback is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 *
 * Declare LONDOMAINCONFIGRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONDOMAINCONFIGRECEIVED_HANDLED
void LonDomainConfigReceived(const LonDomain* const pDomain,
		const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONDOMAINCONFIGRECEVIED_HANDLED */

/*
 * Callback: LonNvConfigReceived
 * Callback following call to <LonQueryNvConfig>.
 *
 * Parameters:
 * pNvConfig - pointer to the <LonNvConfig> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pNvconfig pointer is only valid for the duration of this callback, and
 * only if the success parameter is true. Applications that need to preserve this
 * information beyond the duration of this callback must make a copy of the
 * <LonNvConfig> data. This callback is part of the optional
 * network management query API (LON_NM_QUERY_FUNCTIONS).
 *
 * Declare LONNVCONFIGRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONNVCONFIGRECEIVED_HANDLED
void LonNvConfigReceived(const LonNvConfig* const pNvConfig,
		const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONNVCONFIGRECEIVED_HANDLED */

/*
 * Callback: LonAliasConfigReceived
 * Callback following call to <LonQueryAliasConfig>.
 *
 * Parameters:
 * pAliasConfig - pointer to the <LonAliasConfig> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pAliasConfig pointer is only valid for the duration of this callback, and
 * only if the success parameter is true. Applications that need to preserve this
 * information beyond the duration of this callback must make a copy of the
 * <LonAliasConfig> data.
 * This callback is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 *
 * Declare LONALIASCONFIGRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONALIASCONFIGRECEIVED_HANDLED
void LonAliasConfigReceived(const LonAliasConfig* const pAliasConfig,
		const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONALIASCONFIGRECEIVED_HANDLED */

/*
 * Callback:   LonAddressConfigReceived
 * Callback following call to <LonQueryAddressConfig>.
 *
 * Parameters:
 * pAddress - pointer to the <LonAddress> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pAddress pointer is only valid for the duration of this callback, and
 * only if the success parameter is true. Applications that need to preserve this
 * information beyond the duration of this callback must make a copy of the
 * <LonAddress> data. This callback is part of the optional network management
 * query API (LON_NM_QUERY_FUNCTIONS).
 *
 * Declare LONADDRESSCONFIGRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONADDRESSCONFIGRECEIVED_HANDLED
void LonAddressConfigReceived(const LonAddress* const pAddress,
		const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONADDRESSCONFIGRECEIVED_HANDLED */

/*
 * Callback: LonConfigDataReceived
 * Callback following call to <LonQueryConfigData>.
 *
 * Parameters:
 * pConfigData - pointer to the <LonConfigData> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pConfigData pointer is only valid for the duration of this callback,
 * and only if the success parameter is true. Applications that need to preserve
 * this information beyond the duration of this callback must make a copy of
 * the <LonConfigData> data.
 * This callback is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 *
 * Declare LONCONFIGDATARECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONCONFIGDATARECEIVED_HANDLED
void LonConfigDataReceived(const LonConfigData* const pConfigData,
		const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONCONFIGDATARECEIVED_HANDLED */

/*
 * Callback: LonStatusReceived
 * Callback following call to <LonQueryStatus>.
 *
 * Parameters:
 * pStatus - pointer to the <LonStatus> structure iff successful.
 * success - boolean success indicator
 *
 * Remarks:
 * The pStatus pointer is only valid for the duration of this callback, and only
 * if the success parameter is true. Applications that need to preserve this
 * information beyond the duration of this callback must make a copy of the
 * <LonStatus> data. This callback is part of the optional network management
 * query API (LON_NM_QUERY_FUNCTIONS).
 *
 * Declare LONSTATUSRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONSTATUSRECEIVED_HANDLED
void LonStatusReceived(const LonStatus* const pStatus, const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONSTATUSRECEIVED_HANDLED */

/*
 * Callback: LonTransceiverStatusReceived
 * Callback following a call to <LonQueryTransceiverStatus>.
 *
 * Parameters:
 * pStatus - pointer to the <LonTransceiverParameters> structure iff successful
 * success - boolean success indicator
 *
 * Remarks:
 * The pStatus pointer is only valid for the duration of this callback, and
 * only if the success parameter is true. Applications that need to preserve this
 * information beyond the duration of this callback must make a copy of the
 * <LonTransceiverParameters> data. This callback is part of the optional
 * network management query API (LON_NM_QUERY_FUNCTIONS).
 *
 * The function <LonQueryTransceiverStatus> only works for a Power Line
 * transceiver. If your application invokes that function for any other
 * transceiver type, this callback will declare a failure through the second
 * parameter.
 *
 * Declare LONTRANSCEIVERSTATUSRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONTRANSCEIVERSTATUSRECEIVED_HANDLED
void LonTransceiverStatusReceived(const LonTransceiverParameters* const pStatus,
		const LonBool success)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONTRANSCEIVERSTATUSRECEIVED_HANDLED */

#endif /* LON_NM_QUERY_FUNCTIONS */

#if LON_DMF_ENABLED

/*
 * Callback: LonTranslateWinodwArea
 * Translate a DMF address to a host address.
 *
 * Parameters:
 * dmfAddress - the DMF address, in host byte order
 * size - size of the DMF chunk, in bytes
 *
 * Result:
 * Host address to read from or write to, or NULL if none.
 *
 * DMF transfers use an alleged address within the Smart Transceiver's 64 KB
 * address space. Supported addresses are described by LON_DMF_WINDOW_START
 * and LON_DMF_WINDOW_SIZE. The file directory, template and value files are
 * projected to appear within that window. The LonTranslateWindowArea returns
 * the host-side memory address corresponding to the given DMF address and
 * data size, or returns NULL.
 *
 * Note that this function does not support read or write access across file
 * boundaries.
 *
 * You can supply your own implementation of this callback function by editing
 * the default implementation provided below, or by providing a compatible
 * function elsewhere and defining the LONTRANSLATEWINDOWSAREA_HANDLED symbol.
 */

#ifndef LONTRANSLATEWINDOWSAREA_HANDLED
void* LonTranslateWindowArea(unsigned dmfAddress, unsigned size)
{
	void* result = NULL;
	signed dmfOffset = (int)dmfAddress - LON_DMF_WINDOW_START;
	signed segBase = 0;
	int i = LON_DMF_FILEINDEX_DIRECTORY;

	while (i <= LON_DMF_FILEINDEX_MAXINDEX) {
		unsigned segSize = 0;
		char* segAddr = (char*)LonGetFile(i, &segSize);

		if (segAddr
		&& dmfOffset >= segBase
		&& dmfOffset + size <= segBase + segSize) {
			result = segAddr + dmfOffset - segBase;
			break;
		}
		segBase += segSize;
		++i;
	}

	return result;
}
#endif	/* LONTRANSLATEWINDOWSAREA_HANDLED */

/*
 * Callback: LonMemoryRead
 * Read memory in the ShortStack device's memory space.
 *
 * Parameters:
 * address - virtual address of the memory to be read
 * size - number of bytes to read
 * pData - pointer to a buffer to store the requested data
 *
 * Remarks:
 * The ShortStack event handler calls <LonMemoryRead> whenever it receives a
 * network management memory read request that fits into the registered file
 * access window. This callback function is used to read data starting at the
 * specified virtual Smart Transceiver memory. This applies to template files,
 * configuration property value files, user-defined files, and possibly to other
 * data. The address space for this command is limited to the Smart
 * Transceiver's 64 KB address space.
 *
 * Declare LONMEMORYREAD_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONMEMORYREAD_HANDLED
const LonApiError LonMemoryRead(const unsigned address, const unsigned size, void* const pData)
{
	LonApiError result = LonApiNoError;
	char* pHostAddress = LonTranslateWindowArea(address, size);

	if (pHostAddress) {
		memcpy(pData, pHostAddress, size);
	} else {
		result = LonApiDmfOutOfRange;
	}
	return result;
}
#endif /* LONMEMORYREAD_HANDLED */

/*
 * Callback: LonMemoryWrite
 * Update memory in the ShortStack device's memory space.
 *
 * Parameters:
 * address - virtual address of the memory to be update
 * size - number of bytes to write
 * pData - pointer to the data to write
 *
 * Remarks:
 * The ShortStack event handler calls <LonMemoryWrite> whenever it receives a
 * network management memory write request that fits into the registered file
 * access window. This callback function is used to write data at the specified
 * virtual Smart Transceiver memory.  This applies to configuration property
 * value files, user-defined files, and possibly to other data. The address space
 * for this command is limited to the Smart Transceiver's 64 KB address space.
 *
 * Declare LONMEMORYWRITE_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONMEMORYWRITE_HANDLED
const LonApiError LonMemoryWrite(const unsigned address, const unsigned size, const void* const pData)
{
	LonApiError result = LonApiNoError;
	char* pHostAddress = LonTranslateWindowArea(address, size);

	if (pHostAddress) {
		memcpy(pHostAddress, pData, size);
		result = LonNvdSerializeNvs();
	} else {
		result = LonApiDmfOutOfRange;
	}
	return result;
}
#endif /* LONMEMORYWRITE_HANDLED */
#endif  /* LON_DMF_ENABLED */

#if LON_UTILITY_FUNCTIONS
/*
 * Callback: LonPingReceived
 * The ShortStack Micro Server has sent a ping command.
 *
 * Remarks:
 * The ShortStack Micro Server has sent a ping command. This command can be
 * a response to the ping command sent by the host application
 * to the Micro Server. This callback is part of the optional
 * utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONPINGRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONPINGRECEIVED_HANDLED
void LonPingReceived(void)
{
	/*
	 * TO DO
	 */
}
#endif /* LONPINGRECEIVED_HANDLED */

/*
 * Callback: LonNvIsBoundReceived
 * Callback following a call to <LonNvIsBound>.
 *
 * Parameters:
 * index - index of the network variable
 * bound - indicates whether the network variable is bound
 *
 * Remarks:
 * The Micro Server has responded to the <LonNvIsBound> request. The boolean
 * variable *bound* tells whether the network variable identified by index
 * is bound. This callback is part of the optional
 * utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONNVISBOUNDRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONNVISBOUNDRECEIVED_HANDLED
void LonNvIsBoundReceived(const unsigned index, const LonBool bound)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONNVISBOUNDRECEIVED_HANDLED */

/*
 * Callback: LonMtIsBoundReceived
 * Callback following a call to <LonMtIsBound>.
 *
 * Parameters:
 * index - index of the message tag
 * bound - indicates whether the message tag is bound
 *
 * Remarks:
 * The Micro Server has responded to the <LonMtIsBound> request. The boolean
 * variable *bound* tells whether the message tag identified by index
 * is bound. This callback is part of the optional
 * utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONMTISBOUNDRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONMTISBOUNDRECEIVED_HANDLED
void LonMtIsBoundReceived(const unsigned index, const LonBool bound)
{
	/*
	 * TO DO
	 */
}
#endif /* LONMTISBOUNDRECEIVED_HANDLED */

/*
 * Callback: LonGoUnconfiguredReceived
 * Callback following a call to <LonGoUnconfigured>.
 *
 * Remarks:
 * The Micro Server has responded to the <LonGoUnconfigured> request.
 * If the Micro Server was in a configured state before the <LonGoUnconfigured>
 * request was sent, it will reset after going to the unconfigured state.
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONGOUNCONFIGUREDRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONGOUNCONFIGUREDRECEIVED_HANDLED
void LonGoUnconfiguredReceived(void)
{
	/*
	 * TO DO
	 */
}
#endif	/* LONGOUNCONFIGUREDRECEIVED_HANDLED */

/*
 * Callback: LonGoConfiguredReceived
 * Callback following a call to <LonGoConfigured>.
 *
 * Remarks:
 * The Micro Server has responded to the <LonGoConfigured> request.
 * The Micro Server will not reset after going into the configured state unless
 * some serious error, such as an application checksum error, is detected in the
 * process.
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONGOCONFIGUREDRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONGOCONFIGUREDRECEIVED_HANDLED
void LonGoConfiguredReceived(void)
{
	/*
	 * TO DO
	 */
}
#endif /* LONGOCONFIGUREDRECEIVED_HANDLED */

/*
 * Callback: LonAppSignatureReceived
 * Callback following a call to <LonQueryAppSignature>.
 *
 * Parameter:
 * appSignature - Micro Server's copy of the host's application signature
 *
 * Remarks:
 * The Micro Server has responded to the <LonQueryAppSignature> request.
 * If the *bInvalidate* flag was set to TRUE in the <LonQueryAppSignature>
 * request, the Micro Server has already invalidated the signature by the time
 * this callback is called.
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONAPPSIGNATURERECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONAPPSIGNATURERECEIVED_HANDLED
void LonAppSignatureReceived(LonWord appSignature)
{
	/*
	 * TO DO
	 */
}
#endif /* LONAPPSIGNATURERECEIVED_HANDLED */

/*
 * Callback: LonVersionReceived
 * Callback following a call to <LonQueryVersion>.
 *
 * Parameters:
 * appMajor - the major version number for the Micro Server application, 0..255
 * appMinor - the minor version number for the Micro Server application, 0..255
 * appBuild - the build number for the Micro Server application, 0..255
 * coreMajor - the major version number for the Micro Server core library, 0..255
 * coreMinor - the minor version number for the Micro Server core library, 0..255
 * coreBuild - the build number for the Micro Server core library, 0..255
 *
 * Remarks:
 * The Micro Server has responded to a <LonQueryVersion> request.
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONVERSIONRECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONVERSIONRECEIVED_HANDLED
void LonVersionReceived(unsigned appMajor, unsigned appMinor, unsigned appBuild,
		unsigned coreMajor, unsigned coreMinor, unsigned coreBuild)
{
	/*
	 * TO DO
	 */
}
#endif /* LONVERSIONRECEIVED_HANDLED */

/*
 * Callback: LonEchoReceived
 * Callback following a call to <LonRequestEcho>.
 *
 * Parameters:
 * LON_ECHO_SIZE payload bytes, as sent through <LonRequestEcho>, but modified by the
 * Micro Server by adding one to each byte, using an unsigned 8-bit addition without
 * overflow detection.
 *
 * Remarks:
 * The Micro Server has responded to a <LonRequestEcho> request. The host application
 * should now confirm that the echoed data matches the expectations; mismatching data
 * can indicate an incorrect host application or link layer driver, or permanent or
 * transient link layer errors. These link layer errors could, for example, be
 * introduced by incorrect line termination, excessive coupling or cross-talk, or by
 * excessive or out-of-sync link layer bit rates.
 * This callback is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 *
 * Declare LONECHORECEIVED_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONECHORECEIVED_HANDLED
void LonEchoReceived(const LonByte data[LON_ECHO_SIZE])
{
	/*
	 * TO DO
	 */
}
#endif /* LONECHORECEIVED_HANDLED */
#endif  /* LON_UTILITY_FUNCTIONS */

#ifdef LON_NVDESC_ENCRYPT_MASK
/*
 * Callback: LonEncrypt
 * Callback to engage the application-specific encryption algorithm.
 *
 * Parameters:
 * index - the global index of the related network variable, or -1 if none
 * inSize - the number of bytes that are to be encrypted
 * inData - pointer to first byte that is to be encrypted
 * outSize - output parameter, points to variable holding size of encrypted data in bytes
 * outData - output parameter, points to pointer to encrypted data
 *
 * Remarks:
 * The function encrypts <inSize> bytes, starting at <inData>, and writes the result
 * into a buffer allocated by the function itself. The buffer must remain valid after
 * returning from LonEncrypt(), but may be re-used with the next call to <LonEnrypt>
 * or <LonDecipher> function. If successful, the output parameters <outSize> and
 * <outData> provide access to this buffer. The function is required to provide its
 * own buffer; in-situ encryption, overwriting <inData>, is not permitted.
 * The function returns results through the standard <LonApiError> enumeration.
 * The algorithm used with <LonEncrypt> must be reversible through <LonDecipher>.
 *
 * Note that the provision of application-specific cypher is yields a not
 * interoperable application.
 *
 * Declare LONENCRYPT_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONENCRYPT_HANDLED
LonApiError LonEncrypt(int index,
		unsigned inSize, const void* inData,
		unsigned* const outSize, void** outData)
{
	return LonApiNvUnsupported;
}
#endif /* LONENCRYPT_HANDLED */

/*
 * Callback: LonDecipher
 * Callback to engage the application-specific decryption algorithm.
 *
 * Parameters:
 * index - the global index of the related network variable, or -1 if none
 * inSize - the number of bytes that are to be decrypted
 * inData - pointer to first byte that is to be decrypted
 * outSize - output parameter, points to variable holding size of decrypted data in bytes
 * outData - output parameter, points to pointer to decrypted data
 *
 * Remarks:
 * The function decrypts <inSize> bytes, starting at <inData>, and writes the result
 * into a buffer allocated by the function itself. The buffer must remain valid after
 * returning from LonDecipher(), but may be re-used with the next call to <LonEnrypt>
 * or <LonDecipher> function. If successful, the output parameters <outSize> and
 * <outData> provide access to this buffer. The function is required to provide its
 * own buffer; in-situ encryption, overwriting <inData>, is not permitted.
 * The function returns results through the standard <LonApiError> enumeration.
 * The algorithm used with <LonDecipher> must be reversible through <LonEncrypt>.
 *
 * Note that the provision of application-specific cypher is yields a not
 * interoperable application.
 *
 * Declare LONDECIPHER_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONDECIPHER_HANDLED

LonApiError LonDecipher(int index,
		unsigned inSize, const void* inData,
		unsigned* const outSize, void** outData)
{
	return LonApiNvUnsupported;
}
#endif /* LONDECIPHER_HANDLED */
#endif  /* LON_NVDESC_ENCRYPT_MASK */

/*
 * Callback: LonCustomCommunicationParameters
 * Callback to enforce application-specific communication parameters.
 *
 * Parameters:
 * params: A pointer to 16 communication parameter bytes
 *
 * Result:
 * A boolean flag, indicating whether custom communication parameters have been
 * set.
 *
 * Remarks:
 * WARNING! Use the <LonCustomCommunicationParameters> API with caution, as the
 * specification of incorrect communication parameters may prevent further
 * communication with the Micro Server over the link layer and/or the network,
 * and may render the Micro Server permanently defunct.
 *
 * See ShortStackApi.h for a detailed discussion of this API and its uses.
 *
 * Declare LONCUSTOMCOMMUNICATIONPARAMETERS_HANDLED if you provide a compatible
 * implementation of this event handler elsewhere.
 */
#ifndef LONCUSTOMCOMMUNICATIONPARAMETERS_HANDLED
LonBool LonCustomCommunicationParameters(LonByte* const pParameters) {
	return FALSE; /* no communication parameter override */
}
#endif /* LONCUSTOMCOMMUNICATIONPARAMETERS_HANDLED */
