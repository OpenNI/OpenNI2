#ifndef _PERC_DRIVER_H_
#define _PERC_DRIVER_H_

#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnPlatform.h"

#include "pxcsession.h"
#include "pxcsmartptr.h"
#include "pxccapture.h"

namespace perc_device
{

class PerCDriver 
    : public oni::driver::DriverBase
{
public:
    PerCDriver(OniDriverServices* pDriverServices);
    ~PerCDriver();

    virtual OniStatus initialize(oni::driver::DeviceConnectedCallback connectedCallback, oni::driver::DeviceDisconnectedCallback disconnectedCallback, oni::driver::DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie);

    virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* /*mode*/);
    virtual void deviceClose(oni::driver::DeviceBase* pDevice);

    virtual void shutdown();
protected:
	XN_THREAD_HANDLE m_threadHandle;
	xnl::StringsHash<PerCDevice *> m_devices;
    void clearDevices();
};

}//namespace perc_device

#endif //_PERC_DRIVER_H_