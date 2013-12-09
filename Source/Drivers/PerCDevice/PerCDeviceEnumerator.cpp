#include <stdlib.h>
#include "PerCDeviceEnumerator.h"


namespace perc_device
{
PerCDeviceEnumerator::PerCDeviceEnumerator()
{
    pxcStatus sts = PXCSession_Create(&m_session);
    if (PXC_STATUS_NO_ERROR > sts) 
        return;

    enumDevices();
}

PerCDeviceEnumerator::~PerCDeviceEnumerator()
{
}

XnUInt32 PerCDeviceEnumerator::getDevicesCount()
{
    return m_deviceInfo.GetSize();
}
size_t PerCDeviceEnumerator::getDeviceURI(XnUInt32 idxDevice, char *uri, size_t uriSize)
{
    if (idxDevice >= m_deviceInfo.GetSize())
        return 0;
    return getDeviceURI(getDevice(m_deviceInfo[idxDevice]), uri, uriSize);
}

size_t PerCDeviceEnumerator::getDeviceURI(PXCCapture::Device *device, char *uri, size_t uriSize)
{
    if (NULL == device)
        return 0;

    PXCCapture::DeviceInfo dinfo;
    if (PXC_STATUS_NO_ERROR > device->QueryDevice(&dinfo)) 
        return 0;
       
    size_t convertedChars = 0;    
    wcstombs_s(&convertedChars, uri, uriSize, dinfo.did, _TRUNCATE);
    return convertedChars;
}

XnUInt32 PerCDeviceEnumerator::getDeviceIndex(const char *uri)
{
    XnUInt32 idx = (XnUInt32)-1;
    if (XN_STATUS_OK != m_devicesUri.Get(uri, idx))
        return (XnUInt32)-1;
    return idx;
}


PXCCapture::Device *PerCDeviceEnumerator::getDevice(XnUInt32 idxDevice)
{
    if (idxDevice >= m_deviceInfo.GetSize())
        return NULL;
    return getDevice(m_deviceInfo[idxDevice]);
}

PXCCapture::Device *PerCDeviceEnumerator::getDevice(const DeviceInfo &deviceInfo)
{
    if (NULL == m_session)
        return NULL;

    PXCSmartPtr<PXCCapture> capture;
    if (PXC_STATUS_NO_ERROR > m_session->CreateImpl<PXCCapture>((PXCSession::ImplDesc *)&deviceInfo.descModule, &capture))
        return NULL;

    PXCSmartPtr<PXCCapture::Device> device;
    if (PXC_STATUS_NO_ERROR > capture->CreateDevice(deviceInfo.idxDevice, &device)) 
        return NULL;
    return device.ReleasePtr();
}

void PerCDeviceEnumerator::enumDevices(PXCSession::ImplDesc &descModule)
{
    if (NULL == m_session)
        return;

    PXCSmartPtr<PXCCapture> capture;
    if (PXC_STATUS_NO_ERROR > m_session->CreateImpl<PXCCapture>(&descModule, &capture))
        return;

    static char uri[ONI_MAX_STR];
    for (int idxDevice = 0; ; idxDevice++) 
    {
        PXCSmartPtr<PXCCapture::Device> device = NULL;
        if (PXC_STATUS_NO_ERROR > capture->CreateDevice(idxDevice, &device)) 
            break;

        DeviceInfo info = {descModule, idxDevice};        
        getDeviceURI(device, uri, ONI_MAX_STR);
        m_devicesUri[uri] = m_deviceInfo.GetSize();
        m_deviceInfo.AddLast(info);
    }
}

void PerCDeviceEnumerator::enumDevices()
{
    PXCSession::ImplDesc descTemplate;
    memset(&descTemplate, 0, sizeof(descTemplate));
    descTemplate.group     = PXCSession::IMPL_GROUP_SENSOR;
    descTemplate.subgroup  = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;
    for (int idxModule = 0; ; idxModule++) 
    {
        PXCSession::ImplDesc desc;
        if (PXC_STATUS_NO_ERROR > m_session->QueryImpl(&descTemplate, idxModule, &desc)) 
            break;

        enumDevices(desc);
    }
}

PerCDeviceEnumerator &PerCDeviceEnumerator::getPerCDeviceEnumerator()
{
    static PerCDeviceEnumerator enumerator;
    return enumerator;
}

PerCDeviceEnumerator &deviceEnumerator()
{
    return PerCDeviceEnumerator::getPerCDeviceEnumerator();
}
};