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
	m_videoMode.resolutionX = KINECT_RESOLUTION_X_640;
	m_videoMode.resolutionY = KINECT_RESOLUTION_Y_480;
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
	OniFrame* pFrame = getServices().acquireFrame();
	if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_GRAY16)
	{
		unsigned short* data_in = reinterpret_cast<unsigned short*>(LockedRect.pBits);
		unsigned short* data_out = reinterpret_cast<unsigned short*>(pFrame->data);
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
			pFrame->stride = m_videoMode.resolutionX * 2;
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
			pFrame->stride = m_cropping.width * 2;
		}
	}
	else if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_GRAY8)
	{
		unsigned short* data_in = reinterpret_cast<unsigned short*>(LockedRect.pBits);
		char* data_out = reinterpret_cast<char*>(pFrame->data);
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
			pFrame->stride = m_videoMode.resolutionX;
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
			pFrame->stride = m_cropping.width * 2;

		}
	}
	else
	{
		struct Rgb { unsigned char r, g, b;    };
		unsigned short* data_in = reinterpret_cast<unsigned short*>(LockedRect.pBits);
		Rgb* data_out = reinterpret_cast<Rgb*>(pFrame->data);
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
			pFrame->stride = m_videoMode.resolutionX * 3;
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
			pFrame->stride = m_cropping.width * 3;
		}
	}
		
	if (!m_cropping.enabled)
	{
		pFrame->width = pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
		pFrame->height = pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
	}
	else
	{
		pFrame->width = m_cropping.width;
		pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
		pFrame->height = m_cropping.height; 
		pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
		pFrame->cropOriginX = m_cropping.originX;
		pFrame->cropOriginY = m_cropping.originY;
		pFrame->croppingEnabled = TRUE;
	}
	pFrame->videoMode.pixelFormat = m_videoMode.pixelFormat;
	pFrame->videoMode.fps = m_videoMode.fps;
	pFrame->sensorType = ONI_SENSOR_IR;
	pFrame->frameIndex = imageFrame.dwFrameNumber;
	pFrame->timestamp = imageFrame.liTimeStamp.QuadPart*1000;
	raiseNewFrame(pFrame);
	getServices().releaseFrame(pFrame);
}


