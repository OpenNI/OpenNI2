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
