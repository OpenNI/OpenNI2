/**
*  freenectDevice.cpp
*  Copyright 2012 Benn Snyder <benn.snyder@gmail.com>
*  
*  OpenNI 2.x Alpha
*  Copyright (C) 2012 PrimeSense Ltd.
* 
*  This file is part of OpenNI.
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*****************************************************************************/
/*
 * This implementation of KinectDevice serves as a bridge to libfreenect.h
 * It allows OpenNI 2.x to use libfreenect as a device for Kinect and Kinect for Windows (k4w)
 * This effectively brings Kinect support to OpenNI2 on Linux; please test on OSX!
 * 
 * 
 */


#include "Drivers/OniDriverAPI.h"


//#include "KinectDevice.h"
//#include "DepthKinectStream.h"
//#include "ColorKinectStream.h"
//#include "IRKinectStream.h"
#include "XnLog.h"


using namespace kinect_device;
using namespace oni::driver;
#define DEFAULT_FPS 30
KinectDevice::KinectDevice(INuiSensor * pNuiSensor):m_pDepthStream(NULL), m_pColorStream(NULL),m_pNuiSensor(pNuiSensor)
{
	m_numSensors = 3;

	m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
	m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
	m_sensors[0].numSupportedVideoModes = 3;
	m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_sensors[0].pSupportedVideoModes[0].fps = DEFAULT_FPS;
	m_sensors[0].pSupportedVideoModes[0].resolutionX = 640;
	m_sensors[0].pSupportedVideoModes[0].resolutionY = 480;

	m_sensors[0].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_sensors[0].pSupportedVideoModes[1].fps = DEFAULT_FPS;
	m_sensors[0].pSupportedVideoModes[1].resolutionX = 320;
	m_sensors[0].pSupportedVideoModes[1].resolutionY = 240;

	m_sensors[0].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_sensors[0].pSupportedVideoModes[2].fps = DEFAULT_FPS;
	m_sensors[0].pSupportedVideoModes[2].resolutionX = 80;
	m_sensors[0].pSupportedVideoModes[2].resolutionY = 60;

	m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
	m_sensors[1].sensorType = ONI_SENSOR_COLOR;
	m_sensors[1].numSupportedVideoModes = 3;
	m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_sensors[1].pSupportedVideoModes[0].fps         = 12;
	m_sensors[1].pSupportedVideoModes[0].resolutionX = 1280;
	m_sensors[1].pSupportedVideoModes[0].resolutionY = 960;

	m_sensors[1].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_sensors[1].pSupportedVideoModes[1].fps = DEFAULT_FPS;
	m_sensors[1].pSupportedVideoModes[1].resolutionX = 640;
	m_sensors[1].pSupportedVideoModes[1].resolutionY = 480;

	m_sensors[1].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_YUV422;
	m_sensors[1].pSupportedVideoModes[2].fps         = 15;
	m_sensors[1].pSupportedVideoModes[2].resolutionX = 640;
	m_sensors[1].pSupportedVideoModes[2].resolutionY = 480;

	m_sensors[2].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
	m_sensors[2].sensorType = ONI_SENSOR_IR;
	m_sensors[2].numSupportedVideoModes = 3;
	m_sensors[2].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_sensors[2].pSupportedVideoModes[0].fps = DEFAULT_FPS;
	m_sensors[2].pSupportedVideoModes[0].resolutionX = 640;
	m_sensors[2].pSupportedVideoModes[0].resolutionY = 480;

	m_sensors[2].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
	m_sensors[2].pSupportedVideoModes[1].fps = DEFAULT_FPS;
	m_sensors[2].pSupportedVideoModes[1].resolutionX = 640;
	m_sensors[2].pSupportedVideoModes[1].resolutionY = 480;

	m_sensors[2].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
	m_sensors[2].pSupportedVideoModes[2].fps = DEFAULT_FPS;
	m_sensors[2].pSupportedVideoModes[2].resolutionX = 640;
	m_sensors[2].pSupportedVideoModes[2].resolutionY = 480;
}

KinectDevice::~KinectDevice()
{
	if (m_pNuiSensor)
		m_pNuiSensor->NuiShutdown();

	if (m_pDepthStream != NULL)
		XN_DELETE(m_pDepthStream);

	if (m_pColorStream!= NULL)
		XN_DELETE(m_pColorStream);

	if (m_pNuiSensor)
		m_pNuiSensor->Release();
}

OniStatus KinectDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
	*numSensors = m_numSensors;
	*pSensors = m_sensors;
	return ONI_STATUS_OK;
}

StreamBase* KinectDevice::createStream(OniSensorType sensorType)
{
	BaseKinectStream* pImage = NULL;

	if (sensorType ==  ONI_SENSOR_COLOR )
	{
		if (m_pColorStream == NULL)
		{
			m_pColorStream = XN_NEW(KinectStreamImpl, m_pNuiSensor, sensorType);
			pImage = XN_NEW(ColorKinectStream, m_pColorStream);
		}
		else
		{
			if (m_pColorStream->getSensorType() != sensorType)
			{
				if (!m_pColorStream->isRunning())
				{
					m_pColorStream->setSensorType(sensorType);
					pImage = XN_NEW(ColorKinectStream, m_pColorStream);
				}
			}
			else
			{
				pImage = XN_NEW(ColorKinectStream, m_pColorStream);
			}
		}
	}
	else if (sensorType == ONI_SENSOR_DEPTH )
	{
		if (m_pDepthStream == NULL)
		{
			m_pDepthStream = XN_NEW(KinectStreamImpl, m_pNuiSensor, sensorType);
		}
		pImage = XN_NEW(DepthKinectStream, m_pDepthStream);
	}
	if (sensorType ==  ONI_SENSOR_IR )
	{
		if (m_pColorStream == NULL)
		{
			m_pColorStream = XN_NEW(KinectStreamImpl, m_pNuiSensor, sensorType);
			pImage = XN_NEW(IRKinectStream, m_pColorStream);
		}
		else
		{
			if (m_pColorStream->getSensorType() != sensorType)
			{
				if (!m_pColorStream->isRunning())
				{
					m_pColorStream->setSensorType(sensorType);
					pImage = XN_NEW(IRKinectStream, m_pColorStream);
				}
			}
			else
			{
				pImage = XN_NEW(IRKinectStream, m_pColorStream);
			}
		}
	}
	return pImage;
}

void kinect_device::KinectDevice::destroyStream(oni::driver::StreamBase* pStream)
{
	XN_DELETE(pStream);
}

OniStatus KinectDevice::setProperty(int propertyId, const void* data, int dataSize)
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus KinectDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool KinectDevice::isPropertySupported(int propertyId)
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool KinectDevice::isCommandSupported(int commandId)
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus KinectDevice::tryManualTrigger()
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

void KinectDevice::notifyAllProperties()
{
	return;
}
