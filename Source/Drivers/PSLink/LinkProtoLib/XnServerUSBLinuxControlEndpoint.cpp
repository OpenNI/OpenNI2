#include <XnPlatform.h>

#if (XN_PLATFORM == XN_PLATFORM_LINUX_ARM)

#include "XnServerUSBLinuxControlEndpoint.h"
#include <XnLog.h>
#include <XnOS.h>
#include <XnUSBDevice.h>

namespace xn
{

const XnUInt32 ServerUSBLinuxControlEndpoint::RECEIVE_TIMEOUT = 5000;

ServerUSBLinuxControlEndpoint::ServerUSBLinuxControlEndpoint()
{
	m_pUSBDevice = NULL;
	m_hControlEvent = NULL;
	m_nMaxPacketSize = 0;
	m_bConnected = FALSE;
}

ServerUSBLinuxControlEndpoint::~ServerUSBLinuxControlEndpoint()
{
	Shutdown();
}

XnStatus ServerUSBLinuxControlEndpoint::Init(XnUSBDevice* pUSBDevice, XnUInt16 nMaxPacketSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	m_pUSBDevice = pUSBDevice;
	m_nMaxPacketSize = nMaxPacketSize;
	nRetVal = xnOSCreateEvent(&m_hControlEvent, FALSE);
	XN_IS_STATUS_OK_LOG_ERROR("Create control event", nRetVal);
	return XN_STATUS_OK;
}

void ServerUSBLinuxControlEndpoint::Shutdown()
{
	Disconnect();
	xnOSCloseEvent(&m_hControlEvent);
	m_pUSBDevice = NULL;
}

XnStatus ServerUSBLinuxControlEndpoint::Connect()
{
	XnStatus nRetVal = XN_STATUS_OK;
	XN_VALIDATE_PTR(m_pUSBDevice, XN_STATUS_NOT_INIT);
	//Set command handler
	nRetVal = xnUSBDeviceSetNewControlRequestCallback(m_pUSBDevice, &OnControlRequest, this);
	XN_IS_STATUS_OK_LOG_ERROR("Set new control request callback", nRetVal);
	m_bConnected = TRUE;
	return XN_STATUS_OK;
}

void ServerUSBLinuxControlEndpoint::Disconnect()
{
	if (m_bConnected)
	{
		//Clear commands handler
		xnUSBDeviceSetNewControlRequestCallback(m_pUSBDevice, NULL, NULL);
		m_bConnected = FALSE;
	}
}

XnUInt16 ServerUSBLinuxControlEndpoint::GetMaxPacketSize() const
{
	return m_nMaxPacketSize;
}

XnStatus ServerUSBLinuxControlEndpoint::Receive(void* pData, XnUInt32& nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	if (!m_bConnected)
	{
		return XN_STATUS_DEVICE_NOT_CONNECTED;
	}

	nRetVal = xnOSWaitEvent(m_hControlEvent, RECEIVE_TIMEOUT);
	if (nRetVal == XN_STATUS_OS_EVENT_TIMEOUT)
	{
		return nRetVal;
	}
	XN_IS_STATUS_OK_LOG_ERROR("Wait for control event", nRetVal);
	nRetVal = xnUSBDeviceReceiveControlRequest(m_pUSBDevice, (XnUChar*)pData, &nSize);
	XN_IS_STATUS_OK_LOG_ERROR("Receive control request", nRetVal);
	return XN_STATUS_OK;
}

XnStatus ServerUSBLinuxControlEndpoint::Send(const void* pData, XnUInt32 nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	if (!m_bConnected)
	{
		return XN_STATUS_DEVICE_NOT_CONNECTED;
	}

	nRetVal = xnUSBDeviceSendControlReply(m_pUSBDevice, (const XnUChar*)pData, nSize);
	XN_IS_STATUS_OK_LOG_ERROR("Send control reply", nRetVal);
	return XN_STATUS_OK;
}

void ServerUSBLinuxControlEndpoint::OnControlRequest(XnUSBDevice* /*pDevice*/, void* pCookie)
{
	ServerUSBLinuxControlEndpoint* pThis = (ServerUSBLinuxControlEndpoint*)pCookie;
	XN_ASSERT(pThis);
	xnOSSetEvent(pThis->m_hControlEvent);
}

XnBool ServerUSBLinuxControlEndpoint::IsConnected() const
{
	return m_bConnected;
}

}

#endif