/*
 * ShortStackApi.c
 *
 * Description: This file contains function implementations for the
 * IzoT ShortStack LonTalk Compact API.
 *
 * Copyright (c) Adesto Technologies Corporation 2002-2019.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

#include <string.h>
#include "ShortStackDev.h"
#include "ShortStackApi.h"

#if LON_ISI_ENABLED
#include "ShortStackIsiApi.h"
#endif /* LON_ISI_ENABLED */

#include "ldv.h"

/*
 * Disable warnings related to switch clauses for the popular GNU C Compiler, e.g.
 * warning: case value abc not in enumerated type xyz [-Wswitch]
 * enumeration value abc not handled in switch [-Wswitch]
 */
#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wswitch"
#endif  /* __GNUC__ */

/*
 * Following are a few macros that are used internally to access
 * network variable and application message details.
 */
#define PSICB   ((LonSicb*)pSmipMsg->Payload)
#define EXPMSG  (PSICB->ExplicitMessage)
#define NVMSG   (PSICB->NvMessage)

/*
 * Following is the reset message buffer. Any uplink reset message will be copied
 * into this buffer, which serves as a source for validation of various indices
 * against the Micro Server's capabilities, and as a source for version number and
 * Micro Server Unique Id (Neuron Id). At reset, the buffer is cleared,
 * indicating that the remaining information in this buffer is invalid.
 */
static volatile LonResetNotification lastResetNotification;

/*
 * The initialization sequence concludes with a reset request, and is not complete
 * until the corresponding uplink reset has been received. This variable is used to
 * monitor this uplink reset. If the initialization routine would return to the
 * application before the uplink reset is received, the application couldn't reliably
 * issue any requests to the Micro Server, because the reset notification resets the
 * link layer driver.
 */
static unsigned resetCounter;

/*
 * A global array to hold the message response data.
 * Make no assumptions about the previous contents while using it.
 * If required, NULL out all data before use.
 */
LonByte ResponseData[LON_MAX_MSG_DATA];

/*
 * Typedef used to keep track of local NM/ND messages.
 * The LonTalk protocol has a limitation that the response codes for these
 * messages are not unique. Hence, the API limits only one such request pending
 * at any given time and keeps track of the type of the message.
 */
typedef enum {
    NO_NM_ND_PENDING,
    NM_PENDING,
    ND_PENDING
} NmNdStatus;
NmNdStatus CurrentNmNdStatus = NO_NM_ND_PENDING;

LdvHandle ldv_handle;

/*
 * Forward declarations for functions used internally by the ShortStack Api.
 * These functions are implemented in ShortStackInternal.c.
 */
extern const LonApiError VerifyNvIndex(const unsigned nvIndex);
extern unsigned LonGetTruncatedNvLength(unsigned nvIndex, const LonNvDescription* const nvDescription);
extern const LonNvDescription* const LonGetNvDescription(const unsigned index);
extern void  PrepareNvMessage(LonSmipMsg* pSmipMsg, const LonByte nvIndex, const LonByte* const pData, const LonByte len);
extern const LonApiError SendNv(const LonByte nvIndex);
extern const LonApiError SendNvPollResponse(const LonSmipMsg* pSmipMsg);
extern const LonApiError SendLocal(const LonSmipCmd command, const void* const pData, const LonByte length);
extern const LonApiError WriteNvLocal(const LonByte index, const void* const pData, const LonByte length);

#if LON_ISI_ENABLED
extern LonApiError SendDownlinkRpc(IsiDownlinkRpcCode code, LonByte param1, LonByte param2, void* pData, unsigned len);
extern void HandleUplinkRpcAck(IsiRpcMessage* pMsg, LonBool bSuccess);
extern void HandleUplinkRpc(IsiRpcMessage* pMsg);
#endif /* LON_ISI_ENABLED */

/*
 * Function: InitMicroServer
 * A static function to initialize the Micro Server.
 *
 * InitMicroServer() is an internal utility which initializes the Micro Server.
 * This is part of the overall application initialization provided with the
 * <LonInit> API, but is also called when the API receives a reset notification
 * from an uninitialized Micro Server. This second case occurs when the Micro
 * Server firmware is updated over the network.
 */
static LonApiError InitMicroServer(void)
{
    LonSmipMsg* pSmipMsg = NULL;

    LonByte nTotalNvCount = LonGetNvCount();
    LonByte nTotalNvsSent = 0;
    /* The LonGetAppInitData function returns a structure containing the application
       initialization data followed by the network variable initialization data.
       The structure needs to be parsed to extract this data and then needs to be formatted. */
    const LonByte *pInitData = LonGetAppInitData();
    LonApiError result = LdvAllocateMsgWait(ldv_handle, &pSmipMsg);

    if (result == LonApiNoError) {
        /* Prepare and send the LonInitialization message to the ShortStack Micro Server */
        pSmipMsg->Header.Command = LonNiAppInit;
        pSmipMsg->Header.Length = LON_APP_INIT_MSG_SIZE;
        memcpy(pSmipMsg->Payload, pInitData, LON_APP_INIT_MSG_SIZE);

        if (LonCustomCommunicationParameters(pSmipMsg->Payload + LON_APPINIT_OFFSET_COMMPARAM)) {
            /* Activate these parameters */
            pSmipMsg->Payload[LON_APPINIT_OFFSET_MISC] &= ~LON_USE_DEFAULT_COMMPARAMS;
        }

        result = LdvPutMsg(ldv_handle, pSmipMsg);

        /* Prepare and send the LonNvInitialization messages to the ShortStack Micro Server */
        /* The last byte of the app init message contains the Nv Count */
        nTotalNvCount = pInitData[LON_APP_INIT_MSG_SIZE - 1];
        nTotalNvsSent = 0;

        while (result == LonApiNoError) {
            /* Calculate the number of nvs that can be sent in this message */
            LonByte nStartIndex = nTotalNvsSent;
            LonByte nStopIndex = ((nTotalNvCount - nTotalNvsSent) > LON_MAX_NVS_IN_NV_INIT) ?
                        (nStartIndex + LON_MAX_NVS_IN_NV_INIT) : nTotalNvCount;

            result = LdvAllocateMsgWait(ldv_handle, &pSmipMsg);

            if (result == LonApiNoError) {
                pSmipMsg->Header.Command = LonNiNvInit;
                pSmipMsg->Header.Length = 3 + nStopIndex - nStartIndex;
                pSmipMsg->Payload[0] = nStartIndex;
                pSmipMsg->Payload[1] = nStopIndex;
                pSmipMsg->Payload[2] = nTotalNvCount;
                memcpy(
                    pSmipMsg->Payload + 3,
                    pInitData + LON_APP_INIT_MSG_SIZE + nTotalNvsSent,
                    nStopIndex - nStartIndex
                );

                result = LdvPutMsg(ldv_handle, pSmipMsg);

                if (result == LonApiNoError) {
                    nTotalNvsSent += nStopIndex - nStartIndex;

                    if (nTotalNvsSent == nTotalNvCount) {
                        break;
                    }
                }
            }
        }
    }

    /* Reset the Micro Server so that any configuration change to the ShortStack    */
    /* Micro Server can take effect.                                                */
    if (result == LonApiNoError) {
        result = LdvAllocateMsgWait(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiReset;
            pSmipMsg->Header.Length = 0;
        }

        resetCounter = 0;
        result = LdvPutMsg(ldv_handle, pSmipMsg);

        if (result == LonApiNoError) {
            while(!resetCounter) {
                LonEventHandler();
            }
        }
    }

    return result;
}

/*
 * Function: LonInit
 * Initializes the ShortStack LonTalk Compact API and Micro Server
 *
 * Arguments:
 * ctrl - <LdvCtrl> driver control data defined by your driver
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * The function takes a LdvCtrl parameter, which it passes through
 * to your driver's LdvOpen() function.
 *
 * The function returns a <LonApiError> code to indicate
 * success or failure. The function must be called during application
 * initialization, and prior to invoking any other function of the ShortStack
 * LonTalk Compact API.
 * Note that the Micro Server disables all network communication until this
 * function completes successfully.
 */
const LonApiError LonInit(LdvCtrl* ctrl)
{
    LonApiError result = LdvOpen(ctrl, &ldv_handle);

    /* Clear the information obtained from the last reset notification message */
    memset((void*) &lastResetNotification, 0, sizeof(LonResetNotification));
    CurrentNmNdStatus = NO_NM_ND_PENDING;

    if (result == LonApiNoError) {
    	result = LonReinit();
    }

    return result;
}

/*
 * Function: LonReinit
 *
 * Returns:
 * <LonApiError>
 *
 * The function takes no arguments. It is called from within <LonInit>, but
 * can also be called later during the lifetime of the application in order
 * to re-initialize the Micro Server and framework without having to restart
 * the link layer driver.
 * This is sometimes used by advanced applications which implement pseudo-
 * dynamic interfaces.
 */
const LonApiError LonReinit(void)
{
	LonApiError result = LonApiNoError;

	LonFrameworkInit();

    if (result == LonApiNoError) {
        /* Read the NV values (if any) from persistent storage  */
        result = LonNvdDeserializeNvs();
    }

    /* Send the Initialization data to the ShortStack Micro Server */
    if (result == LonApiNoError) {
        result = InitMicroServer();
    }

    return result;
}

/*
 * Function: LonExit
 * Prepares the application for exiting.
 */
const LonApiError LonExit(void)
{
    return LdvClose(ldv_handle);
}


/*
 * Function: LonEventHandler
 * Periodic service to the ShortStack LonTalk Compact API.
 *
 * Remarks:
 * This function must be called periodically by the application.  This
 * function processes any messages that have been received from the Micro Server.
 * The application can call this function as part of the idle loop, or from a
 * dedicated timer-based thread or interrupt service routine. All API callback
 * functions occur within this function's context. Note that the application is
 * responsible for correct context management and thread synchronization, as
 * (and if) required by the hosting platform.
 *
 * This function must be called at least once every 10 ms.  Use the following
 * formula to determine the minimum call rate:
 *  rate = MaxPacketRate / (InputBufferCount - 1)
 * where MaxPacketRate is the maximum number of packets per second arriving for
 * the device and InputBufferCount is the number of input buffers defined for
 * the application.
 */
void LonEventHandler(void)
{
    LonSmipMsg* pSmipMsg = NULL;
    LonBool requestReinit = FALSE;

    if (LdvGetMsg(ldv_handle, &pSmipMsg) == LonApiNoError) {
        /* A message has been retrieved from driver's receive buffer    */
        LonCorrelator correlator = {0};

        /* Make correlation structure    */
        LON_SET_ATTRIBUTE(correlator, LON_CORRELATOR_PRIORITY, LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY));
        LON_SET_ATTRIBUTE(correlator, LON_CORRELATOR_TAG, LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG));
        LON_SET_ATTRIBUTE(correlator, LON_CORRELATOR_SERVICE, LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE));

        switch (pSmipMsg->Header.Command) {
        case ((LonByte) LonNiComm | (LonByte) LonNiIncoming): {
            /* Is an incoming message    */
            LonBool bFailure = FALSE;

            if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE) == LonMessageNv) {
                /* Process NV messages */
                if (LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_NVPOLL)) {
                    /* Process NV poll message */
                    bFailure = (SendNvPollResponse(pSmipMsg) != LonApiNoError);
                } else {
                    if (VerifyNvIndex(NVMSG.Index) == LonApiNoError) {
                        if (WriteNvLocal(NVMSG.Index, NVMSG.NvData, NVMSG.Length) == LonApiNoError) {
                            /* Process NV update message */
#if LON_EXPLICIT_ADDRESSING
                            LonNvUpdateOccurred(NVMSG.Index, &(EXPMSG.Address.Receive));
#else
                            LonNvUpdateOccurred(NVMSG.Index, NULL);
#endif /* LON_EXPLICIT_ADDRESSING */
                        } else {
                            bFailure = TRUE;
                        }
                    } else {
                        bFailure = TRUE;
                    }
                }
            } else {
                /* Process explicit messages */
                switch (EXPMSG.Code) {
                case LonNmSetNodeMode:

                    /* Process Set Node Mode network management message    */
                    switch(EXPMSG.Data.NodeMode.Mode) {
                    case LonApplicationOffLine:
                        LonOffline();
                        SendLocal(LonNiOffLine, NULL, 0);
                        break;

                    case LonApplicationOnLine:
                        LonOnline();
                        SendLocal(LonNiOnLine, NULL, 0);
                        break;

                    default:
                        bFailure = TRUE;
                        break;
                    }

                    break;

                case LonNmNvFetch:

                    /* Process Nv Fetch network management message        */
                    if (EXPMSG.Data.NvFetch.Index == 0xFF)
                        /* This is the escape index which means that the true index is
                           255 or greater and is in the following two bytes.
                           ShortStack doesn't support NV's with index greater than 254. */
                    {
                        bFailure = TRUE;
                    } else {
                        const unsigned nvIndex = EXPMSG.Data.NvFetch.Index;
                        /* Send NV response to the network.           */
                        const LonNvDescription* const nvDescription = LonGetNvDescription(nvIndex);
                        unsigned nvLength = LonGetTruncatedNvLength(nvIndex, nvDescription);

                        if (VerifyNvIndex(nvIndex) != LonApiNoError) {
                            bFailure = TRUE;
                        } else {
                            void* transmitData = (void*)nvDescription->pData;
                            unsigned transmitLength = nvLength;
                            LonApiError error = LonApiNoError;

                            ResponseData[0] = (LonByte) nvIndex;

#ifdef LON_NVDESC_ENCRYPT_MASK

                            if (nvDescription->Attributes & LON_NVDESC_ENCRYPT_MASK) {
                                error = LonEncrypt(nvIndex,
                                            nvLength, (void const*)nvDescription->pData,
                                            &transmitLength, &transmitData
                                        );
                            }

#endif  /* LON_NVDESC_ENCRYPT_MASK */

                            if ((error != LonApiNoError)
                            || (transmitLength > sizeof(ResponseData) - 1)) {
                                bFailure = TRUE;
                            } else {
                                memcpy(&ResponseData[1], transmitData, transmitLength);
                                error = LonSendResponse(
                                            correlator,
                                            LON_NM_SUCCESS(LonNmNvFetch),
                                            ResponseData,
                                            transmitLength + 1
                                        );
                                bFailure = error != LonApiNoError;
                            }
                        }
                    }

                    break;

#if     LON_DMF_ENABLED

                case LonNmReadMemory:

                    /* Process Read Memory network management message        */
                    bFailure = EXPMSG.Data.ReadMemory.Mode != LonAbsoluteMemory
                             || LonMemoryRead(
                                    LON_GET_UNSIGNED_WORD(EXPMSG.Data.ReadMemory.Address),
                                    EXPMSG.Data.ReadMemory.Count,
                                    &ResponseData[0]
                                )
                             || LonSendResponse(
                                    correlator,
                                    LON_NM_SUCCESS(LonNmReadMemory),
                                    &ResponseData[0],
                                    EXPMSG.Data.ReadMemory.Count
                                );
                    break;

                case LonNmWriteMemory:

                    /* Process Write Memory network management message        */
                    bFailure = EXPMSG.Data.WriteMemory.Mode != LonAbsoluteMemory
                            || LonMemoryWrite(
                                   LON_GET_UNSIGNED_WORD(EXPMSG.Data.WriteMemory.Address),
                                   EXPMSG.Data.WriteMemory.Count,
                                   ((LonByte*) &EXPMSG.Data.WriteMemory.Form) + 1
                               )
                            || LonSendResponse(
                                   correlator,
                                   LON_NM_SUCCESS(LonNmWriteMemory),
                                   NULL,
                                   0
                               );
                    break;
#endif      /* LON_DMF_ENABLED */

                case LonNmQuerySiData: {
                    unsigned offset = LON_GET_UNSIGNED_WORD(EXPMSG.Data.QuerySiDataRequest.Offset);
                    unsigned siDataLength = 0;
                    const LonByte* pSiData = LonGetSiData(&siDataLength);

                    if ((EXPMSG.Data.QuerySiDataRequest.Count > LON_MAX_MSG_DATA)
                    || (offset + EXPMSG.Data.QuerySiDataRequest.Count > siDataLength)
                    || (EXPMSG.Data.QuerySiDataRequest.Count > sizeof(ResponseData))) {
                        bFailure = TRUE;
                    } else {
                        memcpy(&ResponseData[0], pSiData + offset, EXPMSG.Data.QuerySiDataRequest.Count);
                        bFailure = LonSendResponse(
                                       correlator,
                                       LON_NM_SUCCESS(LonNmQuerySiData),
                                       &ResponseData[0],
                                       EXPMSG.Data.QuerySiDataRequest.Count
                                   ) != LonApiNoError;
                    }
                }
                break;

                case LonNmWink:
                    /* Process wink network management message        */
                    LonWink();
                    break;

                default:
                    /* Process explicit application messages here.   */
#if    LON_APPLICATION_MESSAGES
#if    LON_EXPLICIT_ADDRESSING
                    LonMsgArrived(
                        &(EXPMSG.Address.Receive), correlator,
                        (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY),
                        (LonServiceType) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE),
                        (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED),
                        (LonApplicationMessageCode) EXPMSG.Code,
                        EXPMSG.Data.Data,
                        (LonByte)(EXPMSG.Length - 1)
                    );
#else    /* ifndef(LON_EXPLICIT_ADDRESSING)    */
                    LonMsgArrived(
                        NULL,
                        correlator,
                        (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY),
                        (LonServiceType) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE),
                        (LonBool) LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED),
                        (LonApplicationMessageCode) EXPMSG.Code,
                        EXPMSG.Data.Data,
                        (LonByte)(EXPMSG.Length - 1)
                    );
#endif    /* LON_EXPLICIT_ADDRESSING            */
#else     /* ifndef(LON_APPLICATION_MESSAGES)    */
                    bFailure = TRUE;
#endif    /* LON_APPLICATION_MESSAGES            */
                    break;
                }
            }

            if (bFailure) {
                /* Indicates that the receiving network management message   */
                /* or explicit message is not supported by the ShortStack,   */
                /* or that it failed to execute that message.                */
                LonSendResponse(
                    correlator,
                    (LonByte)LON_NM_FAILURE(EXPMSG.Code),
                    NULL,
                    0
                );
            }

            break;
        }

        case ((LonByte) LonNiComm | (LonByte) LonNiResponse): {
            if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_COMPLETIONCODE)) {
                /* Process completion event generated by the ShortStack Micro Server */
                if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE) == LonMessageNv) {
                    LonNvUpdateCompleted(
                        NVMSG.Index,
                        (LonBool)(LON_GET_ATTRIBUTE(NVMSG, LON_NVMSG_COMPLETIONCODE) == LonCompletionSuccess)
                    );
                } else {
#if    LON_APPLICATION_MESSAGES
                    LonMsgCompleted(
                        LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG),
                        (LonBool)(LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_COMPLETIONCODE) == LonCompletionSuccess)
                    );
#endif    /* LON_APPLICATION_MESSAGES */
                }
            } else {
                /* Process response from the network. */
                if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE) == LonMessageNv) {
                    /* NV poll response.  Handle same as NV update.
                     * (An offline node will return an NV update with length 0 to
                     * indicate this fact. If all NV updates are returned this way
                     * then a failure completion event is received).
                     */

                    if (VerifyNvIndex(NVMSG.Index) == LonApiNoError) {
                        if (WriteNvLocal(NVMSG.Index, NVMSG.NvData, NVMSG.Length) == LonApiNoError)
#if LON_EXPLICIT_ADDRESSING
                            LonNvUpdateOccurred(NVMSG.Index, &(EXPMSG.Address.Receive));
#else
                            LonNvUpdateOccurred(NVMSG.Index, NULL);
#endif
                    }
                } else {
                    /* Message response. This could be a response to a local NM/ND
                     * message or an explicit message. If the message tag of the
                     * response is NM_ND_TAG, it is the response to the local
                     * NM/ND message.
                     */
                    if (LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG) == NM_ND_TAG) {
#if LON_NM_QUERY_FUNCTIONS

                        if (CurrentNmNdStatus == NM_PENDING) {
                            /* Process the response to a local NM/ND message    */
                            switch ((EXPMSG.Code & LON_NM_OPCODE_MASK) | LON_NM_OPCODE_BASE) {
                            case LonNmQueryDomain:
                                /* Query Domain response    */
                                LonDomainConfigReceived(
                                    (LonDomain*) &(EXPMSG.Data),
                                    (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryDomain))
                                );
                                break;

                            case LonNmQueryNvConfig:

                                /* Query Nv Config response    */
                                if (EXPMSG.Length == sizeof(EXPMSG.Code) + sizeof(LonNvConfigNonEat)) {
                                    LonNvConfig nvConfig;
                                    LonNvConfigNonEat* nvConfigNonEat = (LonNvConfigNonEat*) & (EXPMSG.Data);

                                    memset(&nvConfig, 0, sizeof(nvConfig));
                                    memcpy(&nvConfig, nvConfigNonEat, sizeof(LonNvConfigNonEat));

                                    if ((nvConfig.LON_NV_ADDRESS_FIELD & LON_NV_ADDRESS_MASK) == LON_NV_ADDRESS_MASK) {
                                        nvConfig.LON_NV_ADDRHIGH_FIELD = LON_NV_ADDRHIGH_MASK;
                                    }

                                    LonNvConfigReceived(
                                        &nvConfig,
                                        (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryNvConfig))
                                    );
                                } else {
                                    LonAliasConfig aliasConfig;
                                    LonAliasConfigNonEat* aliasConfigNonEat = (LonAliasConfigNonEat *) & (EXPMSG.Data);

                                    memset(&aliasConfig, 0, sizeof(aliasConfig));
                                    memcpy(&aliasConfig.Alias, &aliasConfigNonEat->Alias, sizeof(LonNvConfigNonEat));

                                    if ((aliasConfig.Alias.LON_NV_ADDRESS_FIELD & LON_NV_ADDRESS_MASK) == LON_NV_ADDRESS_MASK) {
                                        aliasConfig.Alias.LON_NV_ADDRHIGH_FIELD = LON_NV_ADDRHIGH_MASK;
                                    }

                                    aliasConfig.Primary = aliasConfigNonEat->Primary;
                                    LON_SET_UNSIGNED_WORD(aliasConfig.HostPrimary, 0xFFFF);

                                    LonAliasConfigReceived(
                                        &aliasConfig,
                                        (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryNvConfig))
                                    );
                                }

                                break;

                            case LonNmQueryAddr:
                                /* Query Address response    */
                                LonAddressConfigReceived(
                                    (LonAddress*) &(EXPMSG.Data),
                                    (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmQueryAddr))
                                );
                                break;

                            case LonNmReadMemory:
                                /* Read of configuration data response    */
                                LonConfigDataReceived(
                                    (const LonConfigData * const) &(EXPMSG.Data.Data),
                                    (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmReadMemory))
                                );
                                break;

                            case LonNmExpanded:
                                switch (EXPMSG.Data.Data[0]) {
                                case LonExpQueryNvConfig: {
                                    LonNmQueryNvConfigResponseExp* response = (LonNmQueryNvConfigResponseExp*) &(EXPMSG.Data);

                                    LonNvConfigReceived(
                                        &response->Config,
                                        (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmExpanded))
                                    );
                                }
                                break;

                                case LonExpQueryAliasConfig: {
                                    /* Data type translation required. The LonAliasConfig structure
                                     * includes the short and long primary indices, but this response
                                     * only carries the long primary index.
                                     */
                                    LonAliasConfig alias;
                                    LonNmQueryAliasConfigResponseExp* response = (LonNmQueryAliasConfigResponseExp*) &(EXPMSG.Data);

                                    memset(&alias, 0, sizeof(alias));
                                    memcpy(&alias.Alias, &response->Alias, sizeof(alias.Alias));
                                    alias.Primary = response->Primary.lsb;
                                    alias.HostPrimary = response->Primary;
                                    LonAliasConfigReceived(
                                        &alias,
                                        (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNmExpanded))
                                    );
                                }
                                break;
                                }

                                break;

                            default:
                                break;
                            }
                        } else if (CurrentNmNdStatus == ND_PENDING) {
                            /* Process the response to a local NM/ND message    */
                            switch ((EXPMSG.Code & LON_ND_OPCODE_MASK) | LON_ND_OPCODE_BASE) {
                            case LonNdQueryStatus:
                                /* Query Status response */
                                LonStatusReceived(
                                    &(EXPMSG.Data.QueryStatusResponse.Status),
                                    (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNdQueryStatus))
                                );
                                break;

                            case LonNdQueryXcvr:
                                /* Query Transceiver Status response */
                                LonTransceiverStatusReceived(
                                    (const LonTransceiverParameters * const) & (EXPMSG.Data.QueryXcvrStatusResponse.Status),
                                    (LonBool)(EXPMSG.Code == LON_NM_SUCCESS(LonNdQueryXcvr))
                                );
                                break;

                            default:
                                break;
                            }
                        }

#endif  /* LON_NM_QUERY_FUNCTIONS */
                        CurrentNmNdStatus = NO_NM_ND_PENDING;
                    } else {
                        /* Explicit message response. */
#if    LON_APPLICATION_MESSAGES
#if    LON_EXPLICIT_ADDRESSING
                        LonResponseArrived(
                            &(EXPMSG.Address.Response),
                            LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG),
                            (LonApplicationMessageCode) EXPMSG.Code,
                            EXPMSG.Data.Data,
                            (LonByte)(EXPMSG.Length - 1)
                        );
#else
                        LonResponseArrived(
                            NULL,
                            LON_GET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG),
                            (LonApplicationMessageCode) EXPMSG.Code,
                            EXPMSG.Data.Data,
                            (LonByte)(EXPMSG.Length - 1)
                        );
#endif
#endif    /* LON_APPLICATION_MESSAGES    */
                    }
                }
            }

            break;
        }

        case LonNiReset:
            /* The ShortStack Micro Server resets.    */
            /* Reset the serial driver to get back in sync. */
            ++resetCounter;
            LdvReset(ldv_handle);
            CurrentNmNdStatus = NO_NM_ND_PENDING;
            memcpy(
                (void*)&lastResetNotification,
                (LonResetNotification *) pSmipMsg,
                sizeof(lastResetNotification)
            );

            if (LON_GET_ATTRIBUTE(lastResetNotification, LON_RESET_INITIALIZED)) {
                /*
                 * The Micro Server is initialized. Such a reset occurs
                 * when the device is being commissioned (or reset for
                 * diagnostics). Other possible causes for such a reset
                 * include fatal error conditions, including watchdog
                 * timer resets due to excessive noise on the network.
                 */
                LonResetOccurred((LonResetNotification *) pSmipMsg);
            } else {
                /*
                 * The Micro Server is not initialized. Such a reset
                 * occurs when the Micro Server firmware has been
                 * reloaded. In this event, the Micro Server is in
                 * quiet mode and ignores all network communication
                 * until it has been initialized. We'll do this
                 * right here, but complete the processing of this
                 * uplink notification first in order to free the
                 * buffer used by this transaction before allocating
                 * more buffers for the (re-)initialization.
                 * The application will receive a LonResetOccurred()
                 * event at the end of the (re-)initialization,
                 * because this process always concludes with an explicit
                 * reset request.
                 *
                 * Note that the entire device will enter the
                 * unconfigured state after the Micro Server has been
                 * reinitialized in this manner. This is required to
                 * prevent a possibly fatal network misconfiguration.
                 */
                requestReinit = TRUE;
            }

            break;

        case LonNiService:
            /* Service pin was  pressed.*/
            LonServicePinPressed();
            break;

        case LonNiServiceHeld:
            /* Service pin has been held longer than a configurable period of time. */
            /* See ShortStack User's Guide on how to set the period.                */
            LonServicePinHeld();
            break;

#if LON_UTILITY_FUNCTIONS

        case LonNiUsop:

            /* A response to one of the utility functions has arrived. */
            switch (pSmipMsg->Payload[0]) {
            case LonUsopPing:
                LonPingReceived();
                break;

            case LonUsopNvIsBound:
                LonNvIsBoundReceived(pSmipMsg->Payload[1], (LonBool) pSmipMsg->Payload[2]);
                break;

            case LonUsopMtIsBound:
                LonMtIsBoundReceived(pSmipMsg->Payload[1], (LonBool) pSmipMsg->Payload[2]);
                break;

            case LonUsopGoUcfg:
                LonGoUnconfiguredReceived();
                break;

            case LonUsopGoCfg:
                LonGoConfiguredReceived();
                break;

            case LonUsopQueryAppSignature: {
                LonWord appSignature;
                appSignature.msb = pSmipMsg->Payload[1];
                appSignature.lsb = pSmipMsg->Payload[2];
                LonAppSignatureReceived(appSignature);
                break;
            }

            case LonUsopVersion:
                LonVersionReceived(
                    pSmipMsg->Payload[1],
                    pSmipMsg->Payload[2],
                    pSmipMsg->Payload[3],
                    pSmipMsg->Payload[4],
                    pSmipMsg->Payload[5],
                    pSmipMsg->Payload[6]
                );
                break;

            case LonUsopEcho:
                LonEchoReceived(&pSmipMsg->Payload[1]);
                break;
            }

            break;
#endif /* LON_UTILITY_FUNCTIONS */

#if LON_ISI_ENABLED

        case LonIsiNack:
            /* Received a Nack from the Micro Server regarding the Downlink Rpc.*/
            HandleUplinkRpcAck((IsiRpcMessage*) pSmipMsg, FALSE);
            break;

        case LonIsiAck:
            /* Received an Ack from the Micro Server regarding the Downlink Rpc.*/
            HandleUplinkRpcAck((IsiRpcMessage*) pSmipMsg, TRUE);
            break;

        case LonIsiCmd:
            /* Received an uplink Rpc from the Micro Server. */
            HandleUplinkRpc((IsiRpcMessage*) pSmipMsg);
            break;
#endif /* LON_ISI_ENABLED */
        }

        /* Release the receive buffer back to the serial driver. */
        LdvReleaseMsg(ldv_handle, pSmipMsg);
    }

    if (requestReinit) {
        (void)InitMicroServer();
    }
}

/*
 * Function: LonPollNv
 * Polls a bound, polling, input network variable.
 *
 * Parameters:
 * index - local index of the input network variable
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to poll an input network variable. Polling an input network
 * variable causes this device to solicit all devices that have output network
 * variables connected to this input network variable to send their latest value
 * for the corresponding network variable.
 *
 * The function returns LonApiNoError if the message is successfully put in the
 * transmit queue to send. It is accompanied by the <LonNvUpdateCompleted>
 * completion event.
 * Note the successful completion of this function does not indicate the successful
 * arrival of the requested values. The values received in response to this poll
 * are reported by a series of <LonNvUpdateOccurred> callback invocations.
 *
 * LonPollNv operates on bound input network variables that have been declared
 * with the Neuron C *polled* attribute, only. Also, only output network variables
 * that are bound to the input network variable will be received.
 *
 * Note that it is *not* an error to poll an unbound polling input network
 * variable.  If this is done, the application will not receive any
 * LonNvUpdateOccurred() events, but will receive a LonNvUpdateCompleted() event
 * with the �success� parameter set to TRUE.
 */
const LonApiError LonPollNv(const unsigned nvIndex)
{
    LonApiError result = VerifyNvIndex(nvIndex);

    if (result == LonApiNoError) {
        const LonNvDescription* const nvDescription = LonGetNvDescription(nvIndex);

        if (nvDescription->Attributes & LON_NVDESC_OUTPUT_MASK) {
            /* ...which must not be an output */
            result = LonApiNvPollOutputNv;
        } else if (!(nvDescription->Attributes & LON_NVDESC_POLLED_MASK)) {
            /* ...that has been declared with the polled attribute */
            result = LonApiNvPollNotPolledNv;
        } else {
            /* ...and if we have a buffer for this request */
            LonSmipMsg* pSmipMsg = NULL;
            result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

            if (result == LonApiNoError) {
                PrepareNvMessage(pSmipMsg, (LonByte) nvIndex, NULL, 0);
                LON_SET_ATTRIBUTE(NVMSG, LON_NVMSG_NVPOLL, 1);
                result = LdvPutMsg(ldv_handle, pSmipMsg);
            }
        }
    }

    return result;
}

/*
 * Function: LonPropagateNv
 * Propagates the value of a bound output network variable to the network.
 *
 * Parameters:
 * index - the local index of the network variable
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function returns LonApiNoError if the outgoing NV-update message has been
 * buffered by the driver, otherwise, an appropriate error code is returned.
 * See <LonNvUpdateCompleted> for the completion event that accompanies this API.
 */
const LonApiError LonPropagateNv(const unsigned nvIndex)
{
    LonApiError result = VerifyNvIndex(nvIndex);

    if (result == LonApiNoError) {
        const LonNvDescription* const nvDescription = LonGetNvDescription(nvIndex);

        /* Can only propagate output network variables: */
        if (!(nvDescription->Attributes & LON_NVDESC_OUTPUT_MASK)) {
            result = LonApiNvPropagateInputNv;
        } else {
            result = SendNv((LonByte) nvIndex);
        }
    }

    return result;
}

/*
 * Function: LonGetDeclaredNvSize
 * Gets the declared size of a network variable.
 *
 * Parameters:
 * index - the local index of the network variable
 *
 * Returns:
 * Declared initial size of the network variable as defined in the Neuron C
 * model file.
 * Zero if the network variable corresponding to index doesn't exist.
 *
 * Note that this function *could* be called from the LonGetCurrentNvSize()
 * callback.
 */
const unsigned LonGetDeclaredNvSize(const unsigned nvIndex)
{
    unsigned returnSize = 0;
    LonApiError result = VerifyNvIndex(nvIndex);

    if (result == LonApiNoError) {
        const LonNvDescription* const nvDescription = LonGetNvDescription(nvIndex);
        returnSize = nvDescription->DeclaredSize;
    }

    return returnSize;
}

/*
 * Function: LonGetNvValue
 * Returns a pointer to the network variable value.
 *
 * Parameters:
 * index - the index of the network variable
 *
 * Returns:
 * Pointer to the network variable value, or NULL if invalid.
 *
 * Remarks:
 * Use this function to obtain a pointer to the network variable value.
 * Typically ShortStack applications use the global variable created by
 * the LonTalk Interface Developer directly when accessing network variable
 * values. This function can be used to obtain a pointer to the network variable
 * value.
 */
volatile void* const LonGetNvValue(const unsigned nvIndex)
{
    volatile void* p = NULL;
    LonApiError result = VerifyNvIndex(nvIndex);

    if (result == LonApiNoError) {
        const LonNvDescription* const nvDescription = LonGetNvDescription(nvIndex);
        p = nvDescription->pData;
    }

    return p;
}

/*
 * Function: LonSendResponse
 * Sends a response.
 *
 * Parameters:
 * correlator - message correlator obtained with <LonMsgArrived>
 * code - response message code
 * pData - pointer to response data, can be NULL iff len is zero
 * length - number of valid response data bytes in pData
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function is called to send an explicit message response.  The correlator
 * is passed in to <LonMsgArrived> and must be copied and saved if the response
 * is to be sent after returning from that routine.  A response code should be
 * in the 0x00..0x2f range.
 */
const LonApiError LonSendResponse(
    const LonCorrelator correlator,
    const LonByte code,
    const LonByte* const pData,
    const unsigned length
)
{
    LonApiError result = LonApiNoError;

    if (length > LON_MAX_MSG_DATA) {
        /* Returns failure if the response data is too big */
        result = LonApiMsgLengthTooLong;
    } else if ((LonServiceType) LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_SERVICE) != LonServiceRequest) {
        /* Send response only if the incoming message has service type request/response */
        result = LonApiMsgNotRequest;
    } else {
        LonSmipMsg* pSmipMsg = NULL;

        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            /* OK, construct and post the response: */

            pSmipMsg->Header.Command =
                (LonSmipCmd) (LonNiComm |
                    (LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_PRIORITY) ?
                        (LonByte) LonNiNonTxQueuePriority : (LonByte) LonNiNonTxQueue));
            pSmipMsg->Header.Length = (LonByte)(sizeof(LonExplicitMessage) - sizeof(EXPMSG.Data) + length);

            EXPMSG.Length = (LonByte)(length + 1);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_RESPONSE, 1);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_TAG));
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY, LON_GET_ATTRIBUTE(correlator, LON_CORRELATOR_PRIORITY));
            EXPMSG.Code = code;

            memcpy(&(EXPMSG.Data.Data), pData, length);

            result = LdvPutMsg(ldv_handle, pSmipMsg);
        }
    }

    return result;
}

/*
 * Function: LonGetUniqueId
 * Returns the unique ID (Neuron ID).
 *
 * Parameters:
 * pId   - pointer to the buffer to hold the unique ID
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to obtain the local Micro Server's unique ID, if available.
 * The information might not be available immediately following a reset, prior to
 * successful completion of the initialization sequence, or following an
 * asynchronous reset of the host processor.
 *
 * See also the <LonGetLastResetNotification> function for alternative access
 * to the same data.
 *
 * The *Unique ID* is also known as the *Neuron ID*, however, *Neuron ID* is a
 * deprecated term.
 *
 * Previously named lonGetNeuronId.
 */
const LonApiError LonGetUniqueId(LonUniqueId* const pNid)
{
    LonApiError result = LonApiNeuronIdNotAvailable;

    if (pNid != NULL && lastResetNotification.Version != 0xFF) {
        memcpy(pNid, (void*) & (lastResetNotification.UniqueId), sizeof(LonUniqueId));
        result = LonApiNoError;
    }

    return result;
}

/*
 * Function: LonGetVersion
 * Provides the link layer protocol version number.
 *
 * Parameters:
 * pVersion - pointer to hold the version number
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * The function provides the link layer protocol version number into the location
 * pointed to by *pVersion*, if available.
 * The information may not be available immediately following a reset, prior
 * to successful completion of the initialization sequence, or following
 * an asynchronous reset of the host processor.
 *
 * See also the <LonGetLastResetNotification> function for alternative access
 * to the same data.
 */
const LonApiError LonGetVersion(LonByte* const pVersion)
{
    LonApiError result = LonApiVersionNotAvailable;

    if (pVersion != NULL && lastResetNotification.Version != 0xFF) {
        *pVersion = lastResetNotification.Version;
        result = LonApiNoError;
    }

    return result;
}

/*
 * Function: LonSendServicePin
 * Propagates a service pin message.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to propagate a service pin message to the network.
 * The function will fail if the device is not yet fully initialized.
 */
const LonApiError LonSendServicePin(void)
{
    return SendLocal(LonNiService, NULL, 0);
}

/*
 * Function: LonSendReset
 * Sends a reset message.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to send a reset message to the Micro Server.
 * The function will fail if the device is not yet fully initialized.
 */
const LonApiError LonSendReset(void)
{
    return SendLocal(LonNiReset, NULL, 0);
}

/*
 * Function: LonGetLastResetNotification
 * Returns a pointer to the most recent reset notification, if any.
 *
 * Returns:
 * Pointer to <LonResetNotification>, reporting the most recent reset notification
 * buffered by the API. The returned pointer is NULL if such data is not available.
 *
 * Remarks:
 * The <LonReset> callback occurs when the Micro Server reports a reset, but the
 * <LonResetNotification> data is buffered by the API for future reference.
 * This is used by the API itself, but the application can query this buffer
 * through the <LonGetLastResetNotification> function. This function delivers
 * a superset of the information from the <LonGetUniqueId> and <LonGetVersion>
 * functions.
 */
const volatile LonResetNotification* const LonGetLastResetNotification(void)
{
    return &lastResetNotification;
}

#if LON_APPLICATION_MESSAGES
/*
 * Function: LonSendMsg
 * Send an explicit (non-NV) message.
 *
 * Parameters:
 * tag - message tag for this message
 * priority - priority attribute of the message
 * serviceType - service type for use with this message
 * authenticated - TRUE to use authenticated service
 * pDestAddr - pointer to destination address
 * code - message code
 * pData - message data, is NULL if length is zero
 * length - number of valid bytes available through pData
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function is called to send an explicit message.  For application messages,
 * the message code should be in the range of 0x00..0x2f.  Codes in the 0x30..0x3f
 * range are reserved for protocols such as file transfer.
 *
 * If the tag field specifies one of the bindable messages tags (tag < #
 * bindable message tags), the pDestAddr is ignored (and can be NULL) because the
 * message is sent using implicit addressing.
 *
 * A successful return from this function indicates only that the message has
 * been queued to be sent.  If this function returns success, the ShortStack
 * LonTalk Compact API will call <LonMsgCompleted> with an indication of the
 * transmission success.
 *
 * If the message is a request, <LonResponseArrived> callback handlers are
 * called when corresponding responses arrive.
 *
 * The device must be configured before your application calls this function.
 * If the device is unconfigured, the function will seem to
 * work:  the application will receive a successful completion event (because
 * the API will successfully pass the request to the Micro Server), but there
 * will be no effect, and the application will not receive a callback (if any).
 */
const LonApiError LonSendMsg(
    const unsigned tag,
    const LonBool priority,
    const LonServiceType serviceType,
    const LonBool authenticated,
    const LonSendAddress* const pDestAddr,
    const LonByte code,
    const LonByte* const pData,
    const unsigned length
)
{
    LonApiError result = LonApiNoError;
    LonSmipQueue queue;

    if (length > LON_MAX_MSG_DATA) {
        /* Returns failure if the message data is too big    */
        result = LonApiMsgLengthTooLong;
    } else if ((LonMtCount == 0) || (tag > LonMtCount - 1) || (tag == NM_ND_TAG)) {
        /* LonMtCount contains the number of message tags    */
        /* declared in the Neuron C model file. tag          */
        /* must range from 0 to min(LonMtCount-1, ND_TAG-1). */
        result =  LonApiMsgInvalidMsgTag;
    } else {
        LonSmipMsg* pSmipMsg = NULL;

        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            queue = (serviceType == LonServiceUnacknowledged) ?
                    (priority ? LonNiNonTxQueuePriority : LonNiNonTxQueue) :
                    (priority ? LonNiTxQueuePriority : LonNiTxQueue);

            pSmipMsg->Header.Command = (LonSmipCmd) ((LonByte) LonNiComm | queue);

            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, serviceType);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, tag);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, authenticated);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_MSGTYPE, LonMessageExplicit);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_PRIORITY, priority);
            EXPMSG.Length = (LonByte)(length + 1);
            EXPMSG.Code = code;

            memcpy(EXPMSG.Data.Data, (void *)pData, length);

#if    LON_EXPLICIT_ADDRESSING

            if (pDestAddr != NULL) {
                memcpy(&(EXPMSG.Address), (void *)pDestAddr, sizeof(LonSendAddress));
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_EXPLICITADDR, 1);
            }

#endif

            pSmipMsg->Header.Length = (LonByte) (sizeof(LonExplicitMessage) - sizeof(EXPMSG.Data) + length);
            result = LdvPutMsg(ldv_handle, pSmipMsg);
        }
    }

    return result;
}
#endif    /* LON_APPLICATION_MESSAGES    */

#if LON_NM_QUERY_FUNCTIONS
/*
 * Function: LonQueryDomainConfig
 * Request a copy of a local domain table record.
 *
 * Parameters:
 * index - index of requested domain table record (0, 1)
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of a local domain table record.
 * This is an asynchronous API. This function returns immediately.
 * The domain information will later be delivered to the <LonDomainConfigReceived>
 * callback.
 * This function is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryDomainConfig(const unsigned index)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else if (index > lastResetNotification.MaxDomains - 1u) {
        result = LonApiIndexInvalid;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryDomainRequest));

            EXPMSG.Code = LonNmQueryDomain;
            EXPMSG.Length = (LonByte)(sizeof(LonNmQueryDomainRequest) + 1);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
            EXPMSG.Data.QueryDomainRequest.Index = (LonByte) index;

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonQueryNvConfig
 * Request a copy of network variable configuration data.
 *
 * Parameters:
 * index - index of requested NV configuration table entry
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's
 * network variable configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonNvConfigReceived>
 * callback. This function is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryNvConfig(const unsigned index)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        result = VerifyNvIndex(index);

        if (result == LonApiNoError) {
            LonSmipMsg* pSmipMsg = NULL;
            result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

            if (result == LonApiNoError) {
                if (lastResetNotification.LON_RESET_EAT_FIELD & LON_RESET_EAT_MASK) {
                    /* The Micro Server supports an extended address table and will
                     * require the extended LonQueryNvConfigRequest.
                     */
                    pSmipMsg->Header.Command = LonNiNetManagement;
                    EXPMSG.Code = LonNmExpanded;
                    LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                    LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                    LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                    pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryNvConfigRequestExp));
                    EXPMSG.Length = (LonByte)(1 + sizeof(LonNmQueryNvConfigRequestExp));
                    EXPMSG.Data.QueryNvConfigRequestExp.Subcode = LonExpQueryNvConfig;
                    EXPMSG.Data.QueryNvConfigRequestExp.Index.msb = 0;
                    EXPMSG.Data.QueryNvConfigRequestExp.Index.lsb = (LonByte)index;
                } else {
                    /* The Micro Server does not support the extended address table
                     * and expects a classic LonNmQueryNvAliasRequest request.
                     */
                    pSmipMsg->Header.Command = LonNiNetManagement;
                    EXPMSG.Code = LonNmQueryNvConfig;
                    LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                    LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                    LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                    if (index < 255) {
                        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonByte));
                        EXPMSG.Length = (LonByte)(sizeof(LonByte) + 1);
                        EXPMSG.Data.QueryNvAliasRequest.Index = (LonByte) index;
                    } else {
                        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryNvAliasRequest));
                        EXPMSG.Length = (LonByte)(sizeof(LonNmQueryNvAliasRequest) + 1);
                        EXPMSG.Data.QueryNvAliasRequest.Index = 255;
                        LON_SET_UNSIGNED_WORD(EXPMSG.Data.QueryNvAliasRequest.LongIndex, index);
                    }
                }

                result = LdvPutMsg(ldv_handle, pSmipMsg);

                if (result == LonApiNoError) {
                    CurrentNmNdStatus = NM_PENDING;
                }
            }
        }
    }

    return result;
}

/*
 * Function: LonQueryAliasConfig
 * Request a copy of alias configuration data.
 *
 * Parameters:
 * index - index of requested alias config table entry
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's
 * alias configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonAliasConfigReceived>
 * callback. This function is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryAliasConfig(const unsigned index)
{
    LonApiError result = LonApiNoError;
    unsigned queryIndex = index + LonNvCount;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else if (index > lastResetNotification.MaxAliases - 1u) {
        result = LonApiIndexInvalid;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            if (lastResetNotification.LON_RESET_EAT_FIELD & LON_RESET_EAT_MASK) {
                /* The Micro Server supports an extended address table and will
                 * require the extended LonQueryAliasConfigRequest.
                 */
                pSmipMsg->Header.Command = LonNiNetManagement;
                EXPMSG.Code = LonNmExpanded;
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryAliasConfigRequestExp));
                EXPMSG.Length = (LonByte)(1 + sizeof(LonNmQueryAliasConfigRequestExp));
                EXPMSG.Data.QueryAliasConfigRequestExp.Subcode = LonExpQueryAliasConfig;
                EXPMSG.Data.QueryAliasConfigRequestExp.Index.msb = 0;
                EXPMSG.Data.QueryAliasConfigRequestExp.Index.lsb = (LonByte)index;
            } else {
                /* The Micro Server does not support the extended address table
                 * and expects a classic LonNmQueryNvAliasRequest request.
                 */
                pSmipMsg->Header.Command = LonNiNetManagement;
                EXPMSG.Code = LonNmQueryNvConfig;
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                if (queryIndex < 255u) {
                    pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonByte));
                    EXPMSG.Length = (LonByte)(sizeof(LonByte) + 1);
                    EXPMSG.Data.QueryNvAliasRequest.Index = (LonByte)queryIndex;
                } else {
                    pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryNvAliasRequest));
                    EXPMSG.Length = (LonByte)(sizeof(LonNmQueryNvAliasRequest) + 1);
                    EXPMSG.Data.QueryNvAliasRequest.Index = 255u;
                    LON_SET_UNSIGNED_WORD(EXPMSG.Data.QueryNvAliasRequest.LongIndex, queryIndex);
                }
            }

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonQueryAddressConfig
 * Request a copy of address table configuration data.
 *
 * Parameters:
 * index - index of requested address table entry
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's
 * address table configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonAddressConfigReceived>
 * callback. This function is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryAddressConfig(const unsigned index)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else if (index > lastResetNotification.MaxAddresses - 1u) {
        result = LonApiIndexInvalid;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmQueryAddressRequest));

            EXPMSG.Code = LonNmQueryAddr;
            EXPMSG.Length = (LonByte)(sizeof(LonNmQueryAddressRequest) + 1);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
            EXPMSG.Data.QueryAddressRequest.Index = (LonByte) index;

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonQueryConfigData
 * Request a copy of local configuration data.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request a copy of the local Micro Server's
 * configuration data.
 * This is an asynchronous API. This function returns immediately.
 * The configuration data will later be delivered to the <LonConfigDataReceived>
 * callback. This function is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryConfigData(void)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD + sizeof(LonNmReadMemoryRequest);

            EXPMSG.Code = LonNmReadMemory;
            EXPMSG.Length = (LonByte)(sizeof(LonNmReadMemoryRequest) + 1);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
            EXPMSG.Data.ReadMemory.Mode = LonConfigStructRelative;
            EXPMSG.Data.ReadMemory.Count = (LonByte) sizeof(LonConfigData);
            LON_SET_UNSIGNED_WORD(EXPMSG.Data.ReadMemory.Address, 0);

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonQueryStatus
 * Request local status and statistics.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to obtain the local status and statistics of the ShortStack
 * device. This is an asynchronous API. This function returns immediately.
 * The data will later be delivered to the <LonStatusReceived> callback.
 * This function is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 */
const LonApiError LonQueryStatus(void)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD;

            EXPMSG.Code = LonNdQueryStatus;
            EXPMSG.Length = 1;
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonQueryTransceiverStatus
 * Request local transceiver status information.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:2
 * Call this function to query the local transceiver status for the Micro Server.
 * This is an asynchronous API. This function returns immediately.
 * The transceiver status will later be delivered to the <LonTransceiverStatusReceived>
 * callback.
 * This function is part of the optional network management query API
 * (LON_NM_QUERY_FUNCTIONS).
 *
 * This function works only for a Power Line transceiver. If your application
 * calls this function for any other transceiver type, the function will seem to
 * work, but the corresponding callback handler will declare a failure.
 */
const LonApiError LonQueryTransceiverStatus(void)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD;

            EXPMSG.Code = LonNdQueryXcvr;
            EXPMSG.Length = 1;
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);   /* Needed for node with NM authentication */

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}
#endif /* LON_NM_QUERY_FUNCTIONS */

#if LON_NM_UPDATE_FUNCTIONS
/*
 * Function: LonSetNodeMode
 * Sets the ShortStack Micro Server's mode and state.
 *
 * Parameters:
 * mode - mode of the Micro Server, see <LonNodeMode>
 * state - state of the Micro Server, see <LonNodeState>
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to set the Micro Server's mode, state, or both.
 * If the mode parameter is *LonChangeState*, then the state parameter can be
 * set to either *LonConfigOffLine* or *LonConfigOnLine*.  Otherwise the state
 * parameter should be *LonStateInvalid* (0).  Note that while the <LonNodeState>
 * enumeration is used to report both the state and the mode (see <LonStatus>),
 * it is *not* possible to change both the state and mode (online/offline) at
 * the same time.
 *
 * This function is part of the optional network management update API
 * (LON_NM_UPDATE_FUNCTIONS).

 * You can also use the shorthand functions <LonGoOnline> and <LonGoOffline>.
 *
 * The device must be configured before your application calls this function.
 * If the device is unconfigured, the function will seem to work because the API
 * will successfully pass the request to the Micro Server, but there will be no
 * effect, and the application will not receive a callback (if any).
 */
const LonApiError LonSetNodeMode(LonNodeMode mode, LonNodeState state)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if(result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD + sizeof(LonNmSetNodeModeRequest);

            EXPMSG.Code = LonNmSetNodeMode;
            EXPMSG.Length = sizeof(LonNmSetNodeModeRequest) + 1;
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
            EXPMSG.Data.NodeMode.Mode = (LonNodeMode) mode;
            EXPMSG.Data.NodeMode.State = (LonNodeState) state;

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonUpdateAddressConfig
 * Updates an address table record on the Micro Server.
 *
 * Parameters:
 * index - address table index to update
 * pAddress - pointer to address table record
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to write a record to the local address table.
 * This function is part of the optional network management update API
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateAddressConfig(const unsigned index, const LonAddress* pAddress)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else if (index > lastResetNotification.MaxAddresses - 1u) {
        result = LonApiIndexInvalid;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateAddressRequest);

            EXPMSG.Code = LonNmUpdateAddr;
            EXPMSG.Length = sizeof(LonNmUpdateAddressRequest) + 1;
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
            EXPMSG.Data.UpdateAddressRequest.Index = (LonByte) index;
            EXPMSG.Data.UpdateAddressRequest.Address = *pAddress;

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonUpdateAliasConfig
 * Updates a local alias table record.
 *
 * Parameters:
 * index - index of alias table record to update
 * pAlias - pointer to the alias table record
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function writes a record in the local alias table.
 * This function is part of the optional network management update API
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateAliasConfig(const unsigned index, const LonAliasConfig* pAlias)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else if (index > lastResetNotification.MaxAliases - 1u) {
        result = LonApiIndexInvalid;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if(result == LonApiNoError) {
            if (lastResetNotification.LON_RESET_EAT_FIELD & LON_RESET_EAT_MASK) {
                /*
                 * This Micro Server supports an extended address table and requires
                 * the LonNmExpanded.LonExpUpdateAliasConfig message to update a
                 * network variable configuration table record.
                 */
                pSmipMsg->Header.Command = LonNiNetManagement;
                pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateAliasConfigRequestExp));

                EXPMSG.Code = LonNmExpanded;
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                EXPMSG.Length = (LonByte)(1 + sizeof(LonNmUpdateAliasConfigRequestExp));
                EXPMSG.Data.UpdateAliasConfigRequestExp.Subcode = LonExpUpdateAliasConfig;
                EXPMSG.Data.UpdateAliasConfigRequestExp.Index.msb = 0;
                EXPMSG.Data.UpdateAliasConfigRequestExp.Index.lsb = (LonByte)index;
                memcpy(
                    &EXPMSG.Data.UpdateAliasConfigRequestExp.Alias,
                    (LonNvConfig*)pAlias,
                    sizeof(EXPMSG.Data.UpdateAliasConfigRequestExp.Alias)
                );
                LON_SET_UNSIGNED_WORD(EXPMSG.Data.UpdateAliasConfigRequestExp.Primary, pAlias->Primary);
            } else {
                /*
                 * This Micro Server does not support an extended address table and
                 * requires the classic message format to update a network variable
                 * configuration table record.
                 */
                unsigned actualIndex = index + LonNvCount;

                /*
                 * We must translate the alias configuration data structure back to
                 * its classic form to support non-EAT aware Micro Servers.
                 */
                LonAliasConfigNonEat classicAlias;
                memcpy(
                    &classicAlias,
                    pAlias,
                    sizeof(LonAliasConfigNonEat)
                );
                classicAlias.Primary = (LonByte)pAlias->Primary;

                pSmipMsg->Header.Command = LonNiNetManagement;
                pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateAliasRequest));

                EXPMSG.Code = LonNmUpdateNvConfig;
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                if (actualIndex < 255) {
                    /* Use the short form of the request */
                    EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateAliasRequest*)0x0)->Request.ShortForm) + 1);
                    EXPMSG.Data.UpdateAliasRequest.ShortIndex = (LonByte) actualIndex;
                    memcpy(
                        &EXPMSG.Data.UpdateAliasRequest.Request.ShortForm.AliasConfig,
                        &classicAlias,
                        sizeof(EXPMSG.Data.UpdateAliasRequest.Request.ShortForm.AliasConfig)
                    );
                } else {
                    /* Use the long form of the request */
                    EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateAliasRequest*)0x0)->Request.LongForm) + 1);
                    EXPMSG.Data.UpdateAliasRequest.ShortIndex = 255;
                    LON_SET_UNSIGNED_WORD(EXPMSG.Data.UpdateAliasRequest.Request.LongForm.LongIndex, actualIndex);
                    memcpy(
                        &EXPMSG.Data.UpdateAliasRequest.Request.LongForm.AliasConfig,
                        &classicAlias,
                        sizeof(EXPMSG.Data.UpdateAliasRequest.Request.LongForm.AliasConfig)
                    );
                }
            }

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonUpdateConfigData
 * Updates the configuration data on the Micro Server.
 *
 * Parameters:
 * pConfig - pointer to the <LonConfigData> configuration data
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to update the Micro Server's configuration data based on
 * the configuration stored in the <LonConfigData> structure.
 * This function is part of the optional network management update API
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateConfigData(const LonConfigData* const pConfigData)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmWriteMemoryRequest) + sizeof(LonConfigData));

            EXPMSG.Code = LonNmWriteMemory;
            EXPMSG.Length = (LonByte) (sizeof(LonNmWriteMemoryRequest) + sizeof(LonConfigData) + 1);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
            EXPMSG.Data.WriteMemory.Mode = LonConfigStructRelative;         /* Configuration address relative memory write */
            LON_SET_UNSIGNED_WORD(EXPMSG.Data.WriteMemory.Address, 0);
            EXPMSG.Data.WriteMemory.Count = (LonByte) sizeof(LonConfigData);
            EXPMSG.Data.WriteMemory.Form = LonConfigCsRecalculationReset;   /* recalculate just configuration checksum     */

            memcpy(
                (LonByte*) &(EXPMSG.Data) + sizeof(LonNmWriteMemoryRequest),
                pConfigData,
                sizeof(LonConfigData)
            );

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function:   LonUpdateDomainConfig
 * Updates a domain table record in the Micro Server.
 *
 * Parameters:
 * index - the index of the domain table to update
 * pDomain - pointer to the domain table record
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function can be used to update one record of the domain table.
 * This function is part of the optional network management update API
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateDomainConfig(const unsigned index, const LonDomain* const pDomain)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else if (index > lastResetNotification.MaxDomains - 1u) {
        result = LonApiIndexInvalid;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateDomainRequest));

            EXPMSG.Code = LonNmUpdateDomain;
            EXPMSG.Length = (LonByte)(sizeof(LonNmUpdateDomainRequest) + 1);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);
            EXPMSG.Data.UpdateDomainRequest.Index = (LonByte)index;
            EXPMSG.Data.UpdateDomainRequest.Domain = *pDomain;

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}

/*
 * Function: LonUpdateNvConfig
 * Updates a network variable configuration table record in the Micro Server.
 *
 * Parameter:
 * index - index of network variable
 * pNvConfig - network variable configuration
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function can be used to update one record of the network variable
 * configuration table.
 * This function is part of the optional network management update API
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonUpdateNvConfig(const unsigned index, const LonNvConfig* const pNvConfig)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        result = VerifyNvIndex(index);

        if (result == LonApiNoError) {
            if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
                result = LonApiNmNdAlreadyPending;
            } else {
                LonSmipMsg* pSmipMsg = NULL;
                result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

                if (result == LonApiNoError) {
                    if (lastResetNotification.LON_RESET_EAT_FIELD & LON_RESET_EAT_MASK) {
                        /*
                         * This Micro Server supports an extended address table and requires
                         * the LonNmExpanded.LonExpUpdateNvConfig message to update a
                         * network variable configuration table record.
                         */
                        pSmipMsg->Header.Command = LonNiNetManagement;
                        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateNvConfigRequestExp));

                        EXPMSG.Code = LonNmExpanded;
                        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                        EXPMSG.Length = (LonByte)(1 + sizeof(LonNmUpdateNvConfigRequestExp));
                        EXPMSG.Data.UpdateNvConfigRequestExp.Subcode = LonExpUpdateNvConfig;
                        EXPMSG.Data.UpdateNvConfigRequestExp.Index.msb = 0;
                        EXPMSG.Data.UpdateNvConfigRequestExp.Index.lsb = (LonByte)index;
                        EXPMSG.Data.UpdateNvConfigRequestExp.Config = *pNvConfig;
                    } else {
                        /*
                         * This Micro Server does not support an extended address table and
                         * requires the classic message format to update a network variable
                         * configuration table record.
                         */
                        pSmipMsg->Header.Command = LonNiNetManagement;
                        pSmipMsg->Header.Length = (LonByte)(LON_SICB_MIN_OVERHEAD + sizeof(LonNmUpdateNvRequest));

                        EXPMSG.Code = LonNmUpdateNvConfig;
                        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
                        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
                        LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

                        if (index < 255) { /* index should always be <255 as this ShortStack only supports up to 254 network variables. */
                            /* Use the short form of the request */
                            EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateNvRequest*)0x0)->Request.ShortForm) + 1);
                            EXPMSG.Data.UpdateNvRequest.ShortIndex = (LonByte) index;
                            memcpy(
                                &EXPMSG.Data.UpdateNvRequest.Request.ShortForm.NvConfig,
                                (LonNvConfigNonEat*)pNvConfig,
                                sizeof(EXPMSG.Data.UpdateNvRequest.Request.ShortForm.NvConfig)
                            );
                        } else {
                            /* Use the long form of the request */
                            EXPMSG.Length = (LonByte) (sizeof(LonByte) + sizeof(((LonNmUpdateNvRequest*)0x0)->Request.LongForm) + 1);
                            EXPMSG.Data.UpdateNvRequest.ShortIndex = 255;
                            LON_SET_UNSIGNED_WORD(EXPMSG.Data.UpdateNvRequest.Request.LongForm.LongIndex, index);
                            memcpy(
                                &EXPMSG.Data.UpdateNvRequest.Request.LongForm.NvConfig,
                                (LonNvConfigNonEat*)pNvConfig,
                                sizeof(EXPMSG.Data.UpdateNvRequest.Request.LongForm.NvConfig)
                            );
                        }
                    }

                    result = LdvPutMsg(ldv_handle, pSmipMsg);

                    if (result == LonApiNoError) {
                        CurrentNmNdStatus = NM_PENDING;
                    }
                }
            }
        }
    }

    return result;
}

/*
 * Function: LonClearStatus
 * Clears the status statistics on the Micro Server.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * This function can be used to clear the Micro Server's status and statistics
 * records.
 * This function is part of the optional network management update API
 * (LON_NM_UPDATE_FUNCTIONS).
 */
const LonApiError LonClearStatus(void)
{
    LonApiError result = LonApiNoError;

    if (CurrentNmNdStatus != NO_NM_ND_PENDING) {
        result = LonApiNmNdAlreadyPending;
    } else {
        LonSmipMsg* pSmipMsg = NULL;
        result = LdvAllocateMsg(ldv_handle, &pSmipMsg);

        if (result == LonApiNoError) {
            pSmipMsg->Header.Command = LonNiNetManagement;
            pSmipMsg->Header.Length = LON_SICB_MIN_OVERHEAD;

            EXPMSG.Code = LonNdClearStatus;
            EXPMSG.Length = 1;
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_SERVICE, LonServiceRequest);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_TAG, NM_ND_TAG);
            LON_SET_ATTRIBUTE(EXPMSG, LON_EXPMSG_AUTHENTICATED, TRUE);

            result = LdvPutMsg(ldv_handle, pSmipMsg);

            if (result == LonApiNoError) {
                CurrentNmNdStatus = NM_PENDING;
            }
        }
    }

    return result;
}
#endif    /* LON_NM_UPDATE_FUNCTIONS */

#if LON_UTILITY_FUNCTIONS
/*
 * Function: LonSendPing
 * Sends a ping command to the ShortStack Micro Server.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to send a ping command to the Micro Server
 * to verify that communications with the Micro Server are functional.
 * The <LonPingReceived> callback handler function processes the reply to the query.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonSendPing(void)
{
    /*
     * This is a one-byte command, but the payload must always be at least two
     * bytes to support detection of write-collissions when using SPI for link-
     * layer communications, so we include a second dummy byte.
     */
    LonByte data[2] = {LonUsopPing, 0};
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonNvIsBound
 * Sends a command to the ShortStack Micro Server to query whether
 * a given network variable is bound.
 *
 * Parameter:
 * index - index of network variable
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to query whether the given network variable is bound.
 * You can use this function to ensure that transactions are initiated only for
 * connected network variables.  The <LonNvIsBoundReceived> callback handler
 * function processes the reply to the query.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonNvIsBound(const unsigned index)
{
    LonApiError result = VerifyNvIndex(index);

    if (result == LonApiNoError) {
        LonByte data[2] = {LonUsopNvIsBound, (LonByte) index};
        result = SendLocal(LonNiUsop, data, 2);
    }

    return result;
}

/*
 * Function: LonMtIsBound
 * Sends a command to the ShortStack Micro Server to query whether
 * a given message tag is bound.
 *
 * Parameter:
 * index - index of network variable
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Use this function to query whether the given message tag is bound or not.
 * You can use this function to ensure that transactions are initiated only for
 * connected message tags.  The <LonMtIsBoundReceived> callback handler function
 * processes the reply to the query.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonMtIsBound(const unsigned index)
{
    LonApiError result = LonApiNoError;

    if ((LonMtCount == 0) || (index > LonMtCount - 1)) {
        result = LonApiIndexInvalid;
    } else {
        LonByte data[2] = {LonUsopMtIsBound, (LonByte) index};
        result = SendLocal(LonNiUsop, data, 2);
    }

    return result;
}

/*
 * Function: LonGoUnconfigured
 * Puts the local node into the unconfigured state.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to put the Micro Server into the unconfigured state.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonGoUnconfigured(void)
{
    /* Payload length should be at least two; so send a dummy byte. */
    LonByte data[2] = {LonUsopGoUcfg, 0};
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonGoConfigured
 * Puts the local node into the configured state.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to put the Micro Server in the configured state and online
 * mode.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonGoConfigured(void)
{
    /* Payload length should be at least two; so send a dummy byte. */
    LonByte data[2] = {LonUsopGoCfg, 0};
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonQueryAppSignature
 * Queries the Micro Server's current version of the host application signature.
 *
 * Parameter:
 * bInvalidate - flag to indicate whether to invalidate the signature
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to query the Micro Server's current version of the host
 * application signature. If the bInvalidate flag is TRUE, the Micro Server
 * invalidates its copy of the signature *AFTER* reporting it to the host.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonQueryAppSignature(LonBool bInvalidate)
{
    LonByte data[2] = {
        LonUsopQueryAppSignature,
        bInvalidate ? 1 : 0
    };
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonQueryVersion
 * Request version details from the Micro Server
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Call this function to request the Micro Server application and core library
 * version numbers, both delivered as triplets of major version, minor version
 * and build number (one byte each) through the <LonVersionReceived> callback.
 * This function is part of the optional utility API (LON_UTILITY_FUNCTIONS).
 */
const LonApiError LonQueryVersion()
{
    const LonByte data[2] = {LonUsopVersion, 0};
    return SendLocal(LonNiUsop, data, 2);
}

/*
 * Function: LonRequestEcho
 * Sends arbitrary test data to the Micro Server and requests a modulated echo.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * LonRequestEcho transmits LON_ECHO_SIZE (6) bytes of arbitrary data, chosen by
 * the caller of the function, to the Micro Server. The Micro Server returns these
 * data bytes to the host, but increments each byte by one, using unsigned 8-bit
 * addition without overflow. When the response is received, the API activates the
 * <LonEchoReceived> callback.
 */
const LonApiError LonRequestEcho(const LonByte data[LON_ECHO_SIZE])
{
    LonByte payload[1 + LON_ECHO_SIZE] = {LonUsopEcho};
    memcpy(&payload[1], data, LON_ECHO_SIZE);
    return SendLocal(LonNiUsop, payload, 1 + LON_ECHO_SIZE);
}

/*
 * Function: LonSetPostResetPause
 * Configures the Micro Server's post-reset pause as 0 (zero, disabled) or in
 * the 1..255ms range.
 *
 * Returns:
 * <LonApiError>.
 *
 * Remarks:
 * Starting with release 4.30, the Micro Server enforces a pause immediately
 * after it transmits an uplink reset notification message. This pause is 50ms
 * by default. During this pause, the Micro Server does nothing. This allows
 * the host time to receive and process the reset notification, which may
 * require resetting the host-side link layer driver.
 *
 * This API may be used to disable the post-reset pause (set to zero), or to
 * select a pause duration in the 1..255 ms range. The Micro Server stores the
 * value in on-chip EEPROM memory, so this only needs setting once.
 */
const LonApiError LonSetPostResetPause(LonByte duration)
{
    LonByte payload[] = { LonUsopSetPostResetPause, duration };
    return SendLocal(LonNiUsop, payload, sizeof(payload));
}   /* LonSetPostResetPause */



#endif /* LON_UTILITY_FUNCTIONS */

const LonApiError LonResume()
{
    return LdvResume(ldv_handle);
}

const LonApiError LonSuspend(unsigned mode, unsigned timeout)
{
    return LdvSuspend(ldv_handle, mode, timeout);
}
