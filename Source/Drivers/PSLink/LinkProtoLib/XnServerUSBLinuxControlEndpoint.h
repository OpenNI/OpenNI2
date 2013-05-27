#include "ISyncIOConnection.h"
#include <XnOS.h>

struct XnUSBDevice;

namespace xn
{

class ServerUSBLinuxControlEndpoint : virtual public ISyncIOConnection
{
public:
	ServerUSBLinuxControlEndpoint();
	virtual ~ServerUSBLinuxControlEndpoint();

	virtual XnStatus Init(XnUSBDevice* pUSBDevice, XnUInt16 nMaxPacketSize);
	virtual void Shutdown();

	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnUInt16 GetMaxPacketSize() const;

	virtual XnStatus Receive(void* pData, XnUInt32& nSize);
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);

private:
	static const XnUInt32 RECEIVE_TIMEOUT;

	static void OnControlRequest(XnUSBDevice* pDevice, void* pCookie);

	XnUSBDevice* m_pUSBDevice;
	XN_EVENT_HANDLE m_hControlEvent;
	XnUInt16 m_nMaxPacketSize;
	XnBool m_bConnected;
};

}