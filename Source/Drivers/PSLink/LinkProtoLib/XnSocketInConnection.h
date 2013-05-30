#ifndef __XNSOCKETINCONNECTION_H__
#define __XNSOCKETINCONNECTION_H__

#include "IAsyncInputConnection.h"
#include "XnLinkProtoLibDefs.h"
#include <XnOS.h>

namespace xn
{

class SocketInConnection : public IAsyncInputConnection
{
public:
	SocketInConnection();
	virtual ~SocketInConnection();
	virtual XnStatus Init(const XnChar* strIP, XnUInt16 nPort, XnUInt16 nMaxPacketSize);
	virtual void Shutdown();
	virtual XnStatus Connect();
	virtual XnBool IsConnected() const;
	virtual void Disconnect();
	virtual XnUInt16 GetMaxPacketSize() const;
	//pDataDestination may be NULL and then data is not sent
	virtual XnStatus SetDataDestination(IDataDestination* pDataDestination);

protected:
	static const XnUInt32 BUFFER_NUM_PACKETS;
	static const XnUInt32 RECEIVE_TIMEOUT;
	static const XnUInt32 CONNECT_TIMEOUT;
	static const XnUInt32 READ_THREAD_TERMINATE_TIMEOUT;

	virtual XnStatus ConnectSocket(XN_SOCKET_HANDLE& hSocket, const XnChar* strIP, XnUInt16 nPort) = 0;

protected:
	static XN_THREAD_PROC ReadThreadProc(XN_THREAD_PARAM pThreadParam);
	XnStatus ReadThreadProcImpl();
	//nSize is max size on input, actual size on output
	XnStatus ReceivePacket(XN_SOCKET_HANDLE hSocket, void* pDestBuffer, XnUInt32& nSize, XnBool& bCanceled);
	XnStatus ReceiveExactly(XN_SOCKET_HANDLE hSocket, void* pDestBuffer, XnUInt32 nSize, XnBool& bCanceled);


	XnConnectionString m_strIP;
	XnUInt16 m_nPort;
    XnUInt16 m_nMaxPacketSize;
	XN_THREAD_HANDLE m_hReadThread;
	XN_EVENT_HANDLE m_hConnectEvent; //This event signals we passed the actual connection command (and will be set even if connection failed)
	volatile XnBool m_bStopReadThread; //This is used to signal the read thread to stop
	IDataDestination* m_pDataDestination;
	XnUInt8* m_pBuffer;
	XnUInt32 m_nBufferSize;
	volatile XnStatus m_nConnectionStatus;
};

}

#endif // __XNSOCKETINCONNECTION_H__