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
#include <stdio.h>
#include "OpenNI.h"

#include "OniSampleUtilities.h"

using namespace openni;

void analyzeFrame(const VideoFrameRef& frame)
{
	DepthPixel* pDepth;
	RGB888Pixel* pColor;

	int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;

	switch (frame.getVideoMode().getPixelFormat())
	{
	case PIXEL_FORMAT_DEPTH_1_MM:
	case PIXEL_FORMAT_DEPTH_100_UM:
		pDepth = (DepthPixel*)frame.getData();
		printf("[%08llu] %8d\n", (long long)frame.getTimestamp(),
			pDepth[middleIndex]);
		break;
	case PIXEL_FORMAT_RGB888:
		pColor = (RGB888Pixel*)frame.getData();
		printf("[%08llu] 0x%02x%02x%02x\n", (long long)frame.getTimestamp(),
			pColor[middleIndex].r&0xff,
			pColor[middleIndex].g&0xff,
			pColor[middleIndex].b&0xff);
		break;
	default:
		printf("Unknown format\n");
	}
}

class PrintCallback : public VideoStream::NewFrameListener
{
public:
	void onNewFrame(VideoStream& stream)
	{
		stream.readFrame(&m_frame);

		analyzeFrame(m_frame);
	}
private:
	VideoFrameRef m_frame;
};

class OpenNIDeviceListener : public OpenNI::DeviceConnectedListener,
									public OpenNI::DeviceDisconnectedListener,
									public OpenNI::DeviceStateChangedListener
{
public:
	virtual void onDeviceStateChanged(const DeviceInfo* pInfo, DeviceState state) 
	{
		printf("Device \"%s\" error state changed to %d\n", pInfo->getUri(), state);
	}

	virtual void onDeviceConnected(const DeviceInfo* pInfo)
	{
		printf("Device \"%s\" connected\n", pInfo->getUri());
	}

	virtual void onDeviceDisconnected(const DeviceInfo* pInfo)
	{
		printf("Device \"%s\" disconnected\n", pInfo->getUri());
	}
};

int main()
{
	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}

	OpenNIDeviceListener devicePrinter;

	OpenNI::addDeviceConnectedListener(&devicePrinter);
	OpenNI::addDeviceDisconnectedListener(&devicePrinter);
	OpenNI::addDeviceStateChangedListener(&devicePrinter);

	openni::Array<openni::DeviceInfo> deviceList;
	openni::OpenNI::enumerateDevices(&deviceList);
	for (int i = 0; i < deviceList.getSize(); ++i)
	{
		printf("Device \"%s\" already connected\n", deviceList[i].getUri());
	}

	Device device;
	rc = device.open(ANY_DEVICE);
	if (rc != STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
		return 2;
	}

	VideoStream depth;

	if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
	{
		rc = depth.create(device, SENSOR_DEPTH);
		if (rc != STATUS_OK)
		{
			printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
		}
	}
	rc = depth.start();
	if (rc != STATUS_OK)
	{
		printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
	}


	PrintCallback depthPrinter;

	// Register to new frame
	depth.addNewFrameListener(&depthPrinter);

	// Wait while we're getting frames through the printer
	while (!wasKeyboardHit())
	{
		Sleep(100);
	}

	depth.removeNewFrameListener(&depthPrinter);


	depth.stop();
	depth.destroy();
	device.close();
	OpenNI::shutdown();

	return 0;
}
