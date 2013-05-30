
#ifndef _XNSERVERSOCKETINCONNECTION_H_
#define _XNSERVERSOCKETINCONNECTION_H_

#include "XnSocketInConnection.h"


namespace xn
{

class ServerSocketInConnection : public SocketInConnection
{
public:
	ServerSocketInConnection();
	virtual ~ServerSocketInConnection();
protected:
	virtual XnStatus ConnectSocket(XN_SOCKET_HANDLE& hSocket, const XnChar* strIP, XnUInt16 nPort);

};

}


#endif