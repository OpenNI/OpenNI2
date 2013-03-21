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
#include "XnGMCDebugProcessor.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnGMCDebugProcessor::XnGMCDebugProcessor(XnDevicePrivateData* pDevicePrivateData) :
	XnWholePacketProcessor(pDevicePrivateData, "GMCDebug", XN_SENSOR_PROTOCOL_GMC_MAX_POINTS_IN_PACKET*sizeof(XnHostProtocolGMCPoint_1080)),
	m_DumpTxt(NULL),
	m_DumpBin(NULL),
	m_nGMCTime(0)
{
}

XnGMCDebugProcessor::~XnGMCDebugProcessor()
{
	xnDumpFileClose(m_DumpTxt);
	xnDumpFileClose(m_DumpBin);
}

void XnGMCDebugProcessor::ProcessWholePacket(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData)
{
	XN_PROFILING_START_SECTION("XnGMCDebugProcessor::ProcessPacketChunk")

	m_DumpTxt = xnDumpFileOpenEx("GMCDebug", TRUE, TRUE, "GMC_Points.%d.txt", m_nGMCTime);
	xnDumpFileWriteString(m_DumpTxt, "X,Y,DX,DY\n");

	if (pHeader->nType == XN_SENSOR_PROTOCOL_RESPONSE_GMC_DEBUG)
	{
		m_DumpBin = xnDumpFileOpenEx("GMCDebug", TRUE, TRUE, "GMC_Points.%d.xydxdy.bin", m_nGMCTime);

		XnUInt32 nNumOfPoints = pHeader->nBufSize / sizeof(XnHostProtocolGMCPoint_1080);

		// Dump points
		for (XnUInt32 nIndex = 0; nIndex < nNumOfPoints; ++nIndex)
		{
			XnDeviceSensorGMCPoint* pGMCPoint = (XnDeviceSensorGMCPoint*)pData;

			xnDumpFileWriteString(m_DumpTxt, "%d,%d,%d,%d\n", pGMCPoint->m_X, pGMCPoint->m_Y, pGMCPoint->m_DX, pGMCPoint->m_DY);

			XnDouble aDoubles[4];
			aDoubles[0] = pGMCPoint->m_X;
			aDoubles[1] = pGMCPoint->m_Y;
			aDoubles[2] = pGMCPoint->m_DX;
			aDoubles[3] = pGMCPoint->m_DY;
			xnDumpFileWriteBuffer(m_DumpBin, aDoubles, sizeof(aDoubles));

			pData += sizeof(XnHostProtocolGMCPoint_1080);
		}
	}
	else if (pHeader->nType == XN_SENSOR_PROTOCOL_RESPONSE_GMC_DEBUG_END)
	{
		XnHostProtocolGMCLastPacketData* pGMCLastPacketData = (XnHostProtocolGMCLastPacketData*)pData;

		if (m_pDevicePrivateData->FWInfo.nFWVer < XN_SENSOR_FW_VER_5_2)
		{
			// we should fill the last field ourselves
			pGMCLastPacketData->m_FlashStoredRefOffset = -1000; // not written
		}

		xnDumpFileWriteString(m_DumpTxt, "\nMode,%hd\nCoverage Pass:%d\n", pGMCLastPacketData->m_GMCMode,pGMCLastPacketData->m_CoveragePass);

		xnDumpFileWriteString(m_DumpTxt, "Last Configuration:\nN,%hd\nRICC,%hu\nRICC IIR,%f\n\n", 
			pGMCLastPacketData->m_LastConfData.nLast, pGMCLastPacketData->m_LastConfData.nRICCLast, pGMCLastPacketData->m_LastConfData.fRICC_IIR);

		xnDumpFileWriteString(m_DumpTxt, "New Configuration:\nA,%f\nB,%f\nC,%f\nN,%hd\nRICC,%hu\nStartB,%u\nDeltaB,%u\n",
			pGMCLastPacketData->m_A, pGMCLastPacketData->m_B, pGMCLastPacketData->m_C, pGMCLastPacketData->m_N, pGMCLastPacketData->m_RICC, pGMCLastPacketData->m_StartB, pGMCLastPacketData->m_DeltaB);

		if (pGMCLastPacketData->m_FlashStoredRefOffset == -1000)
		{
			xnDumpFileWriteString(m_DumpTxt, "Flash was not updated.");
		}
		else
		{
			xnDumpFileWriteString(m_DumpTxt, "Flash was updated with new reference offset: %hd", pGMCLastPacketData->m_FlashStoredRefOffset);
		}

		xnDumpFileClose(m_DumpTxt);
		xnDumpFileClose(m_DumpBin);
		m_nGMCTime++;
	}

	XN_PROFILING_END_SECTION
}

