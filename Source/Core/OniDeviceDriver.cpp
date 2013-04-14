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
#include "OniDeviceDriver.h"
#include "Driver/OniDriverTypes.h"
#include "OniStream.h"
#include "XnArray.h"
#include <XnLog.h>

#define XN_MASK_ONI_DEVICE_DRIVER "OniDeviceDriver"

void XN_CALLBACK_TYPE DriverServices_ErrorLog_Append(void* driverServices, const char* format, va_list args)
{
	oni::implementation::DriverServices* pDriverServices = (oni::implementation::DriverServices*)driverServices;
	pDriverServices->getErrorLogger().AppendV(format, args);
}

void XN_CALLBACK_TYPE DriverServices_ErrorLog_Clear(void* driverServices)
{
	oni::implementation::DriverServices* pDriverServices = (oni::implementation::DriverServices*)driverServices;
	pDriverServices->getErrorLogger().Clear();
}

void XN_CALLBACK_TYPE DriverServices_Log(void* /*driverServices*/, int severity, const char* file, int line, const char* mask, const char* message)
{
	xnLogWrite(mask, (XnLogSeverity)severity, file, line, "%s", message);
}

OniDriverServices* CreateDriverServicesForDriver(oni::implementation::DriverServices* driverServices)
{
	OniDriverServices*pDriverServices = XN_NEW(OniDriverServices);

	pDriverServices->driverServices = driverServices;
	pDriverServices->errorLoggerAppend = DriverServices_ErrorLog_Append;
	pDriverServices->errorLoggerClear = DriverServices_ErrorLog_Clear;
	pDriverServices->log = DriverServices_Log;

	return pDriverServices;
}

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

DeviceDriver::DeviceDriver(const char* strDriverFilename, FrameManager& frameManager, xnl::ErrorLogger& errorLogger) :
	m_driverServices(errorLogger),
	m_pDriverServicesForDriver(NULL),
	m_errorLogger(errorLogger),
	m_driverHandler(strDriverFilename, errorLogger),
	m_frameManager(frameManager),
	m_valid(false)
{
	if (!m_driverHandler.isValid())
	{
		m_errorLogger.Append("DeviceDriver: library handle is invalid for file %s", strDriverFilename);
		return;
	}

	m_valid = true;

	m_pDriverServicesForDriver = CreateDriverServicesForDriver(&m_driverServices);
	m_driverHandler.create(m_pDriverServicesForDriver);
}
bool DeviceDriver::initialize()
{
	if (!isValid())
		return false;

	if (ONI_STATUS_OK != m_driverHandler.initialize(driver_DeviceConnected, driver_DeviceDisconnected, driver_DeviceStateChanged, this))
		return false;

	return true;
}
DeviceDriver::~DeviceDriver()
{
	if (m_driverHandler.isValid())
	{
		m_driverHandler.destroy();
	}
	XN_DELETE(m_pDriverServicesForDriver);
}

bool DeviceDriver::tryDevice(const char* uri)
{
	for (xnl::StringsHash<Device*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
	{
		if (xnOSStrCmp(iter->Value()->getInfo()->uri, uri) == 0)
			return true;
	}

	return (m_driverHandler.tryDevice(uri) == ONI_STATUS_OK);
}


OniStatus DeviceDriver::registerDeviceConnectedCallback(DeviceCallback handler, void* pCookie, OniCallbackHandle& handle)
{
	return OniStatusFromXnStatus(m_deviceConnectedEvent.Register(handler, pCookie, (XnCallbackHandle&)handle));
}
void DeviceDriver::unregisterDeviceConnectedCallback(OniCallbackHandle handle)
{
	m_deviceConnectedEvent.Unregister((XnCallbackHandle)handle);
}
OniStatus DeviceDriver::registerDeviceDisconnectedCallback(DeviceCallback handler, void* pCookie, OniCallbackHandle& handle)
{
	return OniStatusFromXnStatus(m_deviceDisconnectedEvent.Register(handler, pCookie, (XnCallbackHandle&)handle));
}
void DeviceDriver::unregisterDeviceDisconnectedCallback(OniCallbackHandle handle)
{
	m_deviceDisconnectedEvent.Unregister((XnCallbackHandle)handle);
}

OniStatus DeviceDriver::registerDeviceStateChangedCallback(DeviceStateCallback handler, void* pCookie, OniCallbackHandle& handle)
{
	return OniStatusFromXnStatus(m_deviceStateChangedEvent.Register(handler, pCookie, (XnCallbackHandle&)handle));
}
void DeviceDriver::unregisterDeviceStateChangedCallback(OniCallbackHandle handle)
{
	m_deviceStateChangedEvent.Unregister((XnCallbackHandle)handle);
}

void ONI_CALLBACK_TYPE DeviceDriver::driver_DeviceConnected(const OniDeviceInfo* pInfo, void* pCookie)
{
	DeviceDriver* pDriver = (DeviceDriver*)pCookie;
	xnLogInfo(XN_MASK_ONI_DEVICE_DRIVER, "Device connected: %s %s (%s)", pInfo->vendor, pInfo->name, pInfo->uri);
	Device* pDevice = XN_NEW(Device, pDriver, pDriver->m_driverHandler, pDriver->m_frameManager, pInfo, pDriver->m_errorLogger);
	if (pDriver != NULL)
	{
		pDriver->m_devices[pInfo->uri] = pDevice;
		pDriver->m_deviceConnectedEvent.Raise(pDevice);
	}
}
void ONI_CALLBACK_TYPE DeviceDriver::driver_DeviceDisconnected(const OniDeviceInfo* pInfo, void* pCookie)
{
	DeviceDriver* pDriver = (DeviceDriver*)pCookie;
	xnLogInfo(XN_MASK_ONI_DEVICE_DRIVER, "Device disconnected: %s %s (%s)", pInfo->vendor, pInfo->name, pInfo->uri);
	pDriver->m_deviceDisconnectedEvent.Raise(pDriver->m_devices[pInfo->uri]);
}
void ONI_CALLBACK_TYPE DeviceDriver::driver_DeviceStateChanged(const OniDeviceInfo* pInfo, OniDeviceState deviceState, void* pCookie)
{
	DeviceDriver* pDriver = (DeviceDriver*)pCookie;
	xnLogInfo(XN_MASK_ONI_DEVICE_DRIVER, "Device state changed: %s %s (%s) to %d", pInfo->vendor, pInfo->name, pInfo->uri, deviceState);
	pDriver->m_deviceStateChangedEvent.Raise(pDriver->m_devices[pInfo->uri], deviceState);
}

void* DeviceDriver::enableFrameSync(VideoStream** pStreams, int streamCount)
{
	// Translate the Stream to driver's stream handle.
	xnl::Array<void*> streams(streamCount);
	streams.SetSize(streamCount);
	for (int i = 0; i < streamCount; ++i)
	{
		streams[i] = pStreams[i]->getHandle();
	}

	return m_driverHandler.enableFrameSync(streams.GetData(), streamCount);
}

void DeviceDriver::disableFrameSync(void* frameSyncGroup)
{
	m_driverHandler.disableFrameSync(frameSyncGroup);
}

ONI_NAMESPACE_IMPLEMENTATION_END
