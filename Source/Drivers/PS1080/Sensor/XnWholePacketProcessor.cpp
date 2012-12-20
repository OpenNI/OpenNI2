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
#include "XnWholePacketProcessor.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnWholePacketProcessor::XnWholePacketProcessor(XnDevicePrivateData* pDevicePrivateData, const XnChar* csName, XnUInt32 nMaxPacketSize) :
	XnDataProcessor(pDevicePrivateData, csName),
	m_nMaxPacketSize(nMaxPacketSize)
{}

XnWholePacketProcessor::~XnWholePacketProcessor()
{}

XnStatus XnWholePacketProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnDataProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_WholePacket.Allocate(m_nMaxPacketSize);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

void XnWholePacketProcessor::ProcessPacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize)
{
	if (nDataOffset == 0 && m_WholePacket.GetSize() != 0)
	{
		// previous packet was not received to its end
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "%s: Expected %d additional bytes in packet (got %d out of %d bytes)!", m_csName, pHeader->nBufSize - m_WholePacket.GetSize(), m_WholePacket.GetSize(), pHeader->nBufSize);
		m_WholePacket.Reset();
	}

	// sanity check
	if (pHeader->nBufSize > m_WholePacket.GetMaxSize())
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "Got a packet which is bigger than max size! (%d > %d)", pHeader->nBufSize, m_WholePacket.GetMaxSize());
	}
	else
	{
		// append data
		m_WholePacket.UnsafeWrite(pData, nDataSize);

		// check if we have entire packet
		if (m_WholePacket.GetSize() == pHeader->nBufSize)
		{
			// process it
			ProcessWholePacket(pHeader, m_WholePacket.GetData());
			m_WholePacket.Reset();
		}
	}
}
