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
#include "XnUncompressedDepthProcessor.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnUncompressedDepthProcessor::XnUncompressedDepthProcessor(XnSensorDepthStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnDepthProcessor(pStream, pHelper, pBufferManager)
{
}

XnUncompressedDepthProcessor::~XnUncompressedDepthProcessor()
{
}

void XnUncompressedDepthProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData, XnUInt32 /*nDataOffset*/, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnUncompressedDepthProcessor::ProcessFramePacketChunk")

	// when depth is uncompressed, we can just copy it directly to write buffer
	XnBuffer* pWriteBuffer = GetWriteBuffer();

	// Check there is enough room for the depth pixels
	if (CheckWriteBufferForOverflow(nDataSize))
	{
		// sometimes, when packets are lost, we get uneven number of bytes, so we need to complete
		// one byte, in order to keep UINT16 alignment
		if (nDataSize % 2 != 0)
		{
			nDataSize--;
			pData++;
		}

		// copy values. Make sure we do not get corrupted shifts
		XnUInt16* pRaw = (XnUInt16*)(pData);
		XnUInt16* pRawEnd = (XnUInt16*)(pData + nDataSize);
		OniDepthPixel* pDepthBuf = (OniDepthPixel*)pWriteBuffer->GetUnsafeWritePointer();

		XnUInt16 shift;
		while (pRaw < pRawEnd)
		{
			shift = (((*pRaw) < (XN_DEVICE_SENSOR_MAX_SHIFT_VALUE-1)) ? (*pRaw) : 0);
			*pDepthBuf = GetOutput(shift);

			++pRaw;
			++pDepthBuf;
		}

 		pWriteBuffer->UnsafeUpdateSize(nDataSize);
	}

	XN_PROFILING_END_SECTION
}

