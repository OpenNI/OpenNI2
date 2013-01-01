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
	FreenectImageStream* image_stream;
	FreenectDepthStream* depth_stream;
	
	// OpenNI
	int m_numSensors;
	OniSensorInfo m_sensors[3];
	

public:
	FreenectDeviceNI(freenect_context *_ctx, int _index) : Freenect::FreenectDevice(_ctx, _index)
	{
		image_stream = NULL;
		depth_stream = NULL;
		
		m_numSensors = 2;
		
		// DEPTH
		m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 2);
		m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
		m_sensors[0].numSupportedVideoModes = 2;
		
		m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		m_sensors[0].pSupportedVideoModes[0].fps = 30;
		m_sensors[0].pSupportedVideoModes[0].resolutionX = 640;
		m_sensors[0].pSupportedVideoModes[0].resolutionY = 480;
	
		m_sensors[0].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		m_sensors[0].pSupportedVideoModes[1].fps = 30;
		m_sensors[0].pSupportedVideoModes[1].resolutionX = 320;
		m_sensors[0].pSupportedVideoModes[1].resolutionY = 240;
	
		// COLOR
		m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
		m_sensors[1].sensorType = ONI_SENSOR_COLOR;
		m_sensors[1].numSupportedVideoModes = 2;
		m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_sensors[1].pSupportedVideoModes[0].fps         = 12; // correct?
		m_sensors[1].pSupportedVideoModes[0].resolutionX = 1280;
		//m_sensors[1].pSupportedVideoModes[0].resolutionY = 960;
		m_sensors[1].pSupportedVideoModes[0].resolutionY = 1024;
	
		m_sensors[1].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_sensors[1].pSupportedVideoModes[1].fps = 30;
		m_sensors[1].pSupportedVideoModes[1].resolutionX = 640;
		m_sensors[1].pSupportedVideoModes[1].resolutionY = 480;
	
		/*
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
				if (depth_stream == NULL)
					depth_stream = XN_NEW(FreenectDepthStream, this);
				startDepth();
				return depth_stream;
			case ONI_SENSOR_COLOR:
				if (image_stream == NULL)
					image_stream = XN_NEW(FreenectImageStream, this);
				startVideo();
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
			stopDepth();
			depth_stream = NULL;
		}
		if (pStream == image_stream)
		{
			stopVideo();
			image_stream = NULL;
		}
		
		XN_DELETE(pStream);
	}
		
	
	// freenect
	// Do not call directly even in child
	void VideoCallback(void *image, uint32_t timestamp)
	{
		image_stream->buildFrame(image, timestamp);
	}
	// Do not call directly even in child
	void DepthCallback(void *depth, uint32_t timestamp)
	{
		depth_stream->buildFrame(depth, timestamp);
	}
};



#endif //_FREENECT_DEVICE_NI_H_
