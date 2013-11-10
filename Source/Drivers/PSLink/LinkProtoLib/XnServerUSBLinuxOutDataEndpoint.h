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
#ifndef XNSERVERUSBLINUXOUTDATAENDPOINT_H
#define XNSERVERUSBLINUXOUTDATAENDPOINT_H

#include "IOutputConnection.h"

struct XnUSBDevice;

namespace xn
{

class ServerUSBLinuxOutDataEndpoint : virtual public IOutputConnection
{
public:
	ServerUSBLinuxOutDataEndpoint();
	virtual ~ServerUSBLinuxOutDataEndpoint();
	virtual XnStatus Init(XnUSBDevice* pUSBDevice, XnUInt16 nEndpointID, XnUInt16 nMaxPacketSize);
	virtual void Shutdown();
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnUInt16 GetMaxPacketSize() const;
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);
	XnStatus Reset();

private:
	XnUSBDevice* m_pUSBDevice;
	XnUInt8 m_nEndpointID;
	XnUInt16 m_nMaxPacketSize;
};

}

#endif // XNSERVERUSBLINUXOUTDATAENDPOINT_H
