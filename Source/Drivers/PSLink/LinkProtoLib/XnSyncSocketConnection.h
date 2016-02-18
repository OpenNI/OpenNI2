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
#ifndef XNSYNCSOCKETCONNECTION_H
#define XNSYNCSOCKETCONNECTION_H

#include "ISyncIOConnection.h"
#include "XnLinkProtoLibDefs.h"

struct xnOSSocket;
typedef struct xnOSSocket* XN_SOCKET_HANDLE;

namespace xn
{

class SyncSocketConnection : virtual public ISyncIOConnection
{
public:
	SyncSocketConnection();
	SyncSocketConnection(const SyncSocketConnection& other);
	virtual ~SyncSocketConnection();
	SyncSocketConnection& operator=(const SyncSocketConnection& other);

// Operations
	virtual XnStatus Init(const XnChar* strIP, XnUInt16 nPort, XnUInt16 nMaxPacketSize);
	virtual void Shutdown();
	XnBool IsInitialized() const;

	virtual XnBool IsConnected() const;
	const XnChar* GetIP() const;
	XnUInt16 GetPort() const;
	
// ISyncIOConnection implementation
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnStatus Receive(void* pData, XnUInt32& nSize);
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);
	virtual XnUInt16 GetMaxPacketSize() const;

	static XnUInt32 CONNECT_TIMEOUT;
	static XnUInt32 RECEIVE_TIMEOUT;
	
protected:
	XnBool m_bInitialized;
	XnConnectionString m_strIP;
	XnUInt16 m_nPort;
    XnUInt16 m_nMaxPacketSize;
	XN_SOCKET_HANDLE m_hSocket;
};

}

#endif // XNSYNCSOCKETCONNECTION_H
