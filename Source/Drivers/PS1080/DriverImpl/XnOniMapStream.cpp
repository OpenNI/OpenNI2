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
#include "XnOniMapStream.h"
#include "XnOniColorStream.h"
#include <XnLog.h>

//---------------------------------------------------------------------------
// XnOniMapStream class
//---------------------------------------------------------------------------

XnOniMapStream::XnOniMapStream(XnSensor* pSensor, const XnChar* strName, OniSensorType sensorType, XnOniDevice* pDevice) : 
	XnOniStream(pSensor, strName, sensorType, pDevice),
	m_nSupportedModesCount(0),
	m_aSupportedModes(NULL)
{
}

XnOniMapStream::~XnOniMapStream()
{
	if (m_aSupportedModes != NULL)
	{
		XN_DELETE_ARR(m_aSupportedModes);
		m_aSupportedModes = NULL;
	}
}

XnStatus XnOniMapStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnOniStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = FillSupportedVideoModes();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

OniStatus XnOniMapStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	//TODO: reconsider return values translation issues
	XnStatus nRetVal = XN_STATUS_ERROR;

	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
		{
			if (*pDataSize != sizeof(OniVideoMode))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", *pDataSize, sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}

			nRetVal = GetVideoMode((OniVideoMode*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		}
		case ONI_STREAM_PROPERTY_MIRRORING:
		{
			if (*pDataSize != sizeof(OniBool))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", *pDataSize, sizeof(OniBool));
				return ONI_STATUS_ERROR;
			}

			nRetVal = GetMirror((OniBool*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		}
		case ONI_STREAM_PROPERTY_CROPPING:
		{
			if (*pDataSize != sizeof(OniCropping))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", *pDataSize, sizeof(OniCropping));
				return ONI_STATUS_ERROR;
			}

			nRetVal = GetCropping(*(OniCropping*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		}
		default:
		{
			return XnOniStream::getProperty(propertyId, data, pDataSize);
		}
	}

	return ONI_STATUS_OK;
}

XnStatus XnOniMapStream::SetPropertyImpl(int propertyId, const void* data, int dataSize)
{
	XnStatus nRetVal = XN_STATUS_ERROR;

	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
		{
			if (dataSize != sizeof(OniVideoMode))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", dataSize, sizeof(OniVideoMode));
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}

			nRetVal = SetVideoMode((OniVideoMode*)data);
			XN_IS_STATUS_OK(nRetVal);

			break;
		}
		case ONI_STREAM_PROPERTY_MIRRORING:
		{
			if (dataSize != sizeof(OniBool))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", dataSize, sizeof(OniBool));
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}

			nRetVal = SetMirror((OniBool*)data);
			XN_IS_STATUS_OK(nRetVal);

			break;
		}
		case ONI_STREAM_PROPERTY_CROPPING:
		{
			if (dataSize != sizeof(OniCropping))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", dataSize, sizeof(OniCropping));
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}

			nRetVal = SetCropping(*(OniCropping*)data);
			XN_IS_STATUS_OK(nRetVal);

			break;
		}
		default:
		{
			return XnOniStream::SetPropertyImpl(propertyId, data, dataSize);
		}
	}

	return XN_STATUS_OK;
}

OniBool XnOniMapStream::isPropertySupported(int propertyId)
{
	return (
		propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE ||
		propertyId == ONI_STREAM_PROPERTY_MIRRORING ||
		propertyId == ONI_STREAM_PROPERTY_CROPPING ||
		XnOniStream::isPropertySupported(propertyId));
}

void XnOniMapStream::notifyAllProperties()
{
	XnOniStream::notifyAllProperties();

	XnUInt32 nValue;
	int size = sizeof(nValue);

	// input format
	getProperty(XN_STREAM_PROPERTY_INPUT_FORMAT, &nValue, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_INPUT_FORMAT, &nValue, size);

	// cropping mode
	getProperty(XN_STREAM_PROPERTY_CROPPING_MODE, &nValue, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_CROPPING_MODE, &nValue, size);
}

XnStatus XnOniMapStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	XnStatus nRetVal;
	XnUInt64 nValue;
	
	// output format
	nRetVal = m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_OUTPUT_FORMAT, &nValue);
	XN_IS_STATUS_OK(nRetVal);
	pVideoMode->pixelFormat = (OniPixelFormat)nValue;

	// resolution
	nRetVal = m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_X_RES, &nValue);
	XN_IS_STATUS_OK(nRetVal);
	pVideoMode->resolutionX  = (int)nValue;
	nRetVal = m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_Y_RES, &nValue);
	XN_IS_STATUS_OK(nRetVal);
	pVideoMode->resolutionY = (int)nValue;

	// fps
	nRetVal = m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_FPS, &nValue);
	XN_IS_STATUS_OK(nRetVal);
	pVideoMode->fps = (int)nValue;

	return XN_STATUS_OK;
}

XnBool EqualsResFPS(const OniVideoMode* mode1, const OniVideoMode* mode2)
{
	return (
		mode1->resolutionX  == mode2->resolutionX &&
		mode1->resolutionY == mode2->resolutionY &&
		mode1->fps               == mode2->fps
		);
}

XnStatus XnOniMapStream::SetVideoMode(OniVideoMode* pVideoMode)
{
	XnStatus nRetVal = XN_STATUS_OK;

	OniVideoMode current;
	GetVideoMode(&current);

	if (!xnOSMemCmp(&current, pVideoMode, sizeof(OniVideoMode)))
	{
		// nothing to do here
		return (ONI_STATUS_OK);
	}

	// check if this mode is supported. If it is, make sure current input format is OK
	// note: in case of color stream, the chosen InputFormat should also suit the desired OutputFormat!
	XnUInt64 nCurrInputFormat;
	nRetVal = m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_INPUT_FORMAT, &nCurrInputFormat);
	XN_IS_STATUS_OK(nRetVal);

	XnUInt32 nChosenInputFormat = XN_MAX_UINT32;

	for (XnUInt32 i = 0; i < m_nSupportedModesCount; ++i)
	{
		if (EqualsResFPS(pVideoMode, &m_aSupportedModes[i].OutputMode))
		{
			// if current input format is supported, it will always be preferred.
			if (m_aSupportedModes[i].nInputFormat == nCurrInputFormat)
			{
				if(m_sensorType == ONI_SENSOR_COLOR &&
					XnOniColorStream::IsSupportedInputFormat((XnIOImageFormats)nCurrInputFormat, pVideoMode->pixelFormat) == FALSE)
				{
					continue;
				}
				nChosenInputFormat = (XnUInt32)nCurrInputFormat;
				break;
			}
			else if (nChosenInputFormat == XN_MAX_UINT32 || 
					(m_sensorType == ONI_SENSOR_COLOR && XnOniColorStream::IsPreferredInputFormat(
															(XnIOImageFormats)m_aSupportedModes[i].nInputFormat, 
															(XnIOImageFormats)nChosenInputFormat, 
															pVideoMode->pixelFormat)))
			{
				nChosenInputFormat = m_aSupportedModes[i].nInputFormat;
				// don't break yet. we might find our input format
			}
		}
	}

	if (nChosenInputFormat == XN_MAX_UINT32) // not found
	{
		xnLogWarning(XN_MASK_DEVICE_SENSOR, "Mode %ux%u@%u is not supported!", pVideoMode->resolutionX, pVideoMode->resolutionY, pVideoMode->fps);
		return XN_STATUS_BAD_PARAM;
	}

	XN_PROPERTY_SET_CREATE_ON_STACK(props);
	XnPropertySetAddModule(&props, m_strType);
	XnPropertySetAddIntProperty(&props, m_strType, XN_STREAM_PROPERTY_X_RES, pVideoMode->resolutionX);
	XnPropertySetAddIntProperty(&props, m_strType, XN_STREAM_PROPERTY_Y_RES, pVideoMode->resolutionY);
	XnPropertySetAddIntProperty(&props, m_strType, XN_STREAM_PROPERTY_FPS,   pVideoMode->fps);

	if (nChosenInputFormat != nCurrInputFormat)
	{
		XnPropertySetAddIntProperty(&props, m_strType, XN_STREAM_PROPERTY_INPUT_FORMAT, nChosenInputFormat);
	}
	// set the remotely desired output format
	XnPropertySetAddIntProperty(&props, m_strType, XN_STREAM_PROPERTY_OUTPUT_FORMAT, pVideoMode->pixelFormat);

	nRetVal = m_pSensor->BatchConfig(&props);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnOniMapStream::FillSupportedVideoModes()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// get supported modes
	XnUInt64 nCount;
	nRetVal = m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_SUPPORT_MODES_COUNT, &nCount);
	XN_IS_STATUS_OK(nRetVal);

	m_aSupportedModes = XN_NEW_ARR(SupportedVideoMode, (int)nCount);
	XN_VALIDATE_ALLOC_PTR(m_aSupportedModes);
	m_nSupportedModesCount = (XnUInt32)nCount;

	const XnUInt32 nAllocCount = 150;
	XnCmosPreset aPresets[nAllocCount];
	XN_ASSERT(nAllocCount >= m_nSupportedModesCount);
	OniGeneralBuffer gb = XnGeneralBufferPack(aPresets,  m_nSupportedModesCount * sizeof(XnCmosPreset));
	nRetVal = m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_SUPPORT_MODES, gb);
	XN_IS_STATUS_OK(nRetVal);

	// Keep those modes
	XnBool bOK = TRUE;
	for (XnUInt32 i = 0; i < m_nSupportedModesCount; ++i)
	{
		m_aSupportedModes[i].nInputFormat = aPresets[i].nFormat;
		bOK = XnDDKGetXYFromResolution((XnResolutions)aPresets[i].nResolution, (XnUInt32*)&m_aSupportedModes[i].OutputMode.resolutionX, (XnUInt32*)&m_aSupportedModes[i].OutputMode.resolutionY);
		XN_ASSERT(bOK);
		XN_REFERENCE_VARIABLE(bOK);
		m_aSupportedModes[i].OutputMode.fps = aPresets[i].nFPS;
		m_aSupportedModes[i].OutputMode.pixelFormat = (OniPixelFormat)-1; // this field is not to be used here.
	}

	return (XN_STATUS_OK);
}

XnStatus XnOniMapStream::GetMirror(OniBool* pEnabled)
{
	XnUInt64 intProperty;
	XnStatus nRetVal = m_pSensor->GetProperty(m_strType, XN_MODULE_PROPERTY_MIRROR, &intProperty);
	XN_IS_STATUS_OK(nRetVal);

	// Convert the received value back to OniBool.
	*pEnabled = (OniBool)intProperty;

	return (XN_STATUS_OK);
}

XnStatus XnOniMapStream::SetMirror(OniBool* pEnabled)
{
	XnStatus nRetVal = m_pSensor->SetProperty(m_strType, XN_MODULE_PROPERTY_MIRROR, (XnUInt64)*pEnabled);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnOniMapStream::GetCropping(OniCropping &cropping)
{
	return m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_CROPPING, XN_PACK_GENERAL_BUFFER(cropping));
}

XnStatus XnOniMapStream::SetCropping(const OniCropping &cropping)
{
	OniGeneralBuffer gbValue = XnGeneralBufferPack((void*)&cropping, sizeof(cropping));
	return m_pSensor->SetProperty(m_strType, XN_STREAM_PROPERTY_CROPPING, gbValue);
}

