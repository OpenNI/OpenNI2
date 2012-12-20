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
#include "XnUncompressedBayerProcessor.h"
#include "Uncomp.h"
#include "Bayer.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnUncompressedBayerProcessor::XnUncompressedBayerProcessor(XnSensorImageStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnImageProcessor(pStream, pHelper, pBufferManager)
{
}

XnUncompressedBayerProcessor::~XnUncompressedBayerProcessor()
{
}

XnStatus XnUncompressedBayerProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnImageProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	switch (GetStream()->GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_GRAY8:
		break;
	case ONI_PIXEL_FORMAT_RGB888:
		XN_VALIDATE_BUFFER_ALLOCATE(m_UncompressedBayerBuffer, GetExpectedOutputSize());
		break;
	default:
		XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_SENSOR_PROTOCOL_IMAGE, "Unsupported image output format: %d", GetStream()->GetOutputFormat());
	}

	return (XN_STATUS_OK);
}

void XnUncompressedBayerProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData, XnUInt32 /*nDataOffset*/, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnUncompressedBayerProcessor::ProcessFramePacketChunk")

	// if output format is Gray8, we can write directly to output buffer. otherwise, we need
	// to write to a temp buffer.
	XnBuffer* pWriteBuffer = (GetStream()->GetOutputFormat() == ONI_PIXEL_FORMAT_GRAY8) ? GetWriteBuffer() : &m_UncompressedBayerBuffer;

	// make sure we have enough room
	if (pWriteBuffer->GetFreeSpaceInBuffer() < nDataSize)
	{
		WriteBufferOverflowed();
	}
	else
	{		
		pWriteBuffer->UnsafeWrite(pData, nDataSize);
	}

	XN_PROFILING_END_SECTION
}

void XnUncompressedBayerProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XN_PROFILING_START_SECTION("XnUncompressedBayerProcessor::OnEndOfFrame")

	// if data was written to temp buffer, convert it now
	switch (GetStream()->GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_GRAY8:
		break;
	case ONI_PIXEL_FORMAT_RGB888:
		{
			Bayer2RGB888(m_UncompressedBayerBuffer.GetData(), GetWriteBuffer()->GetUnsafeWritePointer(), GetActualXRes(), GetActualYRes(), 1);
			GetWriteBuffer()->UnsafeUpdateSize(GetActualXRes()*GetActualYRes()*3);
			m_UncompressedBayerBuffer.Reset();
		}
		break;
	default:
		assert(0);
		return;
	}

	XnImageProcessor::OnEndOfFrame(pHeader);

	XN_PROFILING_END_SECTION
}
