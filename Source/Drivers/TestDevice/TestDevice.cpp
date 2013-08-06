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
#include "TestDevice.h"

TestDevice::TestDevice(OniDeviceInfo* pInfo, oni::driver::DriverServices& driverServices) : m_pInfo(pInfo), m_driverServices(driverServices)
{
	OniSensorType sensorTypes[] = { ONI_SENSOR_DEPTH, ONI_SENSOR_COLOR, ONI_SENSOR_IR };
	int count = sizeof(sensorTypes)/sizeof(sensorTypes[0]);

	XN_ASSERT(count < sizeof(m_sensors)/sizeof(m_sensors[0]));

	for (int i = 0; i < count; ++i)
	{
		m_sensors[i].sensorType = sensorTypes[i];
		m_sensors[i].numSupportedVideoModes = 1;
		m_sensors[i].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
		m_sensors[i].pSupportedVideoModes[0].pixelFormat = TestStream::getDefaultPixelFormat(m_sensors[i].sensorType);
		m_sensors[i].pSupportedVideoModes[0].fps = DEFAULT_FPS;
		m_sensors[i].pSupportedVideoModes[0].resolutionX = DEFAULT_RESOLUTION_X;
		m_sensors[i].pSupportedVideoModes[0].resolutionY = DEFAULT_RESOLUTION_Y;
	}

	m_numSensors = count;
}

OniDeviceInfo* TestDevice::GetInfo()
{
	return m_pInfo;
}

OniStatus TestDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
	*numSensors = m_numSensors;
	*pSensors = m_sensors;

	return ONI_STATUS_OK;
}

oni::driver::StreamBase* TestDevice::createStream(OniSensorType sensorType)
{
	return XN_NEW(TestStream, sensorType);
}

void TestDevice::destroyStream(oni::driver::StreamBase* pStream)
{
	XN_DELETE(pStream);
}

OniStatus TestDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
	OniStatus rc = ONI_STATUS_OK;

	switch (propertyId)
	{
	case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
		{
			if (*pDataSize == sizeof(OniVersion))
			{
				OniVersion* version = (OniVersion*)data;
				version->major = version->minor = version->maintenance = version->build = 2;
			}
			else
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
				rc = ONI_STATUS_ERROR;
			}
		}
		break;
	default:
		m_driverServices.errorLoggerAppend("Unknown property: %d\n", propertyId);
		rc = ONI_STATUS_ERROR;
	}
	return rc;
}
