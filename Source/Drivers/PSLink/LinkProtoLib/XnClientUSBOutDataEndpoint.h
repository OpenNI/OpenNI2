#ifndef __XNUSBOUTDATAENDPOINT_H__
#define __XNUSBOUTDATAENDPOINT_H__

#include "IOutputConnection.h"
#include <XnUSB.h>

struct XnUSBDeviceHandle;
struct XnUSBEndPointHandle;

typedef XnUSBDeviceHandle*  XN_USB_DEV_HANDLE;
typedef XnUSBEndPointHandle* XN_USB_EP_HANDLE;

namespace xn
{

class ClientUSBOutDataEndpoint : virtual public IOutputConnection
{
public:
	ClientUSBOutDataEndpoint(XnUSBEndPointType endpointType);
	virtual ~ClientUSBOutDataEndpoint();

	virtual XnStatus Init(XN_USB_DEV_HANDLE hUSBDevice);
	virtual void Shutdown();
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);
	virtual XnUInt16 GetMaxPacketSize() const;

private:
	XnUSBEndPointType m_endpointType;
	XN_USB_EP_HANDLE m_hEndpoint;
	XN_USB_DEV_HANDLE m_hUSBDevice;
	static const XnUInt16 ENDPOINT_ID;
	static const XnUInt32 SEND_TIMEOUT;
	
	XnUInt16 m_nMaxPacketSize;
	XnBool m_bConnected;
};

}

#endif // __XNUSBOUTDATAENDPOINT_H__