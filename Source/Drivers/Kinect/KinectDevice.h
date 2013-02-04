#ifndef _KINECT_DEVICE_H_
#define _KINECT_DEVICE_H_

#include "Driver\OniDriverAPI.h"
#include "KinectStreamImpl.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"

struct 	INuiSensor;

namespace kinect_device {

class KinectDevice : public oni::driver::DeviceBase 
{
public:
	KinectDevice(INuiSensor * pNuiSensor);
	virtual ~KinectDevice();

	virtual OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSources);

	virtual oni::driver::StreamBase* createStream(OniSensorType streamSource);
	virtual void destroyStream(oni::driver::StreamBase* pStream);

	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniBool isPropertySupported(int propertyId);
	virtual OniBool isCommandSupported(int commandId) ;
	virtual OniStatus tryManualTrigger();

	virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode);

private:
	INuiSensor * m_pNuiSensor;
	KinectStreamImpl* m_pDepthStream;
	KinectStreamImpl* m_pColorStream;
	int m_numSensors;
	OniSensorInfo m_sensors[10];	
};
} // namespace kinect_device
#endif //_KINECT_DRIVER_H_