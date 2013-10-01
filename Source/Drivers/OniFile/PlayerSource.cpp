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

#include "PlayerSource.h"

namespace oni_file {

/// Constructor.
PlayerSource::PlayerSource(const XnChar* strNodeName, OniSensorType sensorType) :
	m_nodeName(strNodeName),
	m_requiredFrameSize(0)
{
	m_sourceInfo.sensorType = sensorType;
	m_sourceInfo.numSupportedVideoModes = 0;
}


/// Destructor.
PlayerSource::~PlayerSource()
{
}

/// Return the source info associated with the source.
OniSensorInfo* PlayerSource::GetInfo()
{
	xnl::AutoCSLocker lock(m_cs);
	return &m_sourceInfo;
}

const XnChar* PlayerSource::GetNodeName()
{
	return m_nodeName.Data();
}

/// Get property.
OniStatus PlayerSource::GetProperty(int propertyId, void* data, int* pDataSize)
{
	xnl::AutoCSLocker lock(m_cs);
	return m_properties.GetProperty(propertyId, data, pDataSize);
}

/// Set property.
OniStatus PlayerSource::SetProperty(int propertyId, const void* data, int dataSize)
{
	xnl::AutoCSLocker lock(m_cs);
	if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
	{
		if(m_sourceInfo.numSupportedVideoModes == 0)
		{
			// at first, we're gonna allocate and set. 
			// next time we'll just update the values.
			m_sourceInfo.numSupportedVideoModes = 1;
			m_sourceInfo.pSupportedVideoModes = XN_NEW(OniVideoMode);
		}
		memcpy(m_sourceInfo.pSupportedVideoModes, data, sizeof(OniVideoMode));

		// Calculate bytes-per-pixel from format and number of maps per frame.
		OniVideoMode* pVideoMode = (OniVideoMode*)data;
		int stride;
		int bytesPerPixel;
		switch (pVideoMode->pixelFormat)
		{
			case ONI_PIXEL_FORMAT_GRAY8:
			{
				bytesPerPixel = 1;
				stride = pVideoMode->resolutionX * bytesPerPixel;
				break;
			}
			case ONI_PIXEL_FORMAT_DEPTH_1_MM:
			case ONI_PIXEL_FORMAT_DEPTH_100_UM:
			case ONI_PIXEL_FORMAT_SHIFT_9_2:
			case ONI_PIXEL_FORMAT_SHIFT_9_3:
			{
				bytesPerPixel = 2;
				stride = pVideoMode->resolutionX * bytesPerPixel;
				break;
			}
			case ONI_PIXEL_FORMAT_GRAY16:
			case ONI_PIXEL_FORMAT_YUV422:
			case ONI_PIXEL_FORMAT_YUYV:
			{
				bytesPerPixel = 2;
				stride = pVideoMode->resolutionX * bytesPerPixel;
				break;
			}
			case ONI_PIXEL_FORMAT_RGB888:
			{
				bytesPerPixel = 3;
				stride = pVideoMode->resolutionX * bytesPerPixel;
				break;
			}
			case ONI_PIXEL_FORMAT_JPEG:
			{
				bytesPerPixel = 0;
				stride = 0;
				break;
			}
			default:
			{
				bytesPerPixel = 0;
				stride = 0;
			}
		}

		m_properties.SetProperty(ONI_STREAM_PROPERTY_BYTES_PER_PIXEL, &bytesPerPixel, sizeof(bytesPerPixel));
		m_properties.SetProperty(ONI_STREAM_PROPERTY_STRIDE, &stride, sizeof(stride));
	}

	return m_properties.SetProperty(propertyId, data, dataSize);
}

// Process new data.
void PlayerSource::ProcessNewData(XnUInt64 nTimeStamp, XnUInt32 nFrameId, void* pData, XnUInt32 nSize)
{
	// Raise the event to all registered callbacks.
	NewDataEventArgs args;
	args.nTimeStamp = nTimeStamp;
	args.nFrameId = nFrameId;
	args.pData = pData;
	args.nSize = nSize;
	m_newDataEvent.Raise(args);
}

// Register for new data event.
OniStatus PlayerSource::RegisterNewDataEvent(NewDataCallback callback, void* pCookie, OniCallbackHandle& handle)
{
	XnStatus rc = m_newDataEvent.Register(callback, pCookie, (XnCallbackHandle&)handle);
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}
	return ONI_STATUS_OK;
}

// Unregister from new data event.
void PlayerSource::UnregisterNewDataEvent(OniCallbackHandle handle)
{
	m_newDataEvent.Unregister((XnCallbackHandle)handle);
}

} // namespace oni_files_player
