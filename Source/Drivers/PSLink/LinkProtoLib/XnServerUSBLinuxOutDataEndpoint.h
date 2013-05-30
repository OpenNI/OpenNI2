#ifndef __XNSERVERUSBLINUXOUTDATAENDPOINT_H__
#define __XNSERVERUSBLINUXOUTDATAENDPOINT_H__

#include "IOutputConnection.h"

struct XnUSBDevice;

namespace xn
{

class ServerUSBLinuxOutDataEndpoint : virtual public IOutputConnection
{
public:
	ServerUSBLinuxOutDataEndpoint();
	virtual ~ServerUSBLinuxOutDataEndpoint();
	virtual XnStatus Init(XnUSBDevice* pUSBDevice, XnUInt16 nEndpointID, XnUInt16 nMaxPacketSize);
	virtual void Shutdown();
	virtual XnStatus Connect();
	virtual void Disconnect();
	virtual XnBool IsConnected() const;
	virtual XnUInt16 GetMaxPacketSize() const;
	virtual XnStatus Send(const void* pData, XnUInt32 nSize);
	XnStatus Reset();

private:
	XnUSBDevice* m_pUSBDevice;
	XnUInt8 m_nEndpointID;
	XnUInt16 m_nMaxPacketSize;
};

}

#endif // __XNSERVERUSBLINUXOUTDATAENDPOINT_H__
