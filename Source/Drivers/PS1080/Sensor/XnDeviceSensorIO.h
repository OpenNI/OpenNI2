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
#ifndef XNDEVICESENSORIO_H
#define XNDEVICESENSORIO_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <XnUSB.h>
#include <XnArray.h>
#include <XnStreamParams.h>
#include <XnDevice.h>
#include "XnFirmwareInfo.h"
#include <OniCTypes.h>
#include <XnStringsHash.h>

//---------------------------------------------------------------------------
// Structures & Enums
//---------------------------------------------------------------------------
typedef struct XnUsbConnection
{
	XN_USB_EP_HANDLE UsbEp;

	XnBool bIsOpen;
	XnUInt8* pUSBBuffer;
	XnUInt32 nUSBBufferReadOffset;
	XnUInt32 nUSBBufferWriteOffset;
	XnUInt32 nMaxPacketSize;
} XnUsbConnection;

typedef struct XnUsbControlConnection
{
	/* When true, control connection is implemented using bulk end points. */
	XnBool bIsBulk;
	XN_USB_EP_HANDLE ControlOutConnectionEp;
	XN_USB_EP_HANDLE ControlInConnectionEp;
} XnUsbControlConnection;

typedef struct XN_SENSOR_HANDLE
{
	XN_USB_DEV_HANDLE USBDevice;

	XnUsbControlConnection ControlConnection;
	XnUsbConnection DepthConnection;
	XnUsbConnection ImageConnection;
	XnUsbConnection MiscConnection;
	XnUInt8 nBoardVer;
} XN_SENSOR_HANDLE;

//---------------------------------------------------------------------------
// Functions Declaration
//---------------------------------------------------------------------------
class XnSensorIO
{
public:
	XnSensorIO(XN_SENSOR_HANDLE* pSensorHandle);
	~XnSensorIO();

	XnStatus OpenDevice(const XnChar* strPath);

	XnStatus OpenDataEndPoints(XnSensorUsbInterface nInterface, const XnFirmwareInfo& fwInfo);

	XnSensorUsbInterface GetCurrentInterface(const XnFirmwareInfo& fwInfo) const;

	XnStatus CloseDevice();

	inline XnBool IsMiscEndpointSupported() const { return m_bMiscSupported; }
	inline XnBool IsLowBandwidth() const { return m_bIsLowBandwidth; }

	const XnChar* GetDevicePath();

private:
	XN_SENSOR_HANDLE* m_pSensorHandle;
	XnBool m_bMiscSupported;
	XnChar m_strDeviceName[XN_DEVICE_MAX_STRING_LENGTH];
	XnBool m_bIsLowBandwidth;
	XnUSBEventCallbackFunctionPtr m_pCallbackPtr; 
	void* m_pCallbackData;
};

#endif // XNDEVICESENSORIO_H
