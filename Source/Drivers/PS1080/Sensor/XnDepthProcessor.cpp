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
#include "XnDepthProcessor.h"
#include "XnSensor.h"
#include <XnProfiling.h>
#include <XnLog.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnDepthProcessor::XnDepthProcessor(XnSensorDepthStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnFrameStreamProcessor(pStream, pHelper, pBufferManager, XN_SENSOR_PROTOCOL_RESPONSE_DEPTH_START, XN_SENSOR_PROTOCOL_RESPONSE_DEPTH_END),
	m_nPaddingPixelsOnEnd(0),
	m_applyRegistrationOnEnd(FALSE),
	m_nExpectedFrameSize(0),
	m_bShiftToDepthAllocated(FALSE),
	m_pShiftToDepthTable(pStream->GetShiftToDepthTable())
{
}

XnDepthProcessor::~XnDepthProcessor()
{
	if (m_bShiftToDepthAllocated)
	{
		xnOSFree(m_pShiftToDepthTable);
	}
}

XnStatus XnDepthProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// init base
	nRetVal = XnFrameStreamProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	switch (GetStream()->GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_SHIFT_9_2:
		{
			// optimization. We create a LUT shift-to-shift. See comment up.
			m_pShiftToDepthTable = (OniDepthPixel*)xnOSMalloc(sizeof(OniDepthPixel)*XN_DEVICE_SENSOR_MAX_SHIFT_VALUE);
			XN_VALIDATE_ALLOC_PTR(m_pShiftToDepthTable);
			for (XnUInt32 i = 0; i < XN_DEVICE_SENSOR_MAX_SHIFT_VALUE; ++i)
			{
				m_pShiftToDepthTable[i] = (OniDepthPixel)i;
			}
			m_bShiftToDepthAllocated = TRUE;
			m_noDepthValue = 2047;
		}
		break;
	case ONI_PIXEL_FORMAT_DEPTH_1_MM:
	case ONI_PIXEL_FORMAT_DEPTH_100_UM:
		m_noDepthValue = 0;
		break;
	default:
		XN_ASSERT(FALSE);
		XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_SENSOR_PROTOCOL_DEPTH, "Unknown Depth output: %d", GetStream()->GetOutputFormat());
	}

	return (XN_STATUS_OK);
}

void XnDepthProcessor::OnStartOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	// call base
	XnFrameStreamProcessor::OnStartOfFrame(pHeader);

	m_nExpectedFrameSize = CalculateExpectedSize();

	m_applyRegistrationOnEnd = (
		(GetStream()->GetOutputFormat() == ONI_PIXEL_FORMAT_DEPTH_1_MM || GetStream()->GetOutputFormat() == ONI_PIXEL_FORMAT_DEPTH_100_UM) && 
		GetStream()->m_DepthRegistration.GetValue() == TRUE && 
		GetStream()->m_FirmwareRegistration.GetValue() == FALSE);

	if (m_pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_1 && pHeader->nTimeStamp != 0)
	{
		// PATCH: starting with v5.1, the timestamp field of the SOF packet, is the number of pixels
		// that should be prepended to the frame.
		XnUInt32 nPaddingPixelsOnStart = pHeader->nTimeStamp >> 16;
		m_nPaddingPixelsOnEnd = pHeader->nTimeStamp & 0x0000FFFF;

		PadPixels(nPaddingPixelsOnStart);
	}
}

XnUInt32 XnDepthProcessor::CalculateExpectedSize()
{
	XnUInt32 nExpectedDepthBufferSize = GetStream()->GetXRes() * GetStream()->GetYRes();

	// when cropping is turned on, actual depth size is smaller
	if (GetStream()->m_FirmwareCropMode.GetValue() != XN_FIRMWARE_CROPPING_MODE_DISABLED)
	{
		nExpectedDepthBufferSize = (XnUInt32)(GetStream()->m_FirmwareCropSizeX.GetValue() * GetStream()->m_FirmwareCropSizeY.GetValue());
	}

	nExpectedDepthBufferSize *= sizeof(OniDepthPixel);

	return nExpectedDepthBufferSize;
}

void XnDepthProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	// pad pixels
	if (m_nPaddingPixelsOnEnd != 0)
	{
		PadPixels(m_nPaddingPixelsOnEnd);
		m_nPaddingPixelsOnEnd = 0 ;
	}

	if (GetWriteBuffer()->GetSize() != GetExpectedSize())
	{
		xnLogWarning(XN_MASK_SENSOR_READ, "Read: Depth buffer is corrupt. Size is %u (!= %u)", GetWriteBuffer()->GetSize(), GetExpectedSize());
		FrameIsCorrupted();
	}
	else
	{
		if (m_applyRegistrationOnEnd)
		{
			GetStream()->ApplyRegistration((OniDepthPixel*)GetWriteBuffer()->GetData());
		}
	}

	OniFrame* pFrame = GetWriteFrame();
	pFrame->sensorType = ONI_SENSOR_DEPTH;

	pFrame->videoMode.pixelFormat = GetStream()->GetOutputFormat();
	pFrame->videoMode.resolutionX = GetStream()->GetXRes();
	pFrame->videoMode.resolutionY = GetStream()->GetYRes();
	pFrame->videoMode.fps = GetStream()->GetFPS();
	
	if (GetStream()->m_FirmwareCropMode.GetValue() != XN_FIRMWARE_CROPPING_MODE_DISABLED)
	{
		pFrame->width = (int)GetStream()->m_FirmwareCropSizeX.GetValue();
		pFrame->height = (int)GetStream()->m_FirmwareCropSizeY.GetValue();
		pFrame->cropOriginX = (int)GetStream()->m_FirmwareCropOffsetX.GetValue();
		pFrame->cropOriginY = (int)GetStream()->m_FirmwareCropOffsetY.GetValue();
		pFrame->croppingEnabled = TRUE;
	}
	else
	{
		pFrame->width = pFrame->videoMode.resolutionX;
		pFrame->height = pFrame->videoMode.resolutionY;
		pFrame->cropOriginX = 0;
		pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
	}

	pFrame->stride = pFrame->width * GetStream()->GetBytesPerPixel();

	// call base
	XnFrameStreamProcessor::OnEndOfFrame(pHeader);
}

void XnDepthProcessor::PadPixels(XnUInt32 nPixels)
{
	XnBuffer* pWriteBuffer = GetWriteBuffer();

	// check for overflow
	if (!CheckWriteBufferForOverflow(nPixels * sizeof(OniDepthPixel)))
	{
		return;
	}

	OniDepthPixel* pDepth = (OniDepthPixel*)GetWriteBuffer()->GetUnsafeWritePointer();

	// place the no-depth value
	for (XnUInt32 i = 0; i < nPixels; ++i, ++pDepth)
    {
		*pDepth = m_noDepthValue;
    }
	pWriteBuffer->UnsafeUpdateSize(nPixels * sizeof(OniDepthPixel));
}

void XnDepthProcessor::OnFrameReady(XnUInt32 nFrameID, XnUInt64 nFrameTS)
{
	XnFrameStreamProcessor::OnFrameReady(nFrameID, nFrameTS);

	m_pDevicePrivateData->pSensor->GetFPSCalculator()->MarkDepth(nFrameID, nFrameTS);
}
