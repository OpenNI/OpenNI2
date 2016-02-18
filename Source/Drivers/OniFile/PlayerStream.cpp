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
/// @file
/// Contains the definition of Device class that implements a virtual OpenNI
/// device, capable of reading data from a *.ONI file.

#include "PlayerStream.h"
#include "PlayerSource.h"
#include "PlayerDevice.h"
#include "FileProperties.h"
#include "XnMemory.h"
#include "OniProperties.h"
#include "XnPlatform.h"
#include <XnLog.h>

namespace oni_file {

PlayerStream::PlayerStream(PlayerDevice* pDevice, PlayerSource* pSource) :
	m_pSource(pSource), m_newDataHandle(NULL), m_isStarted(false), m_requiredFrameSize(0), m_pDevice(pDevice)
{
}

/// Destructor.
PlayerStream::~PlayerStream()
{
	// Destroy the stream (if it was not destroyed before).
	destroy();
}

OniStatus PlayerStream::Initialize()
{
	// Register events in the source.
	OniStatus rc = m_pSource->RegisterNewDataEvent(OnNewDataCallback, this, m_newDataHandle);
	if (rc != ONI_STATUS_OK)
	{
		destroy();
		return rc;
	}

	return ONI_STATUS_OK;
}

void PlayerStream::destroy()
{
	stop();

	if (m_newDataHandle != NULL)
	{
		// Send the destroy event.
		DestroyEventArgs destroyEventArgs;
		destroyEventArgs.pStream = this;
		m_destroyEvent.Raise(destroyEventArgs);

		// Unregister from events.
		m_pSource->UnregisterNewDataEvent(m_newDataHandle);
		m_newDataHandle = NULL;
	}
}

OniStatus PlayerStream::start()
{
	xnl::AutoCSLocker lock(m_cs);
	m_isStarted = true;
	m_requiredFrameSize = getRequiredFrameSize();

	return ONI_STATUS_OK;
}

void PlayerStream::stop()
{
	xnl::AutoCSLocker lock(m_cs);
	m_isStarted = false;
}

PlayerSource* PlayerStream::GetSource()
{
	return m_pSource;
}

OniStatus PlayerStream::getProperty(int propertyId, void* pData, int* pDataSize)
{
	// Check if the property exists.
	xnl::AutoCSLocker lock(m_cs);
	OniStatus rc = m_properties.GetProperty(propertyId, pData, pDataSize);
	if (rc != ONI_STATUS_OK)
	{
		rc = m_pSource->GetProperty(propertyId, pData, pDataSize);
	}

	return rc;
}

OniStatus PlayerStream::setProperty(int propertyId, const void* pData, int dataSize)
{
	OniStatus nRetVal = ONI_STATUS_OK;

	if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
	{
		if (dataSize != sizeof(OniVideoMode))
		{
			return ONI_STATUS_BAD_PARAMETER;
		}

		OniVideoMode currMode;
		int size = sizeof(currMode);
		nRetVal = getProperty(ONI_STREAM_PROPERTY_VIDEO_MODE, &currMode, &size);
		XN_ASSERT(nRetVal == ONI_STATUS_OK);
		XN_REFERENCE_VARIABLE(nRetVal);

		OniVideoMode* pRequestedMode = (OniVideoMode*)pData;
		if (pRequestedMode->resolutionX == currMode.resolutionX &&
			pRequestedMode->resolutionY == currMode.resolutionY &&
			pRequestedMode->fps == currMode.fps &&
			pRequestedMode->pixelFormat == currMode.pixelFormat)
		{
			return ONI_STATUS_OK;
		}

		return ONI_STATUS_BAD_PARAMETER;
	}
	else
	{
		return ONI_STATUS_ERROR;
	}
}

OniStatus PlayerStream::RegisterReadyForDataEvent(ReadyForDataCallback callback, void* pCookie, OniCallbackHandle& handle)
{
	XnStatus rc = m_readyForDataEvent.Register(callback, pCookie, (XnCallbackHandle&)handle);
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}
	return ONI_STATUS_OK;
}

void PlayerStream::UnregisterReadyForDataEvent(OniCallbackHandle handle)
{
	m_readyForDataEvent.Unregister((XnCallbackHandle)handle);
}

OniStatus PlayerStream::RegisterDestroyEvent(DestroyCallback callback, void* pCookie, OniCallbackHandle& handle)
{
	XnStatus rc = m_destroyEvent.Register(callback, pCookie, (XnCallbackHandle&)handle);
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}
	return ONI_STATUS_OK;
}

void PlayerStream::UnregisterDestroyEvent(OniCallbackHandle handle)
{
	m_destroyEvent.Unregister((XnCallbackHandle)handle);
}

void ONI_CALLBACK_TYPE PlayerStream::OnNewDataCallback(const PlayerSource::NewDataEventArgs& newDataEventArgs, void* pCookie)
{
	PlayerStream* pStream = (PlayerStream*)pCookie;
	xnl::AutoCSLocker lock(pStream->m_cs);

	// Don't process new frames until the stream is started.
	if(!pStream->m_isStarted)
	{
		return;
	}

	// Get the video mode.
	OniVideoMode videoMode;
	int valueSize = sizeof(videoMode);
	OniStatus rc = pStream->m_pSource->GetProperty(ONI_STREAM_PROPERTY_VIDEO_MODE, &videoMode, &valueSize);
	if (rc != ONI_STATUS_OK)
	{
		XN_ASSERT(FALSE);
		return;
	}

	// Get stride.
	int stride;
	valueSize = sizeof(stride);
	rc = pStream->m_pSource->GetProperty(ONI_STREAM_PROPERTY_STRIDE, &stride, &valueSize);
	if (rc != ONI_STATUS_OK)
	{
		XN_ASSERT(FALSE);
		return;
	}

	// Set the cropping property.
	OniCropping cropping;
	int dataSize = sizeof(cropping);
	rc = pStream->m_pSource->GetProperty(ONI_STREAM_PROPERTY_CROPPING, &cropping, &dataSize);
	if (rc != ONI_STATUS_OK)
	{
		cropping.enabled = FALSE;
	}

	// Allocate new frame and fill it.
	OniFrame* pFrame = pStream->getServices().acquireFrame();
	if (pFrame == NULL)
	{
		return;
	}

	// Fill the frame.
	pFrame->frameIndex = newDataEventArgs.nFrameId;

	pFrame->videoMode.pixelFormat = videoMode.pixelFormat;
	pFrame->videoMode.resolutionX = videoMode.resolutionX;
	pFrame->videoMode.resolutionY = videoMode.resolutionY;
	pFrame->videoMode.fps = videoMode.fps;
	if (!cropping.enabled)
	{
		// Set the full resolution, stride and origin.
		pFrame->width = videoMode.resolutionX;
		pFrame->height = videoMode.resolutionY;
		pFrame->stride = stride;
		pFrame->cropOriginX = 0;
		pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
	}
	else
	{
		// Take resolution, and origin from cropping and calculate new stride.
		pFrame->width = cropping.width;
		pFrame->height = cropping.height;
		pFrame->stride = (stride / pFrame->videoMode.resolutionX) * cropping.width;
		pFrame->cropOriginX = cropping.originX;
		pFrame->cropOriginY = cropping.originY;
		pFrame->croppingEnabled = TRUE;
	}
	pFrame->sensorType = pStream->m_pSource->GetInfo()->sensorType;
	pFrame->timestamp = newDataEventArgs.nTimeStamp;
	pFrame->dataSize = newDataEventArgs.nSize;
	if (pFrame->dataSize > pStream->m_requiredFrameSize)
	{
		xnLogWarning("Player", "File contains a frame with size %d whereas required frame size is %d", pFrame->dataSize, pStream->m_requiredFrameSize);
		XN_ASSERT(FALSE);
		pFrame->dataSize = pStream->m_requiredFrameSize;
	}
	memcpy(pFrame->data, newDataEventArgs.pData, pFrame->dataSize);

	// Process the new frame.
	pStream->raiseNewFrame(pFrame);
	pStream->getServices().releaseFrame(pFrame);
}

int PlayerStream::getRequiredFrameSize()
{
	xnl::AutoCSLocker lock(m_cs);
	int requiredFrameSize = m_pSource->GetRequiredFrameSize();
	if (requiredFrameSize == 0)
	{
		// not set for some reason, use default one
		requiredFrameSize = StreamBase::getRequiredFrameSize();
	}

	return requiredFrameSize;
}

void PlayerStream::notifyAllProperties()
{
	xnl::AutoCSLocker lock(m_cs);
	raisePropertyChanged(ONI_FILE_PROPERTY_ORIGINAL_DEVICE, m_pDevice->getOriginalDevice(), ONI_MAX_STR);

	for (PlayerProperties::PropertiesHash::ConstIterator property = m_properties.Begin();
		property != m_properties.End(); ++property)
	{
		raisePropertyChanged(property->Key(), property->Value()->data, property->Value()->dataSize);
	}

	for (PlayerProperties::PropertiesHash::ConstIterator property = m_pSource->Begin();
		property != m_pSource->End(); ++property)
	{
		raisePropertyChanged(property->Key(), property->Value()->data, property->Value()->dataSize);
	}
}

} // namespace oni_files_player
