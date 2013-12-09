#ifndef _PERC_DEVICE_ENUMERATOR_H_
#define _PERC_DEVICE_ENUMERATOR_H_

#include <vector>

#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnArray.h"
#include "XnPlatform.h"

#include "pxcsession.h"
#include "pxcsmartptr.h"
#include "pxccapture.h"

namespace perc_device
{

class PerCDeviceEnumerator
{
public:
    PXCSession *getSession(){ return m_session;}

    XnUInt32 getDevicesCount();
    size_t   getDeviceURI(XnUInt32 idxDevice, char *uri, size_t uriSize);
    PXCCapture::Device *getDevice(XnUInt32 idxDevice);
    XnUInt32 getDeviceIndex(const char *uri);
private:
    PXCSmartPtr<PXCSession> m_session;

    struct DeviceInfo
    {
        PXCSession::ImplDesc descModule;
        XnUInt32 idxDevice;
    };
    xnl::Array<DeviceInfo> m_deviceInfo;
    xnl::StringsHash<XnUInt32> m_devicesUri;

    PXCCapture::Device *getDevice(const DeviceInfo &deviceInfo);

    void enumDevices(PXCSession::ImplDesc &descModule);
    void enumDevices();
private:
    PerCDeviceEnumerator();
    ~PerCDeviceEnumerator();

    PerCDeviceEnumerator(const PerCDeviceEnumerator&);
    PerCDeviceEnumerator& operator =(const PerCDeviceEnumerator&);
private:
    static size_t getDeviceURI(PXCCapture::Device *device, char *uri, size_t uriSize);
public:
    static PerCDeviceEnumerator &getPerCDeviceEnumerator();
};

PerCDeviceEnumerator &deviceEnumerator();

}//namespace perc_device

#endif //_PERC_DEVICE_ENUMERATOR_H_


