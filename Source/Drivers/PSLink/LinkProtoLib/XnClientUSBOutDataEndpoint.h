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
#ifndef XNCLIENTUSBOUTDATAENDPOINT_H
#define XNCLIENTUSBOUTDATAENDPOINT_H

#include "IOutputConnection.h"
#include <XnUSB.h>

struct XnUSBDeviceHandle;
struct XnUSBEndPointHandle;

typedef XnUSBDeviceHandle*  XN_USB_DEV_HANDLE;
typedef XnUSBEndPointHandle* XN_USB_EP_HANDLE;

namespace xn
{

class ClientUSBOutDataEndpoint : virtual public IOutputConnection
{
public:
	ClientUSBOutDataEndpoint(XnUSBEndPointType endpointType);
	virtual ~ClientUSBOutDataEndpoint();

	virtual XnStatus Init(XN_USB_DEV_HANDLE hUSBDevice);
	virtual void Shutdown();
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);
	virtual XnUInt16 GetMaxPacketSize() const;

private:
	XnUSBEndPointType m_endpointType;
	XN_USB_EP_HANDLE m_hEndpoint;
	XN_USB_DEV_HANDLE m_hUSBDevice;
	static const XnUInt16 ENDPOINT_ID;
	static const XnUInt32 SEND_TIMEOUT;
	
	XnUInt16 m_nMaxPacketSize;
	XnBool m_bConnected;
};

}

#endif // XNCLIENTUSBOUTDATAENDPOINT_H
