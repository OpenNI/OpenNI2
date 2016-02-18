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
#include "XnClientUSBControlEndpoint.h"
#include <XnOS.h>
#include <XnUSB.h>
#include <XnLog.h>

#define XN_MASK_USB "xnUSB"

namespace xn
{


/*USB control endpoints' REAL max packet size is always 64 bytes. We group a few of these together, so we 
  treat the packet size as a different (larger) number.*/
const XnUInt32 ClientUSBControlEndpoint::USB_LOW_LEVEL_MAX_PACKET_SIZE = 64;
const XnUInt32 ClientUSBControlEndpoint::SEND_TIMEOUT = 5000;
const XnUInt32 ClientUSBControlEndpoint::RECEIVE_TIMEOUT = 5000;

ClientUSBControlEndpoint::ClientUSBControlEndpoint(XnUInt32 nPreControlReceiveSleep)
{
	m_hUSBDevice = NULL;
	m_nPreControlReceiveSleep = nPreControlReceiveSleep;
}

ClientUSBControlEndpoint::~ClientUSBControlEndpoint()
{
	Shutdown();
}

XnStatus ClientUSBControlEndpoint::Init(XN_USB_DEV_HANDLE hUSBDevice)
{
	XN_VALIDATE_INPUT_PTR(hUSBDevice);
	m_hUSBDevice = hUSBDevice;
	
	return XN_STATUS_OK;
}

void ClientUSBControlEndpoint::Shutdown()
{
	m_hUSBDevice = NULL;
}

XnStatus ClientUSBControlEndpoint::Connect()
{
	//Nothing to do here since the control endpoint is always connected when the device is open
	return XN_STATUS_OK;
}

void ClientUSBControlEndpoint::Disconnect()
{
}


XnUInt16 ClientUSBControlEndpoint::GetMaxPacketSize() const
{
	XN_ASSERT(FALSE); //Did you mean the logical packet size?? If the answer is no, remove this assert.
	return USB_LOW_LEVEL_MAX_PACKET_SIZE;
}

XnStatus ClientUSBControlEndpoint::Receive(void* pData, XnUInt32& nSize)
{
	XnUInt32 nBufferSize = nSize;
	XnStatus nRetVal = XN_STATUS_OK;

	// Workaround devices bug: in some devices (Lena for example), one cannot receive
	// immediately after send. The in-request should arrive AFTER the device has
	// finished reading the entire out-request and its data, and clear the state.
	// otherwise, device stalls.
	xnOSSleep(m_nPreControlReceiveSleep);

	nRetVal = xnUSBReceiveControl(m_hUSBDevice, XN_USB_CONTROL_TYPE_VENDOR, 0, 0, 0, 
		reinterpret_cast<XnUChar*>(pData), nBufferSize, &nSize, RECEIVE_TIMEOUT);
	XN_IS_STATUS_OK_LOG_ERROR("Receive buffer from USB", nRetVal);

	return XN_STATUS_OK;	
}

XnStatus ClientUSBControlEndpoint::Send(const void* pData, XnUInt32 nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = xnUSBSendControl(m_hUSBDevice, XN_USB_CONTROL_TYPE_VENDOR, 0, 0, 0, (XnUInt8*)pData, nSize, SEND_TIMEOUT);
	XN_IS_STATUS_OK_LOG_ERROR("Send USB control data", nRetVal);
	return XN_STATUS_OK;
}

XnBool ClientUSBControlEndpoint::IsConnected() const
{
	return TRUE;
}

}