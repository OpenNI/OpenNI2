#ifndef _FREENECT_DEPTH_STREAM_H_
#define _FREENECT_DEPTH_STREAM_H_

#include "FreenectStream.h"


class FreenectDepthStream : public FreenectStream
{
	
public:
	FreenectDepthStream(Freenect::FreenectDevice* pDevice, freenect_depth_format format = FREENECT_DEPTH_11BIT) : FreenectStream(pDevice)
	{
		setFormat(format);
		
		xnl::Pair<int, int> res = convertResolution(device->getDepthResolution());

		video_mode.fps = 30;
		video_mode.resolutionX = res.first;
		video_mode.resolutionY = res.second;
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
	freenect_depth_format getFormat()
	{
		return device->getDepthFormat();
	}
	OniStatus setFormat(freenect_depth_format format, freenect_resolution resolution = FREENECT_RESOLUTION_MEDIUM)
	{
		switch(format)
		{
			default:
				video_mode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
				break;
			case FREENECT_DEPTH_11BIT_PACKED:
			case FREENECT_DEPTH_10BIT_PACKED:
				return ONI_STATUS_NOT_SUPPORTED;
		}
		
		device->setDepthFormat(format, resolution);
		return ONI_STATUS_OK;
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
