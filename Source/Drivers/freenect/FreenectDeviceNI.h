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
	
	// OpenNI
	int m_numSensors;
	OniSensorInfo m_sensors[3];
	

public:
	FreenectDeviceNI(freenect_context *_ctx, int _index) : Freenect::FreenectDevice(_ctx, _index)
	{
		depth_stream = NULL;
		image_stream = NULL;
		
		m_numSensors = 2;

		
		// DEPTH
		m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
		m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
		m_sensors[0].numSupportedVideoModes = 1;
		
		m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		m_sensors[0].pSupportedVideoModes[0].fps = 30;
		m_sensors[0].pSupportedVideoModes[0].resolutionX = 640;
		m_sensors[0].pSupportedVideoModes[0].resolutionY = 480;

		// COLOR
		m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
		m_sensors[1].sensorType = ONI_SENSOR_COLOR;
		m_sensors[1].numSupportedVideoModes = 1;

		m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_sensors[1].pSupportedVideoModes[0].fps = 30;
		m_sensors[1].pSupportedVideoModes[0].resolutionX = 640;
		m_sensors[1].pSupportedVideoModes[0].resolutionY = 480;
	
		/*
		m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_sensors[1].pSupportedVideoModes[0].fps         = 12; // correct?
		m_sensors[1].pSupportedVideoModes[0].resolutionX = 1280;
		//m_sensors[1].pSupportedVideoModes[0].resolutionY = 960;
		m_sensors[1].pSupportedVideoModes[0].resolutionY = 1024;

		m_sensors[1].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_YUV422;
		m_sensors[1].pSupportedVideoModes[2].fps         = 15;
		m_sensors[1].pSupportedVideoModes[2].resolutionX = 640;
		m_sensors[1].pSupportedVideoModes[2].resolutionY = 480;
		*/
		
		/*
		m_sensors[2].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
		m_sensors[2].sensorType = ONI_SENSOR_IR;
		m_sensors[2].numSupportedVideoModes = 3;
		m_sensors[2].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_sensors[2].pSupportedVideoModes[0].fps = DEFAULT_FPS;
		m_sensors[2].pSupportedVideoModes[0].resolutionX = 640;
		m_sensors[2].pSupportedVideoModes[0].resolutionY = 480;
	
		m_sensors[2].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
		m_sensors[2].pSupportedVideoModes[1].fps = DEFAULT_FPS;
		m_sensors[2].pSupportedVideoModes[1].resolutionX = 640;
		m_sensors[2].pSupportedVideoModes[1].resolutionY = 480;
	
		m_sensors[2].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
		m_sensors[2].pSupportedVideoModes[2].fps = DEFAULT_FPS;
		m_sensors[2].pSupportedVideoModes[2].resolutionX = 640;
		m_sensors[2].pSupportedVideoModes[2].resolutionY = 480;
		*/
	}
	
	
	// OpenNI
	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
	{
		*numSensors = m_numSensors;
		*pSensors = m_sensors;
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
