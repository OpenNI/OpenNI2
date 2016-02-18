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
#ifndef KINECTDRIVER_H
#define KINECTDRIVER_H

#include "Driver\OniDriverAPI.h"
#include "XnHash.h"
#include <Shlobj.h>
#include "NuiApi.h"

namespace kinect_device {

class KinectDriver : public oni::driver::DriverBase 
{
public:
	KinectDriver(OniDriverServices* pDriverServices);
	
	virtual OniStatus initialize(oni::driver::DeviceConnectedCallback connectedCallback, oni::driver::DeviceDisconnectedCallback disconnectedCallback, 
												oni::driver::DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie);

	virtual ~KinectDriver();

	virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* mode);
	virtual void deviceClose(oni::driver::DeviceBase* pDevice);

	virtual void shutdown();

	virtual OniStatus tryDevice(const char* uri);

	virtual void* enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount);
	virtual void disableFrameSync(void* frameSyncGroup);
	void StatusUpdate(const OLECHAR* instanceName, bool isConnected);
	static void CALLBACK StatusProc( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName , void* pUserData );
private:
	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> m_devices;
};
} // namespace kinect_device
#endif // KINECTDRIVER_H
