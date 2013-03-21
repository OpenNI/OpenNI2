#include <XnPlatform.h>

#if (XN_PLATFORM == XN_PLATFORM_LINUX_ARM)

#include "XnServerUSBLinuxOutDataEndpoint.h"
#include <XnUSBDevice.h>
#include <XnLog.h>

#define XN_MASK_USB "xnUSB"

namespace xn
{

ServerUSBLinuxOutDataEndpoint::ServerUSBLinuxOutDataEndpoint()
{
	m_pUSBDevice = NULL;
	m_nEndpointID = 0;
	m_nMaxPacketSize = 0;
}

ServerUSBLinuxOutDataEndpoint::~ServerUSBLinuxOutDataEndpoint()
{
	Shutdown();
}
XnStatus ServerUSBLinuxOutDataEndpoint::Reset()
{
	XN_VALIDATE_PTR(m_pUSBDevice, XN_STATUS_NOT_INIT);
	return xnUSBDeviceResetEndpoint(m_pUSBDevice, m_nEndpointID);
}
XnStatus ServerUSBLinuxOutDataEndpoint::Init(XnUSBDevice* pUSBDevice, XnUInt16 nEndpointID, XnUInt16 nMaxPacketSize)
{
	if (nEndpointID > XN_MAX_UINT8)
	{
		xnLogError(XN_MASK_USB, "Bad endpoint ID: %u - max is %u.", nEndpointID, XN_MAX_UINT8);
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}
	m_pUSBDevice = pUSBDevice;
	m_nEndpointID = 0x81 + XnUInt8(nEndpointID);
	m_nMaxPacketSize = nMaxPacketSize;
	return XN_STATUS_OK;
}

void ServerUSBLinuxOutDataEndpoint::Shutdown()
{
	Disconnect();
	m_pUSBDevice = NULL;
	m_nMaxPacketSize = 0;
}

XnStatus ServerUSBLinuxOutDataEndpoint::Connect()
{
	return XN_STATUS_OK;
}

void ServerUSBLinuxOutDataEndpoint::Disconnect()
{
}

XnUInt16 ServerUSBLinuxOutDataEndpoint::GetMaxPacketSize() const
{
	return m_nMaxPacketSize;
}

XnStatus ServerUSBLinuxOutDataEndpoint::Send(const void* pData, XnUInt32 nSize)
{
	XN_VALIDATE_PTR(m_pUSBDevice, XN_STATUS_NOT_INIT);
	return xnUSBDeviceWriteEndpoint(m_pUSBDevice, m_nEndpointID, (const XnUChar*)pData, nSize);
}

XnBool ServerUSBLinuxOutDataEndpoint::IsConnected() const
{
	return TRUE;
}

}

#endif