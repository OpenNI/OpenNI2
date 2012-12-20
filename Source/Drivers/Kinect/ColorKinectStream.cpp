#include "ColorKinectStream.h"

#include <Shlobj.h>
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"

#include "NuiApi.h"

using namespace oni::driver;
using namespace kinect_device;
#define DEFAULT_FPS 30
ColorKinectStream::ColorKinectStream(KinectStreamImpl* pStreamImpl):
		BaseKinectStream(pStreamImpl)
{
	m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_videoMode.fps         = DEFAULT_FPS;
	m_videoMode.resolutionX = KINNECT_RESOLUTION_X_640;
	m_videoMode.resolutionY = KINNECT_RESOLUTION_Y_480;
}

void ColorKinectStream::frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect)
{
	OniDriverFrame* pFrame = NULL;
	pFrame = (OniDriverFrame*)xnOSCalloc(1, sizeof(OniDriverFrame));
	pFrame->frame.dataSize = m_videoMode.resolutionY * m_videoMode.resolutionX * 3;
	pFrame->frame.data =  xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);
	pFrame->pDriverCookie = xnOSMalloc(sizeof(KinectStreamFrameCookie));
	((KinectStreamFrameCookie*)pFrame->pDriverCookie)->refCount = 1;
	if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_RGB888)
	{
		struct Rgba { unsigned char b, g, r, a; };
		struct Rgb { unsigned char r, g, b;    };
		Rgba* data_in = reinterpret_cast<Rgba*>(LockedRect.pBits);
		Rgb* data_out = reinterpret_cast<Rgb*>(pFrame->frame.data);
		Rgba * data_in_end = data_in + (m_videoMode.resolutionY * m_videoMode.resolutionX);
		while (data_in < data_in_end)
		{
			data_out->b = data_in->b;
			data_out->r = data_in->r;
			data_out->g = data_in->g;
			++data_in;
			++data_out;
		}
		pFrame->frame.stride = m_videoMode.resolutionX * 3;
	}
	else
	{
		xnOSMemCopy(pFrame->frame.data, LockedRect.pBits, LockedRect.size);
		pFrame->frame.dataSize = LockedRect.size;
		pFrame->frame.stride = m_videoMode.resolutionX * 2;
	}
	pFrame->frame.width = pFrame->frame.videoMode.resolutionX = m_videoMode.resolutionX;
	pFrame->frame.height = pFrame->frame.videoMode.resolutionY = m_videoMode.resolutionY;
	pFrame->frame.videoMode.pixelFormat = m_videoMode.pixelFormat;
	pFrame->frame.videoMode.fps = m_videoMode.fps;
	pFrame->frame.sensorType = ONI_SENSOR_COLOR;
	pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
	pFrame->frame.croppingEnabled = FALSE;
	pFrame->frame.frameIndex = imageFrame.dwFrameNumber;
	pFrame->frame.timestamp = imageFrame.liTimeStamp.QuadPart*1000;
	raiseNewFrame(pFrame);
}


