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
#include "TestDriver.h"
#include "TestDevice.h"
#include <OniTest.h>

TestDriver::TestDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
{}

oni::driver::DeviceBase* TestDriver::deviceOpen(const char* uri, const char* /*mode*/)
{
	for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
	{
		if (xnOSStrCmp(iter->Key()->uri, uri) == 0)
		{
			// Found
			if (iter->Value() != NULL)
			{
				// already using
				return iter->Value();
			}

			TestDevice* pDevice = XN_NEW(TestDevice, iter->Key(), getServices());
			iter->Value() = pDevice;
			return pDevice;
		}
	}

	getServices().errorLoggerAppend("Looking for '%s'", uri);
	return NULL;
}

void TestDriver::deviceClose(oni::driver::DeviceBase* pDevice)
{
	for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
	{
		if (iter->Value() == pDevice)
		{
			iter->Value() = NULL;
			XN_DELETE(pDevice);
			return;
		}
	}

	// not our device?!
	XN_ASSERT(FALSE);
}

OniStatus TestDriver::tryDevice(const char* uri)
{
	if (xnOSStrCmp(uri, TEST_DEVICE_NAME))
	{
		return ONI_STATUS_ERROR;
	}


	OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
	xnOSStrCopy(pInfo->uri, uri, ONI_MAX_STR);
	xnOSStrCopy(pInfo->vendor, "Test", ONI_MAX_STR);
	m_devices[pInfo] = NULL;

	deviceConnected(pInfo);

	return ONI_STATUS_OK;
}

void TestDriver::shutdown() 
{}

ONI_EXPORT_DRIVER(TestDriver);
