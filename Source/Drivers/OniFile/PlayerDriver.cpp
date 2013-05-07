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
/// @file
/// Contains the definition of Driver class that implements an OpenNI driver,
/// which manages virtual OpenNI devices. Those devices read their data from
/// *.ONI files.

#include "PlayerDriver.h"
#include "XnLib.h"
#include "XnString.h"
#include "PlayerDevice.h"

namespace oni_file {

namespace driver = oni::driver;

namespace {

// A string that stores device vendor name.
const xnl::String kVendorString("PrimeSense, Ltd.");
const xnl::String kDeviceName("oni File");

} // namespace

PlayerDriver::PlayerDriver(OniDriverServices* pDriverServices)
    : driver::DriverBase(pDriverServices), m_fileHandle(XN_INVALID_FILE_HANDLE)
{
}

driver::DeviceBase* PlayerDriver::deviceOpen(const char* strUri, const char* /*mode*/)
{
	PlayerDevice* pDevice = XN_NEW(PlayerDevice, strUri);
	if (pDevice == NULL)
	{
		return NULL;
	}
	
	pDevice->SetEOFEventCallback(EOFReached, this);

	OniStatus rc = pDevice->Initialize();
	if (rc != ONI_STATUS_OK)
	{
		XN_DELETE(pDevice);
		return NULL;
	}

	return pDevice;
}

void PlayerDriver::deviceClose(oni::driver::DeviceBase* pDevice)
{
	XN_DELETE(pDevice);
}

void PlayerDriver::shutdown()
{
}

OniStatus PlayerDriver::tryDevice(const char* strUri)
{
	static XnPlayerInputStreamInterface inputInterface = 
	{
		FileOpen,
		FileRead,
		NULL,
		NULL,
		FileClose,
		NULL,
		NULL,
	};

	// Store the file path.
	m_filePath = strUri;

	XnStatus rc = PlayerNode::ValidateStream(this, &inputInterface);
	if (rc == XN_STATUS_OK)
	{
		OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
		xnOSMemSet(pInfo, 0, sizeof(*pInfo));
		xnOSStrCopy(pInfo->uri,    strUri,               ONI_MAX_STR);
		xnOSStrCopy(pInfo->vendor, kVendorString.Data(), ONI_MAX_STR);
		xnOSStrCopy(pInfo->name,   kDeviceName.Data(),   ONI_MAX_STR);
		deviceConnected(pInfo);
		return ONI_STATUS_OK;
	}

	return DriverBase::tryDevice(strUri);
}

XnStatus XN_CALLBACK_TYPE PlayerDriver::FileOpen(void* pCookie)
{
	PlayerDriver* pThis = (PlayerDriver*)pCookie;
	return xnOSOpenFile(pThis->m_filePath.Data(), XN_OS_FILE_READ, &pThis->m_fileHandle);
}

XnStatus XN_CALLBACK_TYPE PlayerDriver::FileRead(void* pCookie, void* pBuffer, XnUInt32 nSize, XnUInt32* pnBytesRead)
{
	PlayerDriver* pThis = (PlayerDriver*)pCookie;
	XnUInt32 bufferSize = nSize;
	XnStatus rc = xnOSReadFile(pThis->m_fileHandle, pBuffer, &bufferSize);
	*pnBytesRead = bufferSize;
	return rc;
}

void XN_CALLBACK_TYPE PlayerDriver::FileClose(void* pCookie)
{
	PlayerDriver* pThis = (PlayerDriver*)pCookie;
	xnOSCloseFile(&pThis->m_fileHandle);
	pThis->m_fileHandle = 0;
}

void XN_CALLBACK_TYPE PlayerDriver::EOFReached(void* pCookie, const char *strUri)
{
	PlayerDriver* pThis = (PlayerDriver*)pCookie;

	OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
	xnOSMemSet(pInfo, 0, sizeof(*pInfo));
	xnOSStrCopy(pInfo->uri,    strUri,               ONI_MAX_STR);
	xnOSStrCopy(pInfo->vendor, kVendorString.Data(), ONI_MAX_STR);
	xnOSStrCopy(pInfo->name,   kDeviceName.Data(),   ONI_MAX_STR);

	pThis->deviceStateChanged(pInfo, ONI_DEVICE_STATE_EOF);
}

} // namespace oni_file

ONI_EXPORT_DRIVER(oni_file::PlayerDriver)
