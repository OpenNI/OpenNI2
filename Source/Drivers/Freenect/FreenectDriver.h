/**
*  FreenectDriver
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
*/

#ifndef _FREENECT_DRIVER_H_
#define _FREENECT_DRIVER_H_

#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include "XnHash.h"
#include "FreenectDeviceNI.h"


using namespace oni::driver;

class FreenectDriver : public DriverBase, private Freenect::Freenect
{
private:
	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> devices;

public:
	FreenectDriver(OniDriverServices* pDriverServices);
	~FreenectDriver() { shutdown(); }

	// from DriverBase
	OniStatus initialize(DeviceConnectedCallback connectedCallback, DeviceDisconnectedCallback disconnectedCallback, DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie);
	DeviceBase* deviceOpen(const char* uri);
	void deviceClose(DeviceBase* pDevice);
	OniStatus tryDevice(const char* uri);
	void shutdown()
	{
		for (xnl::Hash<OniDeviceInfo*, DeviceBase*>::Iterator iter = devices.Begin(); iter != devices.End(); ++iter)
		{
			deviceClose(iter->Value());
		}
	}


	/* todo : from DriverBase
	virtual void* enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount);
	virtual void disableFrameSync(void* frameSyncGroup);
	*/
};


ONI_EXPORT_DRIVER(FreenectDriver);


#endif //_FREENECT_DRIVER_H_
