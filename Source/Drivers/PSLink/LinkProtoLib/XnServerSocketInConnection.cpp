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
#include "XnServerSocketInConnection.h"
#include <XnLog.h>

#define XN_MASK_SOCKETS "xnSockets"

namespace xn
{

ServerSocketInConnection::ServerSocketInConnection()
{

}

ServerSocketInConnection::~ServerSocketInConnection()
{
	Shutdown();
}

XnStatus ServerSocketInConnection::ConnectSocket(XN_SOCKET_HANDLE& hSocket, const XnChar* strIP, XnUInt16 nPort)
{
	XnStatus nRetVal = XN_STATUS_OK;
	(void)strIP; //Ignore IP parameter - accept connections from any IP.

	XN_SOCKET_HANDLE hListenSocket = NULL;
	nRetVal = xnOSCreateSocket(XN_OS_TCP_SOCKET, "0.0.0.0", nPort, &hListenSocket);
	XN_IS_STATUS_OK_LOG_ERROR("Create data listen socket", nRetVal);
	nRetVal = xnOSBindSocket(hListenSocket);
	if (nRetVal != XN_STATUS_OK)
	{
		xnOSCloseSocket(hListenSocket);
		XN_IS_STATUS_OK_LOG_ERROR("Bind data listen socket", nRetVal);
	}

	nRetVal = xnOSListenSocket(hListenSocket);
	if (nRetVal != XN_STATUS_OK)
	{
		xnOSCloseSocket(hListenSocket);
		XN_IS_STATUS_OK_LOG_ERROR("Listen to data socket", nRetVal);
	}
	xnLogVerbose(XN_MASK_SOCKETS, "Server accepting %s:%u...", strIP, nPort);
	nRetVal = xnOSAcceptSocket(hListenSocket, &hSocket, XN_WAIT_INFINITE);
	xnOSCloseSocket(hListenSocket);
	XN_IS_STATUS_OK_LOG_ERROR("Accept data socket", nRetVal);
	xnLogVerbose(XN_MASK_SOCKETS, "Server accepted connection on port %u", nPort);

	return XN_STATUS_OK;
}

}
