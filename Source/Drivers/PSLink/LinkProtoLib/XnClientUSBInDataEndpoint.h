#ifndef __XNUSBINDATAENDPOINT_H__
#define __XNUSBINDATAENDPOINT_H__

#include "IAsyncInputConnection.h"
#include <XnUSB.h>

struct XnUSBDeviceHandle;
struct XnUSBEndPointHandle;

typedef XnUSBDeviceHandle*  XN_USB_DEV_HANDLE;
typedef XnUSBEndPointHandle* XN_USB_EP_HANDLE;

namespace xn
{

class ClientUSBInDataEndpoint : virtual public IAsyncInputConnection
{
public:
	ClientUSBInDataEndpoint();

	virtual ~ClientUSBInDataEndpoint();

	virtual XnStatus Init(XN_USB_DEV_HANDLE hUSBDevice, XnUInt16 nEndpointID);
	virtual void Shutdown();
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnUInt16 GetMaxPacketSize() const;
	virtual XnStatus SetDataDestination(IDataDestination* pDataDestination);

	inline XnUSBEndPointType GetEndpointType() const { return m_endpointType; }

private:
	static XnBool XN_CALLBACK_TYPE ReadThreadCallback(XnUChar* pBuffer, XnUInt32 nBufferSize, void* pCallbackData);

	static const XnUInt32 READ_THREAD_BUFFER_NUM_PACKETS_ISO;
	static const XnUInt32 READ_THREAD_NUM_BUFFERS_ISO;
	static const XnUInt32 READ_THREAD_TIMEOUT_ISO;
	static const XnUInt32 READ_THREAD_BUFFER_NUM_PACKETS_BULK;
	static const XnUInt32 READ_THREAD_NUM_BUFFERS_BULK;
	static const XnUInt32 READ_THREAD_TIMEOUT_BULK;

	static const XnUInt32 BASE_INPUT_ENDPOINT;

	XnUSBEndPointType m_endpointType;
	XN_USB_EP_HANDLE m_hEndpoint;
	XN_USB_DEV_HANDLE m_hUSBDevice;
	XnUInt16 m_nEndpointID;
	XnUInt16 m_nMaxPacketSize;	
	IDataDestination* m_pDataDestination;
	XnBool m_bConnected;

};

}

#endif // __XNUSBINDATAENDPOINT_H__