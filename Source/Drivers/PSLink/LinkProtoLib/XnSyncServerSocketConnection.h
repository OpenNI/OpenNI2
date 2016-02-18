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
// XnSyncServerSocketConnection.h

#ifndef XNSYNCSERVERSOCKETCONNECTION_H
#define XNSYNCSERVERSOCKETCONNECTION_H

#include "XnSyncSocketConnection.h"
#include "XnStatusCodes.h"
#include "XnServerSocketInConnection.h"


namespace xn
{

class IAsyncInputConnection;

class SyncServerSocketConnection : virtual public SyncSocketConnection
{
public:
	// Overriding connect, and init because this class is allready created with valid socket handle
	virtual XnStatus Init(const XnChar* /*strIP*/, XnUInt16 /*nPort*/, XnUInt16 /*nMaxPacketSize*/)
	{
		return IsConnected() ? XN_STATUS_OK : XN_STATUS_ERROR;
	}
	virtual XnStatus Connect() 
	{
		return IsConnected() ? XN_STATUS_OK : XN_STATUS_ERROR;
	}
	friend class SyncServerSocketListener;
};


class ASyncServerSocketConnection : virtual public ServerSocketInConnection
{
public:
	virtual XnStatus ConnectSocket(XN_SOCKET_HANDLE& hSocket, const XnChar* /*strIP*/, XnUInt16 /*nPort*/)
	{
		hSocket = m_hSocketFromListener;
		return XN_STATUS_OK;
	}

protected:
	friend class SyncServerSocketListener;
	XN_SOCKET_HANDLE m_hSocketFromListener;
};

#define MAX_SERVER_CONTROL_CONNECTIONS 10
#define MAX_DATAOUT_SOCKETS	10

class SyncServerSocketListener
{
public:
	SyncServerSocketListener();
	virtual ~SyncServerSocketListener();

	virtual XnStatus Init(const XnChar* strIP, 
						  XnUInt16 nControlPort,			
						  XnUInt16 nInputPort,
						  XnUInt16 nFirstOutputDataPort,	
						  XnUInt16 nDataOutSockets,
						  XnUInt16 nMaxColntrolPacketSize,
						  XnUInt16 nMaxDataOutPacketSize,
						  XnUInt16 nMaxDataInPacketSize);
	virtual void Shutdown();

	// Returns internal SyncServerSocketConnection object, see note below
	XnStatus GetControlConnection(ISyncIOConnection*& pStream);

	// Creates a NEW SyncServerSocketConnection
	XnStatus CreateOutputDataConnection(int nID, IOutputConnection*& pConn);
	
	// Creates a NEW SyncServerSocketConnection
	XnStatus CreateInputDataConnection(IAsyncInputConnection*& pStream);



protected:
	XN_SOCKET_HANDLE m_hListenControlSocket;
	XN_SOCKET_HANDLE m_hListenDataInSocket;
	XN_SOCKET_HANDLE m_arrhListenDataOutSockets[MAX_DATAOUT_SOCKETS];

	XnUInt16 m_nDataOutSockets;	
	
	XnUInt16 m_nMaxControlPacketSize;
	XnUInt16 m_nMaxDataOutPacketSize;
	XnUInt16 m_nMaxDataInPacketSize;

	
	// The IConnectionFactory GetControlConnection function returns internal connections (not created with new),
	// To use the same API for the server and the client, the server code for IConnectionFactory::GetControlConnection should reurn internal connection too.
	// So here we use a connection pool
	struct PoolControlConnection
	{
		XnBool						fActive;
		SyncServerSocketConnection	syncConnection;
	} m_controlConnPool[MAX_SERVER_CONTROL_CONNECTIONS];
	
};


}

#endif // XNSYNCSERVERSOCKETCONNECTION_H
