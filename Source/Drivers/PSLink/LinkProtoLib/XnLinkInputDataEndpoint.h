#ifndef __XNLINKINPUTDATAENDPOINT_H__
#define __XNLINKINPUTDATAENDPOINT_H__

#include "IAsyncInputConnection.h"
#include "IConnection.h"
#include <XnOSCpp.h>

struct XnDumpFile;

namespace xn
{

class IConnectionFactory;
class LinkInputStreamsMgr;

class ILinkDataEndpointNotifications
{
public:
	virtual ~ILinkDataEndpointNotifications() {}
	virtual void HandleLinkDataEndpointDisconnection(XnUInt16 nEndpointID) = 0;
};

class LinkInputDataEndpoint : public IDataDestination, public IConnection
{
public:
	LinkInputDataEndpoint();
	virtual ~LinkInputDataEndpoint();
	XnStatus Init(XnUInt16 nEndpointID, 
				  IConnectionFactory* pConnectionFactory,
				  LinkInputStreamsMgr* pLinkInputStreamsMgr,
				  ILinkDataEndpointNotifications* pNotifications);
    XnBool IsInitialized() const;

	void Shutdown();
	XnStatus Connect();
	void Disconnect();
	XnBool IsConnected() const;
	XnUInt16 GetMaxPacketSize() const;

	/* IDataDestination Implementation */
	virtual XnStatus IncomingData(const void* pData, XnUInt32 nSize);
	virtual void HandleDisconnection();

private:
    XnUInt16 m_nEndpointID;
    LinkInputStreamsMgr* m_pLinkInputStreamsMgr;
    ILinkDataEndpointNotifications* m_pNotifications;
	IAsyncInputConnection* m_pConnection;
	IConnectionFactory* m_pConnectionFactory;
	XnBool m_bInitialized;
	volatile XnUInt32 m_nConnected;
    XN_CRITICAL_SECTION_HANDLE m_hCriticalSection;
	XnDumpFile* m_pDumpFile;
};

}

#endif // __XNLINKINPUTDATAENDPOINT_H__
