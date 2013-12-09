#ifndef _PERC_DEVICE_H_
#define _PERC_DEVICE_H_

#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnArray.h"

#include "pxccapture.h"


namespace perc_device
{
class PerCDevice 
    : public oni::driver::DeviceBase
{
private:
	PerCDevice(const PerCDevice&);
	void operator=(const PerCDevice&);
public:
	PerCDevice(const char *uri, oni::driver::DriverServices& driverServices);
    ~PerCDevice();

    bool isValid() const
    {
        return ((XnUInt32)-1 != m_idxDeviceInternal);
    }
public:
    OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors);

	oni::driver::StreamBase* createStream(OniSensorType sensorType);
	void destroyStream(oni::driver::StreamBase* pStream);

	OniStatus  getProperty(int propertyId, void* data, int* pDataSize);
private:
	oni::driver::DriverServices& m_driverServices;

    xnl::Array<OniSensorInfo> m_sensors;

    XnUInt32 m_idxDeviceInternal;

    struct SensorSetting
    {
        int             idxStream;
        OniVideoMode    videoModeDef;
    };
    SensorSetting m_streamColor;
    SensorSetting m_streamDepth;
    SensorSetting m_streamIR;

    bool fillColorSensorVideoMode(OniSensorInfo &sensor, PXCCapture::Device *device, int idxStream);
    bool fillDepthSensorVideoMode(OniSensorInfo &sensor, PXCCapture::Device *device, int idxStream);
    
    void fillStreamArray();
    void clearSensorsList();
};
}// namespace perc_device

#endif //_PERC_DEVICE_H_