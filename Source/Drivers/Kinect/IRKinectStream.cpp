#include "IRKinectStream.h"

#include <Shlobj.h>
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"
#include "KinectStreamImpl.h"
#include "NuiApi.h"

using namespace oni::driver;
using namespace kinect_device;

#define DEFAULT_FPS 30

IRKinectStream::IRKinectStream(KinectStreamImpl* pStreamImpl):
		BaseKinectStream(pStreamImpl)
{
	m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
	m_videoMode.fps = DEFAULT_FPS;
	m_videoMode.resolutionX = KINNECT_RESOLUTION_X_640;
	m_videoMode.resolutionY = KINNECT_RESOLUTION_Y_480;
}

OniStatus IRKinectStream::start()
{
	OniStatus status = ONI_STATUS_ERROR;
	if (m_pStreamImpl->getSensorType() == ONI_SENSOR_IR || m_pStreamImpl->isRunning() == false)
	{
		m_pStreamImpl->setSensorType(ONI_SENSOR_IR);
		status = m_pStreamImpl->start();
		if (status == ONI_STATUS_OK)
			m_running = TRUE;
		
	}
	return status;
}


void IRKinectStream::frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect)
{
	OniDriverFrame* pFrame = NULL;
	pFrame = (OniDriverFrame*)xnOSCalloc(1, sizeof(OniDriverFrame));
	pFrame->pDriverCookie = xnOSMalloc(sizeof(KinectStreamFrameCookie));
	((KinectStreamFrameCookie*)pFrame->pDriverCookie)->refCount = 1;
	if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_GRAY16)
	{
		pFrame->frame.dataSize = m_videoMode.resolutionY * m_videoMode.resolutionX * 2;
		pFrame->frame.data =  xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);

		unsigned short* data_in = reinterpret_cast<unsigned short*>(LockedRect.pBits);
		unsigned short* data_out = reinterpret_cast<unsigned short*>(pFrame->frame.data);
		if (!m_cropping.enabled)
		{
			unsigned short* data_in_end = data_in + (m_videoMode.resolutionY * m_videoMode.resolutionX);
			while (data_in < data_in_end)
			{
				unsigned short intensity = (*data_in)>> 6;
				*data_out = intensity;
				++data_in;
				++data_out;
			}
			pFrame->frame.stride = m_videoMode.resolutionX * 2;
		}
		else
		{
			int cropY = m_cropping.originY;
			while (cropY < m_cropping.originY + m_cropping.height)
			{
				int cropX = m_cropping.originX;
				while (cropX < m_cropping.originX + m_cropping.width)
				{
					unsigned short *iter = data_in + (cropY * m_videoMode.resolutionX + cropX++);
					*(data_out++) = (*iter) >> 6;
				}
				cropY++;
			}
			pFrame->frame.stride = m_cropping.width * 2;
		}
	}
	else if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_GRAY8)
	{
		pFrame->frame.dataSize = m_videoMode.resolutionY * m_videoMode.resolutionX;
		pFrame->frame.data =  xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);

		unsigned short* data_in = reinterpret_cast<unsigned short*>(LockedRect.pBits);
		char* data_out = reinterpret_cast<char*>(pFrame->frame.data);
		if (!m_cropping.enabled)
		{
			unsigned short* data_in_end = data_in + (m_videoMode.resolutionY * m_videoMode.resolutionX);
			while (data_in < data_in_end)
			{
				char intensity = (*data_in)>> 8;
				(*data_out) = intensity;
				++data_in;
				++data_out;
			}
			pFrame->frame.stride = m_videoMode.resolutionX;
		}
		else
		{
			int cropY = m_cropping.originY;
			while (cropY < m_cropping.originY + m_cropping.height)
			{
				int cropX = m_cropping.originX;
				while (cropX < m_cropping.originX + m_cropping.width)
				{
					unsigned short *iter = data_in + (cropY * m_videoMode.resolutionX + cropX++);
					char intensity = (*iter) >> 8;
					*(data_out++) = intensity;
				}
				cropY++;
			}
			pFrame->frame.stride = m_cropping.width * 2;

		}
	}
	else
	{
		pFrame->frame.dataSize = m_videoMode.resolutionY * m_videoMode.resolutionX * 3;
		pFrame->frame.data =  xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);

		struct Rgb { unsigned char r, g, b;    };
		unsigned short* data_in = reinterpret_cast<unsigned short*>(LockedRect.pBits);
		Rgb* data_out = reinterpret_cast<Rgb*>(pFrame->frame.data);
		if (!m_cropping.enabled)
		{
			unsigned short* data_in_end = data_in + (m_videoMode.resolutionY * m_videoMode.resolutionX);
			while (data_in < data_in_end)
			{
				char intensity = (*data_in)>> 8;
				data_out->b = intensity;
				data_out->r = intensity;
				data_out->g = intensity;
				++data_in;
				++data_out;
			}
			pFrame->frame.stride = m_videoMode.resolutionX * 3;
		}
		else
		{
			int cropY = m_cropping.originY;
			while (cropY < m_cropping.originY + m_cropping.height)
			{
				int cropX = m_cropping.originX;
				while (cropX < m_cropping.originX + m_cropping.width)
				{
					unsigned short *iter = data_in + (cropY * m_videoMode.resolutionX + cropX++);
					char intensity = (*iter) >> 8;
					data_out->b = intensity;
					data_out->r = intensity;
					data_out++->g = intensity;
				}
				cropY++;
			}
			pFrame->frame.stride = m_cropping.width * 3;
		}
	}
		
	if (!m_cropping.enabled)
	{
		pFrame->frame.width = pFrame->frame.videoMode.resolutionX = m_videoMode.resolutionX;
		pFrame->frame.height = pFrame->frame.videoMode.resolutionY = m_videoMode.resolutionY;
		pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
		pFrame->frame.croppingEnabled = FALSE;
	}
	else
	{
		pFrame->frame.width = m_cropping.width;
		pFrame->frame.videoMode.resolutionX = m_videoMode.resolutionX;
		pFrame->frame.height = m_cropping.height; 
		pFrame->frame.videoMode.resolutionY = m_videoMode.resolutionY;
		pFrame->frame.cropOriginX = m_cropping.originX;
		pFrame->frame.cropOriginY = m_cropping.originY;
		pFrame->frame.croppingEnabled = TRUE;
	}
	pFrame->frame.videoMode.pixelFormat = m_videoMode.pixelFormat;
	pFrame->frame.videoMode.fps = m_videoMode.fps;
	pFrame->frame.sensorType = ONI_SENSOR_IR;
	pFrame->frame.frameIndex = imageFrame.dwFrameNumber;
	pFrame->frame.timestamp = imageFrame.liTimeStamp.QuadPart*1000;
	raiseNewFrame(pFrame);
}


