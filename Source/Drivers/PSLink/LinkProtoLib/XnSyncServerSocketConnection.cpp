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
// XnSyncServerSocketConnection.cpp


#include "XnSyncServerSocketConnection.h"

namespace xn
{

SyncServerSocketListener::SyncServerSocketListener()
{
	// Nullify all sockets
	m_hListenControlSocket = NULL;
	m_hListenDataInSocket = NULL;
	m_nDataOutSockets = 0;	
	m_nMaxControlPacketSize = 0;
	m_nMaxDataOutPacketSize = 0;
	m_nMaxDataInPacketSize = 0;

	for (XnUInt32 nDataOutSocket = 0; nDataOutSocket < MAX_DATAOUT_SOCKETS; nDataOutSocket++)
	{
		m_arrhListenDataOutSockets[nDataOutSocket] = NULL;
	}
	
	// Init control connection pool
	for (XnUInt32 nConn = 0; nConn < MAX_SERVER_CONTROL_CONNECTIONS; nConn++)
	{
		m_controlConnPool[nConn].fActive = false;
	}	
}

SyncServerSocketListener::~SyncServerSocketListener()
{
	if (m_hListenControlSocket || m_hListenDataInSocket)
	{
		Shutdown();
	}
}

XnStatus	SyncServerSocketListener::Init(const XnChar* strIP, 
										   XnUInt16 nControlPort,			
										   XnUInt16 nInputPort,
										   XnUInt16 nFirstDataPort,	
										   XnUInt16 nDataOutSockets,
										   XnUInt16 nMaxColntrolPacketSize,
										   XnUInt16 nMaxDataOutPacketSize,
										   XnUInt16 nMaxDataInPacketSize)
{
	XnStatus rc = XN_STATUS_OK;

	XN_ASSERT(!m_hListenControlSocket);
	XN_ASSERT(!m_hListenDataInSocket);

	XN_ASSERT(nDataOutSockets < MAX_DATAOUT_SOCKETS);

	m_nMaxControlPacketSize = nMaxColntrolPacketSize;
	m_nMaxDataOutPacketSize	= nMaxDataOutPacketSize;
	m_nMaxDataInPacketSize	= nMaxDataInPacketSize;
	m_nDataOutSockets		= nDataOutSockets;

	const XnUInt16 NON_DATA_OUT_CONNECTIONS = 2; 
	XnUInt16 nSocketsToCreate = NON_DATA_OUT_CONNECTIONS + nDataOutSockets; // Control + + All data outs
	
	for (XnUInt16 nSockToCreate = 0; nSockToCreate < nSocketsToCreate; nSockToCreate++)
	{
		XN_SOCKET_HANDLE newSocket = NULL;
		XnUInt16 nCurrSocketPort =	nSockToCreate == 0 ? nControlPort :			// First create control socket
									nSockToCreate == 1 ? nInputPort :			// Input port
									nFirstDataPort + (nSockToCreate - NON_DATA_OUT_CONNECTIONS);		// Output ports
				
		// Create listen socket for control connections:
		rc = xnOSCreateSocket(XN_OS_TCP_SOCKET, strIP, nCurrSocketPort, &newSocket);
		if (rc != XN_STATUS_OK)
		{
			Shutdown();break;
		}	
		rc = xnOSBindSocket(newSocket);
		if (rc != XN_STATUS_OK)
		{
			Shutdown();break;
		}
		rc = xnOSListenSocket(newSocket);
		if (rc != XN_STATUS_OK)
		{
			Shutdown();break;
		}	

		// Store the socket
		if (nSockToCreate == 0)			m_hListenControlSocket	= newSocket;
		else if (nSockToCreate == 1)	m_hListenDataInSocket	= newSocket;
		else							
		{
			XnUInt32 nOutSocketIndex = nSockToCreate - NON_DATA_OUT_CONNECTIONS;
			XN_ASSERT(nOutSocketIndex < MAX_DATAOUT_SOCKETS);
			m_arrhListenDataOutSockets[nOutSocketIndex] = newSocket;
		}
	}

	// Set the timeout for the control connection to something low enough to allow many clients on one thread
	SyncSocketConnection::RECEIVE_TIMEOUT = 50;

	return rc;
}

void		SyncServerSocketListener::Shutdown()
{
	if (m_hListenControlSocket)
	{
		xnOSCloseSocket(m_hListenControlSocket);
		m_hListenControlSocket = NULL;
	}
	if (m_hListenDataInSocket)
	{
		xnOSCloseSocket(m_hListenDataInSocket);
		m_hListenDataInSocket = NULL;
	}
	for (XnUInt32 nOutConn = 0; nOutConn < MAX_SERVER_CONTROL_CONNECTIONS; nOutConn++)
	{
		if (m_arrhListenDataOutSockets[nOutConn])
		{
			xnOSCloseSocket(m_arrhListenDataOutSockets[nOutConn]);
			m_arrhListenDataOutSockets[nOutConn] = NULL;
		}
	}	
}

// Returns SyncServerSocketConnection
XnStatus SyncServerSocketListener::GetControlConnection(ISyncIOConnection*& pStream)
{
	XN_SOCKET_HANDLE hClientSocket = NULL;

	// Find free connection and destroy terminated connections
	const XnUInt32 FREECONN_NA = (XnUInt32)-1;
	XnUInt32 nFreeConn = FREECONN_NA;

	for (XnUInt32 nConn = 0; nConn < MAX_SERVER_CONTROL_CONNECTIONS; nConn++)
	{
		PoolControlConnection& CurrConn = m_controlConnPool[nConn];

		// Deactivate disconnected connections
		if (CurrConn.fActive)
		{
			if (!CurrConn.syncConnection.IsConnected())
			{
				CurrConn.fActive = false;
			}
		}
		
		// See if we can use this connection
		if (!CurrConn.fActive && nFreeConn == FREECONN_NA)
		{
			nFreeConn = nConn;
		}
	}

	// See if we got inactive connection from the pool
	if (nFreeConn == FREECONN_NA)
	{
		return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
	}

	// Accept the connection (blocking)
	XnStatus rc = xnOSAcceptSocket(m_hListenControlSocket, &hClientSocket, XN_WAIT_INFINITE);
	if (rc == XN_STATUS_OK)
	{
		XN_ASSERT(m_hListenControlSocket);

		PoolControlConnection& FreeConn = m_controlConnPool[nFreeConn];
		FreeConn.syncConnection.m_hSocket = hClientSocket;
		FreeConn.syncConnection.m_nMaxPacketSize = m_nMaxControlPacketSize;

		FreeConn.fActive = true;
		pStream = &FreeConn.syncConnection;
	}

	return rc;
	
}

// Creates a NEW SyncServerSocketConnection
XnStatus SyncServerSocketListener::CreateOutputDataConnection(int nID, IOutputConnection*& pConn)
{
	XN_SOCKET_HANDLE hClientSocket = NULL;

	XN_ASSERT(nID < MAX_DATAOUT_SOCKETS);
	XN_ASSERT(m_arrhListenDataOutSockets[nID]);

	// Accept the connection (blocking)
	XnStatus rc = xnOSAcceptSocket(m_arrhListenDataOutSockets[nID], &hClientSocket, XN_WAIT_INFINITE);
	if (rc == XN_STATUS_OK)
	{
		XN_ASSERT(hClientSocket);

		SyncServerSocketConnection* pSocket = XN_NEW(SyncServerSocketConnection);
		pSocket->m_hSocket = hClientSocket;
		pSocket->m_nMaxPacketSize = m_nMaxDataOutPacketSize;
		pConn = pSocket;
	}

	return rc;
}

// Creates a NEW SyncServerSocketConnection
XnStatus SyncServerSocketListener::CreateInputDataConnection(IAsyncInputConnection*& pStream)
{
	XN_SOCKET_HANDLE hClientSocket = NULL;

	XN_ASSERT(m_hListenDataInSocket);
	
	// Accept the connection (blocking)
	XnStatus rc = xnOSAcceptSocket(m_hListenDataInSocket, &hClientSocket, XN_WAIT_INFINITE);
	if (rc == XN_STATUS_OK)
	{
		XN_ASSERT(hClientSocket);

		ASyncServerSocketConnection* pSocket = XN_NEW(ASyncServerSocketConnection);
		pSocket->m_hSocketFromListener = hClientSocket;

		rc = pSocket->Init("", 0, m_nMaxDataInPacketSize);
		if (rc != XN_STATUS_OK)
		{
			XN_DELETE(pSocket);
			xnOSCloseSocket(hClientSocket);
		}

		pStream = pSocket;
	}

	return rc;
}



}