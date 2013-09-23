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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LinkOniDriver.h"
#include "LinkOniDevice.h"
#include "LinkDeviceEnumeration.h"
#include <XnOS.h>
#include <XnLogWriterBase.h>

#define LINK_CONFIGURATION_FILE "PSLink.ini"

//---------------------------------------------------------------------------
// LinkOniDriver class
//---------------------------------------------------------------------------
LinkOniDriver::LinkOpenNILogWriter::LinkOpenNILogWriter(OniDriverServices* pDriverServices) : m_pDriverServices(pDriverServices)
{
}

void LinkOniDriver::LinkOpenNILogWriter::WriteEntry(const XnLogEntry* pEntry)
{
	m_pDriverServices->log(m_pDriverServices, pEntry->nSeverity, pEntry->strFile, pEntry->nLine, pEntry->strMask, pEntry->strMessage);
}

void LinkOniDriver::LinkOpenNILogWriter::WriteUnformatted(const XnChar* /*strMessage*/)
{
	// DO NOTHING
}

OniStatus LinkOniDriver::initialize(oni::driver::DeviceConnectedCallback deviceConnectedCallback, oni::driver::DeviceDisconnectedCallback deviceDisconnectedCallback, oni::driver::DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie)
{
	OniStatus nRetVal = DriverBase::initialize(deviceConnectedCallback, deviceDisconnectedCallback, deviceStateChangedCallback, pCookie);
	if (nRetVal != ONI_STATUS_OK)
	{
		return (nRetVal);
	}

	xnLogSetMaskMinSeverity(XN_LOG_MASK_ALL, XN_LOG_VERBOSE);
	m_writer.Register();

	XnStatus rc = LinkDeviceEnumeration::ConnectedEvent().Register(OnDeviceConnected, this, m_connectedEventHandle);
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}

	rc = LinkDeviceEnumeration::DisconnectedEvent().Register(OnDeviceDisconnected, this, m_disconnectedEventHandle);
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}

	rc = LinkDeviceEnumeration::Initialize();
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}

	resolveConfigFilePath();

	return ONI_STATUS_OK;
}

void LinkOniDriver::shutdown() 
{
	if (m_connectedEventHandle != NULL)
	{
		LinkDeviceEnumeration::ConnectedEvent().Unregister(m_connectedEventHandle);
		m_connectedEventHandle = NULL;
	}

	if (m_disconnectedEventHandle != NULL)
	{
		LinkDeviceEnumeration::DisconnectedEvent().Unregister(m_disconnectedEventHandle);
		m_disconnectedEventHandle = NULL;
	}

	// Close all open devices and release the memory
	for (xnl::StringsHash<LinkOniDevice*>::Iterator it = m_devices.Begin(); it != m_devices.End(); ++it)
	{
		XN_DELETE(it->Value());
	}

	m_devices.Clear();

	LinkDeviceEnumeration::Shutdown();
}

oni::driver::DeviceBase* LinkOniDriver::deviceOpen(const char* uri, const char* mode)
{
	LinkOniDevice* pDevice = NULL;

	// if device was already opened for this uri, return the previous one
	if (m_devices.Get(uri, pDevice) == XN_STATUS_OK)
	{
		getServices().errorLoggerAppend("Device is already open.");
		return NULL;
	}

	pDevice = XN_NEW(LinkOniDevice, m_configFilePath, uri, getServices(), this);
	XnStatus nRetVal = pDevice->Init(mode);
	if (nRetVal != XN_STATUS_OK)
	{
		getServices().errorLoggerAppend("Could not open \"%s\": %s", uri, xnGetStatusString(nRetVal));
		return NULL;
	}

/* TODO impl
	// Register to error state property changed.
	XnCallbackHandle handle;
	nRetVal = pDevice->GetSensor()->RegisterToPropertyChange(XN_MODULE_NAME_DEVICE, 
																XN_MODULE_PROPERTY_ERROR_STATE, 
																OnDevicePropertyChanged, pDevice, handle);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(pDevice);
		return NULL;
	}
*/

	// Add the device and return it.
	m_devices[uri] = pDevice;
	return pDevice;
}

void LinkOniDriver::deviceClose(oni::driver::DeviceBase* pDevice)
{
	for (xnl::StringsHash<LinkOniDevice*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
	{
		if (iter->Value() == pDevice)
		{
			m_devices.Remove(iter);
			XN_DELETE(pDevice);
			return;
		}
	}

	// not our device?!
	XN_ASSERT(FALSE);
}

/*
void* LinkOniDriver::enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount)
{
	// Make sure all the streams belong to same device.
	LinkOniDevice* pDevice = NULL;
	for (int i = 0; i < streamCount; ++i)
	{
		LinkOniStream* pStream = dynamic_cast<LinkOniStream*>(pStreams[i]);
		if (pStreams == NULL)
		{
			// Not allowed.
			return NULL;
		}

		// Check if device was not set before.
		if (pDevice == NULL)
		{
			pDevice = pStream->GetDevice();
		}
		// Compare device to stream's device.
		else if (pDevice != pStream->GetDevice())
		{
			// Streams from different devices are currently not allowed.
			return NULL;
		}
	}

	// Create the frame sync group handle.
	FrameSyncGroup* pFrameSyncGroup = XN_NEW(FrameSyncGroup);
	if (pFrameSyncGroup == NULL)
	{
		return NULL;
	}
	pFrameSyncGroup->pDevice = pDevice;

	// Enable the frame sync.
	OniStatus rc = pDevice->EnableFrameSync((LinkOniStream**)pStreams, streamCount);
	if (rc != ONI_STATUS_OK)
	{
		XN_DELETE(pFrameSyncGroup);
		return NULL;
	}

	// Return the created handle.
	return pFrameSyncGroup;
}

void LinkOniDriver::disableFrameSync(void* frameSyncGroup)
{
	FrameSyncGroup* pFrameSyncGroup = (FrameSyncGroup*)frameSyncGroup;

	// Find device in driver.
	xnl::StringsHash<LinkOniDevice*>::ConstIterator iter = m_devices.Begin();
	while (iter != m_devices.End())
	{
		// Make sure device belongs to driver.
		if ((*iter).second == pFrameSyncGroup->pDevice)
		{
			// Disable frame sync in device.
			pFrameSyncGroup->pDevice->DisableFrameSync();
			return;
		}
		++iter;
	}
}
*/

void XN_CALLBACK_TYPE LinkOniDriver::OnDevicePropertyChanged(const XnChar* /*ModuleName*/, XnUInt32 /*nPropertyId*/, void* /*pCookie*/)
{
	//TODO impl!
	/*
	LinkOniDevice* pDevice = (LinkOniDevice*)pCookie;
	LinkOniDriver* pThis = pDevice->GetDriver();

	if (nPropertyId == XN_MODULE_PROPERTY_ERROR_STATE)
	{
		XnSensor* pSensor = (XnSensor*)pDevice->GetSensor();

		// Get the property value.
		XnUInt64 errorState = 0;
		XnStatus nRetVal = pSensor->GetProperty(ModuleName, XN_MODULE_PROPERTY_ERROR_STATE, &errorState);
		if (nRetVal == XN_STATUS_OK)
		{
			if (errorState == XN_STATUS_DEVICE_NOT_CONNECTED)
			{
				pThis->deviceDisconnected(pDevice->GetInfo());
			}
			else
			{
				int errorStateValue = XN_ERROR_STATE_OK;
				switch (errorState)
				{
					case XN_STATUS_OK:
					{
						errorStateValue = XN_ERROR_STATE_OK;
						break;
					}
					case XN_STATUS_DEVICE_PROJECTOR_FAULT:
					{
						errorStateValue = XN_ERROR_STATE_DEVICE_PROJECTOR_FAULT;
						break;
					}
					case XN_STATUS_DEVICE_OVERHEAT:
					{
						errorStateValue = XN_ERROR_STATE_DEVICE_OVERHEAT;
						break;
					}
					default:
					{
						// Invalid value.
						XN_ASSERT(FALSE);
					}
				}
				pThis->deviceStateChanged(pDevice->GetInfo(), errorStateValue);
			}
		}
	}
	*/
}

void XN_CALLBACK_TYPE LinkOniDriver::OnDeviceConnected(const OniDeviceInfo& deviceInfo, void* pCookie)
{
	LinkOniDriver* pThis = (LinkOniDriver*)pCookie;
	pThis->deviceConnected(&deviceInfo);
}

void XN_CALLBACK_TYPE LinkOniDriver::OnDeviceDisconnected(const OniDeviceInfo& deviceInfo, void* pCookie)
{
	LinkOniDriver* pThis = (LinkOniDriver*)pCookie;
	pThis->deviceDisconnected(&deviceInfo);
}

void LinkOniDriver::resolveConfigFilePath()
{
	XnChar strModulePath[XN_FILE_MAX_PATH];

#if XN_PLATFORM == XN_PLATFORM_ANDROID_ARM
	XnStatus rc = XN_STATUS_OK;
	// support for applications
	xnOSGetApplicationFilesDir(strModulePath, XN_FILE_MAX_PATH);
	xnOSAppendFilePath(strModulePath, LINK_CONFIGURATION_FILE, XN_FILE_MAX_PATH);

	XnBool bExists;
	xnOSDoesFileExist(strModulePath, &bExists);

	if (!bExists)
	{
		// support for native use - search in current dir
		xnOSStrCopy(strModulePath, LINK_CONFIGURATION_FILE, XN_FILE_MAX_PATH);
	}
#else
	if (xnOSGetModulePathForProcAddress(reinterpret_cast<void*>(&LinkOniDriver::OnDeviceConnected), strModulePath) != XN_STATUS_OK ||
		xnOSGetDirName(strModulePath, m_configFilePath, sizeof(m_configFilePath)) != XN_STATUS_OK)
	{
		// Something wrong happened. Use the current directory as the fall-back.
		xnOSStrCopy(m_configFilePath, ".", sizeof(m_configFilePath));
	}

	xnOSAppendFilePath(m_configFilePath, "PSLink.ini", sizeof(m_configFilePath));
#endif
}

