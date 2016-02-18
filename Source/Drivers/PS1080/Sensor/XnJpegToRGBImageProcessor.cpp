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
#include <XnJpeg.h>
#include "XnJpegToRGBImageProcessor.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnJpegToRGBImageProcessor::XnJpegToRGBImageProcessor(XnSensorImageStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager)
: XnImageProcessor(pStream, pHelper, pBufferManager)
, mp_JPEGContext(NULL)
{
	SetAllowDoubleSOFPackets(TRUE);
}

XnJpegToRGBImageProcessor::~XnJpegToRGBImageProcessor()
{
    XnStreamFreeUncompressImageJ(&mp_JPEGContext);
}

XnStatus XnJpegToRGBImageProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnImageProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_BUFFER_ALLOCATE(m_RawData, GetExpectedOutputSize());

	nRetVal = XnStreamInitUncompressImageJ(&mp_JPEGContext);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

void XnJpegToRGBImageProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData, XnUInt32 /*nDataOffset*/, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnJpegToRGBImageProcessor::ProcessFramePacketChunk")

	// append to raw buffer
	if (m_RawData.GetFreeSpaceInBuffer() < nDataSize)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL_IMAGE, "Bad overflow image! %d", m_RawData.GetSize());
		FrameIsCorrupted();
		m_RawData.Reset();
	}
	else
	{
		m_RawData.UnsafeWrite(pData, nDataSize);
	}

	XN_PROFILING_END_SECTION
}

void XnJpegToRGBImageProcessor::OnStartOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnImageProcessor::OnStartOfFrame(pHeader);
	m_RawData.Reset();
}

void XnJpegToRGBImageProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XN_PROFILING_START_SECTION("XnJpegToRGBImageProcessor::OnEndOfFrame")

//	xnOSSaveFile("c:\\temp\\fromSensor.jpeg", m_RawData.GetData(), m_RawData.GetSize());

	XnBuffer* pWriteBuffer = GetWriteBuffer();

	XnUInt32 nOutputSize = pWriteBuffer->GetMaxSize();
    XnStatus nRetVal = XnStreamUncompressImageJ(&mp_JPEGContext, m_RawData.GetData(), m_RawData.GetSize(), pWriteBuffer->GetUnsafeWritePointer(), &nOutputSize);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL_IMAGE, "Failed to uncompress JPEG for frame %d: %s (%d)\n", GetCurrentFrameID(), xnGetStatusString(nRetVal), pWriteBuffer->GetSize());
		FrameIsCorrupted();

		XnDumpFile* badImageDump = xnDumpFileOpen(XN_DUMP_BAD_IMAGE, "BadImage_%d.jpeg", GetCurrentFrameID());
		xnDumpFileWriteBuffer(badImageDump, m_RawData.GetData(), m_RawData.GetSize());
		xnDumpFileClose(badImageDump);
	}

	pWriteBuffer->UnsafeUpdateSize(nOutputSize);

	m_RawData.Reset();
	XnImageProcessor::OnEndOfFrame(pHeader);

	XN_PROFILING_END_SECTION
}

