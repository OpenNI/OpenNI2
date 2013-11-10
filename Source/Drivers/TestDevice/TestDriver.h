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
#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <Driver\OniDriverAPI.h>
#include <XnLib.h>
#include <XnHash.h>

class TestDriver : public oni::driver::DriverBase
{
public:
	TestDriver(OniDriverServices* pDriverServices);

	virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* mode);
	virtual void deviceClose(oni::driver::DeviceBase* pDevice);
	virtual OniStatus tryDevice(const char* uri);

	void shutdown();

protected:
	XN_THREAD_HANDLE m_threadHandle;
	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> m_devices;
};

#endif // TESTDRIVER_H

