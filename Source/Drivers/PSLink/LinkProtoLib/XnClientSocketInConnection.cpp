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
#include "XnClientSocketInConnection.h"
#include <XnLog.h>

#define XN_MASK_SOCKETS "xnSockets"

namespace xn
{

ClientSocketInConnection::ClientSocketInConnection()
{

}

ClientSocketInConnection::~ClientSocketInConnection()
{
	Shutdown();
}

XnStatus ClientSocketInConnection::ConnectSocket(XN_SOCKET_HANDLE& hSocket, const XnChar* strIP, XnUInt16 nPort)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = xnOSCreateSocket(XN_OS_TCP_SOCKET, strIP, nPort, &hSocket);
	XN_IS_STATUS_OK_LOG_ERROR("Create input socket", nRetVal);

	xnLogVerbose(XN_MASK_SOCKETS, "Client connecting to %s:%u...", strIP, nPort);
	nRetVal = xnOSConnectSocket(hSocket, CONNECT_TIMEOUT);
	XN_IS_STATUS_OK_LOG_ERROR("Connect input socket", nRetVal);
	xnLogVerbose(XN_MASK_SOCKETS, "Client connected to %s:%u", strIP, nPort);

	return XN_STATUS_OK;
}

}
