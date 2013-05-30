#ifndef __XNLINKOUTPUTDATAENDPOINT_H__
#define __XNLINKOUTPUTDATAENDPOINT_H__

#include <XnPlatform.h>
#include <XnStatus.h>

namespace xn
{

class LinkOutputStreamsMgr;
class IConnectionFactory;
class IOutputConnection;

class LinkOutputDataEndpoint 
{
public:
	LinkOutputDataEndpoint();
	virtual ~LinkOutputDataEndpoint();

	XnStatus Init(XnUInt16 nEndpointID, 
	              IConnectionFactory* pConnectionFactory);
	XnBool IsInitialized() const;
	void Shutdown();
	XnStatus Connect();
	void Disconnect();
	XnBool IsConnected() const;
	XnUInt16 GetMaxPacketSize() const;

	XnStatus SendData(const void* pData, XnUInt32 nSize);

private:
	IOutputConnection* m_pConnection;
	XnBool m_bInitialized;
	XnBool m_bConnected;
	XnUInt16 m_nEndpointID;
};

}

#endif // __XNLINKOUTPUTDATAENDPOINT_H__
