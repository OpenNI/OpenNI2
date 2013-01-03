#ifndef _FREENECT_DEPTH_STREAM_H_
#define _FREENECT_DEPTH_STREAM_H_

#include "FreenectStream.h"


class FreenectDepthStream : public FreenectStream
{	
public:
	FreenectDepthStream(Freenect::FreenectDevice* pDevice, freenect_depth_format format = FREENECT_DEPTH_11BIT) : FreenectStream(pDevice)
	{
		setVideoMode(getSupportedModes()[0]);
		
		xnl::Pair<int, int> res = convertResolution(device->getDepthResolution());
		video_mode.fps = 30;
		video_mode.resolutionX = res.first;
		video_mode.resolutionY = res.second;
		
		printf("FreenectDepthStream: resolutionX = %d; resolutionY = %d\n", video_mode.resolutionX, video_mode.resolutionY);
	}

	int getBytesPerPixel() { return sizeof(OniDepthPixel); }
	
  /*
	FREENECT_DEPTH_11BIT        = 0, //< 11 bit depth information in one uint16_t/pixel
	FREENECT_DEPTH_10BIT        = 1, //< 10 bit depth information in one uint16_t/pixel
	FREENECT_DEPTH_11BIT_PACKED = 2, //< 11 bit packed depth information
	FREENECT_DEPTH_10BIT_PACKED = 3, //< 10 bit packed depth information
	FREENECT_DEPTH_REGISTERED   = 4, //< processed depth data in mm, aligned to 640x480 RGB
	FREENECT_DEPTH_MM           = 5, //< depth to each pixel in mm, but left unaligned to RGB image
	FREENECT_DEPTH_DUMMY        = 2147483647, //< Dummy value to force enum to be 32 bits wide
	*/
	/*
	ONI_PIXEL_FORMAT_DEPTH_1_MM = 100,
	ONI_PIXEL_FORMAT_DEPTH_100_UM = 101,
	ONI_PIXEL_FORMAT_SHIFT_9_2 = 102,
	ONI_PIXEL_FORMAT_SHIFT_9_3 = 103,
	*/
	
	OniStatus setVideoMode(OniVideoMode requested_mode)
	{
		OniVideoMode* supported_modes = getSupportedModes();
		freenect_depth_format format;
		freenect_resolution resolution;
		
		if (video_mode == supported_modes[0])
		{
			format = FREENECT_DEPTH_11BIT;
			resolution = FREENECT_RESOLUTION_MEDIUM;
		}
		else
		{
			return ONI_STATUS_NOT_SUPPORTED;
		}
		
		try { device->setDepthFormat(format, resolution); }
		catch (std::runtime_error e)
		{
			printf("format-resolution combination not supported: %d-%d\n", format, resolution);
			return ONI_STATUS_NOT_SUPPORTED;
		}
		
		video_mode = requested_mode;
		return ONI_STATUS_OK;
	}
	
	
	OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		switch(propertyId)
		{
			default:
				return ONI_STATUS_NOT_SUPPORTED;
			case ONI_STREAM_PROPERTY_VIDEO_MODE:
				if (dataSize != sizeof(OniVideoMode))
				{
					printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniVideoMode));
					return ONI_STATUS_ERROR;
				}
				return setVideoMode(*(reinterpret_cast<const OniVideoMode*>(data)));
		}
	}
	
	// todo : represent this as static const in implementation
	static OniVideoMode* getSupportedModes()
	{
		OniVideoMode * supported_modes = new OniVideoMode[1];
		// pixelFormat, resolutionX, resolutionY, fps
		supported_modes[0] = { ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30 };
		return supported_modes;
	}
	

	void buildFrame(void* depth, OniDriverFrame* pFrame)
	{
		pFrame->frame.sensorType = ONI_SENSOR_DEPTH;
		pFrame->frame.stride = video_mode.resolutionX*sizeof(OniDepthPixel);
		pFrame->frame.dataSize = device->getDepthBufferSize();
		pFrame->frame.data = xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);
		if (pFrame->frame.data == NULL)
		{
			XN_ASSERT(FALSE);
			return;
		}
		
		uint8_t* _data = static_cast<uint8_t*>(depth);
		uint8_t* frame_data = static_cast<uint8_t*>(pFrame->frame.data);
		std::copy(_data, _data+pFrame->frame.dataSize, frame_data);
	}
	
};

#endif // _FREENECT_DEPTH_STREAM_H_
