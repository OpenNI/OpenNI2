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
#include "XnClientUSBOutDataEndpoint.h"
#include <XnUSB.h>
#include <XnLog.h>

#define XN_MASK_USB "xnUSB"

namespace xn
{

const XnUInt16 ClientUSBOutDataEndpoint::ENDPOINT_ID = 0x0001;
const XnUInt32 ClientUSBOutDataEndpoint::SEND_TIMEOUT = 2000;

ClientUSBOutDataEndpoint::ClientUSBOutDataEndpoint(XnUSBEndPointType endpointType)
{
	m_hEndpoint = NULL;
	m_hUSBDevice = NULL;
	m_nMaxPacketSize = 0;
	m_endpointType = endpointType;
	m_bConnected = FALSE;
}

ClientUSBOutDataEndpoint::~ClientUSBOutDataEndpoint()
{
	Shutdown();
}

XnStatus ClientUSBOutDataEndpoint::Init(XN_USB_DEV_HANDLE hUSBDevice)
{
	XN_VALIDATE_INPUT_PTR(hUSBDevice);
	m_hUSBDevice = hUSBDevice;
	return XN_STATUS_OK;
}

void ClientUSBOutDataEndpoint::Shutdown()
{
	Disconnect();
	m_hUSBDevice = NULL;
}

XnStatus ClientUSBOutDataEndpoint::Connect()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (!m_bConnected)
	{
		nRetVal = xnUSBOpenEndPoint(m_hUSBDevice, ENDPOINT_ID, m_endpointType, XN_USB_DIRECTION_OUT, &m_hEndpoint);
		XN_IS_STATUS_OK_LOG_ERROR("Open USB Out Data Endpoint", nRetVal);
		XnUInt32 nTempMaxPacketSize = 0;
		nRetVal = xnUSBGetEndPointMaxPacketSize(m_hEndpoint, &nTempMaxPacketSize);
		XN_IS_STATUS_OK_LOG_ERROR("Get USB Out Data endpoint max packet size", nRetVal);
		if (nTempMaxPacketSize > XN_MAX_UINT16)
		{
			xnLogError(XN_MASK_USB, "Max packet size exceeds max uint16 value ?!");
			XN_ASSERT(FALSE);
			return XN_STATUS_ERROR;
		}
		m_nMaxPacketSize = static_cast<XnUInt16>(nTempMaxPacketSize);
		m_bConnected = TRUE;
	}
	return XN_STATUS_OK;	
}

void ClientUSBOutDataEndpoint::Disconnect()
{
	if (m_bConnected)
	{
		xnUSBCloseEndPoint(m_hEndpoint);
		m_hEndpoint = NULL;
	}
}

XnStatus ClientUSBOutDataEndpoint::Send(const void* pData, XnUInt32 nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	/* TEMP TEMP TEMP - Patch to bypass USB driver bug */
	{
		XnUInt32 nBlockSize = 8 * m_nMaxPacketSize;
		XnUInt32 nRemainderSize = nSize % nBlockSize;
		if (nRemainderSize > 0)
		{
			xnLogVerbose(XN_MASK_USB, "Temporary USB patch: rounded up size to %u (instead of %u) before sending data", nSize + nBlockSize - nRemainderSize, nSize);
			// memset rest of buffer (otherwise it will contain old headers)
			xnOSMemSet((XnUInt8*)pData + nSize, 0, nBlockSize - nRemainderSize);
			nSize += (nBlockSize - nRemainderSize);
		}
	}
	/* TEMP TEMP TEMP - Patch to bypass USB driver bug */

	nRetVal = xnUSBWriteEndPoint(m_hEndpoint, (XnUChar*)pData, nSize, SEND_TIMEOUT);
	
	/* TEMP TEMP TEMP - Patch - prevent USB driver buffer from overflowing */
	//xnOSSleep(2000);
	/* TEMP TEMP TEMP - Patch - prevent USB driver buffer from overflowing */
	
	XN_IS_STATUS_OK_LOG_ERROR("Write to USB data endpoint", nRetVal);
	return XN_STATUS_OK;
}

XnUInt16 ClientUSBOutDataEndpoint::GetMaxPacketSize() const
{
	XN_ASSERT(m_hEndpoint != NULL); //Are we even connected?
	return m_nMaxPacketSize;
}

XnBool ClientUSBOutDataEndpoint::IsConnected() const
{
	return m_bConnected;
}

}
