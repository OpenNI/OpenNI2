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
#include "LinkOniMapStream.h"
//#include "LinkOniColorStream.h"
#include <XnLog.h>

//---------------------------------------------------------------------------
// LinkOniMapStream class
//---------------------------------------------------------------------------

LinkOniMapStream::LinkOniMapStream(const char* configFile, const char* configSection, xn::PrimeClient* pSensor, OniSensorType sensorType, LinkOniDevice* pDevice) : 
	LinkOniStream(configFile, configSection, pSensor, sensorType, pDevice),
	m_nSupportedModesCount(0),
	m_aSupportedModes(NULL)
{
}

LinkOniMapStream::~LinkOniMapStream()
{
	if (m_aSupportedModes != NULL)
	{
		XN_DELETE_ARR(m_aSupportedModes);
		m_aSupportedModes = NULL;
	}
}

XnStatus LinkOniMapStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = LinkOniStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = FillSupportedVideoModes();
	XN_IS_STATUS_OK(nRetVal);

	// read video mode
	XnChar videoModeSection[255];
	sprintf(videoModeSection, "%s.VideoMode", m_configSection);
	OniVideoMode videoMode;
	GetVideoMode(&videoMode);

	// override with streams default values
	GetDefaultVideoMode(&videoMode);

	// override with INI config
	XnInt32 temp32;
	if (XN_STATUS_OK == xnOSReadIntFromINI(m_configFile, videoModeSection, "XResolution", &temp32))
	{
		videoMode.resolutionX = (int)temp32;
	}
	if (XN_STATUS_OK == xnOSReadIntFromINI(m_configFile, videoModeSection, "YResolution", &temp32))
	{
		videoMode.resolutionY = (int)temp32;
	}
	if (XN_STATUS_OK == xnOSReadIntFromINI(m_configFile, videoModeSection, "FPS", &temp32))
	{
		videoMode.fps = (int)temp32;
	}
	if (XN_STATUS_OK == xnOSReadIntFromINI(m_configFile, videoModeSection, "PixelFormat", &temp32))
	{
		videoMode.pixelFormat = (OniPixelFormat)temp32;
	}

	nRetVal = SetVideoMode(&videoMode);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = setIntPropertyFromINI("LinkPixelFormat", LINK_PROP_PIXEL_FORMAT);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = setIntPropertyFromINI("Compression", LINK_PROP_COMPRESSION);
	XN_IS_STATUS_OK(nRetVal);

	OniBool bMirror = TRUE;
	if (XN_STATUS_OK == xnOSReadIntFromINI(m_configFile, m_configSection, "Mirror", &temp32))
	{
		bMirror = (temp32 == 1);
	}

	nRetVal = SetMirror(bMirror);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

OniStatus LinkOniMapStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	XnStatus nRetVal = XN_STATUS_ERROR;

	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
			EXACT_PROP_SIZE(*pDataSize, OniVideoMode);
			nRetVal = GetVideoMode((OniVideoMode*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		
		case ONI_STREAM_PROPERTY_MIRRORING:
			EXACT_PROP_SIZE(*pDataSize, OniBool);
			nRetVal = GetMirror((OniBool*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		
		case ONI_STREAM_PROPERTY_CROPPING:
			EXACT_PROP_SIZE(*pDataSize, OniCropping);
			nRetVal = GetCropping(*(OniCropping*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;

		case LINK_PROP_PIXEL_FORMAT:
			ENSURE_PROP_SIZE(*pDataSize, XnLinkPixelFormat);
			ASSIGN_PROP_VALUE_INT(data, *pDataSize, m_pInputStream->GetVideoMode().m_nPixelFormat);
			break;
			
		case LINK_PROP_COMPRESSION:
			ENSURE_PROP_SIZE(*pDataSize, XnLinkCompressionType);
			ASSIGN_PROP_VALUE_INT(data, *pDataSize, m_pInputStream->GetVideoMode().m_nCompression);
			break;

		case PS_PROPERTY_GAIN:
			{
				ENSURE_PROP_SIZE(*pDataSize, XnUInt16);
				XnUInt16 gain;
				nRetVal = m_pInputStream->GetGain(gain);
				XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
				ASSIGN_PROP_VALUE_INT(data, *pDataSize, gain);
			}
			break;

		default:
		{
			return LinkOniStream::getProperty(propertyId, data, pDataSize);
		}
	}

	return ONI_STATUS_OK;
}

OniStatus LinkOniMapStream::setProperty(int propertyId, const void* data, int dataSize)
{
	XnStatus nRetVal = XN_STATUS_ERROR;

	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
			EXACT_PROP_SIZE(dataSize, OniVideoMode);
			nRetVal = SetVideoMode((OniVideoMode*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		
		case ONI_STREAM_PROPERTY_MIRRORING:
			EXACT_PROP_SIZE(dataSize, OniBool);
			nRetVal = SetMirror(*(OniBool*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		
		case ONI_STREAM_PROPERTY_CROPPING:
			EXACT_PROP_SIZE(dataSize, OniCropping);
			nRetVal = SetCropping(*(OniCropping*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;

		case LINK_PROP_PIXEL_FORMAT:
			{
				ENSURE_PROP_SIZE(dataSize, XnLinkPixelFormat);
				XnFwStreamVideoMode mode = m_pInputStream->GetVideoMode();
				mode.m_nPixelFormat = *(XnFwPixelFormat*)data;
				nRetVal = m_pInputStream->SetVideoMode(mode);
				XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
				break;
			}

		case LINK_PROP_COMPRESSION:
			{
				ENSURE_PROP_SIZE(dataSize, XnLinkCompressionType);
				XnFwStreamVideoMode mode = m_pInputStream->GetVideoMode();
				mode.m_nCompression = *(XnFwCompressionType*)data;
				nRetVal = m_pInputStream->SetVideoMode(mode);
				XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
				break;
			}

		case PS_PROPERTY_GAIN:
			{
				ENSURE_PROP_SIZE(dataSize, XnUInt16);
				nRetVal = m_pInputStream->SetGain(*(XnUInt16*)data);
				XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
				break;
			}

		default:
			return LinkOniStream::setProperty(propertyId, data, dataSize);
	}

	return ONI_STATUS_OK;
}

OniBool LinkOniMapStream::isPropertySupported(int propertyId)
{
	switch (propertyId)
	{
	case ONI_STREAM_PROPERTY_VIDEO_MODE:
	case ONI_STREAM_PROPERTY_MIRRORING:
	case ONI_STREAM_PROPERTY_CROPPING:
	case LINK_PROP_PIXEL_FORMAT:
	case LINK_PROP_COMPRESSION:
		return true;
	default:
		return LinkOniStream::isPropertySupported(propertyId);
	}
}

void LinkOniMapStream::notifyAllProperties()
{
	LinkOniStream::notifyAllProperties();

	int nValue;
	int size = sizeof(int);
	
	getProperty(LINK_PROP_PIXEL_FORMAT, &nValue, &size);
	raisePropertyChanged(LINK_PROP_PIXEL_FORMAT, &nValue, size);

	getProperty(LINK_PROP_COMPRESSION, &nValue, &size);
	raisePropertyChanged(LINK_PROP_COMPRESSION, &nValue, size);
}

XnStatus LinkOniMapStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	// output format
	pVideoMode->pixelFormat = m_pInputStream->GetOutputFormat();

	// resolution
	pVideoMode->resolutionX = (int)m_pInputStream->GetVideoMode().m_nXRes;
	pVideoMode->resolutionY = (int)m_pInputStream->GetVideoMode().m_nYRes;
	
	// fps
	pVideoMode->fps = (int)m_pInputStream->GetVideoMode().m_nFPS;
	
	return XN_STATUS_OK;
}

XnStatus LinkOniMapStream::SetVideoMode(OniVideoMode* pVideoMode)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnFwStreamVideoMode currFwMode = m_pInputStream->GetVideoMode();

	if ((int)currFwMode.m_nXRes == pVideoMode->resolutionX &&
		(int)currFwMode.m_nYRes == pVideoMode->resolutionY &&
		(int)currFwMode.m_nFPS == pVideoMode->fps &&
		m_pInputStream->GetOutputFormat() == pVideoMode->pixelFormat)
	{
		// nothing to do here
		return (ONI_STATUS_OK);
	}

	// now look for the first mode that matches
	const xnl::Array<XnFwStreamVideoMode>& supportedModes = m_pInputStream->GetSupportedVideoModes();
	XnInt32 selectedIndex = -1;
	for (XnUInt32 i = 0; i < supportedModes.GetSize(); ++i)
	{
		if (pVideoMode->resolutionX == (int)supportedModes[i].m_nXRes &&
			pVideoMode->resolutionY == (int)supportedModes[i].m_nYRes &&
			pVideoMode->fps         == (int)supportedModes[i].m_nFPS)
		{
			// prefer the one that also keeps on other parameters. If no such mode exists, switch to the first one on the list
			if (supportedModes[i].m_nPixelFormat == currFwMode.m_nPixelFormat &&
				supportedModes[i].m_nCompression == currFwMode.m_nCompression)
			{
				selectedIndex = i;
				break;
			}
			else if (selectedIndex == -1)
			{
				selectedIndex = i;
			}		
		}
	}

	if (selectedIndex == -1)
	{
		xnLogError(XN_MASK_LINK, "Tried to set unsupported mode: %ux%u@%u fps", 
			pVideoMode->resolutionX, pVideoMode->resolutionY, pVideoMode->fps);
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}

	nRetVal = m_pInputStream->SetOutputFormat(pVideoMode->pixelFormat);
	XN_IS_STATUS_OK_LOG_ERROR("Set output format", nRetVal);

	nRetVal = m_pInputStream->SetVideoMode(supportedModes[selectedIndex]);
	XN_IS_STATUS_OK_LOG_ERROR("Set video mode", nRetVal);

	xnLogVerbose(XN_MASK_LINK,"Set video mode to  %ux%u@%u fps & pixel format: %u", pVideoMode->resolutionX, pVideoMode->resolutionY, pVideoMode->fps, pVideoMode->pixelFormat);
	return XN_STATUS_OK;
}

XnStatus LinkOniMapStream::FillSupportedVideoModes()
{
	int nCount;
	const xnl::Array<XnFwStreamVideoMode> *pSupported;
	pSupported = &m_pInputStream->GetSupportedVideoModes();
	nCount = (int)pSupported->GetSize();

	m_aSupportedModes = XN_NEW_ARR(SupportedVideoMode, nCount);
	XN_VALIDATE_ALLOC_PTR(m_aSupportedModes);
	m_nSupportedModesCount = nCount;

	for (int i = 0; i < nCount; ++i)
	{
		m_aSupportedModes[i].nInputFormat			= pSupported->GetData()[i].m_nPixelFormat;
		
		m_aSupportedModes[i].OutputMode.resolutionX	= pSupported->GetData()[i].m_nXRes;
		m_aSupportedModes[i].OutputMode.resolutionY	= pSupported->GetData()[i].m_nYRes;;
		m_aSupportedModes[i].OutputMode.fps			= pSupported->GetData()[i].m_nFPS;;
		m_aSupportedModes[i].OutputMode.pixelFormat	= (OniPixelFormat)-1; // this field is not to be used here.;
	}

	return (XN_STATUS_OK);
}

XnStatus LinkOniMapStream::GetMirror(OniBool* pEnabled)
{
	*pEnabled = (OniBool)m_pInputStream->GetMirror();
	return (XN_STATUS_OK);
}

XnStatus LinkOniMapStream::SetMirror(OniBool enabled)
{
	return m_pInputStream->SetMirror((XnBool)enabled);
}

XnStatus LinkOniMapStream::GetCropping(OniCropping &cropping)
{
	const OniCropping &pCropping = m_pInputStream->GetCropping();
	xnOSMemCopy(&cropping, &pCropping, sizeof(OniCropping));
	return (XN_STATUS_OK);
}

XnStatus LinkOniMapStream::SetCropping(const OniCropping &cropping)
{
	return m_pInputStream->SetCropping(cropping);
}

XnStatus LinkOniMapStream::GetDefaultVideoMode( OniVideoMode* /*pVideoMode*/ )
{
	return XN_STATUS_IS_EMPTY;
}

