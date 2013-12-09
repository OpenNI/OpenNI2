#include "PerCDevice.h"
#include "PerCDriver.h"
#include "PerCDeviceEnumerator.h"

namespace perc_device
{

static const char VENDOR_VAL[] = "Intel";
static const char NAME_VAL[] = "PerCDevice";

PerCDriver::PerCDriver(OniDriverServices* pDriverServices) 
    : DriverBase(pDriverServices)
{
}

PerCDriver::~PerCDriver()
{
    clearDevices();
}

OniStatus PerCDriver::initialize(oni::driver::DeviceConnectedCallback connectedCallback, oni::driver::DeviceDisconnectedCallback disconnectedCallback, oni::driver::DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie)
{
    OniStatus ret = DriverBase::initialize(connectedCallback, disconnectedCallback, deviceStateChangedCallback, pCookie);
    if (ONI_STATUS_OK != ret)
    {
        return (ret);
    }

    XnUInt32 count = deviceEnumerator().getDevicesCount();
    for (XnUInt32 idx = 0; idx < count; idx++)
    {
        OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);

        deviceEnumerator().getDeviceURI(idx, pInfo->uri, ONI_MAX_STR);
        //xnOSStrCopy(pInfo->uri, uri.Data(), ONI_MAX_STR);
        xnOSStrCopy(pInfo->vendor, VENDOR_VAL, ONI_MAX_STR);
        xnOSStrCopy(pInfo->name, NAME_VAL, ONI_MAX_STR);        

        deviceConnected(pInfo);
        deviceStateChanged(pInfo, 0);
    }
    return ONI_STATUS_OK;
}

oni::driver::DeviceBase* PerCDriver::deviceOpen(const char* uri, const char* /*mode*/)
{
    PerCDevice *pDevice = NULL;

    if (m_devices.Get(uri, pDevice) == XN_STATUS_OK)
    {
        getServices().errorLoggerAppend("Device is already open.");
        return NULL;
    }

    pDevice = XN_NEW(PerCDevice, uri, getServices());
    if (NULL == pDevice)
    	return NULL;
    if (!pDevice->isValid())
    {
        XN_DELETE(pDevice);
    	return NULL;
    }

    // Add the device and return it.
    m_devices[uri] = pDevice;
    return pDevice;
}

void PerCDriver::deviceClose(oni::driver::DeviceBase* pDevice)
{
	for (xnl::StringsHash<PerCDevice *>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
	{
		if (iter->Value() == pDevice)
		{
			m_devices.Remove(iter);
			XN_DELETE(pDevice);
			return;
		}
	}

	// not our device?!
	XN_ASSERT(FALSE);
}

void PerCDriver::shutdown() 
{
	// Close all open devices and release the memory
    clearDevices();
}

void PerCDriver::clearDevices()
{
	for (xnl::StringsHash<PerCDevice *>::Iterator it = m_devices.Begin(); it != m_devices.End(); ++it)
	{
		XN_DELETE(it->Value());
	}

	m_devices.Clear();
}

}//namespace perc_device

ONI_EXPORT_DRIVER(perc_device::PerCDriver);