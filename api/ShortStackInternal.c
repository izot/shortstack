/*
 * Filename: ShortStackInternal.c
 *
 * Description: This file contains the implementation of some internal
 * functions used by the ShortStack API.
 *
 * Copyright (c) Echelon Corporation 2002-2015.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

#include <string.h>
#include "ShortStackDev.h"
#include "ShortStackApi.h"
#include "ldv.h"

/*
 * Following are a few macros that are used internally to access
 * network variable and application message details.
 */
#define PSICB   ((LonSicb*)pSmipMsg->Payload)
#define EXPMSG  (PSICB->ExplicitMessage)
#define NVMSG   (PSICB->NvMessage)

/*
 * Forward declarations for functions used internally by the ShortStack Api.
 */
const LonApiError VerifyNvIndex(const unsigned index);
unsigned LonGetTruncatedNvLength(unsigned nvIndex, const LonNvDescription* const nvDescription);
const LonNvDescription* const LonGetNvDescription(const unsigned index);
void  PrepareNvMessage(LonSmipMsg* pSmipMsg, const LonByte nvIndex, const LonByte* const pData, const LonByte len);
const LonApiError SendNv(const LonByte nvIndex);
const LonApiError SendNvPollResponse(const LonSmipMsg* pSmipMsg);
const LonApiError SendLocal(const LonSmipCmd command, const void* const pData, const LonByte length);
const LonApiError WriteNvLocal(const LonByte index, const void* const pData, const LonByte length);

extern LdvHandle ldv_handle;

/***********************************************************************************
 * VerifyNvIndex
 *
 * This is a utility to verify the validity of a local network variable index.
 *
 * Arguments:  index -- index of the network variable; will be checked for validity
 *
 * Returns:    LonApiNoError for success, or an appropriate error code otherwise
 ********************************************************************************** */

const LonApiError VerifyNvIndex(const unsigned nvIndex)
{
    const unsigned nvCount = LonGetNvCount();
    return nvIndex < nvCount ? LonApiNoError : LonApiNvIndexInvalid;
}

/***********************************************************************************
 * PrepareNvMessage
 *
 * Description: Prepares a generic network variable message, taking
 *              care of cases when the nv index is greater than 63.
 *
 * Arguments:   pSmipMsg -- pointer to the message to be prepared
 *              nvIndex -- index of the network variable to send
 *              pData   -- pointer to the network variable data
 *              len     -- length of the network variable data
 *
 * Returns:     Non-zero error code if an error occurs.
 *              Zero if the message was prepared successfully.
 **********************************************************************************/
void PrepareNvMessage(
    LonSmipMsg* pSmipMsg,
    const LonByte nvIndex,
    const LonByte* const pData,
    const LonByte len
)
{
    /* if the nv index is less than 63, it is also stored in the command byte */
    if (nvIndex < LON_NV_ESCAPE_SEQUENCE) {
        pSmipMsg->Header.Command = LonNiNv | nvIndex;
    } else {
        pSmipMsg->Header.Command = LonNiNv | LON_NV_ESCAPE_SEQUENCE;
        pSmipMsg->ExtHdr.Index = nvIndex;
    }

    NVMSG.Index = nvIndex;
    LON_SET_ATTRIBUTE(NVMSG, LON_NVMSG_MSGTYPE, LonMessageNv);
    NVMSG.Length = len;

    if (len  &&  pData != NULL) {
        memcpy(NVMSG.NvData, pData, len);
    }

    pSmipMsg->Header.Length = (LonByte)(sizeof(LonNvMessage) - sizeof(NVMSG.NvData) + len);
}

/***********************************************************************************
 * SendNv
 *
 * Description: Send a network variable update message onto the network.
 *
 * Arguments:   index -- index of the network variable to send
 *              pValue  -- pointer to the network variable data
 *
 * Returns:     Non-zero error code if an error occurs.
 *              Zero if the outgoing NV-update message has been buffered by the driver.
 *
 * Caveats:     On success, this will result in network traffic.
 **********************************************************************************/
const LonApiError SendNv(const LonByte nvIndex)
{
    LonApiError result = VerifyNvIndex(nvIndex);

    if (result == LonApiNoError) {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            const LonNvDescription* pNvDescription = LonGetNvDescription(nvIndex);
            const LonByte* pData = (const LonByte*)pNvDescription->pData;
            unsigned length = LonGetTruncatedNvLength(nvIndex, pNvDescription);
            unsigned transmitLength = length;
            void* transmitData = (void*)pData;

#ifdef LON_NVDESC_ENCRYPT_MASK

            if (pNvDescription->Attributes & LON_NVDESC_ENCRYPT_MASK) {
                result = LonEncrypt(
                             nvIndex, length, (void const*)pData, &transmitLength, &transmitData
                         );
            }

#endif  /* LON_NVDESC_ENCRYPT_MASK */

            if (result == LonApiNoError) {
                PrepareNvMessage(
                    pSmipMsg, nvIndex, (const LonByte *)transmitData, (LonByte)transmitLength
                );
                result = LdvPutMsg(ldv_handle, pSmipMsg);
            }
        }
    }

    return result;
}

/***********************************************************************************
 * SendNvPollResponse
 *
 * Description: Send a response to an NV-poll request.
 *
 * Arguments:   pSmipMsg -- incoming NV-poll request message
 *
 * Returns:     Non-zero error code if an error occurs.
 *              Zero if the NV-poll response message has been buffered by the driver.
 **********************************************************************************/
const LonApiError SendNvPollResponse(const LonSmipMsg* pSmipMsg)
{
    const unsigned nvIndex = NVMSG.Index;
    LonApiError result = VerifyNvIndex(nvIndex);

    if (result == LonApiNoError) {
        LonSmipMsg* pSmipResponse = NULL;

        result = LdvAllocateMsg(ldv_handle, &pSmipResponse);

        if (result == LonApiNoError) {
            const unsigned aliasIndex = NVMSG.AliasIndex;
            const LonNvDescription* const nvDesc = LonGetNvDescription(nvIndex);
            const unsigned nvLength = LonGetTruncatedNvLength(nvIndex, nvDesc);
            LonNvMessage* const pNvResponse = (LonNvMessage * const)pSmipResponse->Payload;
            unsigned transmitLength = nvLength;
            void* transmitData = (void*)nvDesc->pData;

            /* Copy the correlator fields namely Tag, MsgType and Priority. */
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_TAG, LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_TAG));
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_MSGTYPE, LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_MSGTYPE));
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_PRIORITY, LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_PRIORITY));
            /* Set the response attribute */
            LON_SET_ATTRIBUTE((*pNvResponse), LON_NVMSG_RESPONSE, 1);

#ifdef LON_NVDESC_ENCRYPT_MASK

            if (nvDesc->Attributes & LON_NVDESC_ENCRYPT_MASK) {
                result = LonEncrypt(
                             nvIndex, nvLength, (const void *)nvDesc->pData, &transmitLength, &transmitData
                         );
            }

#endif /* LON_NVDESC_ENCRYPT_MASK */

            if (result != LonApiNoError) {
                LdvReleaseMsg(ldv_handle, pSmipResponse);
            } else {
                pNvResponse->Length = (LonByte) transmitLength;
                /* If alias index is valid, i.e., it is not 128, then treat as alias. */
                pNvResponse->Index = (LonByte) ((aliasIndex & 0x80) ? nvIndex : aliasIndex); /* To Do: Define a macro for 0x80 */
                pNvResponse->AliasIndex = aliasIndex;

                memcpy(pNvResponse->NvData, transmitData, transmitLength);
                pSmipResponse->Header.Length = (LonByte)(sizeof(LonNvMessage) - sizeof(NVMSG.NvData) + transmitLength);
                pSmipResponse->Header.Command = (LonSmipCmd) (nvIndex < LON_NV_ESCAPE_SEQUENCE ?
                        (LonNiNv | nvIndex) : (LonNiNv | LON_NV_ESCAPE_SEQUENCE));
                result = LdvPutMsg(ldv_handle, pSmipResponse);
            }
        }
    }

    return result;
}


/***********************************************************************************
 * SendLocal
 *
 * Description: Send a local network interface command to the ShortStack Micro Server
 *
 * Arguments:   command -- network interface command to send
 *
 * Returns:     None-Zero error code if an error occurs.
 *              Zero if the local NI command message has been buffered by the driver.
 **********************************************************************************/
const LonApiError SendLocal(const LonSmipCmd command, const void* const pData, const LonByte length)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonApiError result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

    if (result == LonApiNoError) {
        /* OK, construct and post message: */
        pSmipMsg->Header.Length = length;
        pSmipMsg->Header.Command = command;
        memcpy(PSICB, pData, length);

        result = LdvPutMsg(ldv_handle, pSmipMsg);
    }

    return result;
}

/***********************************************************************************
 * WriteNvLocal
 *
 * Description: Write network variable value locally. This is called when an NV
 *              update or non-zero NV poll response arrives. The index must be
 *              verified before calling this function. The function then decrypts
 *              NV data (if necessary), and validates the data size.
 *
 * Arguments:   index -- index of the network variable to be written.
 *              pData   -- pointer to the network variable value
 *              length -- length of network variable value
 *
 * Returns:     LonApiNoError if the NV was updated successfully.
 *              None-zero error code if an error occurs
 **********************************************************************************/
const LonApiError WriteNvLocal(const LonByte index, const void* const pData, const LonByte length)
{
    const unsigned expectedLength = LonGetCurrentNvSize(index);
    const LonNvDescription* const pNvDescription = LonGetNvDescription(index);
    LonApiError result = LonApiNoError;
    unsigned finalLength = length;
    void* finalData = (void*)pData;

#ifdef LON_NVDESC_ENCRYPT_MASK

    if (pNvDescription->Attributes & LON_NVDESC_ENCRYPT_MASK) {
        /* encrypted network variable. Try to decrypt: */
        result = LonDecipher(index, length, pData, &finalLength, &finalData);
    }

#endif /* LON_NVDESC_ENCRYPT_MASK */

    if (result == LonApiNoError) {
        /* Is the length acceptable? */
#ifdef LON_NVDESC_TRUNCATE_MASK
        if (pNvDescription->Attributes & LON_NVDESC_TRUNCATE_MASK) {
            if (finalLength < 1 || finalLength > expectedLength) {
                result = LonApiNvLengthMismatch;
            }
        } else if (expectedLength != finalLength) {
            result = LonApiNvLengthMismatch;
        }

#else

        if (expectedLength != finalLength) {
            result = LonApiNvLengthMismatch;
        }

#endif /* LON_NVDESC_TRUNCATE_MASK */
    }

    if (result == LonApiNoError) {
        /* update NV value */
        memcpy((void*)pNvDescription->pData, finalData, finalLength);
#ifdef LON_NVDESC_TRUNCATE_MASK

        if (finalLength < expectedLength) {
            /* clear local data which wasn't updated with incoming data due to truncation */
            memset(
                ((unsigned char*)pNvDescription->pData) + finalLength,
                0x00,
                expectedLength - finalLength
            );
        }

#endif /* LON_NVDESC_TRUNCATE_MASK */

        if (pNvDescription->Attributes & LON_NVDESC_PERSISTENT_MASK) {
            result = LonNvdSerializeNvs();
        }
    }

    return result;
}

/***********************************************************************************
 * LonGetTruncatedNvLength
 *
 * Description: LonGetTruncatedNvLength returns the current lenght of the NV described
 *              through the function's arguments for network variables which do not
 *              support truncation, and returns the truncated length otherwise. The
 *              truncated length is the total network variable length, minus the number
 *              of any trailing 0x00 bytes. The first byte, even if it is 0x00, is never
 *              truncated.
 *
 * Arguments:   nvIndex -- index to the network variable to be examined
 *              nvDescription -- pointer to the nvTable record for this NV
 *
 * Returns:     Truncated length, or current full network variable length for those
 *              network variables which don't support truncation.
 **********************************************************************************/
unsigned LonGetTruncatedNvLength(unsigned nvIndex, const LonNvDescription* const nvDescription)
{
    unsigned length = LonGetCurrentNvSize(nvIndex);

#ifdef LON_NVDESC_TRUNCATE_MASK

    if (length > 1 && (nvDescription->Attributes & LON_NVDESC_TRUNCATE_MASK)) {
        const unsigned char* pData = (const unsigned char*)nvDescription->pData;
        pData += length - 1;

        while (length > 1 && *pData-- == '\0') {
            --length;
        }
    }

#endif  /* LON_NVDESC_TRUNCATE_MASK */

    return length;
}


/***********************************************************************************
 * LonGetNvDescription
 *
 * Description: LonGetNvDescription returns a pointer to the record within the NV
 *              table. The function always succeeds, but must be called with a valid
 *              index.
 *
 * Arguments:   index -- index to the network variable of interest
 *
 * Returns:     Pointer to this network variable's description record
 **********************************************************************************/
const LonNvDescription* const LonGetNvDescription(const unsigned index)
{
    const LonNvDescription* const pNvTable =
    		(const LonNvDescription* const)LonGetNvTable();
    return &pNvTable[index];
}
