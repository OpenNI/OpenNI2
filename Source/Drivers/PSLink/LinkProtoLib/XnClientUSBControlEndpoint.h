#ifndef __XNUSBCONTROLENDPOINT_H__
#define __XNUSBCONTROLENDPOINT_H__

#include "ISyncIOConnection.h"

struct XnUSBDeviceHandle;
typedef XnUSBDeviceHandle*  XN_USB_DEV_HANDLE;

namespace xn
{

class ClientUSBControlEndpoint : virtual public ISyncIOConnection
{
public:
	ClientUSBControlEndpoint(XnUInt32 nPreControlReceiveSleep);
	virtual ~ClientUSBControlEndpoint();
	// Operations
	XnStatus Init(XN_USB_DEV_HANDLE hUSBDevice);
	void Shutdown();


	// ISyncIOConnection implementation
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnUInt16 GetMaxPacketSize() const;
	
	//nSize is max size on input, actual size on output
	virtual XnStatus Receive(void* pData, XnUInt32& nSize);
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);

private:
	//Low level usb packet size - not to be confused with our "logical" packet size which is bigger
	static const XnUInt32 USB_LOW_LEVEL_MAX_PACKET_SIZE; 
	
	static const XnUInt32 SEND_TIMEOUT;
	static const XnUInt32 RECEIVE_TIMEOUT;

	XN_USB_DEV_HANDLE m_hUSBDevice;
	XnUInt32 m_nPreControlReceiveSleep;
};

}

#endif // __XNUSBCONTROLENDPOINT_H__