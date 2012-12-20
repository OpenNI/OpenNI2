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
#include "XnFrameStreamProcessor.h"
#include "XnSensor.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnFrameStreamProcessor::XnFrameStreamProcessor(XnFrameStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager, XnUInt16 nTypeSOF, XnUInt16 nTypeEOF) :
	XnStreamProcessor(pStream, pHelper),
	m_nTypeSOF(nTypeSOF),
	m_nTypeEOF(nTypeEOF),
	m_pTripleBuffer(pBufferManager),
	m_InDump(NULL),
	m_InternalDump(NULL),
	m_bFrameCorrupted(FALSE),
	m_bAllowDoubleSOF(FALSE),
	m_nLastSOFPacketID(0),
	m_nFirstPacketTimestamp(0)
{
	sprintf(m_csInDumpMask, "%sIn", pStream->GetType());
	sprintf(m_csInternalDumpMask, "Internal%s", pStream->GetType());
	m_InDump = xnDumpFileOpen(m_csInDumpMask, "%s_0.raw", m_csInDumpMask);
	m_InternalDump = xnDumpFileOpen(m_csInternalDumpMask, "%s_0.raw", m_csInternalDumpMask);
}

XnFrameStreamProcessor::~XnFrameStreamProcessor()
{
	xnDumpFileClose(m_InDump);
	xnDumpFileClose(m_InternalDump);
}

void XnFrameStreamProcessor::ProcessPacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnFrameStreamProcessor::ProcessPacketChunk");

	// if first data from SOF packet
	if (pHeader->nType == m_nTypeSOF && nDataOffset == 0)
	{
		if (!m_bAllowDoubleSOF || pHeader->nPacketID != (m_nLastSOFPacketID + 1))
		{
			m_nLastSOFPacketID = pHeader->nPacketID;
			OnStartOfFrame(pHeader);
		}
	}

	if (!m_bFrameCorrupted)
	{
		xnDumpFileWriteBuffer(m_InDump, pData, nDataSize);
		ProcessFramePacketChunk(pHeader, pData, nDataOffset, nDataSize);
	}

	// if last data from EOF packet
	if (pHeader->nType == m_nTypeEOF && (nDataOffset + nDataSize) == pHeader->nBufSize)
	{
		OnEndOfFrame(pHeader);
	}

	XN_PROFILING_END_SECTION
}

void XnFrameStreamProcessor::OnPacketLost()
{
	FrameIsCorrupted();
}

void XnFrameStreamProcessor::OnStartOfFrame(const XnSensorProtocolResponseHeader* /*pHeader*/)
{
	m_bFrameCorrupted = FALSE;
	m_pTripleBuffer->GetWriteBuffer()->Reset();
	if (m_pDevicePrivateData->pSensor->ShouldUseHostTimestamps())
	{
		m_nFirstPacketTimestamp = GetHostTimestamp();
	}
}

void XnFrameStreamProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	// write dump
	XnBuffer* pCurWriteBuffer = m_pTripleBuffer->GetWriteBuffer();
	xnDumpFileWriteBuffer(m_InternalDump, pCurWriteBuffer->GetData(), pCurWriteBuffer->GetSize());
	xnDumpFileClose(m_InternalDump);
	xnDumpFileClose(m_InDump);

	if (!m_bFrameCorrupted)
	{
		// mark the buffer as stable
		XnUInt64 nTimestamp;
		if (m_pDevicePrivateData->pSensor->ShouldUseHostTimestamps())
		{
			// use the host timestamp of the first packet
			nTimestamp = m_nFirstPacketTimestamp;
		}
		else
		{
			// use timestamp in last packet
			nTimestamp = CreateTimestampFromDevice(pHeader->nTimeStamp);
		}

		OniFrame* pFrame = m_pTripleBuffer->GetWriteFrame();
		pFrame->timestamp = nTimestamp;
		
		XnUInt32 nFrameID;
		m_pTripleBuffer->MarkWriteBufferAsStable(&nFrameID);

		// let inheriting classes do their stuff
		OnFrameReady(nFrameID, nTimestamp);
	}
	else
	{
		// restart
		m_pTripleBuffer->GetWriteBuffer()->Reset();
	}

	// log bandwidth
	XnUInt64 nSysTime;
	xnOSGetTimeStamp(&nSysTime);
	xnDumpFileWriteString(m_pDevicePrivateData->BandwidthDump, "%llu,%s,%d,%d\n", 
		nSysTime, m_csName, GetCurrentFrameID(), m_nBytesReceived);

	// re-init dumps
	m_InDump = xnDumpFileOpen(m_csInDumpMask, "%s_%d.raw", m_csInDumpMask, GetCurrentFrameID());
	m_InternalDump = xnDumpFileOpen(m_csInternalDumpMask, "%s_%d.raw", m_csInternalDumpMask, GetCurrentFrameID());
	m_nBytesReceived = 0;
}

void XnFrameStreamProcessor::FrameIsCorrupted()
{
	if (!m_bFrameCorrupted)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "%s frame is corrupt!", m_csName);
		m_bFrameCorrupted = TRUE;
	}
}

void XnFrameStreamProcessor::WriteBufferOverflowed()
{
	XnBuffer* pBuffer = GetWriteBuffer();
	xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "%s Frame Buffer overflow! current size: %d", m_csName, pBuffer->GetSize());
	FrameIsCorrupted();
}
