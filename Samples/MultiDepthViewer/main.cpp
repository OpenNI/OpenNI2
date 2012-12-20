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


int main(int argc, char** argv)
{
	openni::Status rc = openni::STATUS_OK;

	openni::Device device1, device2;
	openni::VideoStream depth1, depth2;


	rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK)
	{
		printf("%s: Initialize failed\n%s\n", argv[0], openni::OpenNI::getExtendedError());
		return 1;
	}

	openni::Array<openni::DeviceInfo> deviceList;
	openni::OpenNI::enumerateDevices(&deviceList);

	const char* device1Uri;
	const char* device2Uri;

	switch (argc)
	{
	case 1:
		if (deviceList.getSize() < 2)
		{
			printf("Missing devices\n");
			openni::OpenNI::shutdown();
			return 1;
		}
		device1Uri = deviceList[1].getUri();
		device2Uri = deviceList[0].getUri();
		break;
	case 2:
		if (deviceList.getSize() < 1)
		{
			printf("Missing devices\n");
			openni::OpenNI::shutdown();
			return 1;
		}
		device1Uri = argv[1];
		if (strcmp(deviceList[0].getUri(), device1Uri) != 0)
			device2Uri = deviceList[0].getUri();
		else
			device2Uri = deviceList[1].getUri();
		break;
	default:
		device1Uri = argv[1];
		device2Uri = argv[2];
	}

	rc = device1.open(device1Uri);
	if (rc != openni::STATUS_OK)
	{
		printf("%s: Couldn't open device %s\n%s\n", argv[0], device1Uri, openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 3;
	}

	rc = device2.open(device2Uri);
	if (rc != openni::STATUS_OK)
	{
		printf("%s: Couldn't open device %s\n%s\n", argv[0], device2Uri, openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 3;
	}

	rc = depth1.create(device1, openni::SENSOR_DEPTH);
	if (rc != openni::STATUS_OK)
	{
		printf("%s: Couldn't create stream %d on device %s\n%s\n", argv[0], openni::SENSOR_DEPTH, device1Uri, openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 4;
	}
	rc = depth2.create(device2, openni::SENSOR_DEPTH);
	if (rc != openni::STATUS_OK)
	{
		printf("%s: Couldn't create stream %d on device %s\n%s\n", argv[0], openni::SENSOR_DEPTH, device2Uri, openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 4;
	}

	rc = depth1.start();
	if (rc != openni::STATUS_OK)
	{
		printf("%s: Couldn't start stream %d on device %s\n%s\n", argv[0], openni::SENSOR_DEPTH, device1Uri, openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 5;
	}
	rc = depth2.start();
	if (rc != openni::STATUS_OK)
	{
		printf("%s: Couldn't start stream %d on device %s\n%s\n", argv[0], openni::SENSOR_DEPTH, device2Uri, openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 5;
	}

	if (!depth1.isValid() && !depth2.isValid())
	{
		printf("SimpleViewer: No valid streams. Exiting\n");
		openni::OpenNI::shutdown();
		return 6;
	}

	SampleViewer sampleViewer("Simple Viewer", depth1, depth2);

	rc = sampleViewer.init(argc, argv);
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Initialization failed\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return 7;
	}
	sampleViewer.run();
}
