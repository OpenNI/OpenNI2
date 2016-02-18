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
#include "XnOniDriver.h"
#include "XnOniDevice.h"
#include <XnCommon.h>
#include <XnOS.h>
#include "../Sensor/XnSensor.h"
#include "../Sensor/XnDeviceEnumeration.h"
#include <XnLogWriterBase.h>

//---------------------------------------------------------------------------
// XnOniDriver class
//---------------------------------------------------------------------------
XnOniDriver::XnOpenNILogWriter::XnOpenNILogWriter(OniDriverServices* pDriverServices) : m_pDriverServices(pDriverServices)
{
}

void XnOniDriver::XnOpenNILogWriter::WriteEntry(const XnLogEntry* pEntry)
{
	m_pDriverServices->log(m_pDriverServices, pEntry->nSeverity, pEntry->strFile, pEntry->nLine, pEntry->strMask, pEntry->strMessage);
}

void XnOniDriver::XnOpenNILogWriter::WriteUnformatted(const XnChar* /*strMessage*/)
{
	// DO NOTHING
}

OniStatus XnOniDriver::initialize(oni::driver::DeviceConnectedCallback deviceConnectedCallback, oni::driver::DeviceDisconnectedCallback deviceDisconnectedCallback, oni::driver::DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie)
{
	OniStatus nRetVal = DriverBase::initialize(deviceConnectedCallback, deviceDisconnectedCallback, deviceStateChangedCallback, pCookie);
	if (nRetVal != ONI_STATUS_OK)
	{
		return (nRetVal);
	}

	xnLogSetMaskMinSeverity(XN_LOG_MASK_ALL, XN_LOG_VERBOSE);
	m_writer.Register();

	XnStatus rc = XnDeviceEnumeration::ConnectedEvent().Register(OnDeviceConnected, this, m_connectedEventHandle);
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}

	rc = XnDeviceEnumeration::DisconnectedEvent().Register(OnDeviceDisconnected, this, m_disconnectedEventHandle);
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}

	rc = XnDeviceEnumeration::Initialize();
	if (rc != XN_STATUS_OK)
	{
		return ONI_STATUS_ERROR;
	}

	return ONI_STATUS_OK;
}

void XnOniDriver::shutdown() 
{
	if (m_connectedEventHandle != NULL)
	{
		XnDeviceEnumeration::ConnectedEvent().Unregister(m_connectedEventHandle);
		m_connectedEventHandle = NULL;
	}

	if (m_disconnectedEventHandle != NULL)
	{
		XnDeviceEnumeration::DisconnectedEvent().Unregister(m_disconnectedEventHandle);
		m_disconnectedEventHandle = NULL;
	}

	// Close all open devices and release the memory
	for (xnl::StringsHash<XnOniDevice*>::Iterator it = m_devices.Begin(); it != m_devices.End(); ++it)
	{
		XN_DELETE(it->Value());
	}

	m_devices.Clear();

	XnDeviceEnumeration::Shutdown();
}

oni::driver::DeviceBase* XnOniDriver::deviceOpen(const char* uri, const char* mode)
{
	XnOniDevice* pDevice = NULL;

	// if device was already opened for this uri, return the previous one
	if (m_devices.Get(uri, pDevice) == XN_STATUS_OK)
	{
		getServices().errorLoggerAppend("Device is already open.");
		return NULL;
	}

	pDevice = XN_NEW(XnOniDevice, uri, getServices(), this);
	XnStatus nRetVal = pDevice->Init(mode);
	if (nRetVal != XN_STATUS_OK)
	{
		getServices().errorLoggerAppend("Could not open \"%s\": %s", uri, xnGetStatusString(nRetVal));
		return NULL;
	}

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

	// Add the device and return it.
	m_devices[uri] = pDevice;
	return pDevice;
}

void XnOniDriver::deviceClose(oni::driver::DeviceBase* pDevice)
{
	for (xnl::StringsHash<XnOniDevice*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
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

void* XnOniDriver::enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount)
{
	// Make sure all the streams belong to same device.
	XnOniDevice* pDevice = NULL;
	for (int i = 0; i < streamCount; ++i)
	{
		XnOniStream* pStream = dynamic_cast<XnOniStream*>(pStreams[i]);
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
	OniStatus rc = pDevice->EnableFrameSync((XnOniStream**)pStreams, streamCount);
	if (rc != ONI_STATUS_OK)
	{
		XN_DELETE(pFrameSyncGroup);
		return NULL;
	}

	// Return the created handle.
	return pFrameSyncGroup;
}

void XnOniDriver::disableFrameSync(void* frameSyncGroup)
{
	FrameSyncGroup* pFrameSyncGroup = (FrameSyncGroup*)frameSyncGroup;

	// Find device in driver.
	xnl::StringsHash<XnOniDevice*>::ConstIterator iter = m_devices.Begin();
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

void XN_CALLBACK_TYPE XnOniDriver::OnDevicePropertyChanged(const XnChar* ModuleName, XnUInt32 nPropertyId, void* pCookie)
{
	XnOniDevice* pDevice = (XnOniDevice*)pCookie;
	XnOniDriver* pThis = pDevice->GetDriver();

	if (nPropertyId == XN_MODULE_PROPERTY_ERROR_STATE)
	{
		XnSensor* pSensor = (XnSensor*)pDevice->GetSensor();

		// Get the property value.
		XnUInt64 errorState = 0;
		XnStatus nRetVal = pSensor->GetProperty(ModuleName, XN_MODULE_PROPERTY_ERROR_STATE, &errorState);
		if (nRetVal == XN_STATUS_OK)
		{
			// ignore NOT_CONNECTED state. It's already handled by the DeviceEnumeration class
			if (errorState != XN_STATUS_DEVICE_NOT_CONNECTED)
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
}

void XN_CALLBACK_TYPE XnOniDriver::OnDeviceConnected(const OniDeviceInfo& deviceInfo, void* pCookie)
{
	XnOniDriver* pThis = (XnOniDriver*)pCookie;
	pThis->deviceConnected(&deviceInfo);
}

void XN_CALLBACK_TYPE XnOniDriver::OnDeviceDisconnected(const OniDeviceInfo& deviceInfo, void* pCookie)
{
	XnOniDriver* pThis = (XnOniDriver*)pCookie;
	pThis->deviceDisconnected(&deviceInfo);
}
