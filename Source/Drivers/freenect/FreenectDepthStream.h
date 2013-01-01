#ifndef _FREENECT_DEPTH_STREAM_H_
#define _FREENECT_DEPTH_STREAM_H_

#include "FreenectStream.h"


class FreenectDepthStream : public FreenectStream
{	
public:
	FreenectDepthStream(Freenect::FreenectDevice* pDevice) : FreenectStream(pDevice)
	{}
	

	int getBytesPerPixel() { return sizeof(OniDepthPixel); }
	
	OniStatus getVideoMode(OniVideoMode* pVideoMode)
	{
		//pVideoMode->pixelFormat = format_convert(device->getVideoFormat());
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
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
		
		xnl::Pair<int, int> res = resolution_convert(device->getDepthResolution());
		int TEST_RESOLUTION_X = res.first;
		int TEST_RESOLUTION_Y = res.second;
		
		
		OniDriverFrame* pFrame = (OniDriverFrame*)xnOSCalloc(1, sizeof(OniDriverFrame));
		if (pFrame == NULL)
		{
			XN_ASSERT(FALSE);
			return;
		}

		int dataSize = res.first * res.second * getBytesPerPixel();
		pFrame->frame.data = xnOSMallocAligned(dataSize, XN_DEFAULT_MEM_ALIGN);
		if (pFrame->frame.data == NULL)
		{
			XN_ASSERT(FALSE);
			return;
		}

		pFrame->pDriverCookie = xnOSMalloc(sizeof(FreenectStreamFrameCookie));
		((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount = 1;


		pFrame->frame.frameIndex = frameId++;
		pFrame->frame.dataSize = dataSize;
		pFrame->frame.sensorType = ONI_SENSOR_DEPTH;
		pFrame->frame.stride = TEST_RESOLUTION_X*sizeof(OniDepthPixel);
		//pFrame->frame.timestamp = m_frameId*33000;
		pFrame->frame.timestamp = timestamp;

		pFrame->frame.videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		pFrame->frame.videoMode.resolutionX = TEST_RESOLUTION_X;
		pFrame->frame.videoMode.resolutionY = TEST_RESOLUTION_Y;
		pFrame->frame.videoMode.fps = 30;

		pFrame->frame.width = TEST_RESOLUTION_X;
		pFrame->frame.height = TEST_RESOLUTION_Y;

		pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
		pFrame->frame.croppingEnabled = FALSE;

		
		raiseNewFrame(pFrame);
	}
	
};

#endif // _FREENECT_DEPTH_STREAM_H_
