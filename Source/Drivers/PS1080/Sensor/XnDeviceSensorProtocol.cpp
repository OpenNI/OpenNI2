/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDeviceSensorProtocol.h"
#include "XnDeviceSensorIO.h"
#include "Uncomp.h"
#include "XnHostProtocol.h"
#include <XnLog.h>
#include <XnProfiling.h>
#include "XnStreamProcessor.h"
#include "XnSensor.h"
#include <XnOS.h>

FILE* g_fUSBDump;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnBool XN_CALLBACK_TYPE XnDeviceSensorProtocolUsbEpCb(XnUChar* pBuffer, XnUInt32 nBufferSize, void* pCallbackData)
{
	XN_PROFILING_START_MT_SECTION("XnDeviceSensorProtocolUsbEpCb");

	XnUInt32 nReadBytes;
	XnUInt16 nMagic;

	XnSpecificUsbDevice* pDevice = (XnSpecificUsbDevice*)pCallbackData;
	XnDevicePrivateData* pDevicePrivateData = pDevice->pDevicePrivateData;
	XnUChar* pBufEnd = pBuffer + nBufferSize;

	XnSpecificUsbDeviceState* pCurrState = &pDevice->CurrState;

	while (pBuffer < pBufEnd)
	{
		switch (pCurrState->State)
		{
		case XN_WAITING_FOR_CONFIGURATION:
			pCurrState->State = XN_IGNORING_GARBAGE;
			pCurrState->nMissingBytesInState = pDevice->nIgnoreBytes;
			break;

		case XN_IGNORING_GARBAGE:
			// ignore first bytes on this endpoint. NOTE: due to a bug in the firmware, the first data received
			// on each endpoint is corrupt, causing wrong timestamp calculation, causing future (true) timestamps
			// to be calculated wrongly. By ignoring the first data received on each endpoint we hope to get
			// only valid data.
			nReadBytes = XN_MIN((XnUInt32)(pBufEnd - pBuffer), pCurrState->nMissingBytesInState);

			if (nReadBytes > 0)
			{
				xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "ignoring %d bytes - ignore garbage phase!", nReadBytes);
				pCurrState->nMissingBytesInState -= nReadBytes;
				pBuffer += nReadBytes;
			}

			if (pCurrState->nMissingBytesInState == 0)
			{
				pCurrState->State = XN_LOOKING_FOR_MAGIC;
				pCurrState->nMissingBytesInState = sizeof(XnUInt16);
			}
			break;

		case XN_LOOKING_FOR_MAGIC:
			nMagic = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->FWInfo.nFWMagic);

			if (pCurrState->nMissingBytesInState == sizeof(XnUInt8) && // first byte already found
				pBuffer[0] == ((XnUInt8*)&nMagic)[1])	// we have here second byte
			{
				// move to next byte
				pBuffer++;

				// move to next state
				pCurrState->CurrHeader.nMagic = nMagic;
				pCurrState->State = XN_PACKET_HEADER;
				pCurrState->nMissingBytesInState = sizeof(XnSensorProtocolResponseHeader);
				break;
			}

			while (pBuffer < pBufEnd)
			{
				if ((pBuffer + sizeof(XnUInt16) <= pBufEnd) && 
					nMagic == *(XnUInt16*)(pBuffer))
				{
					pCurrState->CurrHeader.nMagic = nMagic;
					pCurrState->State = XN_PACKET_HEADER;
					pCurrState->nMissingBytesInState = sizeof(XnSensorProtocolResponseHeader);
					break;
				}
				else
				{
					pBuffer++;
				}
			}

			if (pBuffer == pBufEnd &&					// magic wasn't found
				pBuffer[-1] == ((XnUInt8*)&nMagic)[0])	// last byte in buffer is first in magic
			{
				// mark that we found first one
				pCurrState->nMissingBytesInState--;
			}

			break;

		case XN_PACKET_HEADER:
			nReadBytes = XN_MIN((XnUInt32)(pBufEnd - pBuffer), pCurrState->nMissingBytesInState);
			xnOSMemCopy((XnUChar*)&pCurrState->CurrHeader + sizeof(XnSensorProtocolResponseHeader) - pCurrState->nMissingBytesInState, 
				pBuffer, nReadBytes);
			pCurrState->nMissingBytesInState -= nReadBytes;
			pBuffer += nReadBytes;

			if (pCurrState->nMissingBytesInState == 0)
			{
				// we have entire header. Fix it
				pCurrState->CurrHeader.nBufSize = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nBufSize);
				pCurrState->CurrHeader.nMagic = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nMagic);
				pCurrState->CurrHeader.nPacketID = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nPacketID);
				pCurrState->CurrHeader.nTimeStamp = XN_PREPARE_VAR32_IN_BUFFER(pCurrState->CurrHeader.nTimeStamp);
				pCurrState->CurrHeader.nType = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nType);
				pCurrState->CurrHeader.nBufSize = xnOSEndianSwapUINT16(pCurrState->CurrHeader.nBufSize);
				pCurrState->CurrHeader.nBufSize -= sizeof(XnSensorProtocolResponseHeader);

				pCurrState->State = XN_PACKET_DATA;
				pCurrState->nMissingBytesInState = pCurrState->CurrHeader.nBufSize;
			}
			break;

		case XN_PACKET_DATA:
			nReadBytes = XN_MIN((XnUInt32)(pBufEnd - pBuffer), pCurrState->nMissingBytesInState);
			pDevicePrivateData->pSensor->GetFirmware()->GetStreams()->ProcessPacketChunk(&pCurrState->CurrHeader, pBuffer, pCurrState->CurrHeader.nBufSize - pCurrState->nMissingBytesInState, nReadBytes);
			pBuffer += nReadBytes;
			pCurrState->nMissingBytesInState -= nReadBytes;

			if (pCurrState->nMissingBytesInState == 0)
			{
				pCurrState->State = XN_LOOKING_FOR_MAGIC;
				pCurrState->nMissingBytesInState = sizeof(XnUInt16);
			}
			break;
		}
	}

	XN_PROFILING_END_SECTION;

	return TRUE;
}

XnStatus XnDeviceSensorProtocolFindStreamOfType(XnDevicePrivateData* pDevicePrivateData, const XnChar* strType, const XnChar** ppStreamName)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	const XnChar* strNames[100];
	XnUInt32 nCount = 100;

	nRetVal = pDevicePrivateData->pSensor->GetStreamNames(strNames, &nCount);
	XN_IS_STATUS_OK(nRetVal);

	for (XnUInt32 i = 0; i < nCount; ++i)
	{
		XnChar strCurType[XN_DEVICE_MAX_STRING_LENGTH];
		nRetVal = pDevicePrivateData->pSensor->GetProperty(strNames[i], XN_STREAM_PROPERTY_TYPE, strCurType);
		XN_IS_STATUS_OK(nRetVal);

		if (strcmp(strType, strCurType) == 0)
		{
			*ppStreamName = strNames[i];
			return (XN_STATUS_OK);
		}
	}

	*ppStreamName = NULL;
	return (XN_STATUS_NO_MATCH);
}

