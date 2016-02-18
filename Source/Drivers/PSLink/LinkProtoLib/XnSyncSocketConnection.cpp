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
#include "XnSyncSocketConnection.h"
#include "XnLinkProto.h"
#include "XnLinkProtoUtils.h"
#include <XnLog.h>
#include <XnOS.h>

#define XN_MASK_SYNC_SOCKET "xnSyncSocket"

namespace xn
{

/*TODO: Check if CONNECT_TIMEOUT is enough for actual remote host... We probably 
  need to increase it, but that means enumeration will take longer. */

// Removed const to allow changes for the server
XnUInt32 SyncSocketConnection::CONNECT_TIMEOUT = XN_SOCKET_DEFAULT_TIMEOUT;
XnUInt32 SyncSocketConnection::RECEIVE_TIMEOUT = 35000;

//TEMP TEMP TEMP
//const XnUInt32 SyncSocketConnection::CONNECT_TIMEOUT = XN_WAIT_INFINITE;
//const XnUInt32 SyncSocketConnection::RECEIVE_TIMEOUT = XN_WAIT_INFINITE;
//TEMP TEMP TEMP

SyncSocketConnection::SyncSocketConnection()
{
	m_hSocket = NULL;
	xnOSMemSet(m_strIP, 0, sizeof(m_strIP));
	m_nPort = 0;
	m_nMaxPacketSize = 0;
	m_bInitialized = FALSE;
}

SyncSocketConnection::SyncSocketConnection(const SyncSocketConnection& other)
{
	*this = other;
}

SyncSocketConnection::~SyncSocketConnection()
{
	Shutdown();
}

SyncSocketConnection& SyncSocketConnection::operator=(const SyncSocketConnection& other)
{
	xnOSStrCopy(m_strIP, other.m_strIP, sizeof(m_strIP));
	m_nPort = other.m_nPort;
	m_hSocket = NULL; //We DON'T take the socket from the other connection
	m_nMaxPacketSize = other.m_nMaxPacketSize;
	return *this;
}

XnStatus SyncSocketConnection::Init(const XnChar* strIP, XnUInt16 nPort, XnUInt16 nMaxPacketSize)
{
	Disconnect();
	XnStatus nRetVal = xnOSStrCopy(m_strIP, strIP, sizeof(m_strIP));
	XN_IS_STATUS_OK_LOG_ERROR("Copy IP", nRetVal);
	m_nPort = nPort;
	m_nMaxPacketSize = nMaxPacketSize;
	m_bInitialized = TRUE;

	return XN_STATUS_OK;
}

void SyncSocketConnection::Shutdown()
{
	Disconnect();
	m_bInitialized = FALSE;
}


XnBool SyncSocketConnection::IsInitialized() const
{
	return m_bInitialized;
}

XnStatus SyncSocketConnection::Connect()
{
	if (IsConnected())
	{
		return XN_STATUS_OK;
	}

	XnStatus nRetVal = xnOSCreateSocket(XN_OS_TCP_SOCKET, m_strIP, m_nPort, &m_hSocket);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SYNC_SOCKET, "Failed to create socket %s:%u: %s", m_strIP, m_nPort, xnGetStatusString(nRetVal));
		m_hSocket = NULL;
		return nRetVal;
	}
	nRetVal = xnOSConnectSocket(m_hSocket, CONNECT_TIMEOUT);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SYNC_SOCKET, "Failed to connect socket %s:%u: %s", m_strIP, m_nPort, xnGetStatusString(nRetVal));
		xnOSCloseSocket(m_hSocket);
		m_hSocket = NULL;
		return nRetVal;
	}

	return XN_STATUS_OK;
}

void SyncSocketConnection::Disconnect()
{
	if (m_hSocket != NULL)
	{
		xnOSCloseSocket(m_hSocket);
		m_hSocket = NULL;
	}
}

XnBool SyncSocketConnection::IsConnected() const
{
	return (m_hSocket != NULL);
}

const XnChar* SyncSocketConnection::GetIP() const
{
	return m_strIP;
}

XnUInt16 SyncSocketConnection::GetPort() const
{
	return m_nPort;
}

XnStatus SyncSocketConnection::Receive(void* pData, XnUInt32& nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	LinkPacketHeader* pLinkPacketHeader = reinterpret_cast<LinkPacketHeader*>(pData);
	XnUInt32 nMaxSize = nSize;
	XnUInt32 nTotalBytesReceived = sizeof(LinkPacketHeader);

	nRetVal = xnOSReceiveNetworkBuffer(m_hSocket, (XnChar*)pData, &nTotalBytesReceived, RECEIVE_TIMEOUT);
	if (nRetVal == XN_STATUS_OS_NETWORK_TIMEOUT) // Do not use XN_IS_STATUS_OK_LOG_ERROR for timeout
	{
		return nRetVal;
	}
	//XN_IS_STATUS_OK_LOG_ERROR("Receive network buffer", nRetVal);
	XN_IS_STATUS_OK(nRetVal);
	
	if (nTotalBytesReceived < sizeof(LinkPacketHeader))
	{
		xnLogError(XN_MASK_SYNC_SOCKET, "Partial link packet header received :(");
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	XnUInt32 nBytesToRead = pLinkPacketHeader->GetSize();
	if (nBytesToRead > nMaxSize)
	{
		xnLogError(XN_MASK_SYNC_SOCKET, "Specified buffer of size %u is not large enough to hold received packet of size %u", nMaxSize, nBytesToRead);
		XN_ASSERT(FALSE);
		return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
	}

	XnUInt32 nIterationBytesReceived = 0;
	while (nTotalBytesReceived < nBytesToRead)
	{
		nIterationBytesReceived = (nBytesToRead - nTotalBytesReceived);
		nRetVal = xnOSReceiveNetworkBuffer(m_hSocket, 
		                                   ((XnChar*)pData) + nTotalBytesReceived, 
										   &nIterationBytesReceived, 
										   RECEIVE_TIMEOUT);
		XN_IS_STATUS_OK_LOG_ERROR("Receive network buffer", nRetVal);
		nTotalBytesReceived += nIterationBytesReceived;
	}

	nSize = nTotalBytesReceived;

	return XN_STATUS_OK;
}

XnStatus SyncSocketConnection::Send(const void* pData, XnUInt32 nSize)
{
	if (nSize > 0)
	{
		XnStatus nRetVal = xnOSSendNetworkBuffer(m_hSocket, reinterpret_cast<const XnChar*>(pData), nSize);
		XN_IS_STATUS_OK(nRetVal);
	}
	return XN_STATUS_OK;
}

XnUInt16 SyncSocketConnection::GetMaxPacketSize() const
{
	return m_nMaxPacketSize;
}

}
