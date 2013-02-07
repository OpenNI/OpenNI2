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
#include "XnDeviceSensor.h"
#include "XnDeviceSensorInit.h"
#include "XnDeviceSensorProtocol.h"
#include "Bayer.h"
#include "XnHostProtocol.h"
#include <XnLog.h>
#include "XnSensor.h"

#define XN_HOST_PROTOCOL_MUTEX_NAME_PREFIX	"HostProtocolMutex"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnStatus XnDeviceSensorInit(XnDevicePrivateData* pDevicePrivateData)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnDeviceSensorAllocateBuffers(pDevicePrivateData);
	XN_IS_STATUS_OK(nRetVal);

#if XN_PLATFORM == XN_PLATFORM_ANDROID_ARM
	nRetVal = xnOSCreateMutex(&pDevicePrivateData->hExecuteMutex);
	XN_IS_STATUS_OK(nRetVal);
#else
	XnChar strMutexName[XN_FILE_MAX_PATH];
	XnUInt32 nCharsWritten = 0;
	nRetVal = xnOSStrFormat(strMutexName, XN_FILE_MAX_PATH, &nCharsWritten, "%s%s", XN_HOST_PROTOCOL_MUTEX_NAME_PREFIX, pDevicePrivateData->pSensor->GetUSBPath());
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSCreateNamedMutex(&pDevicePrivateData->hExecuteMutex, strMutexName);
	XN_IS_STATUS_OK(nRetVal);
#endif

	nRetVal = XnDeviceSensorConfigureVersion(pDevicePrivateData);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceSensorOpenInputThreads(XnDevicePrivateData* pDevicePrivateData)
{
	// Depth
	pDevicePrivateData->pSpecificDepthUsb = (XnSpecificUsbDevice*)xnOSMallocAligned(sizeof(XnSpecificUsbDevice), XN_DEFAULT_MEM_ALIGN);
	pDevicePrivateData->pSpecificDepthUsb->pDevicePrivateData = pDevicePrivateData;
	pDevicePrivateData->pSpecificDepthUsb->pUsbConnection = &pDevicePrivateData->SensorHandle.DepthConnection;
	pDevicePrivateData->pSpecificDepthUsb->CurrState.State = XN_WAITING_FOR_CONFIGURATION;

	if (pDevicePrivateData->pSpecificDepthUsb->pUsbConnection->bIsISO == TRUE)
	{
		if (pDevicePrivateData->pSensor->IsLowBandwidth())
		{
			pDevicePrivateData->pSpecificDepthUsb->nChunkReadBytes = XN_SENSOR_USB_DEPTH_BUFFER_SIZE_MULTIPLIER_LOWBAND_ISO * pDevicePrivateData->SensorHandle.DepthConnection.nMaxPacketSize;
		}
		else
		{
			pDevicePrivateData->pSpecificDepthUsb->nChunkReadBytes = XN_SENSOR_USB_DEPTH_BUFFER_SIZE_MULTIPLIER_ISO * pDevicePrivateData->SensorHandle.DepthConnection.nMaxPacketSize;
		}

		pDevicePrivateData->pSpecificDepthUsb->nTimeout = XN_SENSOR_READ_THREAD_TIMEOUT_ISO;
	}
	else
	{
		pDevicePrivateData->pSpecificDepthUsb->nChunkReadBytes = XN_SENSOR_USB_DEPTH_BUFFER_SIZE_MULTIPLIER_BULK * pDevicePrivateData->SensorHandle.DepthConnection.nMaxPacketSize;

		pDevicePrivateData->pSpecificDepthUsb->nTimeout = XN_SENSOR_READ_THREAD_TIMEOUT_BULK;
	}

	pDevicePrivateData->pSpecificDepthUsb->nIgnoreBytes = (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_0) ? 0 : pDevicePrivateData->pSpecificDepthUsb->nChunkReadBytes;

	// Image
	pDevicePrivateData->pSpecificImageUsb = (XnSpecificUsbDevice*)xnOSMallocAligned(sizeof(XnSpecificUsbDevice), XN_DEFAULT_MEM_ALIGN);
	pDevicePrivateData->pSpecificImageUsb->pDevicePrivateData = pDevicePrivateData;
	pDevicePrivateData->pSpecificImageUsb->pUsbConnection = &pDevicePrivateData->SensorHandle.ImageConnection;
	pDevicePrivateData->pSpecificImageUsb->CurrState.State = XN_WAITING_FOR_CONFIGURATION;

	if (pDevicePrivateData->pSpecificImageUsb->pUsbConnection->bIsISO == TRUE)
	{
		if (pDevicePrivateData->pSensor->IsLowBandwidth())
		{
			pDevicePrivateData->pSpecificImageUsb->nChunkReadBytes = XN_SENSOR_USB_IMAGE_BUFFER_SIZE_MULTIPLIER_LOWBAND_ISO * pDevicePrivateData->SensorHandle.ImageConnection.nMaxPacketSize;
		}
		else
		{
			pDevicePrivateData->pSpecificImageUsb->nChunkReadBytes = XN_SENSOR_USB_IMAGE_BUFFER_SIZE_MULTIPLIER_ISO * pDevicePrivateData->SensorHandle.ImageConnection.nMaxPacketSize;
		}

		pDevicePrivateData->pSpecificImageUsb->nTimeout = XN_SENSOR_READ_THREAD_TIMEOUT_ISO;
	}
	else
	{
		pDevicePrivateData->pSpecificImageUsb->nChunkReadBytes = XN_SENSOR_USB_IMAGE_BUFFER_SIZE_MULTIPLIER_BULK * pDevicePrivateData->SensorHandle.ImageConnection.nMaxPacketSize;

		pDevicePrivateData->pSpecificImageUsb->nTimeout = XN_SENSOR_READ_THREAD_TIMEOUT_BULK;
	}

	pDevicePrivateData->pSpecificImageUsb->nIgnoreBytes = (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_0) ? 0 : pDevicePrivateData->pSpecificImageUsb->nChunkReadBytes;

	// Misc
	if (pDevicePrivateData->pSensor->IsMiscSupported())
	{
		pDevicePrivateData->pSpecificMiscUsb = (XnSpecificUsbDevice*)xnOSMallocAligned(sizeof(XnSpecificUsbDevice), XN_DEFAULT_MEM_ALIGN);
		pDevicePrivateData->pSpecificMiscUsb->pDevicePrivateData = pDevicePrivateData;
		pDevicePrivateData->pSpecificMiscUsb->pUsbConnection = &pDevicePrivateData->SensorHandle.MiscConnection;
		pDevicePrivateData->pSpecificMiscUsb->CurrState.State = XN_WAITING_FOR_CONFIGURATION;

		if (pDevicePrivateData->pSpecificMiscUsb->pUsbConnection->bIsISO == TRUE)
		{
			if (pDevicePrivateData->pSensor->IsLowBandwidth())
			{
				pDevicePrivateData->pSpecificMiscUsb->nChunkReadBytes = XN_SENSOR_USB_MISC_BUFFER_SIZE_MULTIPLIER_LOWBAND_ISO * pDevicePrivateData->SensorHandle.MiscConnection.nMaxPacketSize;
			}
			else
			{
				pDevicePrivateData->pSpecificMiscUsb->nChunkReadBytes = XN_SENSOR_USB_MISC_BUFFER_SIZE_MULTIPLIER_ISO * pDevicePrivateData->SensorHandle.MiscConnection.nMaxPacketSize;
			}

			pDevicePrivateData->pSpecificMiscUsb->nTimeout = XN_SENSOR_READ_THREAD_TIMEOUT_ISO;
		}
		else
		{
			pDevicePrivateData->pSpecificMiscUsb->nChunkReadBytes = XN_SENSOR_USB_MISC_BUFFER_SIZE_MULTIPLIER_BULK * pDevicePrivateData->SensorHandle.MiscConnection.nMaxPacketSize;

			pDevicePrivateData->pSpecificMiscUsb->nTimeout = XN_SENSOR_READ_THREAD_TIMEOUT_BULK;
		}

		pDevicePrivateData->pSpecificMiscUsb->nIgnoreBytes = (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_0) ? 0 : pDevicePrivateData->pSpecificMiscUsb->nChunkReadBytes;
	}

	// Switch depth & image EPs for older FWs
	if (pDevicePrivateData->FWInfo.nFWVer <= XN_SENSOR_FW_VER_5_1)
	{
		XnSpecificUsbDevice* pTempUsbDevice = pDevicePrivateData->pSpecificDepthUsb;
		pDevicePrivateData->pSpecificDepthUsb = pDevicePrivateData->pSpecificImageUsb;
		pDevicePrivateData->pSpecificImageUsb = pTempUsbDevice;
	}

	return XN_STATUS_OK;
}

XnStatus XnDeviceSensorAllocateBuffers(XnDevicePrivateData* pDevicePrivateData)
{
	pDevicePrivateData->SensorHandle.DepthConnection.pUSBBuffer = (XnUInt8*)xnOSCallocAligned(XN_SENSOR_PROTOCOL_USB_BUFFER_SIZE, sizeof(XnUInt8), XN_DEFAULT_MEM_ALIGN);
	pDevicePrivateData->SensorHandle.DepthConnection.nUSBBufferReadOffset = 0;
	pDevicePrivateData->SensorHandle.DepthConnection.nUSBBufferWriteOffset = 0;

	pDevicePrivateData->SensorHandle.ImageConnection.pUSBBuffer = (XnUInt8*)xnOSCallocAligned(XN_SENSOR_PROTOCOL_USB_BUFFER_SIZE, sizeof(XnUInt8), XN_DEFAULT_MEM_ALIGN);
	pDevicePrivateData->SensorHandle.ImageConnection.nUSBBufferReadOffset = 0;
	pDevicePrivateData->SensorHandle.ImageConnection.nUSBBufferWriteOffset = 0;

	if (pDevicePrivateData->pSensor->IsMiscSupported())
	{
		pDevicePrivateData->SensorHandle.MiscConnection.pUSBBuffer = (XnUInt8*)xnOSCallocAligned(XN_SENSOR_PROTOCOL_USB_BUFFER_SIZE, sizeof(XnUInt8), XN_DEFAULT_MEM_ALIGN);
		pDevicePrivateData->SensorHandle.MiscConnection.nUSBBufferReadOffset = 0;
		pDevicePrivateData->SensorHandle.MiscConnection.nUSBBufferWriteOffset = 0;
	}
	else
	{
		pDevicePrivateData->SensorHandle.MiscConnection.pUSBBuffer = NULL;
	}

	return (XN_STATUS_OK);
}

XnStatus XnDeviceSensorFreeBuffers(XnDevicePrivateData* pDevicePrivateData)
{
	if (pDevicePrivateData->SensorHandle.DepthConnection.pUSBBuffer != NULL)
	{
		XN_ALIGNED_FREE_AND_NULL(pDevicePrivateData->SensorHandle.DepthConnection.pUSBBuffer);
	}

	if (pDevicePrivateData->SensorHandle.ImageConnection.pUSBBuffer != NULL)
	{
		XN_ALIGNED_FREE_AND_NULL(pDevicePrivateData->SensorHandle.ImageConnection.pUSBBuffer);
	}

	if (pDevicePrivateData->SensorHandle.MiscConnection.pUSBBuffer != NULL)
	{
		XN_ALIGNED_FREE_AND_NULL(pDevicePrivateData->SensorHandle.MiscConnection.pUSBBuffer);
	}

	if (pDevicePrivateData->pSpecificDepthUsb != NULL)
	{
		XN_ALIGNED_FREE_AND_NULL(pDevicePrivateData->pSpecificDepthUsb);
	}

	if (pDevicePrivateData->pSpecificImageUsb != NULL)
	{
		XN_ALIGNED_FREE_AND_NULL(pDevicePrivateData->pSpecificImageUsb);
	}

	if (pDevicePrivateData->pSpecificMiscUsb != NULL)
	{
		XN_ALIGNED_FREE_AND_NULL(pDevicePrivateData->pSpecificMiscUsb);
	}

	return (XN_STATUS_OK);
}

XnStatus XnDeviceSensorConfigureVersion(XnDevicePrivateData* pDevicePrivateData)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// GetVersion is exactly the same in all versions, except a change that was made in version 5.1.
	// so, we'll start with that, and if doesn't work we'll try previous protocols
	XnHostProtocolUsbCore usb = XN_USB_CORE_JANGO;
	nRetVal = XnHostProtocolInitFWParams(pDevicePrivateData, 5, 1, 0, usb);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnHostProtocolGetVersion(pDevicePrivateData, pDevicePrivateData->Version);
	
	// Strange bug: sometimes, when sending first command to device, no reply is received, so try again
	if (nRetVal == XN_STATUS_USB_TRANSFER_TIMEOUT)
	{
		xnOSSleep(2000);
		nRetVal = XnHostProtocolGetVersion(pDevicePrivateData, pDevicePrivateData->Version);
	}
	
	// if command failed for any reason, try again with older protocol
	if (nRetVal != XN_STATUS_OK)
	{
		nRetVal = XnHostProtocolInitFWParams(pDevicePrivateData, 5, 0, 0, usb);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = XnHostProtocolGetVersion(pDevicePrivateData, pDevicePrivateData->Version);
	}

	// if it still fails, give up
	XN_IS_STATUS_OK(nRetVal);

	// check which usb core is used (don't check error code. If this fails, assume JANGO
	if (XN_STATUS_OK != XnHostProtocolGetUsbCoreType(pDevicePrivateData, usb))
	{
		usb = XN_USB_CORE_JANGO;
	}

	// Now that we have the actual version, configure protocol accordingly
	nRetVal = XnHostProtocolInitFWParams(pDevicePrivateData, pDevicePrivateData->Version.nMajor, pDevicePrivateData->Version.nMinor, pDevicePrivateData->Version.nBuild, usb);
	XN_IS_STATUS_OK(nRetVal);

	pDevicePrivateData->HWInfo.nHWVer = pDevicePrivateData->Version.HWVer;
	pDevicePrivateData->ChipInfo.nChipVer = pDevicePrivateData->Version.ChipVer;

	return (XN_STATUS_OK);
}
