#ifndef __SYNCSOCKETCONNECTION_H__
#define __SYNCSOCKETCONNECTION_H__

#include "ISyncIOConnection.h"
#include "XnLinkProtoLibDefs.h"

struct xnOSSocket;
typedef struct xnOSSocket* XN_SOCKET_HANDLE;

namespace xn
{

class SyncSocketConnection : virtual public ISyncIOConnection
{
public:
	SyncSocketConnection();
	SyncSocketConnection(const SyncSocketConnection& other);
	virtual ~SyncSocketConnection();
	SyncSocketConnection& operator=(const SyncSocketConnection& other);

// Operations
	virtual XnStatus Init(const XnChar* strIP, XnUInt16 nPort, XnUInt16 nMaxPacketSize);
	virtual void Shutdown();
	XnBool IsInitialized() const;

	virtual XnBool IsConnected() const;
	const XnChar* GetIP() const;
	XnUInt16 GetPort() const;
	
// ISyncIOConnection implementation
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnStatus Receive(void* pData, XnUInt32& nSize);
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);
	virtual XnUInt16 GetMaxPacketSize() const;

	static XnUInt32 CONNECT_TIMEOUT;
	static XnUInt32 RECEIVE_TIMEOUT;
	
protected:
	XnBool m_bInitialized;
	XnConnectionString m_strIP;
	XnUInt16 m_nPort;
    XnUInt16 m_nMaxPacketSize;
	XN_SOCKET_HANDLE m_hSocket;
};

}

#endif // __SYNCSOCKETCONNECTION_H__