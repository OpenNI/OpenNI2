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
#include <OpenNI.h>
#include "Viewer.h"

using namespace openni;

int main(int argc, char** argv)
{
	openni::Status rc = openni::STATUS_OK;

	openni::Device device;
	openni::VideoStream depth, color;
	const char* deviceURI = openni::ANY_DEVICE;
	if (argc > 1)
	{
		deviceURI = argv[1];
	}

	setenv("OPENNI2_DRIVERS_PATH","./",1);

	rc = openni::OpenNI::initialize();

	printf("After initialization:\n%s\n", openni::OpenNI::getExtendedError());

	rc = device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 1;
	}

	rc = depth.create(device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		printf("SimpleViewer: Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	rc = color.create(device, openni::SENSOR_COLOR);
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	const SensorInfo& depth_info = depth.getSensorInfo();
	const Array<VideoMode>& dmodes = depth_info.getSupportedVideoModes();
	for( int i=0; i < dmodes.getSize(); i++ )
	{
		VideoMode bla = dmodes[i];
		if(dmodes[i].getResolutionX() == 640 &&
			dmodes[i].getResolutionY() == 480 &&
			dmodes[i].getPixelFormat() == PIXEL_FORMAT_DEPTH_1_MM)
		{
			rc = depth.setVideoMode(dmodes[i]);
			if(rc != STATUS_OK)
			{
				printf("Failed to set depth video mode");
				return -1;
			}
			break;
		}
		else if(i == dmodes.getSize()-1)
		{
			printf("Failed to find a supported depth mode");
			return -1;
		}
	}

	const SensorInfo& color_info = color.getSensorInfo();
	const Array<VideoMode>& cmodes = color_info.getSupportedVideoModes();
	for( int i=0; i < cmodes.getSize(); i++ )
	{
		if(cmodes[i].getResolutionX() == 640 &&
			cmodes[i].getResolutionY() == 480 &&
			cmodes[i].getPixelFormat() == PIXEL_FORMAT_RGB888)
		{
			rc = color.setVideoMode(cmodes[i]);
			if(rc != STATUS_OK)
			{
				printf("Failed to set color video mode\n");
				return -1;
			}
			break;
		}
		else if(i == cmodes.getSize()-1)
		{
			printf("Failed to find a supported color mode\n");
			return -1;
		}
	}

	rc = device.setDepthColorSyncEnabled(TRUE);
	if(rc != STATUS_OK)
	{
		printf("Unable to enable depth-color sync\n");
		return -1;
	}

	rc = device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	if(rc != STATUS_OK)
	{
		printf("Failed to set image registration mode\n");
		return -1;
	}


	rc = depth.start();
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
		depth.destroy();
	}

	rc = color.start();
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
		color.destroy();
	}

	if (!depth.isValid() || !color.isValid())
	{
		printf("SimpleViewer: No valid streams. Exiting\n");
		openni::OpenNI::shutdown();
		return 2;
	}

	SampleViewer sampleViewer("Simple Viewer", device, depth, color);

	rc = sampleViewer.init(argc, argv);
	if (rc != openni::STATUS_OK)
	{
		openni::OpenNI::shutdown();
		return 3;
	}
	sampleViewer.run();
}