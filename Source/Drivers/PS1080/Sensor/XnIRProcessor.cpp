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
#include "XnIRProcessor.h"
#include <XnProfiling.h>
#include "XnSensor.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

/* The size of an input element for unpacking. */
#define XN_INPUT_ELEMENT_SIZE 5
/* The size of an output element for unpacking. */
#define XN_OUTPUT_ELEMENT_SIZE 8

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnIRProcessor::XnIRProcessor(XnSensorIRStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnFrameStreamProcessor(pStream, pHelper, pBufferManager, XN_SENSOR_PROTOCOL_RESPONSE_IMAGE_START, XN_SENSOR_PROTOCOL_RESPONSE_IMAGE_END),
	m_nRefTimestamp(0),
	m_DepthCMOSType(pHelper->GetFixedParams()->GetDepthCmosType())
{
}

XnIRProcessor::~XnIRProcessor()
{
}

XnStatus XnIRProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnFrameStreamProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_BUFFER_ALLOCATE(m_ContinuousBuffer, XN_INPUT_ELEMENT_SIZE);

	switch (GetStream()->GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_GRAY16:
		break;
	case ONI_PIXEL_FORMAT_RGB888:
		XN_VALIDATE_BUFFER_ALLOCATE(m_UnpackedBuffer, GetExpectedOutputSize());
		break;
	default:
		assert(0);
		return XN_STATUS_ERROR;
	}

	return (XN_STATUS_OK);
}

XnStatus XnIRProcessor::Unpack10to16(const XnUInt8* pcInput, const XnUInt32 nInputSize, XnUInt16* pnOutput, XnUInt32* pnActualRead, XnUInt32* pnOutputSize)
{
	XnInt32 cInput = 0;
	const XnUInt8* pOrigInput = pcInput;

	XnUInt32 nElements = nInputSize / XN_INPUT_ELEMENT_SIZE; // floored
	XnUInt32 nNeededOutput = nElements * XN_OUTPUT_ELEMENT_SIZE;

	*pnActualRead = 0;

	if (*pnOutputSize < nNeededOutput)
	{
		*pnOutputSize = 0;
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	// Convert the 10bit packed data into 16bit shorts

	for (XnUInt32 nElem = 0; nElem < nElements; ++nElem)
	{
		//1a 
		cInput = *pcInput;
		*pnOutput = (cInput & 0xFF) << 2;

		//1b
		pcInput++;
		cInput = *pcInput;
		*pnOutput = *pnOutput | ((cInput & 0xC0) >> 6);
		pnOutput++;

		//2a
		*pnOutput = (cInput & 0x3F) << 4;

		//2b
		pcInput++;
		cInput = *pcInput;
		*pnOutput = *pnOutput | ((cInput & 0xF0) >> 4);
		pnOutput++;

		//3a
		*pnOutput = (cInput & 0x0F) << 6;

		//3b
		pcInput++;
		cInput = *pcInput;
		*pnOutput = *pnOutput | ((cInput & 0xFC) >> 2);
		pnOutput++;

		//4a
		*pnOutput = (cInput & 0x3) << 8;

		//4b
		pcInput++;
		cInput = *pcInput;
		*pnOutput = *pnOutput | (cInput & 0xFF);
		pnOutput++;

		pcInput++;
	}

	*pnActualRead = (XnUInt32)(pcInput - pOrigInput);
	*pnOutputSize = nNeededOutput;
	return XN_STATUS_OK;
}

void XnIRProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData, XnUInt32 /*nDataOffset*/, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnIRProcessor::ProcessFramePacketChunk")

	// if output format is Gray16, we can write directly to output buffer. otherwise, we need
	// to write to a temp buffer.
	XnBuffer* pWriteBuffer = (GetStream()->GetOutputFormat() == ONI_PIXEL_FORMAT_GRAY16) ? GetWriteBuffer() : &m_UnpackedBuffer;

	if (m_ContinuousBuffer.GetSize() != 0)
	{
		// fill in to a whole element
		XnUInt32 nReadBytes = XN_MIN(nDataSize, XN_INPUT_ELEMENT_SIZE - m_ContinuousBuffer.GetSize());
		m_ContinuousBuffer.UnsafeWrite(pData, nReadBytes);
		pData += nReadBytes;
		nDataSize -= nReadBytes;

		if (m_ContinuousBuffer.GetSize() == XN_INPUT_ELEMENT_SIZE)
		{
			// process it
			XnUInt32 nActualRead = 0;
			XnUInt32 nOutputSize = pWriteBuffer->GetFreeSpaceInBuffer();
			if (XN_STATUS_OK != Unpack10to16(m_ContinuousBuffer.GetData(), XN_INPUT_ELEMENT_SIZE, (XnUInt16*)pWriteBuffer->GetUnsafeWritePointer(), &nActualRead, &nOutputSize))
				WriteBufferOverflowed();
			else
				pWriteBuffer->UnsafeUpdateSize(nOutputSize);

			m_ContinuousBuffer.Reset();
		}
	}

	XnUInt32 nActualRead = 0;
	XnUInt32 nOutputSize = pWriteBuffer->GetFreeSpaceInBuffer();
	if (XN_STATUS_OK != Unpack10to16(pData, nDataSize, (XnUInt16*)pWriteBuffer->GetUnsafeWritePointer(), &nActualRead, &nOutputSize))
	{
		WriteBufferOverflowed();
	}
	else
	{
		pWriteBuffer->UnsafeUpdateSize(nOutputSize);

		pData += nActualRead;
		nDataSize -= nActualRead;

		// if we have any bytes left, store them for next packet
		if (nDataSize > 0)
		{
			// no need to check for overflow. there can not be a case in which more than XN_INPUT_ELEMENT_SIZE
			// are left.
			m_ContinuousBuffer.UnsafeWrite(pData, nDataSize);
		}
	}

	XN_PROFILING_END_SECTION
}

void IRto888(XnUInt16* pInput, XnUInt32 nInputSize, XnUInt8* pOutput, XnUInt32* pnOutputSize)
{
	XnUInt16* pInputEnd = pInput + nInputSize;
	XnUInt8* pOutputOrig = pOutput;
	XnUInt8* pOutputEnd = pOutput + *pnOutputSize;

	while (pInput != pInputEnd && pOutput < pOutputEnd)
	{
		*pOutput = (XnUInt8)((*pInput)>>2);
		*(pOutput+1) = *pOutput;
		*(pOutput+2) = *pOutput;

		pOutput+=3;
		pInput++;
	}

	*pnOutputSize = (XnUInt32)(pOutput - pOutputOrig);
}

void XnIRProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XN_PROFILING_START_SECTION("XnIRProcessor::OnEndOfFrame")

	// if there are bytes left in continuous buffer, then we have a corrupt frame
	if (m_ContinuousBuffer.GetSize() != 0)
	{
		xnLogWarning(XN_MASK_SENSOR_READ, "IR buffer is corrupt. There are left over bytes (invalid size)");
		FrameIsCorrupted();
	}

	// if data was written to temp buffer, convert it now
	switch (GetStream()->GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_GRAY16:
		break;
	case ONI_PIXEL_FORMAT_RGB888:
		{
			XnUInt32 nOutputSize = GetWriteBuffer()->GetFreeSpaceInBuffer();
			IRto888((XnUInt16*)m_UnpackedBuffer.GetData(), m_UnpackedBuffer.GetSize() / sizeof(XnUInt16), GetWriteBuffer()->GetUnsafeWritePointer(), &nOutputSize);
			GetWriteBuffer()->UnsafeUpdateSize(nOutputSize);
			m_UnpackedBuffer.Reset();
		}
		break;
	default:
		assert(0);
		return;
	}

	// calculate expected size
	XnUInt32 width = GetStream()->GetXRes();
	XnUInt32 height = GetStream()->GetYRes();
	XnUInt32 actualHeight = height;

	// when cropping is turned on, actual depth size is smaller
	if (GetStream()->m_FirmwareCropMode.GetValue() != XN_FIRMWARE_CROPPING_MODE_DISABLED)
	{
		width = (XnUInt32)GetStream()->m_FirmwareCropSizeX.GetValue();
		height = (XnUInt32)GetStream()->m_FirmwareCropSizeY.GetValue();
		actualHeight = height;
	}
	else if (GetStream()->GetResolution() != XN_RESOLUTION_SXGA)
	{
		if (m_DepthCMOSType == XN_DEPTH_CMOS_MT9M001)
		{
			// there are additional 8 rows (this is how the CMOS is configured)
			actualHeight += 8;
		}	
	}
	else
	{
		if (m_DepthCMOSType == XN_DEPTH_CMOS_AR130)
		{
			// there missing 64 rows (this is how the CMOS is configured)
			actualHeight -= 64;
		}
	}

	XnUInt32 nExpectedBufferSize = width * actualHeight * GetStream()->GetBytesPerPixel();

	if (GetWriteBuffer()->GetSize() != nExpectedBufferSize)
	{
		xnLogWarning(XN_MASK_SENSOR_READ, "IR buffer is corrupt. Size is %u (!= %u)", GetWriteBuffer()->GetSize(), nExpectedBufferSize);
		FrameIsCorrupted();
	}

	// don't report additional rows out (so we're not using the expected buffer size)
	GetWriteBuffer()->UnsafeSetSize(width * height * GetStream()->GetBytesPerPixel());

	OniFrame* pFrame = GetWriteFrame();
	pFrame->sensorType = ONI_SENSOR_IR;

	pFrame->videoMode.pixelFormat = GetStream()->GetOutputFormat();
	pFrame->videoMode.resolutionX = GetStream()->GetXRes();
	pFrame->videoMode.resolutionY = GetStream()->GetYRes();
	pFrame->videoMode.fps = GetStream()->GetFPS();
	pFrame->width = (int)width;
	pFrame->height = (int)height;

	if (GetStream()->m_FirmwareCropMode.GetValue() != XN_FIRMWARE_CROPPING_MODE_DISABLED)
	{
		pFrame->cropOriginX = (int)GetStream()->m_FirmwareCropOffsetX.GetValue();
		pFrame->cropOriginY = (int)GetStream()->m_FirmwareCropOffsetY.GetValue();
		pFrame->croppingEnabled = TRUE;
	}
	else
	{
		pFrame->cropOriginX = 0;
		pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
	}

	pFrame->stride = pFrame->width * GetStream()->GetBytesPerPixel();

	XnFrameStreamProcessor::OnEndOfFrame(pHeader);
	m_ContinuousBuffer.Reset();

	XN_PROFILING_END_SECTION
}

XnUInt64 XnIRProcessor::CreateTimestampFromDevice(XnUInt32 nDeviceTimeStamp)
{
	XnUInt64 nNow;
	xnOSGetHighResTimeStamp(&nNow);

	// There's a firmware bug, causing IR timestamps not to advance if depth stream is off.
	// If so, we need to create our own timestamps.
	if (m_pDevicePrivateData->pSensor->GetFirmware()->GetParams()->m_Stream1Mode.GetValue() != XN_VIDEO_STREAM_DEPTH)
	{
		if (m_nRefTimestamp == 0)
		{
			m_nRefTimestamp = nNow;
		}

		return nNow - m_nRefTimestamp;
	}
	else
	{
		XnUInt64 nResult = XnFrameStreamProcessor::CreateTimestampFromDevice(nDeviceTimeStamp);

		// keep it as ref so that if depth is turned off, we'll continue from there
		m_nRefTimestamp = nNow - nResult;

		return nResult;
	}
}

void XnIRProcessor::OnFrameReady(XnUInt32 nFrameID, XnUInt64 nFrameTS)
{
	XnFrameStreamProcessor::OnFrameReady(nFrameID, nFrameTS);

	m_pDevicePrivateData->pSensor->GetFPSCalculator()->MarkIr(nFrameID, nFrameTS);
}
