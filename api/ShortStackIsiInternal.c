/*
 * Filename: ShortStackIsiInternal.c
 *
 * Description: This file contains the implementation for internal
 * functions used by the ShortStack ISI API.
 *
 * Copyright (c) Echelon Corporation 2008-2015.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

#include <string.h>

#include "ShortStackDev.h"

#if	LON_ISI_ENABLED
#include "ShortStackIsiApi.h"
#include "ShortStackApi.h"
#include "ldv.h"

static LonByte IsiSequenceNumber = 0x80;

extern LdvHandle ldv_handle;

/*
 * Internal functions
 */
LonApiError SendDownlinkRpc(IsiDownlinkRpcCode code, LonByte param1, LonByte param2, void* pData, unsigned len);
void HandleDownlinkRpcAck(IsiRpcMessage* pMsg, LonBool bSuccess);
void HandleUplinkRpc(IsiRpcMessage* pMsg);

/*
 * SendDownlinkRpc
 *
 * This function handles making an ISI call down to the Micro Server.
 * It returns an error if a buffer can't be allocated.
 */
LonApiError SendDownlinkRpc(IsiDownlinkRpcCode code, LonByte param1, LonByte param2, void* pData, unsigned len)
{
    IsiRpcMessage* pMsg = NULL;
    LonApiError result = LdvAllocateMsg(ldv_handle, (LonSmipMsg**)&pMsg);

    if (result == LonApiNoError) {

        pMsg->rpcMsg.Header.Command    = LonIsiCmd;
        pMsg->rpcMsg.RpcCode           = code;
        pMsg->rpcMsg.SequenceNumber    = IsiSequenceNumber ++;
        pMsg->rpcMsg.Parameters[0]     = param1;
        pMsg->rpcMsg.Parameters[1]     = param2;
        pMsg->rpcMsg.RpcData.Length    = len;
        pMsg->rpcMsg.Header.Length     = IsiRpcMessageLength(pMsg);

        if (len  &&  pData != NULL) {
            memcpy(&pMsg->rpcMsg.RpcData.Data, pData, len);
        }

        result = LdvPutMsg(ldv_handle, &pMsg->smipMsg);
    }

    return result;
}

/*
 * HandleDownlinkRpcAck
 */
void HandleDownlinkRpcAck(IsiRpcMessage* pMsg, LonBool bSuccess)
{
    LonByte param1 = pMsg->rpcMsg.Parameters[0];
    LonByte param2 = pMsg->rpcMsg.Parameters[1];

    if (bSuccess) {
        switch (pMsg->rpcMsg.RpcCode) {
        case IsiRpcIsConnected: {
            IsiIsConnectedReceived(param1, param2);
            break;
        }

        case IsiRpcImplementationVersion: {
            IsiImplementationVersionReceived(param1);
            break;
        }

        case IsiRpcProtocolVersion: {
            IsiProtocolVersionReceived(param1);
            break;
        }

        case IsiRpcIsRunning: {
            IsiIsRunningReceived(param1);
            break;
        }

        case IsiRpcIsBecomingHost: {
            IsiIsBecomingHostReceived(param1, param2);
            break;
        }
        }
    }

    IsiApiComplete((IsiDownlinkRpcCode) pMsg->rpcMsg.RpcCode, pMsg->rpcMsg.SequenceNumber, bSuccess);
}

/*
 * HandleUplinkRpc
 *
 * This routine handles the callbacks from the Micro Server to
 * the host.  It handles all the byte swapping for the user.
 * All the routines are passed two 1 byte parameters and a single
 * data structure (or null).  All return one 1 byte value and a single data
 * structure (or null).
 */
void HandleUplinkRpc(IsiRpcMessage* pMsg)
{
    LonByte returnValue = 0;
    LonSmipCmd returnCommand = LonIsiAck;
    LonByte param1 = pMsg->rpcMsg.Parameters[0];
    LonByte param2 = pMsg->rpcMsg.Parameters[1];
    IsiRpcMessage* pResp = NULL;

    if (!(pMsg->rpcMsg.RpcCode & IsiRpcUnacknowledged)) {
        /* If a response needs to be send, we need to make sure that we send it
           because there isn't a retry mechanism built into the Micro Server.
           So, if a transmit buffer is not available in the driver, keep calling
           LonEventHandler() which will process both incoming and outgoing
           messages and help freeing up transmit buffers.
         */
        while (LdvAllocateMsg(ldv_handle, (LonSmipMsg**)&pResp) != LonApiNoError) {
            LonEventHandler();
        }
    }

    switch (pMsg->rpcMsg.RpcCode) {
#ifdef ISI_HOST_CREATEPERIODICMSG

    case IsiRpcCreatePeriodicMsg:
        returnValue = IsiCreatePeriodicMsg();
        break;
#endif

#ifdef ISI_HOST_UPDATEUSERINTERFACE

    case IsiRpcUpdateUserInterface:
        IsiUpdateUserInterface((IsiEvent) param1, param2);
        break;
#endif

#ifdef ISI_HOST_CREATECSMO

    case IsiRpcCreateCsmo: {
        IsiCsmoData csmo;
        IsiCreateCsmo(param1, &csmo);
        memcpy(pResp->rpcMsg.RpcData.Data, &csmo, sizeof(IsiCsmoData));
        pResp->rpcMsg.RpcData.Length = sizeof(IsiCsmoData);
        break;
    }

#endif

#ifdef ISI_HOST_GETPRIMARYGROUP

    case IsiRpcGetPrimaryGroup:
        returnValue = IsiGetPrimaryGroup(param1);
        break;
#endif

#ifdef ISI_HOST_GETASSEMBLY

    case IsiRpcGetAssembly:
        returnValue = IsiGetAssembly((IsiCsmoData*) & ((pMsg->rpcMsg.RpcData).Data), param1);
        break;
#endif

#ifdef ISI_HOST_GETNEXTASSEMBLY

    case IsiRpcGetNextAssembly:
        returnValue = IsiGetNextAssembly((IsiCsmoData*) & (pMsg->rpcMsg.RpcData.Data), param1, param2);
        break;
#endif

#ifdef ISI_HOST_GETNVINDEX

    case IsiRpcGetNvIndex:
        returnValue = IsiGetNvIndex(param1, param2);
        break;
#endif

#ifdef ISI_HOST_GETNEXTNVINDEX

    case IsiRpcGetNextNvIndex:
        returnValue = IsiGetNextNvIndex(param1, param2, pMsg->rpcMsg.RpcData.Data[0]);
        break;
#endif

#ifdef ISI_HOST_GETPRIMARYDID

    case IsiRpcGetPrimaryDid: {
        const LonByte* p;
        p = (LonByte*) IsiGetPrimaryDid((unsigned*) &returnValue);
        memcpy(&(pResp->rpcMsg.RpcData.Data), p, returnValue);
        pResp->rpcMsg.RpcData.Length = returnValue;
        break;
    }

#endif

#ifdef ISI_HOST_GETWIDTH

    case IsiRpcGetWidth:
        returnValue = IsiGetWidth(param1);
        break;
#endif

    case IsiRpcGetNvValue: {
        const void* p;
        p = IsiGetNvValue(param1, &returnValue);
        memcpy(&(pResp->rpcMsg.RpcData.Data), p, returnValue);
        pResp->rpcMsg.RpcData.Length = returnValue;
        break;
    }

#ifdef ISI_HOST_CONNECTIONTABLE

    case IsiRpcGetConnTabSize:
        returnValue = IsiGetConnectionTableSize();
        break;

    case IsiRpcGetConnection: {
        const LonByte* p;
        p = (LonByte*) IsiGetConnection(param1);
        memcpy(&(pResp->rpcMsg.RpcData.Data), p, sizeof(IsiConnection));
        pResp->rpcMsg.RpcData.Length = sizeof(IsiConnection);
        break;
    }

    case IsiRpcSetConnection:
        IsiSetConnection((IsiConnection*) & (pMsg->rpcMsg.RpcData.Data), param1);
        break;
#endif

#ifdef ISI_HOST_QUERYHEARTBEAT

    case IsiRpcQueryHeartbeat:
        returnValue = IsiQueryHeartbeat(param1);
        break;
#endif

#ifdef ISI_HOST_GETREPEATCOUNT

    case IsiRpcGetRepeatCount:
        returnValue = IsiGetRepeatCount();
        break;
#endif

    case IsiRpcUserCommand:
        returnValue = IsiUserCommand(param1, param2, (pMsg->rpcMsg.RpcData).Data, (pMsg->rpcMsg.RpcData).Length);

        if (returnValue == 0xFF) {
            returnCommand = LonIsiNack;
        }

        break;

    default:
        returnCommand = LonIsiNack;
        break;
    }

    /*
     * Send a response if necessary.  Note that if the response can't be sent,
     * we simply drop it.  It is up to the ShortStack Micro Server to retry.
     */
    if (!(pMsg->rpcMsg.RpcCode & IsiRpcUnacknowledged)) {
        pResp->rpcMsg.Header.Command = returnCommand;
        pResp->rpcMsg.RpcCode = pMsg->rpcMsg.RpcCode;
        pResp->rpcMsg.SequenceNumber = pMsg->rpcMsg.SequenceNumber;
        pResp->rpcMsg.Parameters[0] = returnValue;
        pResp->rpcMsg.Header.Length = IsiRpcMessageLength(pResp);
        (void)LdvPutMsg(ldv_handle, &pResp->smipMsg);
    }
}
#endif	// LON_ISI_ENABLED
