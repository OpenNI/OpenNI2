#ifndef __PS1200DEVICE_H__
#define __PS1200DEVICE_H__

#include "PrimeClient.h"
#include <XnUSB.h>

namespace xn
{

class ClientUSBConnectionFactory;

class PS1200Device : public PrimeClient
{
public:
	PS1200Device();
	virtual ~PS1200Device();

	virtual XnStatus Init(const XnChar* strConnString, XnTransportType transportType);
	virtual void Shutdown();
	virtual XnBool IsInitialized() const;

	XnStatus SetUsbAltInterface(XnUInt8 altInterface);
	XnStatus GetUsbAltInterface(XnUInt8& altInterface) const;

	virtual XnStatus UsbTest(XnUInt32 nSeconds, XnUInt32& endpointsCount, XnUsbTestEndpointResult* endpoints);

protected:
	const ClientUSBConnectionFactory* GetConnectionFactory() const;
	ClientUSBConnectionFactory* GetConnectionFactory();
	IConnectionFactory* CreateConnectionFactory(XnTransportType transportType);

private:
	static const XnUInt32 WAIT_FOR_FREE_BUFFER_TIMEOUT_MS;
	static const XnUInt32 USB_READ_BUFFERS;
	static const XnUInt32 USB_READ_BUFFER_PACKETS;
	static const XnUInt32 USB_READ_TIMEOUT;
	static const XnUSBEndPointType ENDPOINTS_TYPE;
	static const XnUInt32 NUM_OUTPUT_CONNECTIONS;
	static const XnUInt32 NUM_INPUT_CONNECTIONS;
	static const XnUInt32 PRE_CONTROL_RECEIVE_SLEEP;

	//Data members
	XnBool m_bInitialized;

	XnCallbackHandle m_hInputInterruptCallback;

	//Hide assignment operator
	PS1200Device& operator=(const PS1200Device&);
};

}

#endif // __PS1200DEVICE_H__
