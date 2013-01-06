#ifndef _FREENECT_DEVICE_NI_H_
#define _FREENECT_DEVICE_NI_H_

#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include "FreenectDepthStream.h"
#include "FreenectVideoStream.h"


using namespace oni::driver;

class FreenectDeviceNI : public DeviceBase, public Freenect::FreenectDevice
{
protected:
	FreenectDepthStream* depth_stream;
	FreenectVideoStream* video_stream;

public:
	FreenectDeviceNI(freenect_context *_ctx, int _index);
	~FreenectDeviceNI();

	// from DeviceBase
	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors);
	StreamBase* createStream(OniSensorType sensorType);
	void destroyStream(StreamBase* pStream);
	// property and command handlers are empty skeletons by default
	// only add here if the property is generic to all children
	// otherwise, implement in child and call these in default case
	OniBool isPropertySupported(int propertyId) { return (getProperty(propertyId, NULL, NULL) != ONI_STATUS_NOT_SUPPORTED); }
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		switch(propertyId)
		{
			default:
			case ONI_DEVICE_PROPERTY_FIRMWARE_VERSION:				// string
			case ONI_DEVICE_PROPERTY_DRIVER_VERSION:					// OniVersion
			case ONI_DEVICE_PROPERTY_HARDWARE_VERSION:				// int
			case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:						// string
			case ONI_DEVICE_PROPERTY_ERROR_STATE:							// ?
			case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:			// OniImageRegistrationMode
			// files
			case ONI_DEVICE_PROPERTY_PLAYBACK_SPEED:					// float
			case ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED:	// OniBool
				return ONI_STATUS_NOT_SUPPORTED;
		}
	}
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		switch(propertyId)
		{
			default:
			case ONI_DEVICE_PROPERTY_FIRMWARE_VERSION:				// By implementation
			case ONI_DEVICE_PROPERTY_DRIVER_VERSION:					// OniVersion
			case ONI_DEVICE_PROPERTY_HARDWARE_VERSION:				// int
			case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:						// string
			case ONI_DEVICE_PROPERTY_ERROR_STATE:							// ?
			case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:			// OniImageRegistrationMode
			// files
			case ONI_DEVICE_PROPERTY_PLAYBACK_SPEED:					// float
			case ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED:	// OniBool
				return ONI_STATUS_NOT_SUPPORTED;
		}
	}
	OniBool isCommandSupported(int propertyId) { return (invoke(propertyId, NULL, NULL) != ONI_STATUS_NOT_SUPPORTED); }
	virtual OniStatus invoke(int commandId, const void* data, int dataSize)
	{
		switch (commandId)
		{
			default:
			case ONI_DEVICE_COMMAND_SEEK:	// OniSeek
				return ONI_STATUS_NOT_SUPPORTED;
		}
	}

	// from Freenect::FreenectDevice
	// Do not call these directly, even in child
	void DepthCallback(void *depth, uint32_t timestamp)
	{
		depth_stream->acquireFrame(depth, timestamp);
	}
	void VideoCallback(void *image, uint32_t timestamp)
	{
		video_stream->acquireFrame(image, timestamp);
	}
	
	
	/* todo : from DeviceBase
	virtual OniStatus tryManualTrigger() {return ONI_STATUS_OK;}
	virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode) { return (mode == ONI_IMAGE_REGISTRATION_OFF); }
	*/
};


#endif //_FREENECT_DEVICE_NI_H_
