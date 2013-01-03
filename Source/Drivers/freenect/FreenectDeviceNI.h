#ifndef _FREENECT_DEVICE_NI_H_
#define _FREENECT_DEVICE_NI_H_

#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include "XnLog.h"
#include "FreenectDepthStream.h"
#include "FreenectImageStream.h"


using namespace oni::driver;

#define DEFAULT_FPS 30
class FreenectDeviceNI : public oni::driver::DeviceBase, public Freenect::FreenectDevice
{
private:
	FreenectDepthStream* depth_stream;
	FreenectImageStream* image_stream;	

public:
	FreenectDeviceNI(freenect_context *_ctx, int _index) : Freenect::FreenectDevice(_ctx, _index)
	{
		depth_stream = NULL;
		image_stream = NULL;
	}
	
	
	// OpenNI
	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
	{
		*numSensors = 2;
		
		OniSensorInfo * sensors = new OniSensorInfo[*numSensors];
		
		// DEPTH
		OniVideoMode* supported_modes = FreenectDepthStream::getSupportedModes();
		sensors[0].sensorType = ONI_SENSOR_DEPTH;		
		sensors[0].numSupportedVideoModes = SIZE(supported_modes);
		sensors[0].pSupportedVideoModes = supported_modes;
		
		// COLOR
		supported_modes = FreenectImageStream::getSupportedModes();
		sensors[1].sensorType = ONI_SENSOR_COLOR;
		sensors[1].numSupportedVideoModes = SIZE(supported_modes);
		sensors[1].pSupportedVideoModes = supported_modes;
		
		
		*pSensors = sensors;
		return ONI_STATUS_OK;
	}

	StreamBase* createStream(OniSensorType sensorType)
	{
		switch(sensorType)
		{
			case ONI_SENSOR_DEPTH:
				Freenect::FreenectDevice::startDepth();
				if (depth_stream == NULL)
					depth_stream = XN_NEW(FreenectDepthStream, this);
				return depth_stream;
			case ONI_SENSOR_COLOR:
				Freenect::FreenectDevice::startVideo();
				if (image_stream == NULL)
					image_stream = XN_NEW(FreenectImageStream, this);
				return image_stream;
			// todo: IR
			default:
				//m_driverServices.errorLoggerAppend("FreenectDeviceNI: Can't create a stream of type %d", sensorType);
				return NULL;
		}
	}

	void destroyStream(StreamBase* pStream)
	{
		if (pStream == depth_stream)
		{
			Freenect::FreenectDevice::stopDepth();
			depth_stream = NULL;
		}
		if (pStream == image_stream)
		{
			Freenect::FreenectDevice::stopVideo();
			image_stream = NULL;
		}
		
		XN_DELETE(pStream);
	}
	
	
	// freenect
	// Do not call directly even in child
	void DepthCallback(void *depth, uint32_t timestamp)
	{
		depth_stream->acquireFrame(depth, timestamp);
	}
	// Do not call directly even in child
	void VideoCallback(void *image, uint32_t timestamp)
	{
		image_stream->acquireFrame(image, timestamp);
	}
	
	
	/* unimplemented from DeviceBase
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize) {return ONI_STATUS_NOT_IMPLEMENTED;}
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize) {return ONI_STATUS_NOT_IMPLEMENTED;}
	virtual OniBool isPropertySupported(int propertyId) {return FALSE;}
	virtual OniStatus invoke(int commandId, const void* data, int dataSize) {return ONI_STATUS_NOT_IMPLEMENTED;}
	virtual OniBool isCommandSupported(int commandId) {return FALSE;}
	virtual OniStatus tryManualTrigger() {return ONI_STATUS_OK;}
	virtual void setPropertyChangedCallback(PropertyChangedCallback handler, void* pCookie) { m_propertyChangedCallback = handler; m_propertyChangedCookie = pCookie; }
	virtual void notifyAllProperties() { return; }
	virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode) { return (mode == ONI_IMAGE_REGISTRATION_OFF); }
	*/
};

/* Device Properties
 	ONI_DEVICE_PROPERTY_FIRMWARE_VERSION		= 0, // By implementation
	ONI_DEVICE_PROPERTY_DRIVER_VERSION		= 1, // OniVersion
	ONI_DEVICE_PROPERTY_HARDWARE_VERSION		= 2, // int
	ONI_DEVICE_PROPERTY_SERIAL_NUMBER		= 3, // string
	ONI_DEVICE_PROPERTY_ERROR_STATE			= 4, // ??
	ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION		= 5, // OniImageRegistrationMode

	// Files
	ONI_DEVICE_PROPERTY_PLAYBACK_SPEED		= 100, // float
	ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED			= 101, // OniBool
*/

#endif //_FREENECT_DEVICE_NI_H_
