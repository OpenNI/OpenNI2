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
