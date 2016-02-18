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
#ifndef XNSERVERUSBLINUXCONTROLENDPOINT_H
#define XNSERVERUSBLINUXCONTROLENDPOINT_H

#include "ISyncIOConnection.h"
#include <XnOS.h>

struct XnUSBDevice;

namespace xn
{

class ServerUSBLinuxControlEndpoint : virtual public ISyncIOConnection
{
public:
	ServerUSBLinuxControlEndpoint();
	virtual ~ServerUSBLinuxControlEndpoint();

	virtual XnStatus Init(XnUSBDevice* pUSBDevice, XnUInt16 nMaxPacketSize);
	virtual void Shutdown();

	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnUInt16 GetMaxPacketSize() const;

	virtual XnStatus Receive(void* pData, XnUInt32& nSize);
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);

private:
	static const XnUInt32 RECEIVE_TIMEOUT;

	static void OnControlRequest(XnUSBDevice* pDevice, void* pCookie);

	XnUSBDevice* m_pUSBDevice;
	XN_EVENT_HANDLE m_hControlEvent;
	XnUInt16 m_nMaxPacketSize;
	XnBool m_bConnected;
};

}
#endif // XNSERVERUSBLINUXCONTROLENDPOINT_H
