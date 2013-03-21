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
#include "XnFirmwareStreams.h"
#include <XnProfiling.h>
#include "XnSensor.h"

#include "XnWavelengthCorrectionDebugProcessor.h"
#include "XnGMCDebugProcessor.h"
#include "XnTecDebugProcessor.h"
#include "XnNesaDebugProcessor.h"
#include "XnGeneralDebugProcessor.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnFirmwareStreams::XnFirmwareStreams(XnDevicePrivateData* pDevicePrivateData) :
	m_pDevicePrivateData(pDevicePrivateData)
{
}

XnFirmwareStreams::~XnFirmwareStreams()
{
}

XnStatus XnFirmwareStreams::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnFirmwareStreamData tempData;
	xnOSMemSet(&tempData, 0, sizeof(XnFirmwareStreamData));

	// Depth
	nRetVal = m_DepthProcessor.Init();
	XN_IS_STATUS_OK(nRetVal);

	tempData.pProcessorHolder = &m_DepthProcessor;
	tempData.strType = XN_STREAM_TYPE_DEPTH;
	nRetVal = m_FirmwareStreams.Set(XN_STREAM_TYPE_DEPTH, tempData);
	XN_IS_STATUS_OK(nRetVal);

	// Image
	nRetVal = m_ImageProcessor.Init();
	XN_IS_STATUS_OK(nRetVal);

	tempData.pProcessorHolder = &m_ImageProcessor;
	tempData.strType = XN_STREAM_TYPE_IMAGE;
	nRetVal = m_FirmwareStreams.Set(XN_STREAM_TYPE_IMAGE, tempData);
	XN_IS_STATUS_OK(nRetVal);

	// IR (currently uses the same processor
	tempData.pProcessorHolder = &m_ImageProcessor;
	tempData.strType = XN_STREAM_TYPE_IR;
	nRetVal = m_FirmwareStreams.Set(XN_STREAM_TYPE_IR, tempData);
	XN_IS_STATUS_OK(nRetVal);

	// Audio
	nRetVal = m_AudioProcessor.Init();
	XN_IS_STATUS_OK(nRetVal);

	tempData.pProcessorHolder = &m_AudioProcessor;
	tempData.strType = XN_STREAM_TYPE_AUDIO;
	nRetVal = m_FirmwareStreams.Set(XN_STREAM_TYPE_AUDIO, tempData);
	XN_IS_STATUS_OK(nRetVal);

	// GMC debug processor
	nRetVal = m_GMCDebugProcessor.Init();
	XN_IS_STATUS_OK(nRetVal);

	XnDataProcessor* pProcessor;
	XN_VALIDATE_NEW_AND_INIT(pProcessor, XnGMCDebugProcessor, m_pDevicePrivateData);
	m_GMCDebugProcessor.Replace(pProcessor);

	// Wave length debug processor
	nRetVal = m_WavelengthCorrectionDebugProcessor.Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_NEW_AND_INIT(pProcessor, XnWavelengthCorrectionDebugProcessor, m_pDevicePrivateData);
	m_WavelengthCorrectionDebugProcessor.Replace(pProcessor);

	// Tec Debug processor
	nRetVal = m_TecDebugProcessor.Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_NEW_AND_INIT(pProcessor, XnTecDebugProcessor, m_pDevicePrivateData);
	m_TecDebugProcessor.Replace(pProcessor);

	// NESA Debug processor
	nRetVal = m_NesaDebugProcessor.Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_NEW_AND_INIT(pProcessor, XnNesaDebugProcessor, m_pDevicePrivateData);
	m_NesaDebugProcessor.Replace(pProcessor);

	// General Debug processors
	nRetVal = m_GeneralDebugProcessor1.Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_NEW_AND_INIT(pProcessor, XnGeneralDebugProcessor, m_pDevicePrivateData);
	m_GeneralDebugProcessor1.Replace(pProcessor);

	nRetVal = m_GeneralDebugProcessor2.Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_NEW_AND_INIT(pProcessor, XnGeneralDebugProcessor, m_pDevicePrivateData);
	m_GeneralDebugProcessor2.Replace(pProcessor);

	return (XN_STATUS_OK);
}

XnStatus XnFirmwareStreams::CheckClaimStream(const XnChar* strType, XnResolutions nRes, XnUInt32 nFPS, XnDeviceStream* pOwner)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// first of all, make sure this stream isn't claimed already
	XnFirmwareStreamData* pStreamData = NULL;
	nRetVal = m_FirmwareStreams.Get(strType, pStreamData);
	XN_IS_STATUS_OK(nRetVal);

	if (pStreamData->pOwner != NULL && pStreamData->pOwner != pOwner)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Cannot open more than one %s stream at a time!", strType);
	}

	if (strcmp(strType, XN_STREAM_TYPE_DEPTH) == 0)
	{
		// check if IR stream is configured
		XnFirmwareStreamData* pIRStreamData = NULL;
		nRetVal = m_FirmwareStreams.Get(XN_STREAM_TYPE_IR, pIRStreamData);
		XN_IS_STATUS_OK(nRetVal);

		if (pIRStreamData->pOwner != NULL)
		{
			// check res
			if (pIRStreamData->nRes != nRes && (pIRStreamData->nRes != XN_RESOLUTION_SXGA || nRes != XN_RESOLUTION_VGA))
			{
				XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Cannot set depth stream to resolution %d when IR is set to resolution %d!", nRes, pIRStreamData->nRes);
			}

			// check FPS
			if (pIRStreamData->nFPS != nFPS)
			{
				XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Depth and IR streams must have the same FPS!");
			}
		}
	}
	else if (strcmp(strType, XN_STREAM_TYPE_IR) == 0)
	{
		// check if image is configured
		XnFirmwareStreamData* pImageStreamData = NULL;
		nRetVal = m_FirmwareStreams.Get(XN_STREAM_TYPE_IMAGE, pImageStreamData);
		XN_IS_STATUS_OK(nRetVal);

		if (pImageStreamData->pOwner != NULL)
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Cannot open IR stream when image stream is on!");
		}

		// check if depth is configured
		XnFirmwareStreamData* pDepthStreamData = NULL;
		nRetVal = m_FirmwareStreams.Get(XN_STREAM_TYPE_DEPTH, pDepthStreamData);
		XN_IS_STATUS_OK(nRetVal);

		if (pDepthStreamData->pOwner != NULL)
		{
			// check res
			if (pDepthStreamData->nRes != nRes && (nRes != XN_RESOLUTION_SXGA || pDepthStreamData->nRes != XN_RESOLUTION_VGA))
			{
				if (m_pDevicePrivateData->FWInfo.nFWVer < XN_SENSOR_FW_VER_5_6) 
				{
					XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Cannot set IR stream to resolution %d when Depth is set to resolution %d!", nRes, pDepthStreamData->nRes);
				}
			}

			// check FPS
			if (pDepthStreamData->nFPS != nFPS)
			{
				XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Depth and IR streams must have the same FPS!");
			}
		}
	}
	else if (strcmp(strType, XN_STREAM_TYPE_IMAGE) == 0)
	{
		// check if IR is configured
		XnFirmwareStreamData* pIRStreamData;
		nRetVal = m_FirmwareStreams.Get(XN_STREAM_TYPE_IR, pIRStreamData);
		XN_IS_STATUS_OK(nRetVal);

		if (pIRStreamData->pOwner != NULL)
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Cannot open Image stream when IR stream is on!");
		}
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnFirmwareStreams::ClaimStream(const XnChar* strType, XnResolutions nRes, XnUInt32 nFPS, XnDeviceStream* pOwner)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// check constraints
	nRetVal = CheckClaimStream(strType, nRes, nFPS, pOwner);
	XN_IS_STATUS_OK(nRetVal);

	// get stream data
	XnFirmwareStreamData* pData = NULL;
	nRetVal = m_FirmwareStreams.Get(strType, pData);
	XN_IS_STATUS_OK(nRetVal);

	// update stream
	pData->nRes = nRes;
	pData->nFPS = nFPS;
	pData->pOwner = pOwner;

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "FW Stream %s was claimed by %s", strType, pOwner->GetName());
	
	return (XN_STATUS_OK);
}

XnStatus XnFirmwareStreams::ReleaseStream(const XnChar* strType, XnDeviceStream* pOwner)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// get stream data
	XnFirmwareStreamData* pData = NULL;
	nRetVal = m_FirmwareStreams.Get(strType, pData);
	XN_IS_STATUS_OK(nRetVal);

	if (pData->pOwner == NULL || pData->pOwner != pOwner)
	{
		return XN_STATUS_ERROR;
	}

	// release it
	pData->pOwner = NULL;
	pData->pProcessorHolder->Replace(NULL);

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Stream %s released FW Stream %s", pOwner->GetName(), strType);
	
	return (XN_STATUS_OK);
}

XnStatus XnFirmwareStreams::LockStreamProcessor(const XnChar* strType, XnDeviceStream* pOwner)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// get stream data
	XnFirmwareStreamData* pData = NULL;
	nRetVal = m_FirmwareStreams.Get(strType, pData);
	XN_IS_STATUS_OK(nRetVal);

	if (pData->pOwner != pOwner)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DEVICE_SENSOR, "Internal error: Trying to lock a processor for a non-owned stream!");
	}

	pData->pProcessorHolder->Lock();

	return XN_STATUS_OK;
}

XnStatus XnFirmwareStreams::UnlockStreamProcessor(const XnChar* strType, XnDeviceStream* pOwner)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// get stream data
	XnFirmwareStreamData* pData = NULL;
	nRetVal = m_FirmwareStreams.Get(strType, pData);
	XN_IS_STATUS_OK(nRetVal);

	if (pData->pOwner != pOwner)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DEVICE_SENSOR, "Internal error: Trying to unlock a processor for a non-owned stream!");
	}

	pData->pProcessorHolder->Unlock();

	return XN_STATUS_OK;
}

XnStatus XnFirmwareStreams::ReplaceStreamProcessor(const XnChar* strType, XnDeviceStream* pOwner, XnDataProcessor* pProcessor)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// get stream data
	XnFirmwareStreamData* pData = NULL;
	nRetVal = m_FirmwareStreams.Get(strType, pData);
	XN_IS_STATUS_OK(nRetVal);

	if (pData->pOwner != pOwner)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DEVICE_SENSOR, "Internal error: Trying to replace a processor for a non-owned stream!");
	}

	pData->pProcessorHolder->Replace(pProcessor);

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Firmware stream '%s' processor was replaced.", strType);

	return (XN_STATUS_OK);
}

XnBool XnFirmwareStreams::IsClaimed(const XnChar* strType, XnDeviceStream* pStream)
{
	XnFirmwareStreamData* pData = NULL;
	if (XN_STATUS_OK == m_FirmwareStreams.Get(strType, pData) && pData->pOwner == pStream)
		return TRUE;
	else
		return FALSE;
}

void XnFirmwareStreams::ProcessPacketChunk(XnSensorProtocolResponseHeader* pHeader, XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize)
{
	XN_PROFILING_START_MT_SECTION("XnFirmwareStreams::ProcessPacketChunk")

	XnDataProcessorHolder* pStreamProcessor = NULL;

	switch (pHeader->nType)
	{
	case XN_SENSOR_PROTOCOL_RESPONSE_DEPTH_START:
	case XN_SENSOR_PROTOCOL_RESPONSE_DEPTH_BUFFER:
	case XN_SENSOR_PROTOCOL_RESPONSE_DEPTH_END:
		pStreamProcessor = &m_DepthProcessor;
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_IMAGE_START:
	case XN_SENSOR_PROTOCOL_RESPONSE_IMAGE_BUFFER:
	case XN_SENSOR_PROTOCOL_RESPONSE_IMAGE_END:
		pStreamProcessor = &m_ImageProcessor;
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_AUDIO_BUFFER:
		pStreamProcessor = &m_AudioProcessor;
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_GMC_DEBUG:
	case XN_SENSOR_PROTOCOL_RESPONSE_GMC_DEBUG_END:
		pStreamProcessor = &m_GMCDebugProcessor;
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_WAVELENGTH_CORRECTION_DEBUG:
		pStreamProcessor = &m_WavelengthCorrectionDebugProcessor;
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_TEC_DEBUG:
		pStreamProcessor = &m_TecDebugProcessor;
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_NESA_DEBUG:
		pStreamProcessor = &m_NesaDebugProcessor;
		break;
	case XN_SENSOR_PROTOCOL_DEBUG_DATA_EP1:
		pStreamProcessor = &m_GeneralDebugProcessor1;
		break;
	case XN_SENSOR_PROTOCOL_DEBUG_DATA_EP2:
		pStreamProcessor = &m_GeneralDebugProcessor2;
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_PROJECTOR_FAULT_EVENT:
		m_pDevicePrivateData->pSensor->SetErrorState(XN_STATUS_DEVICE_PROJECTOR_FAULT);
		break;
	case XN_SENSOR_PROTOCOL_RESPONSE_OVERHEAT:
		m_pDevicePrivateData->pSensor->SetErrorState(XN_STATUS_DEVICE_OVERHEAT);
		break;
	default:
		{
			xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "Unknown packet type (0x%x)!!!", pHeader->nType);
		}
	}

	if (pStreamProcessor != NULL)
	{
		if (m_pDevicePrivateData->pSensor->GetErrorState() != XN_STATUS_OK)
		{
			// all OK now.
			m_pDevicePrivateData->pSensor->SetErrorState(XN_STATUS_OK);
		}

		pStreamProcessor->ProcessData(pHeader, pData, nDataOffset, nDataSize);
	}

	XN_PROFILING_END_SECTION
}
