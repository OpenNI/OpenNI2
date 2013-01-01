#ifndef _FREENECT_IMAGE_STREAM_H_
#define _FREENECT_IMAGE_STREAM_H_

#include "FreenectStream.h"


class FreenectImageStream : public FreenectStream
{
public:
	FreenectImageStream(Freenect::FreenectDevice* pDevice) : FreenectStream(pDevice)
	{}
	
	
	int getBytesPerPixel() { return sizeof(OniRGB888Pixel); }

	
	OniStatus getVideoMode(OniVideoMode* pVideoMode)
	{
		//pVideoMode->pixelFormat = format_convert(device->getVideoFormat());
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		pVideoMode->fps = 30;
		
		xnl::Pair<int, int> res = resolution_convert(device->getDepthResolution());		
		pVideoMode->resolutionX = res.first;
		pVideoMode->resolutionY = res.second;
		
		return ONI_STATUS_OK;
	}
	
	
	void buildFrame(void* data, uint32_t timestamp)
	{
		if (!running)
			return;
	}
};

#endif // _FREENECT_IMAGE_STREAM_H_
