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
#include "XnDeviceSensorIO.h"
#include "XnDeviceSensor.h"
#include <XnStringsHash.h>
#include "XnDeviceEnumeration.h"

//---------------------------------------------------------------------------
// Enums
//---------------------------------------------------------------------------
typedef enum
{
	XN_FW_USB_INTERFACE_ISO = 0,
	XN_FW_USB_INTERFACE_BULK = 1,
} XnFWUsbInterface;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnSensorIO::XnSensorIO(XN_SENSOR_HANDLE* pSensorHandle) :
	m_pSensorHandle(pSensorHandle),
	m_bMiscSupported(FALSE),
	m_bIsLowBandwidth(FALSE)
{
}

XnSensorIO::~XnSensorIO()
{
}

XnStatus XnSensorIO::OpenDevice(const XnChar* strPath)
{
	XnStatus nRetVal;

	xnLogVerbose(XN_MASK_DEVICE_IO, "Connecting to USB device...");

	// try to open the device
	xnLogVerbose(XN_MASK_DEVICE_IO, "Trying to open sensor '%s'...", strPath);
	nRetVal = xnUSBOpenDeviceByPath(strPath, &m_pSensorHandle->USBDevice);
	XN_IS_STATUS_OK(nRetVal);

	// on older firmwares, control was sent over BULK endpoints. Check if this is the case
	xnLogVerbose(XN_MASK_DEVICE_IO, "Trying to open endpoint 0x4 for control out (for old firmwares)...");
	nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, 0x4, XN_USB_EP_BULK, XN_USB_DIRECTION_OUT, &m_pSensorHandle->ControlConnection.ControlOutConnectionEp);
	if (nRetVal == XN_STATUS_USB_ENDPOINT_NOT_FOUND || nRetVal == XN_STATUS_USB_WRONG_ENDPOINT_TYPE || nRetVal == XN_STATUS_USB_WRONG_ENDPOINT_DIRECTION)
	{
		// this is not the case. use regular control endpoint (0)
		m_pSensorHandle->ControlConnection.bIsBulk = FALSE;
	}
	else
	{
		XN_IS_STATUS_OK(nRetVal);

		xnLogVerbose(XN_MASK_DEVICE_IO, "Opening endpoint 0x85 for control in...");
		nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, 0x85, XN_USB_EP_BULK, XN_USB_DIRECTION_IN, &m_pSensorHandle->ControlConnection.ControlInConnectionEp);
		XN_IS_STATUS_OK(nRetVal);

		m_pSensorHandle->ControlConnection.bIsBulk = TRUE;
	}

	nRetVal = XnDeviceEnumeration::IsSensorLowBandwidth(strPath, &m_bIsLowBandwidth);
	XN_IS_STATUS_OK(nRetVal);

	xnLogInfo(XN_MASK_DEVICE_IO, "Connected to USB device%s", m_bIsLowBandwidth ? " (LowBand)" : "");

	// check if we're currently on BULK interfaces or ISO ones
	XN_USB_EP_HANDLE hEP;
	nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, 0x82, XN_USB_EP_BULK, XN_USB_DIRECTION_IN, &hEP);
	if (nRetVal == XN_STATUS_USB_WRONG_ENDPOINT_TYPE)
	{
		m_interface = XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS;
	}
	else
	{
		m_interface = XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS;
		xnUSBCloseEndPoint(hEP);
	}

	strcpy(m_strDeviceName, strPath);

	return XN_STATUS_OK;
}

XnStatus XnSensorIO::OpenDataEndPoints(XnSensorUsbInterface nInterface, const XnFirmwareInfo& fwInfo)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// try to set requested interface
	if (nInterface != XN_SENSOR_USB_INTERFACE_DEFAULT)
	{
		XnUInt8 nAlternativeInterface = 0;

		switch (nInterface)
		{
		case XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS:
			nAlternativeInterface = fwInfo.nISOAlternativeInterface;
			break;
		case XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS:
			nAlternativeInterface = fwInfo.nBulkAlternativeInterface;
			break;
		default:
			XN_ASSERT(FALSE);
			XN_LOG_WARNING_RETURN(XN_STATUS_USB_INTERFACE_NOT_SUPPORTED, XN_MASK_DEVICE_IO, "Unknown interface type: %d", nInterface);
		}

		xnLogVerbose(XN_MASK_DEVICE_IO, "Setting USB alternative interface to %d...", nAlternativeInterface);
		nRetVal = xnUSBSetInterface(m_pSensorHandle->USBDevice, 0, nAlternativeInterface);
		XN_IS_STATUS_OK(nRetVal);
	}

	xnLogVerbose(XN_MASK_DEVICE_IO, "Opening endpoints...");

	// up until v3.0/4.0, Image went over 0x82, depth on 0x83, audio on 0x86, and control was using bulk EPs, at 0x4 and 0x85.
	// starting v3.0/4.0, Image is at 0x81, depth at 0x82, audio/misc at 0x83, and control is using actual control EPs.
	// This means we are using the new Jungo USB Code
	XnBool bNewUSB = TRUE;

	// Depth
	m_pSensorHandle->DepthConnection.bIsISO = FALSE;

	xnLogVerbose(XN_MASK_DEVICE_IO, "Opening endpoint 0x81 for depth...");
	nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, 0x81, XN_USB_EP_BULK, XN_USB_DIRECTION_IN, &m_pSensorHandle->DepthConnection.UsbEp);
	if (nRetVal == XN_STATUS_USB_ENDPOINT_NOT_FOUND)
	{
		bNewUSB = FALSE;
		xnLogVerbose(XN_MASK_DEVICE_IO, "Endpoint 0x81 does not exist. Trying old USB: Opening 0x82 for depth...");
		nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, 0x82, XN_USB_EP_BULK, XN_USB_DIRECTION_IN, &m_pSensorHandle->DepthConnection.UsbEp);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		if (nRetVal == XN_STATUS_USB_WRONG_ENDPOINT_TYPE)
		{
			nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, 0x81, XN_USB_EP_ISOCHRONOUS, XN_USB_DIRECTION_IN, &m_pSensorHandle->DepthConnection.UsbEp);

			m_pSensorHandle->DepthConnection.bIsISO = TRUE;
		}

		bNewUSB = TRUE;

		XN_IS_STATUS_OK(nRetVal);

		if (m_pSensorHandle->DepthConnection.bIsISO  == TRUE)
		{
			xnLogVerbose(XN_MASK_DEVICE_IO, "Depth endpoint is isochronous.");
		}
		else
		{
			xnLogVerbose(XN_MASK_DEVICE_IO, "Depth endpoint is bulk.");
		}
	}
	m_pSensorHandle->DepthConnection.bIsOpen = TRUE;

	nRetVal = xnUSBGetEndPointMaxPacketSize(m_pSensorHandle->DepthConnection.UsbEp, &m_pSensorHandle->DepthConnection.nMaxPacketSize);
	XN_IS_STATUS_OK(nRetVal);

	// check this matches requested interface (unless DEFAULT was requested)
	if ((nInterface == XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS && m_pSensorHandle->DepthConnection.bIsISO) ||
	    (nInterface == XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS && !m_pSensorHandle->DepthConnection.bIsISO))
	{
		return (XN_STATUS_USB_INTERFACE_NOT_SUPPORTED);
	}

	m_interface = m_pSensorHandle->DepthConnection.bIsISO ? XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS : XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS;

	// Image
	XnUInt16 nImageEP = bNewUSB ? 0x82 : 0x83;

	m_pSensorHandle->ImageConnection.bIsISO = FALSE;

	xnLogVerbose(XN_MASK_DEVICE_IO, "Opening endpoint 0x%hx for image...", nImageEP);
	nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, nImageEP, XN_USB_EP_BULK, XN_USB_DIRECTION_IN, &m_pSensorHandle->ImageConnection.UsbEp);
	if (nRetVal == XN_STATUS_USB_WRONG_ENDPOINT_TYPE)
	{
		nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, nImageEP, XN_USB_EP_ISOCHRONOUS, XN_USB_DIRECTION_IN, &m_pSensorHandle->ImageConnection.UsbEp);

		m_pSensorHandle->ImageConnection.bIsISO = TRUE;
	}

	XN_IS_STATUS_OK(nRetVal);

	if (m_pSensorHandle->ImageConnection.bIsISO  == TRUE)
	{
		xnLogVerbose(XN_MASK_DEVICE_IO, "Image endpoint is isochronous.");
	}
	else
	{
		xnLogVerbose(XN_MASK_DEVICE_IO, "Image endpoint is bulk.");
	}

	m_pSensorHandle->ImageConnection.bIsOpen = TRUE;

	nRetVal = xnUSBGetEndPointMaxPacketSize(m_pSensorHandle->ImageConnection.UsbEp, &m_pSensorHandle->ImageConnection.nMaxPacketSize);
	XN_IS_STATUS_OK(nRetVal);

	// Misc
	XnUInt16 nMiscEP = bNewUSB ? 0x83 : 0x86;

	m_pSensorHandle->MiscConnection.bIsISO = FALSE;

	xnLogVerbose(XN_MASK_DEVICE_IO, "Opening endpoint 0x%hx for misc...", nMiscEP);
	nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, nMiscEP, XN_USB_EP_BULK, XN_USB_DIRECTION_IN, &m_pSensorHandle->MiscConnection.UsbEp);
	if (nRetVal == XN_STATUS_USB_WRONG_ENDPOINT_TYPE)
	{
		nRetVal = xnUSBOpenEndPoint(m_pSensorHandle->USBDevice, nMiscEP, XN_USB_EP_ISOCHRONOUS, XN_USB_DIRECTION_IN, &m_pSensorHandle->MiscConnection.UsbEp);

		m_pSensorHandle->MiscConnection.bIsISO = TRUE;
	}
	if (nRetVal == XN_STATUS_USB_ENDPOINT_NOT_FOUND)
	{
		// Firmware does not support misc...
		m_pSensorHandle->MiscConnection.bIsOpen = FALSE;
		m_bMiscSupported = FALSE;

		xnLogVerbose(XN_MASK_DEVICE_IO, "Misc endpoint is not supported...");
	}
	else if (nRetVal == XN_STATUS_OK)
	{
		m_pSensorHandle->MiscConnection.bIsOpen = TRUE;
		m_bMiscSupported = TRUE;

		if (m_pSensorHandle->MiscConnection.bIsISO  == TRUE)
		{ 
			xnLogVerbose(XN_MASK_DEVICE_IO, "Misc endpoint is isochronous.");
		}
		else
		{
			xnLogVerbose(XN_MASK_DEVICE_IO, "Misc endpoint is bulk.");
		}
	}
	else
	{
		return nRetVal;
	}

	if (m_pSensorHandle->MiscConnection.bIsOpen)
	{
		nRetVal = xnUSBGetEndPointMaxPacketSize(m_pSensorHandle->MiscConnection.UsbEp, &m_pSensorHandle->MiscConnection.nMaxPacketSize);
		XN_IS_STATUS_OK(nRetVal);
	}

	xnLogInfo(XN_MASK_DEVICE_IO, "Endpoints open");

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnSensorIO::CloseDevice()
{
	XnStatus nRetVal;

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Shutting down USB depth read thread...");
	xnUSBShutdownReadThread(m_pSensorHandle->DepthConnection.UsbEp);

	if (m_pSensorHandle->DepthConnection.UsbEp != NULL)
	{
		nRetVal = xnUSBCloseEndPoint(m_pSensorHandle->DepthConnection.UsbEp);
		XN_IS_STATUS_OK(nRetVal);
		m_pSensorHandle->DepthConnection.UsbEp = NULL;
	}

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Shutting down USB image read thread...");
	xnUSBShutdownReadThread(m_pSensorHandle->ImageConnection.UsbEp);

	if (m_pSensorHandle->ImageConnection.UsbEp != NULL)
	{
		nRetVal = xnUSBCloseEndPoint(m_pSensorHandle->ImageConnection.UsbEp);
		XN_IS_STATUS_OK(nRetVal);
		m_pSensorHandle->ImageConnection.UsbEp = NULL;
	}

	if (m_pSensorHandle->MiscConnection.bIsOpen)
	{
		xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Shutting down USB misc read thread...");
		xnUSBShutdownReadThread(m_pSensorHandle->MiscConnection.UsbEp);

		if (m_pSensorHandle->MiscConnection.UsbEp != NULL)
		{
			nRetVal = xnUSBCloseEndPoint(m_pSensorHandle->MiscConnection.UsbEp);
			XN_IS_STATUS_OK(nRetVal);
			m_pSensorHandle->MiscConnection.UsbEp = NULL;
		}
	}

	if (m_pSensorHandle->ControlConnection.bIsBulk)
	{
		if (m_pSensorHandle->ControlConnection.ControlInConnectionEp != NULL)
		{
			nRetVal = xnUSBCloseEndPoint(m_pSensorHandle->ControlConnection.ControlInConnectionEp);
			XN_IS_STATUS_OK(nRetVal);
			m_pSensorHandle->ControlConnection.ControlInConnectionEp = NULL;
		}

		if (m_pSensorHandle->ControlConnection.ControlOutConnectionEp != NULL)
		{
			nRetVal = xnUSBCloseEndPoint(m_pSensorHandle->ControlConnection.ControlOutConnectionEp);
			XN_IS_STATUS_OK(nRetVal);
			m_pSensorHandle->ControlConnection.ControlOutConnectionEp = NULL;
		}
	}

	if (m_pSensorHandle->USBDevice != NULL)
	{
		nRetVal = xnUSBCloseDevice(m_pSensorHandle->USBDevice);
		XN_IS_STATUS_OK(nRetVal);
		m_pSensorHandle->USBDevice = NULL;
	}

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Device closed successfully");

	// All is good...
	return (XN_STATUS_OK);
}

const XnChar* XnSensorIO::GetDevicePath()
{
	return m_strDeviceName;
}
