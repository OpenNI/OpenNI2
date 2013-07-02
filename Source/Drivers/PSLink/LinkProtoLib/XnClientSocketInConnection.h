#ifndef __XNCLIENTSOCKETINCONNECTION_H__
#define __XNCLIENTSOCKETINCONNECTION_H__

#include "XnSocketInConnection.h"

namespace xn
{

class ClientSocketInConnection : public SocketInConnection
{
public:
	ClientSocketInConnection();
	virtual ~ClientSocketInConnection();
protected:
	virtual XnStatus ConnectSocket(XN_SOCKET_HANDLE& hSocket, const XnChar* strIP, XnUInt16 nPort);
};

}

#endif // __XNCLIENTSOCKETINCONNECTION_H__
