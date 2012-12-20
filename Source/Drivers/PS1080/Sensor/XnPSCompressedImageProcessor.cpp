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
#include "XnPSCompressedImageProcessor.h"
#include "Uncomp.h"
#include "YUV.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnPSCompressedImageProcessor::XnPSCompressedImageProcessor(XnSensorImageStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnImageProcessor(pStream, pHelper, pBufferManager)
{
}

XnPSCompressedImageProcessor::~XnPSCompressedImageProcessor()
{
}

XnStatus XnPSCompressedImageProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnImageProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_BUFFER_ALLOCATE(m_ContinuousBuffer, GetExpectedOutputSize());

	switch (GetStream()->GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_YUV422:
		break;
	case ONI_PIXEL_FORMAT_RGB888:
		XN_VALIDATE_BUFFER_ALLOCATE(m_UncompressedYUVBuffer, GetExpectedOutputSize());
		break;
	default:
		XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_SENSOR_PROTOCOL_IMAGE, "Unsupported image output format: %d", GetStream()->GetOutputFormat());
	}

	return (XN_STATUS_OK);
}

void XnPSCompressedImageProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnPSCompressedImageProcessor::ProcessFramePacketChunk")

	// if output format is YUV, we can write directly to output buffer. otherwise, we need
	// to write to a temp buffer.
	XnBuffer* pWriteBuffer = (GetStream()->GetOutputFormat() == ONI_PIXEL_FORMAT_YUV422) ? GetWriteBuffer() : &m_UncompressedYUVBuffer;

	const XnUChar* pBuf = NULL;
	XnUInt32 nBufSize = 0;

	// check if we have bytes stored from previous calls
	if (m_ContinuousBuffer.GetSize() > 0)
	{
		// we have no choice. We need to append current buffer to previous bytes
		if (m_ContinuousBuffer.GetFreeSpaceInBuffer() < nDataSize)
		{
			xnLogWarning(XN_MASK_SENSOR_PROTOCOL_DEPTH, "Bad overflow image! %d", m_ContinuousBuffer.GetSize());
			FrameIsCorrupted();
			m_ContinuousBuffer.Reset();
		}
		else
		{
			m_ContinuousBuffer.UnsafeWrite(pData, nDataSize);
		}

		pBuf = m_ContinuousBuffer.GetData();
		nBufSize = m_ContinuousBuffer.GetSize();
	}
	else
	{
		// we can process the data directly
		pBuf = pData;
		nBufSize = nDataSize;
	}

	XnUInt32 nOutputSize = pWriteBuffer->GetFreeSpaceInBuffer();
	XnUInt32 nWrittenOutput = nOutputSize;
	XnUInt32 nActualRead = 0;
	XnBool bLastPart = pHeader->nType == XN_SENSOR_PROTOCOL_RESPONSE_IMAGE_END && (nDataOffset + nDataSize) == pHeader->nBufSize;
	XnStatus nRetVal = XnStreamUncompressYUVImagePS(pBuf, nBufSize, pWriteBuffer->GetUnsafeWritePointer(), 
		&nWrittenOutput, (XnUInt16)(GetActualXRes()*2), &nActualRead, bLastPart);

	if (nRetVal != XN_STATUS_OK)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL_IMAGE, "Image decompression failed: %s (%d of %d, requested %d, last %d)", xnGetStatusString(nRetVal), nWrittenOutput, nBufSize, nOutputSize, bLastPart);
		FrameIsCorrupted();
	}

	pWriteBuffer->UnsafeUpdateSize(nWrittenOutput);

	nBufSize -= nActualRead;
	m_ContinuousBuffer.Reset();

	// if we have any bytes left, keep them for next time
	if (nBufSize > 0)
	{
		pBuf += nActualRead;
		m_ContinuousBuffer.UnsafeWrite(pBuf, nBufSize);
	}

	XN_PROFILING_END_SECTION
}

void XnPSCompressedImageProcessor::OnStartOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnImageProcessor::OnStartOfFrame(pHeader);
	m_ContinuousBuffer.Reset();
}

void XnPSCompressedImageProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XN_PROFILING_START_SECTION("XnPSCompressedImageProcessor::OnEndOfFrame")

	// if data was written to temp buffer, convert it now
	switch (GetStream()->GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_YUV422:
		break;
	case ONI_PIXEL_FORMAT_RGB888:
		{
			XnUInt32 nActualRead = 0;
			XnUInt32 nOutputSize = GetWriteBuffer()->GetFreeSpaceInBuffer();
			YUV422ToRGB888(m_UncompressedYUVBuffer.GetData(), GetWriteBuffer()->GetUnsafeWritePointer(), m_UncompressedYUVBuffer.GetSize(), &nActualRead, &nOutputSize);
			GetWriteBuffer()->UnsafeUpdateSize(nOutputSize);
			m_UncompressedYUVBuffer.Reset();
		}
		break;
	default:
		assert(0);
		return;
	}

	XnImageProcessor::OnEndOfFrame(pHeader);
	m_ContinuousBuffer.Reset();

	XN_PROFILING_END_SECTION
}
