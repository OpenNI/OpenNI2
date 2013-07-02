#include "ColorKinectStream.h"

#include <Shlobj.h>
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"

#include "KinectStreamImpl.h"

#include "NuiApi.h"

using namespace oni::driver;
using namespace kinect_device;
#define DEFAULT_FPS 30
ColorKinectStream::ColorKinectStream(KinectStreamImpl* pStreamImpl):
		BaseKinectStream(pStreamImpl)
{
	m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_videoMode.fps         = DEFAULT_FPS;
	m_videoMode.resolutionX = KINECT_RESOLUTION_X_640;
	m_videoMode.resolutionY = KINECT_RESOLUTION_Y_480;
}

OniStatus ColorKinectStream::start()
{
	OniStatus status = ONI_STATUS_ERROR;
	if (m_pStreamImpl->getSensorType() == ONI_SENSOR_COLOR || m_pStreamImpl->isRunning() == false)
	{
		m_pStreamImpl->setSensorType(ONI_SENSOR_COLOR);
		status = m_pStreamImpl->start();
		if (status == ONI_STATUS_OK)
			m_running = TRUE;
		
	}
	return status;
}

void ColorKinectStream::frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect)
{
	OniFrame* pFrame = getServices().acquireFrame();
	if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_RGB888)
	{
		struct Rgba { unsigned char b, g, r, a; };
		struct Rgb { unsigned char r, g, b;    };
		if (!m_cropping.enabled)
		{
			Rgba* data_in = reinterpret_cast<Rgba*>(LockedRect.pBits);
			Rgb* data_out = reinterpret_cast<Rgb*>(pFrame->data);
			pFrame->dataSize = m_videoMode.resolutionY * m_videoMode.resolutionX * 3;
			Rgba * data_in_end = data_in + (m_videoMode.resolutionY * m_videoMode.resolutionX);
			while (data_in < data_in_end)
			{
				data_out->b = data_in->b;
				data_out->r = data_in->r;
				data_out->g = data_in->g;
				++data_in;
				++data_out;
			}
			pFrame->stride = m_videoMode.resolutionX * 3;
		}
		else
		{
			Rgba* data_in = reinterpret_cast<Rgba*>(LockedRect.pBits);
			Rgb* data_out = reinterpret_cast<Rgb*>(pFrame->data);
			pFrame->dataSize = m_cropping.height * m_cropping.width * 3;
			int cropX = m_cropping.originX;
			int cropY = m_cropping.originY;
			while (cropY < m_cropping.originY + m_cropping.height)
			{
				while (cropX < m_cropping.originX + m_cropping.width)
				{
					Rgba* iter = data_in + (cropX + m_videoMode.resolutionX * cropY);
					unsigned char c = iter->b;
					data_out->b = c;
					data_out->r = iter->r;
					data_out->g = iter->g;
					++data_out;
					++cropX;
				}
				++cropY;
				cropX = m_cropping.originX;
			}
			pFrame->stride = m_cropping.width * 3;
		}
	}
	else
	{
		if (!m_cropping.enabled)
		{
			xnOSMemCopy(pFrame->data, LockedRect.pBits, LockedRect.size);
			pFrame->dataSize = LockedRect.size;
			pFrame->stride = m_videoMode.resolutionX * 2;
		}
		else
		{
			unsigned short* data_in = reinterpret_cast<unsigned short*>(LockedRect.pBits);
			unsigned short* data_out = reinterpret_cast<unsigned short*>(pFrame->data);
			pFrame->dataSize = m_cropping.height * m_cropping.width * 2;

			int cropX = m_cropping.originX;
			int cropY = m_cropping.originY;
			while (cropY < m_cropping.originY + m_cropping.height)
			{
				while (cropX < m_cropping.originX + m_cropping.width)
				{
					unsigned short* iter = data_in + (cropX + m_videoMode.resolutionX * cropY);
					*data_out = *iter;
					++data_out;
					++cropX;
				}
				cropY++;
				cropX = m_cropping.originX;
			}
			pFrame->stride = m_cropping.width * 2;

		}
	}
	pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
	pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
	if (m_cropping.enabled)
	{
		pFrame->width = m_cropping.width;
		pFrame->height = m_cropping.height;
		pFrame->cropOriginX = m_cropping.originX; 
		pFrame->cropOriginY = m_cropping.originY;
		pFrame->croppingEnabled = m_cropping.enabled;
	}
	else
	{
		pFrame->cropOriginX = 0; 
		pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = m_cropping.enabled;
		pFrame->width = m_videoMode.resolutionX;
		pFrame->height = m_videoMode.resolutionY;
	}
	pFrame->videoMode.pixelFormat = m_videoMode.pixelFormat;
	pFrame->videoMode.fps = m_videoMode.fps;
	pFrame->sensorType = ONI_SENSOR_COLOR;
	pFrame->frameIndex = imageFrame.dwFrameNumber;
	pFrame->timestamp = imageFrame.liTimeStamp.QuadPart*1000;
	raiseNewFrame(pFrame);
	getServices().releaseFrame(pFrame);
}

OniStatus ColorKinectStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	OniStatus status = ONI_STATUS_NOT_IMPLEMENTED;
	switch (propertyId)
	{
		case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
			status = m_pStreamImpl->getAutoWhitBalance((BOOL*)data);
			break;
		case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
			status = m_pStreamImpl->getAutoExposure((BOOL*)data);
			break;
		default:
			status = BaseKinectStream::getProperty(propertyId, data, pDataSize);
	}
	return status;
}

OniStatus ColorKinectStream::setProperty(int propertyId, const void* data, int pDataSize)
{
	OniStatus status = ONI_STATUS_NOT_IMPLEMENTED;
	switch (propertyId)
	{
	case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
		status = m_pStreamImpl->setAutoWhiteBalance(*((BOOL*)data));
		break;
	case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
		status = m_pStreamImpl->setAutoExposure(*((BOOL*)data));
		break;
	default:
		status = BaseKinectStream::setProperty(propertyId, data, pDataSize);
	}
	return status;
}

OniBool ColorKinectStream::isPropertySupported(int propertyId)
{
	OniBool status = FALSE;
	switch (propertyId)
	{
	case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
	case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
		status = TRUE;
		break;
	default:
		status = BaseKinectStream::isPropertySupported(propertyId);
	}
	return status;
}
