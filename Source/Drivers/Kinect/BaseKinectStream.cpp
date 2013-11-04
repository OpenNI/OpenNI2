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
#include "BaseKinectStream.h"
#include "KinectStreamImpl.h"
#include <Shlobj.h>
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"
#include "NuiApi.h"
#include "PS1080.h"
#include "XnMath.h"

using namespace oni::driver;
using namespace kinect_device;

BaseKinectStream::BaseKinectStream(KinectStreamImpl* pStreamImpl):
	m_pStreamImpl(pStreamImpl)
{
	m_running = false;
	m_cropping.enabled = FALSE;
	m_mirroring = FALSE;
	pStreamImpl->addStream(this);
}

BaseKinectStream::~BaseKinectStream()
{
	destroy();
}

OniStatus BaseKinectStream::start()
{
	OniStatus status = m_pStreamImpl->start();
	if (status == ONI_STATUS_OK)
		m_running = TRUE;
	return status;
}

void BaseKinectStream::stop()
{
	m_running = FALSE;
	m_pStreamImpl->stop();
}

void BaseKinectStream::destroy()
{
	stop();
	m_pStreamImpl->removeStream(this);
}

// TODO: EXACT_PROP_SIZE_OR_XXX should be used for property size checking rather than copy-and-pasting
OniStatus BaseKinectStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	OniStatus status = ONI_STATUS_NOT_SUPPORTED;
	switch (propertyId)
	{
	case ONI_STREAM_PROPERTY_CROPPING:
		if (*pDataSize != sizeof(OniCropping))
		{
			printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniCropping));
			status = ONI_STATUS_ERROR;
		}
		else
		{
			status = GetCropping((OniCropping*)data);
		}
		break;
	case ONI_STREAM_PROPERTY_MIRRORING:
		{
			EXACT_PROP_SIZE_OR_RETURN(*pDataSize, OniBool);
			*(OniBool*)data = m_mirroring;
			status = ONI_STATUS_OK;
		}
		break;
	case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
		{
			float* val = (float*)data;
			XnDouble tmp;
			if (m_videoMode.resolutionX == 640)
				tmp =  NUI_CAMERA_COLOR_NOMINAL_HORIZONTAL_FOV * xnl::Math::DTR;
			else
				tmp = NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV * xnl::Math::DTR;
			*val = (float)tmp;
			status = ONI_STATUS_OK;
			break;
		}		
	case ONI_STREAM_PROPERTY_VERTICAL_FOV:
		{
			float* val = (float*)data;
			XnDouble tmp;
			if (m_videoMode.resolutionY == 480)
				tmp =  NUI_CAMERA_COLOR_NOMINAL_VERTICAL_FOV * xnl::Math::DTR;
			else
				tmp = NUI_CAMERA_DEPTH_NOMINAL_VERTICAL_FOV * xnl::Math::DTR;
			*val = (float)tmp;
			status = ONI_STATUS_OK;
			break;
		}
	case ONI_STREAM_PROPERTY_VIDEO_MODE:
		{
			if (*pDataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVideoMode));
				status = ONI_STATUS_ERROR;
			}
			else
			{
				status = GetVideoMode((OniVideoMode*)data);
			}
			
			break;
		}		
	default:
		status = ONI_STATUS_NOT_SUPPORTED;
		break;
	}

	return status;
}

OniStatus BaseKinectStream::setProperty(int propertyId, const void* data, int dataSize)
{
	OniStatus status = ONI_STATUS_NOT_SUPPORTED;
	if (propertyId == ONI_STREAM_PROPERTY_CROPPING)
	{
		if (dataSize != sizeof(OniCropping))
		{
			printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniCropping));
			status = ONI_STATUS_ERROR;
		}
		status = SetCropping((OniCropping*)data);
	}
	else if (propertyId == ONI_STREAM_PROPERTY_MIRRORING)
	{
		EXACT_PROP_SIZE_OR_RETURN(dataSize, OniBool);
		m_mirroring = *(OniBool*)data;
		status = ONI_STATUS_OK;
	}
	else if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
	{
		if (dataSize != sizeof(OniVideoMode))
		{
			printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniVideoMode));
			 status = ONI_STATUS_ERROR;
		}
		status = SetVideoMode((OniVideoMode*)data);
	}
	return status;
}

OniBool BaseKinectStream::isPropertySupported(int propertyId)
{
	OniBool status = FALSE;
	switch (propertyId)
	{
	case ONI_STREAM_PROPERTY_CROPPING:
	case ONI_STREAM_PROPERTY_MIRRORING:
	case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
	case ONI_STREAM_PROPERTY_VERTICAL_FOV:
	case ONI_STREAM_PROPERTY_VIDEO_MODE:
		status = TRUE;
		break;
	default:
		status = FALSE;
		break;
	}
	return status;
}

OniStatus BaseKinectStream::SetVideoMode(OniVideoMode* videoMode)
{
	if (!m_pStreamImpl->isRunning())
	{
		m_videoMode = *videoMode;
		m_pStreamImpl->setVideoMode(videoMode);
		return ONI_STATUS_OK;
	}
	
	return ONI_STATUS_OUT_OF_FLOW;
}

OniStatus BaseKinectStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	*pVideoMode = m_videoMode;
	return ONI_STATUS_OK;
}

OniStatus BaseKinectStream::SetCropping(OniCropping* cropping)
{
	m_cropping = *cropping;
	return ONI_STATUS_OK;
}

OniStatus BaseKinectStream::GetCropping(OniCropping* cropping)
{
	*cropping = m_cropping;
	return ONI_STATUS_OK;
}
