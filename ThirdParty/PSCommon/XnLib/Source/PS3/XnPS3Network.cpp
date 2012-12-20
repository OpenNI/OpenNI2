/*****************************************************************************
*                                                                            *
*  PrimeSense PSCommon Library                                               *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PSCommon.                                            *
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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#undef XN_CROSS_PLATFORM
#include <XnOS.h>

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
XN_BOOL g_XnOSNetworkWasInit = FALSE;

//---------------------------------------------------------------------------
// Structs
//---------------------------------------------------------------------------
/** The Xiron OS network socket structure. */ 
typedef struct XnOSSocket
{
} XnOSSocket;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XN_CORE_API XnStatus XnOSInitNetwork()
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSShutdownNetwork()
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);}

XN_CORE_API XnStatus XnOSCreateSocket(const XnOSSocketType SocketType, const XN_CHAR* cpIPAddress, const XN_UINT16 nPort, XN_SOCKET_HANDLE* SocketPtr)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSCloseSocket(XN_SOCKET_HANDLE Socket)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSBindSocket(XN_SOCKET_HANDLE Socket)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSListenSocket(XN_SOCKET_HANDLE Socket)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSAcceptSocket(XN_SOCKET_HANDLE ListenSocket, XN_SOCKET_HANDLE* AcceptSocketPtr)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSConnectSocket(XN_SOCKET_HANDLE Socket)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSSetSocketBufferSize(XN_SOCKET_HANDLE Socket, const XN_UINT32 nSocketBufferSize)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSSendNetworkBuffer(XN_SOCKET_HANDLE Socket, const XN_CHAR* cpBuffer, const XN_UINT32 nBufferSize)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSSendToNetworkBuffer(XN_SOCKET_HANDLE Socket, const XN_CHAR* cpBuffer, const XN_UINT32 nBufferSize, XN_SOCKET_HANDLE SocketTo)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSReceiveNetworkBuffer(XN_SOCKET_HANDLE Socket, XN_CHAR* cpBuffer, XN_UINT32* pnBufferSize, XN_UINT32 nMicroSecondTimeout)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSReceiveFromNetworkBuffer(XN_SOCKET_HANDLE Socket, XN_CHAR* cpBuffer, XN_UINT32* pnBufferSize, XN_SOCKET_HANDLE* SocketFrom)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}
