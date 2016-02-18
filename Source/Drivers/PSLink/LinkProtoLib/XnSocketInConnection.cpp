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
#include "XnSocketInConnection.h"
#include "XnLinkProto.h"
#include "XnLinkProtoUtils.h"
#include "XnLinkDefs.h"
#include <XnOS.h>
#include <XnLog.h>

namespace xn
{

//const XnUInt32 SocketInConnection::BUFFER_NUM_PACKETS = 8*12;
const XnUInt32 SocketInConnection::BUFFER_NUM_PACKETS = 1;
const XnUInt32 SocketInConnection::RECEIVE_TIMEOUT = 50;

const XnUInt32 SocketInConnection::CONNECT_TIMEOUT = 10000;
const XnUInt32 SocketInConnection::READ_THREAD_TERMINATE_TIMEOUT = 10000;

//TEMP TEMP TEMP
//const XnUInt32 SocketInConnection::CONNECT_TIMEOUT = XN_WAIT_INFINITE;
//const XnUInt32 SocketInConnection::READ_THREAD_TERMINATE_TIMEOUT = XN_WAIT_INFINITE;
//TEMP TEMP TEMP

SocketInConnection::SocketInConnection()
{
	xnOSMemSet(m_strIP, 0, sizeof(m_strIP));
	m_nPort = 0;
	m_hReadThread = NULL;
	m_hConnectEvent = NULL;
	m_bStopReadThread = FALSE;
	m_pDataDestination = NULL;
	m_nBufferSize = 0;
	m_nMaxPacketSize = 0;
	m_nConnectionStatus = XN_STATUS_OS_NETWORK_CONNECTION_CLOSED;
	m_pBuffer = NULL;
}

SocketInConnection::~SocketInConnection()
{
	Shutdown();
}

XnStatus SocketInConnection::Init(const XnChar* strIP, XnUInt16 nPort, XnUInt16 nMaxPacketSize)
{
	XN_VALIDATE_INPUT_PTR(strIP);
	XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = xnOSStrCopy(m_strIP, strIP, sizeof(m_strIP));
	XN_IS_STATUS_OK_LOG_ERROR("Copy IP", nRetVal);
	m_nPort = nPort;
	m_nMaxPacketSize = nMaxPacketSize;
	m_nBufferSize = m_nMaxPacketSize * BUFFER_NUM_PACKETS;
	m_pBuffer = reinterpret_cast<XnUInt8*>(xnOSMallocAligned(m_nBufferSize, XN_DEFAULT_MEM_ALIGN));
	XN_VALIDATE_ALLOC_PTR(m_pBuffer);
	nRetVal = xnOSCreateEvent(&m_hConnectEvent, FALSE);
	XN_IS_STATUS_OK_LOG_ERROR("Create event", nRetVal);
	xnLogVerbose(XN_MASK_LINK, "Event created for socket %u", m_nPort);
	return XN_STATUS_OK;
}

void SocketInConnection::Shutdown()
{
	xnLogVerbose(XN_MASK_LINK, "Socket in connection %u shutting down", m_nPort);
	Disconnect();
	xnOSFreeAligned(m_pBuffer);
	m_pBuffer = NULL;
	xnOSCloseEvent(&m_hConnectEvent);
}

XnStatus SocketInConnection::Connect()
{
	XnStatus nRetVal = XN_STATUS_OK;
	Disconnect(); // In case we're already connected
	nRetVal = xnOSCreateThread(&ReadThreadProc, this, &m_hReadThread);
	XN_IS_STATUS_OK_LOG_ERROR("Create input socket read thread", nRetVal);
	xnLogVerbose(XN_MASK_LINK, "Waiting for connection on socket %u...", m_nPort);
	nRetVal = xnOSWaitEvent(m_hConnectEvent, CONNECT_TIMEOUT);
	XN_IS_STATUS_OK_LOG_ERROR("Wait for input socket to connect", nRetVal);
	if (m_nConnectionStatus != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_LINK, "Failed to connect to socket %u: %s", m_nPort, xnGetStatusString(m_nConnectionStatus));
		XN_ASSERT(FALSE);
		return m_nConnectionStatus;
	}
	xnLogVerbose(XN_MASK_LINK, "Socket %u connected.", m_nPort);
	nRetVal = xnOSSetThreadPriority(m_hReadThread, XN_PRIORITY_CRITICAL);
	XN_IS_STATUS_OK_LOG_ERROR("Set read thread priority", nRetVal);
	return XN_STATUS_OK;
}

XnBool SocketInConnection::IsConnected() const
{
	return (m_nConnectionStatus == XN_STATUS_OK);
}

void SocketInConnection::Disconnect()
{
	XnStatus nRetVal = XN_STATUS_OK;
	if (m_hReadThread != NULL)
	{
		m_bStopReadThread = TRUE; //Signal read thread to stop running
		nRetVal = xnOSWaitAndTerminateThread(&m_hReadThread, READ_THREAD_TERMINATE_TIMEOUT);
		if (nRetVal != XN_STATUS_OK)
		{
			xnLogWarning("Failed to terminate input socket read thread: %s", xnGetStatusString(nRetVal));
			XN_ASSERT(FALSE);
		}
		m_bStopReadThread = FALSE;
	}
}


XnUInt16 SocketInConnection::GetMaxPacketSize() const
{
	return m_nMaxPacketSize;
}

XnStatus SocketInConnection::SetDataDestination(IDataDestination* pDataDestination)
{
	m_pDataDestination = pDataDestination;
	return XN_STATUS_OK;
}

XN_THREAD_PROC SocketInConnection::ReadThreadProc(XN_THREAD_PARAM pThreadParam)
{
	SocketInConnection* pThis = reinterpret_cast<SocketInConnection*>(pThreadParam);
	if (pThis == NULL)
	{
		xnLogError(XN_MASK_LINK, "Got NULL in socket read thread param :(");
		XN_ASSERT(FALSE);
		return NULL;
	}
	
	pThis->ReadThreadProcImpl();
	
	XN_THREAD_PROC_RETURN(0);
}

XnStatus SocketInConnection::ReadThreadProcImpl()
{
	XnStatus nRetVal = XN_STATUS_OK;
	XN_SOCKET_HANDLE hSocket = NULL;
	XnBool bCanceled = FALSE;
	XnUInt32 nPacketBytesRead = 0;
	XnUInt32 nTotalBytesRead = 0;

	m_nConnectionStatus = ConnectSocket(hSocket, m_strIP, m_nPort);
	XN_IS_STATUS_OK_LOG_ERROR("Connect socket", m_nConnectionStatus);
	nRetVal = xnOSSetEvent(m_hConnectEvent);
	XN_IS_STATUS_OK_LOG_ERROR("Set connect event", nRetVal);

	while (!m_bStopReadThread)
	{
		//Fill buffer with received packets
		nTotalBytesRead = 0;
		for (XnUInt32 nPacket = 0; (nPacket < BUFFER_NUM_PACKETS); nPacket++)
		{
			nPacketBytesRead = m_nMaxPacketSize;
			m_nConnectionStatus = ReceivePacket(hSocket, m_pBuffer + nTotalBytesRead, nPacketBytesRead, bCanceled);
			if (m_nConnectionStatus != XN_STATUS_OK)
			{
				m_pDataDestination->HandleDisconnection();				
				xnLogError(XN_MASK_LINK, "Failed to receive packet: %s", xnGetStatusString(m_nConnectionStatus));
				//XN_ASSERT(FALSE);
				return m_nConnectionStatus;
			}

			if (bCanceled)
			{
				//Ignore packet and exit loop
				break;
			}

			if (nTotalBytesRead == m_nBufferSize)
			{
				xnLogError(XN_MASK_LINK, "Read thread buffer overflowed :(");
				XN_ASSERT(FALSE);
				return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
			}

			nTotalBytesRead += nPacketBytesRead;
		}

		if (m_pDataDestination != NULL)	
		{
			//Send data in buffer to its destination.
			//Even if at this point the read thread should be stopped, first we send all the complete packets we got.
			if (nTotalBytesRead > 0)
			{
				m_pDataDestination->IncomingData(m_pBuffer, nTotalBytesRead);
			}
		}
	}

	nRetVal = xnOSCloseSocket(hSocket);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogWarning(XN_MASK_LINK, "Failed to close input data socket :(");
		XN_ASSERT(FALSE);
	}
	m_nConnectionStatus = XN_STATUS_OS_NETWORK_CONNECTION_CLOSED;

	return XN_STATUS_OK;	
}

XnStatus SocketInConnection::ReceivePacket(XN_SOCKET_HANDLE hSocket, void* pDestBuffer, XnUInt32& nSize, XnBool& bCanceled)
{
	XnStatus nRetVal = XN_STATUS_OK;
	LinkPacketHeader* pPacket = reinterpret_cast<LinkPacketHeader*>(pDestBuffer);

	XN_ASSERT(nSize >= sizeof(LinkPacketHeader));
	/* We first receive the packet's header to know its size, and then receive exactly as many bytes as needed.
	   If we just received max packet size, we might overrun a smaller packet and receive part of the next packet.
	   (We don't have this problem with USB cuz we always get a whole packet there).*/

	nRetVal = ReceiveExactly(hSocket, pPacket, sizeof(LinkPacketHeader), bCanceled);
	if (bCanceled)
	{
		//The request to receive a packet was canceled
		return XN_STATUS_OK;
	}
	//XN_IS_STATUS_OK_LOG_ERROR("Receive packet header", nRetVal);
	XN_IS_STATUS_OK(nRetVal);
	
	if (!pPacket->IsMagicValid())
	{
		xnLogError(XN_MASK_LINK, "Got bad link packet header magic :(");
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}
	XnUInt16 nPacketSize = pPacket->GetSize();
	if (nSize < nPacketSize)
	{
		xnLogError(XN_MASK_LINK, "Insufficient buffer (%u bytes) to hold packet of %u bytes", nSize, nPacketSize);
		XN_ASSERT(FALSE);
		return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
	}
	nSize = 0; //In case we get canceled
	nRetVal = ReceiveExactly(hSocket, pPacket->GetPacketData(), nPacketSize - sizeof(LinkPacketHeader), bCanceled);
	XN_IS_STATUS_OK_LOG_ERROR("Receive packet body", nRetVal);
	if (bCanceled)
	{
		//The request to receive a packet was canceled
		return XN_STATUS_OK;
	}
	nSize = nPacketSize;
	
	return XN_STATUS_OK;	
}

XnStatus SocketInConnection::ReceiveExactly(XN_SOCKET_HANDLE hSocket, void* pDestBuffer, XnUInt32 nSize, XnBool& bCanceled)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 nTotalBytesReceived = 0;
	XnUInt32 nIterationBytesReceived = 0;
	bCanceled = FALSE;
	while ((nTotalBytesReceived < nSize) && (!m_bStopReadThread))
	{
		nIterationBytesReceived = (nSize - nTotalBytesReceived);
		nRetVal = xnOSReceiveNetworkBuffer(hSocket, ((XnChar*)pDestBuffer) + nTotalBytesReceived, &nIterationBytesReceived, RECEIVE_TIMEOUT);
		if (nRetVal == XN_STATUS_OS_NETWORK_TIMEOUT)
		{
			//No data, no problem
			continue;
		}
		/*else if (nRetVal == XN_STATUS_OS_NETWORK_CONNECTION_CLOSED)
		{
			//This is ok - same as cancel
			break;
		}*/
		XN_IS_STATUS_OK(nRetVal);
		nTotalBytesReceived += nIterationBytesReceived;
	}

	if (nTotalBytesReceived < nSize)
	{
		//We didn't get all the data we expected - we were canceled.
		bCanceled = TRUE;
	}
	
	return nRetVal;
}

}
