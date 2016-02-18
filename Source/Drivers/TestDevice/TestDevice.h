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
#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <Driver\OniDriverAPI.h>
#include "TestStream.h"

class TestDevice : public oni::driver::DeviceBase
{
public:
	TestDevice(OniDeviceInfo* pInfo, oni::driver::DriverServices& driverServices);
	OniDeviceInfo* GetInfo();

	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors);

	oni::driver::StreamBase* createStream(OniSensorType sensorType);

	void destroyStream(oni::driver::StreamBase* pStream);

	OniStatus getProperty(int propertyId, void* data, int* pDataSize);

private:
	TestDevice(const TestDevice&);
	void operator=(const TestDevice&);

	OniDeviceInfo* m_pInfo;
	int m_numSensors;
	OniSensorInfo m_sensors[10];
	oni::driver::DriverServices& m_driverServices;
};

#endif // TESTDEVICE_H
