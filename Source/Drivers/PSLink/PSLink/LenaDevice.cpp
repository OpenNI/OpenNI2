#include "LenaDevice.h"
#include "XnClientUSBConnectionFactory.h"
#include "XnSocketConnectionFactory.h"
#include <XnLog.h>

#define XN_MASK_LENA "Lena"

namespace xn
{

const XnUSBEndPointType LenaDevice::ENDPOINTS_TYPE = XN_USB_EP_BULK;
const XnUInt32 LenaDevice::NUM_OUTPUT_CONNECTIONS = 0;
const XnUInt32 LenaDevice::NUM_INPUT_CONNECTIONS = 2;
const XnUInt32 LenaDevice::PRE_CONTROL_RECEIVE_SLEEP = 50;

LenaDevice::LenaDevice()
{

}

LenaDevice::~LenaDevice()
{
	Shutdown();
}

IConnectionFactory* LenaDevice::CreateConnectionFactory(XnTransportType transportType)
{
	switch (transportType)
	{
		case XN_TRANSPORT_TYPE_USB:
			return XN_NEW(ClientUSBConnectionFactory, 
			              NUM_INPUT_CONNECTIONS, 
						  NUM_OUTPUT_CONNECTIONS, 
						  PRE_CONTROL_RECEIVE_SLEEP);
			break;
		case XN_TRANSPORT_TYPE_SOCKETS:
			return XN_NEW(SocketConnectionFactory, SocketConnectionFactory::TYPE_CLIENT);
			break;
		default:
			xnLogError(XN_MASK_LENA, "Bad transport type: %u", transportType);
			XN_ASSERT(FALSE);
			return NULL;
	}
}


}