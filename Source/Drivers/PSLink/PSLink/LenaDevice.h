#include "PrimeClient.h"
#include <XnUSB.h>

namespace xn
{

class LenaDevice : public PrimeClient
{
public:
	LenaDevice();
	virtual ~LenaDevice();
	
protected:
	virtual IConnectionFactory* CreateConnectionFactory(XnTransportType transportType);

private:
	static const XnUInt32 NUM_INPUT_CONNECTIONS;
	static const XnUInt32 NUM_OUTPUT_CONNECTIONS;
	static const XnUSBEndPointType ENDPOINTS_TYPE;
	static const XnUInt32 USB_READ_TIMEOUT;
	static const XnUInt32 USB_READ_BUFFER_PACKETS;
	static const XnUInt32 USB_READ_BUFFERS;
	static const XnUInt32 PRE_CONTROL_RECEIVE_SLEEP;
};

}