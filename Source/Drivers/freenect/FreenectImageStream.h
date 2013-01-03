#ifndef _FREENECT_IMAGE_STREAM_H_
#define _FREENECT_IMAGE_STREAM_H_

#include "FreenectStream.h"




class FreenectImageStream : public FreenectStream
{
public:
	FreenectImageStream(Freenect::FreenectDevice* pDevice, freenect_video_format format = FREENECT_VIDEO_RGB) : FreenectStream(pDevice)
	{
		setFormat(format, FREENECT_RESOLUTION_MEDIUM);
		
		xnl::Pair<int, int> res = convertResolution(device->getVideoResolution());
		video_mode.fps = 30;			
		video_mode.resolutionX = res.first;
		video_mode.resolutionY = res.second;
		
		//printf("FreenectImageStream: resolutionX = %d; resolutionY = %d\n", video_mode.resolutionX, video_mode.resolutionY);
	}
	
	int getBytesPerPixel() { return sizeof(OniRGB888Pixel); }
	
	/*
	FREENECT_VIDEO_RGB             = 0, //< Decompressed RGB mode (demosaicing done by libfreenect)
	FREENECT_VIDEO_BAYER           = 1, //< Bayer compressed mode (raw information from camera)
	FREENECT_VIDEO_IR_8BIT         = 2, //< 8-bit IR mode
	FREENECT_VIDEO_IR_10BIT        = 3, //< 10-bit IR mode
	FREENECT_VIDEO_IR_10BIT_PACKED = 4, //< 10-bit packed IR mode
	FREENECT_VIDEO_YUV_RGB         = 5, //< YUV RGB mode
	FREENECT_VIDEO_YUV_RAW         = 6, //< YUV Raw mode
	FREENECT_VIDEO_DUMMY           = 2147483647, //< Dummy value to force enum to be 32 bits wide
	*/
	/*
	ONI_PIXEL_FORMAT_RGB888 = 200,
	ONI_PIXEL_FORMAT_YUV422 = 201,
	ONI_PIXEL_FORMAT_GRAY8 = 202,
	ONI_PIXEL_FORMAT_GRAY16 = 203,
	ONI_PIXEL_FORMAT_JPEG = 204,
	*/
	freenect_video_format getFormat()
	{
		return device->getVideoFormat();
	}
	OniStatus setFormat(freenect_video_format format, freenect_resolution resolution = FREENECT_RESOLUTION_MEDIUM)
	{
		switch(format)
		{			
			default:
			case FREENECT_VIDEO_BAYER:
			case FREENECT_VIDEO_IR_8BIT:
			case FREENECT_VIDEO_IR_10BIT:
			case FREENECT_VIDEO_IR_10BIT_PACKED:
				return ONI_STATUS_NOT_IMPLEMENTED;
				
			case FREENECT_VIDEO_RGB:
			case FREENECT_VIDEO_YUV_RGB:
			case FREENECT_VIDEO_YUV_RAW:
				try { device->setVideoFormat(format, resolution); }
				catch (std::runtime_error e)
				{
					printf("format-resolution combination not supported: %d-%d\ntrying format with default freenect resolution\n", format, resolution);	
					try { device->setVideoFormat(format); } catch (std::runtime_error e) { printf("format not supported %d\n", format); }
					return ONI_STATUS_NOT_SUPPORTED;
				}

				video_mode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
				return ONI_STATUS_OK;		
		}
		
		switch(resolution)
		{
			default:
				break;
			case FREENECT_RESOLUTION_LOW:
				break;
			case FREENECT_RESOLUTION_MEDIUM:
				break;
			case FREENECT_RESOLUTION_HIGH:
			//	video_mode.resolutionX = 1280;
			//	video_mode.resolutionY = 1024; // 960 ?
				break;
		}
	}
	
	
	void buildFrame(void* image, OniDriverFrame* pFrame)
	{
		pFrame->frame.sensorType = ONI_SENSOR_COLOR;
		pFrame->frame.stride = video_mode.resolutionX*3;
		pFrame->frame.dataSize = device->getVideoBufferSize();
		pFrame->frame.data = xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);
		if (pFrame->frame.data == NULL)
		{
			XN_ASSERT(FALSE);
			return;
		}
		
		unsigned char* _data = static_cast<unsigned char*>(image);
		unsigned char* frame_data = static_cast<unsigned char*>(pFrame->frame.data);
		std::copy(_data, _data+pFrame->frame.dataSize, frame_data);		
	}
};

#endif // _FREENECT_IMAGE_STREAM_H_
