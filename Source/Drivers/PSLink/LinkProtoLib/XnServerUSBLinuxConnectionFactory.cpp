#include <XnPlatform.h>

#if (XN_PLATFORM == XN_PLATFORM_LINUX_ARM)

#include "XnServerUSBLinuxConnectionFactory.h"
#include "XnLinkProtoLibDefs.h"
#include "XnServerUSBLinuxConnectionFactory.h"
#include "XnServerUSBLinuxOutDataEndpoint.h"
#include <XnUSBDevice.h>
#include <XnLog.h>

namespace xn
{


#define CONTROL_MAX_PACKET_SIZE 512
#define OUT_DATA_ENDPOINT_PACKET_SIZE 9216

#define NUM_OUTPUT_ENDPOINTS 1
#define VENDOR_NAME_ID 1
#define PRODUCT_NAME_ID 2
#define PRODUCT_NAME "PrimeSense Lena Device"
#define INTERFACE_NAME_ID 3
#define INTERFACE_NAME "Default"

#define XN_MASK_USB "xnUSB"

XnUSBStringDescriptor aStrings[] =
{
	{ VENDOR_NAME_ID, XN_VENDOR_PRIMESENSE },
	{ PRODUCT_NAME_ID,  PRODUCT_NAME},
	{ INTERFACE_NAME_ID, INTERFACE_NAME },
};

static XnUSBEndpointDescriptor depth_ep_desc =
{
	USB_DT_ENDPOINT_SIZE, // bLength
	USB_DT_ENDPOINT, // bDescriptorType
	0x81, // bEndpointAddress
	USB_ENDPOINT_XFER_BULK, // bmAttributes
	XN_PREPARE_VAR16_IN_BUFFER(0x0200), // wMaxPacketSize
	1, //bInterval
};

static XnUSBEndpointDescriptor ee_ep_desc =
{
	USB_DT_ENDPOINT_SIZE, // bLength
	USB_DT_ENDPOINT, // bDescriptorType
	0x82, // bEndpointAddress
	USB_ENDPOINT_XFER_BULK, // bmAttributes
	XN_PREPARE_VAR16_IN_BUFFER(0x0200), // wMaxPacketSize
	1, //bInterval
};

static XnUSBEndpointDescriptor* default_interface_eps[] =
{
	&depth_ep_desc,
	&ee_ep_desc,
};

static XnUSBInterfaceDescriptorHolder default_interface =
{
	{ // descriptor
		USB_DT_INTERFACE_SIZE, // bLength
		USB_DT_INTERFACE, // bDescriptorType
		0, // bInterfaceNumber
		0, // bAlternateSetting
		sizeof(default_interface_eps)/sizeof(default_interface_eps[0]), // bNumEndpoints
		USB_CLASS_VENDOR_SPEC, // bInterfaceClass
		0xFF, // bInterfaceSubClass
		0, // bInterfaceProtocol
		INTERFACE_NAME_ID, // iInterface
	},
	default_interface_eps, // aEndpoints
};

static XnUSBInterfaceDescriptorHolder* default_config_interfaces[] = { &default_interface };

static XnUSBConfigDescriptorHolder default_config =
{
	{ // descriptor
		USB_DT_CONFIG_SIZE, // bLength
		USB_DT_CONFIG, // bDescriptorType
		0, // wTotalLength
		sizeof(default_config_interfaces)/sizeof(default_config_interfaces[0]), // bNumInterfaces
		1, // bConfigurationValue
		0, // iConfiguration
		USB_CONFIG_ATT_ONE, // bmAttributes
		250, // 500 mW // bMaxPower
	},
	default_config_interfaces, // aInterfaces
};

static XnUSBConfigDescriptorHolder* configurations[] = { &default_config };

static XnUSBDeviceDescriptorHolder device_descriptor =
{
	{ // descriptor
		USB_DT_DEVICE_SIZE, // bLength
		USB_DT_DEVICE, // bDescriptorType
		XN_PREPARE_VAR16_IN_BUFFER(0x0200), // bcdUSB
		0, // bDeviceClass
		0, // bDeviceSubClass
		0, // bDeviceProtocol
		0, // bMaxPacketSize0
		XN_PREPARE_VAR16_IN_BUFFER(XN_VENDOR_ID), // idVendor
		XN_PREPARE_VAR16_IN_BUFFER(XN_PRODUCT_ID_LENA), // idProduct
		0x100, // bcdDevice
		VENDOR_NAME_ID, // iManufacturer
		PRODUCT_NAME_ID, // iProduct
		0, // iSerialNumber
		sizeof(configurations)/sizeof(configurations[0]), // bNumConfigurations
	},
	configurations, // aConfigurations
	aStrings, // aStrings
	sizeof(aStrings)/sizeof(aStrings[0]) // nStrings
};

ServerUSBLinuxConnectionFactory::ServerUSBLinuxConnectionFactory()
{
	m_pDevice = NULL;
	m_bInitialized = FALSE;
}

ServerUSBLinuxConnectionFactory::~ServerUSBLinuxConnectionFactory()
{
	Shutdown();
}

XnStatus ServerUSBLinuxConnectionFactory::Init(const XnChar* /*strConnString*/)
{
	XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = xnUSBDeviceInit(&device_descriptor, CONTROL_MAX_PACKET_SIZE, &m_pDevice);
	XN_IS_STATUS_OK_LOG_ERROR("Init usb device", nRetVal);
	nRetVal = m_controlEndpoint.Init(m_pDevice, CONTROL_MAX_PACKET_SIZE);
	XN_IS_STATUS_OK_LOG_ERROR("Init control connection", nRetVal);
	m_bInitialized = TRUE;
	return XN_STATUS_OK;
}

void ServerUSBLinuxConnectionFactory::Shutdown()
{
	if (m_bInitialized)
	{
		m_controlEndpoint.Shutdown();
		xnUSBDeviceShutdown(m_pDevice);
		m_pDevice = NULL;
		m_bInitialized = FALSE;
	}
}

XnBool ServerUSBLinuxConnectionFactory::IsInitialized() const
{
	return m_bInitialized;
}

XnUInt16 ServerUSBLinuxConnectionFactory::GetNumInputDataConnections() const
{
	return 0;
}

XnUInt16 ServerUSBLinuxConnectionFactory::GetNumOutputDataConnections() const
{
	return NUM_OUTPUT_ENDPOINTS;
}

XnStatus ServerUSBLinuxConnectionFactory::GetControlConnection(ISyncIOConnection*& pConn)
{
	if (!m_bInitialized)
	{
		return XN_STATUS_NOT_INIT;
	}

	pConn = &m_controlEndpoint;
	return XN_STATUS_OK;
}

XnStatus ServerUSBLinuxConnectionFactory::CreateOutputDataConnection(XnUInt16 nID, IOutputConnection*& pConn)
{
	if (!m_bInitialized)
	{
		return XN_STATUS_NOT_INIT;
	}

	XnStatus nRetVal = XN_STATUS_OK;
	if (nID >= NUM_OUTPUT_ENDPOINTS)
	{
		xnLogError(XN_MASK_USB, "Output data connection number %u is not supported - max endpoint ID is %u", 
			nID, (NUM_OUTPUT_ENDPOINTS - 1));
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}
	
	ServerUSBLinuxOutDataEndpoint* pOutDataEndpoint = XN_NEW(ServerUSBLinuxOutDataEndpoint);
	XN_VALIDATE_ALLOC_PTR(pOutDataEndpoint);
	nRetVal = pOutDataEndpoint->Init(m_pDevice, nID, OUT_DATA_ENDPOINT_PACKET_SIZE);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_USB, "Failed to initialize output data endpoint: %s", 
			xnGetStatusString(nRetVal));
		XN_ASSERT(FALSE);
		XN_DELETE(pOutDataEndpoint);
	}

	pConn = pOutDataEndpoint;
	return XN_STATUS_OK;
}

XnStatus ServerUSBLinuxConnectionFactory::CreateInputDataConnection(XnUInt16 /*nID*/, IAsyncInputConnection*& /*pConn*/)
{
	//Input endpoints not implemented yet
	return XN_STATUS_NOT_IMPLEMENTED;
}

}

#endif
