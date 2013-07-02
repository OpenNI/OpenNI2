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
#include "XnUncompressedYUV422toRGBImageProcessor.h"
#include "YUV.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnUncompressedYUV422toRGBImageProcessor::XnUncompressedYUV422toRGBImageProcessor(XnSensorImageStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnImageProcessor(pStream, pHelper, pBufferManager)
{
}

XnUncompressedYUV422toRGBImageProcessor::~XnUncompressedYUV422toRGBImageProcessor()
{
}

XnStatus XnUncompressedYUV422toRGBImageProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnImageProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_BUFFER_ALLOCATE(m_ContinuousBuffer, XN_YUV_TO_RGB_INPUT_ELEMENT_SIZE)
	
	return (XN_STATUS_OK);
}

void XnUncompressedYUV422toRGBImageProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData, XnUInt32 /*nDataOffset*/, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnUncompressedYUV422toRGBImageProcessor::ProcessFramePacketChunk")

	XnBuffer* pWriteBuffer = GetWriteBuffer();

	if (m_ContinuousBuffer.GetSize() != 0)
	{
		// fill in to a whole element
		XnUInt32 nReadBytes = XN_MIN(nDataSize, XN_YUV_TO_RGB_INPUT_ELEMENT_SIZE - m_ContinuousBuffer.GetSize());
		m_ContinuousBuffer.UnsafeWrite(pData, nReadBytes);
		pData += nReadBytes;
		nDataSize -= nReadBytes;

		if (m_ContinuousBuffer.GetSize() == XN_YUV_TO_RGB_INPUT_ELEMENT_SIZE)
		{
			if (CheckWriteBufferForOverflow(XN_YUV_TO_RGB_OUTPUT_ELEMENT_SIZE))
			{
				// process it
				XnUInt32 nActualRead = 0;
				XnUInt32 nOutputSize = pWriteBuffer->GetFreeSpaceInBuffer();
				YUV422ToRGB888(m_ContinuousBuffer.GetData(), pWriteBuffer->GetUnsafeWritePointer(), XN_YUV_TO_RGB_INPUT_ELEMENT_SIZE, &nActualRead, &nOutputSize);
				pWriteBuffer->UnsafeUpdateSize(XN_YUV_TO_RGB_OUTPUT_ELEMENT_SIZE);
			}

			m_ContinuousBuffer.Reset();
		}
	}

	if (CheckWriteBufferForOverflow(nDataSize / XN_YUV_TO_RGB_INPUT_ELEMENT_SIZE * XN_YUV_TO_RGB_OUTPUT_ELEMENT_SIZE))
	{
		XnUInt32 nActualRead = 0;
		XnUInt32 nOutputSize = pWriteBuffer->GetFreeSpaceInBuffer();
		YUV422ToRGB888(pData, pWriteBuffer->GetUnsafeWritePointer(), nDataSize, &nActualRead, &nOutputSize);
		pWriteBuffer->UnsafeUpdateSize(nOutputSize);
		pData += nActualRead;
		nDataSize -= nActualRead;

		// if we have any bytes left, store them for next packet.
		if (nDataSize > 0)
		{
			// no need to check for overflow. there can not be a case in which more than XN_INPUT_ELEMENT_SIZE
			// are left.
			m_ContinuousBuffer.UnsafeWrite(pData, nDataSize);
		}
	}

	XN_PROFILING_END_SECTION
}

void XnUncompressedYUV422toRGBImageProcessor::OnStartOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnImageProcessor::OnStartOfFrame(pHeader);
	m_ContinuousBuffer.Reset();
}

void XnUncompressedYUV422toRGBImageProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnImageProcessor::OnEndOfFrame(pHeader);
	m_ContinuousBuffer.Reset();
}
