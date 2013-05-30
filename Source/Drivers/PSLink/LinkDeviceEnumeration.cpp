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
#include "LinkDeviceEnumeration.h"
#include "XnLinkProtoLibDefs.h"
#include <XnUSB.h>

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
XnBool LinkDeviceEnumeration::ms_initialized = FALSE;
LinkDeviceEnumeration::DeviceConnectivityEvent LinkDeviceEnumeration::ms_connectedEvent;
LinkDeviceEnumeration::DeviceConnectivityEvent LinkDeviceEnumeration::ms_disconnectedEvent;
LinkDeviceEnumeration::DevicesHash LinkDeviceEnumeration::ms_devices;
xnl::Array<XnRegistrationHandle> LinkDeviceEnumeration::ms_aRegistrationHandles;
XN_CRITICAL_SECTION_HANDLE LinkDeviceEnumeration::ms_lock;

LinkDeviceEnumeration::XnUsbId LinkDeviceEnumeration::ms_supportedProducts[] = 
{
	{ 0x1D27, 0x1250 },
	{ 0x1D27, 0x1260 },
	{ 0x1D27, 0x1270 },
	{ 0x1D27, 0x1280 },
	{ 0x1D27, 0x1290 },
	{ 0x1D27, 0xf9db },
};

XnUInt32 LinkDeviceEnumeration::ms_supportedProductsCount = sizeof(LinkDeviceEnumeration::ms_supportedProducts) / sizeof(LinkDeviceEnumeration::ms_supportedProducts[0]);

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnStatus LinkDeviceEnumeration::Initialize()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (ms_initialized)
	{
		return XN_STATUS_OK;
	}

	nRetVal = xnUSBInit();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSCreateCriticalSection(&ms_lock);
	XN_IS_STATUS_OK(nRetVal);

	const XnUSBConnectionString* astrDevicePaths;
	XnUInt32 nCount;

	// check all products
	for (XnUInt32 i = 0; i < ms_supportedProductsCount; ++i)
	{
		// register for USB events
		XnRegistrationHandle hRegistration = NULL;
		nRetVal = xnUSBRegisterToConnectivityEvents(ms_supportedProducts[i].vendorID, ms_supportedProducts[i].productID, OnConnectivityEventCallback, &ms_supportedProducts[i], &hRegistration);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = ms_aRegistrationHandles.AddLast(hRegistration);
		XN_IS_STATUS_OK(nRetVal);

		// and enumerate for existing ones
		nRetVal = xnUSBEnumerateDevices(ms_supportedProducts[i].vendorID, ms_supportedProducts[i].productID, &astrDevicePaths, &nCount);
		XN_IS_STATUS_OK(nRetVal);

		for (XnUInt32 j = 0; j < nCount; ++j)
		{
			OnConnectivityEvent(astrDevicePaths[j], XN_USB_EVENT_DEVICE_CONNECT, ms_supportedProducts[i]);
		}

		xnUSBFreeDevicesList(astrDevicePaths);
	}

	ms_initialized = TRUE;

	return XN_STATUS_OK;
}

void LinkDeviceEnumeration::Shutdown()
{
	if (ms_initialized)
	{
		for (XnUInt32 i = 0; i < ms_aRegistrationHandles.GetSize(); ++i)
		{
			xnUSBUnregisterFromConnectivityEvents(ms_aRegistrationHandles[i]);
		}
		ms_aRegistrationHandles.Clear();
		ms_connectedEvent.Clear();
		ms_disconnectedEvent.Clear();

		xnOSCloseCriticalSection(&ms_lock);

		xnUSBShutdown();

		ms_devices.Clear();

		ms_initialized = FALSE;
	}
}

void LinkDeviceEnumeration::OnConnectivityEvent(const XnChar* uri, XnUSBEventType eventType, XnUsbId usbId)
{
	xnl::AutoCSLocker lock(ms_lock);

	if (eventType == XN_USB_EVENT_DEVICE_CONNECT)
	{
		if (ms_devices.Find(uri) == ms_devices.End())
		{
			OniDeviceInfo deviceInfo;
			deviceInfo.usbVendorId  = usbId.vendorID;
			deviceInfo.usbProductId = usbId.productID;
			xnOSStrCopy(deviceInfo.uri,    uri,                  sizeof(deviceInfo.uri));
			xnOSStrCopy(deviceInfo.vendor, XN_VENDOR_PRIMESENSE, sizeof(deviceInfo.vendor));
			xnOSStrCopy(deviceInfo.name,   "PSLink",             sizeof(deviceInfo.name));

			// add it to hash
			ms_devices.Set(uri, deviceInfo);

			// raise event
			ms_connectedEvent.Raise(deviceInfo);
		}
	}
	else if (eventType == XN_USB_EVENT_DEVICE_DISCONNECT)
	{
		OniDeviceInfo deviceInfo;
		if (XN_STATUS_OK == ms_devices.Get(uri, deviceInfo))
		{
			// raise event
			ms_disconnectedEvent.Raise(deviceInfo);

			// remove it
			ms_devices.Remove(uri);
		}
	}
}

void XN_CALLBACK_TYPE LinkDeviceEnumeration::OnConnectivityEventCallback(XnUSBEventArgs* pArgs, void* pCookie)
{
	XnUsbId usbId = *(XnUsbId*)pCookie;
	OnConnectivityEvent(pArgs->strDevicePath, pArgs->eventType, usbId);
}

OniDeviceInfo* LinkDeviceEnumeration::GetDeviceInfo(const XnChar* uri)
{
	OniDeviceInfo* pInfo = NULL;
	xnl::AutoCSLocker lock(ms_lock);

	if (ms_devices.Get(uri, pInfo) == XN_STATUS_OK)
	{
		return pInfo;
	}
	else
	{
		return NULL;
	}
}

