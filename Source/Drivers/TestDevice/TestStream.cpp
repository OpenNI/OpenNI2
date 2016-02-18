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
#include "TestStream.h"
#include <OniTest.h>

TestStream::TestStream(OniSensorType sensorType) : oni::driver::StreamBase()
{
	m_osEvent.Create(TRUE);
	m_sensorType = sensorType;
	m_videoMode.resolutionX = DEFAULT_RESOLUTION_X;
	m_videoMode.resolutionY = DEFAULT_RESOLUTION_Y;
	m_videoMode.fps = DEFAULT_FPS;
	m_videoMode.pixelFormat = getDefaultPixelFormat(sensorType);
	m_frameIndex = 0;
}

TestStream::~TestStream()
{
	stop();
}

OniPixelFormat TestStream::getDefaultPixelFormat(OniSensorType sensorType)
{
	switch (sensorType)
	{
	case ONI_SENSOR_DEPTH:
		return ONI_PIXEL_FORMAT_DEPTH_1_MM;
	case ONI_SENSOR_COLOR:
		return ONI_PIXEL_FORMAT_RGB888;
	case ONI_SENSOR_IR:
		return ONI_PIXEL_FORMAT_GRAY16;
	default:
		XN_ASSERT(FALSE);
		return (OniPixelFormat)-1;
	}
}

OniStatus TestStream::start()
{
	m_running = true;
	return ONI_STATUS_OK;
}

void TestStream::stop()
{
	m_running = false;
}

OniStatus TestStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
	{
		if (*pDataSize != sizeof(OniVideoMode))
		{
			printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVideoMode));
			return ONI_STATUS_ERROR;
		}
		return getVideoMode((OniVideoMode*)data);
	}

	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus TestStream::setProperty(int propertyId, const void* data, int dataSize)
{
	if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
	{
		if (dataSize != sizeof(OniVideoMode))
		{
			printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniVideoMode));
			return ONI_STATUS_ERROR;
		}
		return setVideoMode((OniVideoMode*)data);
	}

	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus TestStream::setVideoMode(OniVideoMode* pVideoMode)
{
	xnOSMemCopy(&m_videoMode, pVideoMode, sizeof(m_videoMode));
	return ONI_STATUS_OK;
}

OniStatus TestStream::getVideoMode(OniVideoMode* pVideoMode)
{
	xnOSMemCopy(pVideoMode, &m_videoMode, sizeof(m_videoMode));
	return ONI_STATUS_OK;
}

int getBytesPerPixel(OniPixelFormat format)
{
	switch (format)
	{
	case ONI_PIXEL_FORMAT_GRAY8:
		return 1;
	case ONI_PIXEL_FORMAT_DEPTH_1_MM:
	case ONI_PIXEL_FORMAT_DEPTH_100_UM:
	case ONI_PIXEL_FORMAT_SHIFT_9_2:
	case ONI_PIXEL_FORMAT_SHIFT_9_3:
	case ONI_PIXEL_FORMAT_GRAY16:
		return 2;
	case ONI_PIXEL_FORMAT_RGB888:
		return 3;
	case ONI_PIXEL_FORMAT_YUV422:
	case ONI_PIXEL_FORMAT_YUYV:
		return 2;
	case ONI_PIXEL_FORMAT_JPEG:
		return 1;
	default:
		XN_ASSERT(FALSE);
		return 0;
	}
}

OniStatus TestStream::invoke(int commandId, void* data, int dataSize)
{
	switch (commandId)
	{
	case TEST_COMMAND_ISSUE_FRAME:
		{
			if (dataSize != sizeof(TestCommandIssueFrame))
			{
				printf("Unexpected size: %d != %d\n", dataSize, sizeof(TestCommandIssueFrame));
				return ONI_STATUS_BAD_PARAMETER;
			}

			if (!m_running)
			{
				printf("Can't issue a frame if stream is not started\n");
				return ONI_STATUS_BAD_PARAMETER;
			}

			TestCommandIssueFrame* pArgs = (TestCommandIssueFrame*)data;

			OniFrame* pFrame = getServices().acquireFrame();
			pFrame->frameIndex = ++m_frameIndex;
			pFrame->timestamp = pArgs->timestamp;
			xnOSMemCopy(pFrame->data, pArgs->data, pFrame->dataSize);

			pFrame->videoMode = m_videoMode;

			pFrame->width = m_videoMode.resolutionX;
			pFrame->height = m_videoMode.resolutionY;

			pFrame->cropOriginX = pFrame->cropOriginY = 0;
			pFrame->croppingEnabled = FALSE;

			pFrame->sensorType = m_sensorType;
			pFrame->stride = m_videoMode.resolutionX * getBytesPerPixel(m_videoMode.pixelFormat);

			raiseNewFrame(pFrame);
			getServices().releaseFrame(pFrame);
		}
		break;

	default:
		return ONI_STATUS_BAD_PARAMETER;
	}

	return ONI_STATUS_OK;
}

