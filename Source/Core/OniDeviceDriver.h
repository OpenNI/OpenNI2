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
#ifndef ONIDEVICEDRIVER_H
#define ONIDEVICEDRIVER_H

#include "OniDriverHandler.h"
#include "OniFrameManager.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "OniDevice.h"
#include "OniCommon.h"
#include "OniDriverServices.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

typedef void ONI_CALLBACK_TYPE DeviceCallback(Device*, void*);
typedef void ONI_CALLBACK_TYPE DeviceStateCallback(Device*, OniDeviceState, void*);

class VideoStream;

class DeviceDriver
{
public:
	DeviceDriver(const char* strDriverFileName, FrameManager& frameManager, xnl::ErrorLogger& errorLogger);
	bool initialize();
	~DeviceDriver();

	bool isValid() {return m_valid;}

	bool tryDevice(const char* uri);

	OniStatus registerDeviceConnectedCallback(DeviceCallback handler, void* pCookie, OniCallbackHandle& handle);
	void unregisterDeviceConnectedCallback(OniCallbackHandle handle);
	OniStatus registerDeviceDisconnectedCallback(DeviceCallback handler, void* pCookie, OniCallbackHandle& handle);
	void unregisterDeviceDisconnectedCallback(OniCallbackHandle handle);
	OniStatus registerDeviceStateChangedCallback(DeviceStateCallback handler, void* pCookie, OniCallbackHandle& handle);
	void unregisterDeviceStateChangedCallback(OniCallbackHandle handle);

	void* enableFrameSync(VideoStream** pStreams, int streamCount);
	void disableFrameSync(void* frameSyncGroup);

protected:
	static void ONI_CALLBACK_TYPE driver_DeviceConnected(const OniDeviceInfo* pInfo, void* pCookie);
	static void ONI_CALLBACK_TYPE driver_DeviceDisconnected(const OniDeviceInfo* pInfo, void* pCookie);
	static void ONI_CALLBACK_TYPE driver_DeviceStateChanged(const OniDeviceInfo* pInfo, OniDeviceState deviceState, void* pCookie);

	DriverServices m_driverServices;
	OniDriverServices* m_pDriverServicesForDriver;
	xnl::ErrorLogger& m_errorLogger;
	DriverHandler m_driverHandler;
	FrameManager& m_frameManager;

	bool m_valid;
	xnl::StringsHash<Device*> m_devices;

	xnl::Event1Arg<Device*> m_deviceConnectedEvent;
	xnl::Event1Arg<Device*> m_deviceDisconnectedEvent;
	xnl::Event2Args<Device*, OniDeviceState> m_deviceStateChangedEvent;

private:
	DeviceDriver(const DeviceDriver& other);
	DeviceDriver& operator=(const DeviceDriver& other);
};

ONI_NAMESPACE_IMPLEMENTATION_END

#endif // ONIDEVICEDRIVER_H
