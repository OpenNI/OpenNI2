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
#ifndef XNCLIENTUSBCONNECTIONFACTORY_H
#define XNCLIENTUSBCONNECTIONFACTORY_H

#include "IConnectionFactory.h"
#include "XnLinkProtoLibDefs.h"
#include "XnClientUSBControlEndpoint.h"
#include <XnStringsHash.h>
#include <XnUSB.h>

struct XnUSBDeviceHandle;
typedef XnUSBDeviceHandle*  XN_USB_DEV_HANDLE;

namespace xn
{

class ClientUSBConnectionFactory : public IConnectionFactory
{
public:
	ClientUSBConnectionFactory(XnUInt16 nInputConnections,
	                           XnUInt16 nOutputConnections,
							   XnUInt32 nPreControlReceiveSleep);

	virtual ~ClientUSBConnectionFactory();
	virtual XnStatus Init(const XnChar* strConnString);
	virtual void Shutdown();
	virtual XnBool IsInitialized() const;

	XnStatus SetUsbAltInterface(XnUInt8 interfaceNum);
	XnStatus GetUsbAltInterface(XnUInt8* pInterfaceNum) const;

	virtual XnUInt16 GetNumOutputDataConnections() const;
	virtual XnUInt16 GetNumInputDataConnections() const;

	virtual XnStatus GetControlConnection(ISyncIOConnection*& pConn);
	virtual XnStatus CreateOutputDataConnection(XnUInt16 nID, IOutputConnection*& pConn);
	virtual XnStatus CreateInputDataConnection(XnUInt16 nID, IAsyncInputConnection*& pConn);
	
	static XnStatus EnumerateConnStrings(XnUInt16 nProductID, XnConnectionString*& astrConnStrings, XnUInt32& nCount);
	static void FreeConnStringsList(XnConnectionString* astrConnStrings);

private:
	XnUInt16 m_nInputConnections;
	XnUInt16 m_nOutputConnections;
	XnUInt32 m_nPreControlReceiveSleep;
	XnUInt8 m_nAltInterface;

	ClientUSBControlEndpoint m_controlEndpoint;
	static const XnUInt16 NUM_INPUT_CONNECTIONS;
	XN_USB_DEV_HANDLE m_hUSBDevice;
	XnBool m_bInitialized;
	XnBool m_bUsbInitialized;
	XnBool m_dataOpen;
};

}

#endif // XNCLIENTUSBCONNECTIONFACTORY_H
