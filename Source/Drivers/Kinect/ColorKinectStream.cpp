/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
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
		copyFrameRGB888(pFrame, imageFrame, LockedRect);
	}
	else if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_YUV422)
	{
		copyFrameYUV422(pFrame, imageFrame, LockedRect);
	}
	else
	{
		XN_ASSERT(FALSE); // unsupported format. should not come here.
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

void ColorKinectStream::copyFrameRGB888(OniFrame* pFrame, NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect)
{
	struct Rgba { unsigned char b, g, r, a; };

	struct RgbaToRgbPixelCopier
	{
		void operator()(const Rgba* const in, OniRGB888Pixel* const out)
		{
			out->r = in->r;
			out->g = in->g;
			out->b = in->b;
		}
	};

	typedef LineCopier<Rgba, OniRGB888Pixel, RgbaToRgbPixelCopier, ForwardMover<Rgba> > ForwardLineCopier;
	typedef RectCopier<Rgba, OniRGB888Pixel, RgbaToRgbPixelCopier, ForwardMover<Rgba>, ForwardMover<Rgba> > ForwardRectCopier;
	typedef RectCopier<Rgba, OniRGB888Pixel, RgbaToRgbPixelCopier, BackwardMover<Rgba>, ForwardMover<Rgba> > MirrorRectCopier;

	Rgba* in = reinterpret_cast<Rgba*>(LockedRect.pBits);
	OniRGB888Pixel* out = reinterpret_cast<OniRGB888Pixel*>(pFrame->data);

	FrameCopier<Rgba, OniRGB888Pixel, ForwardLineCopier, ForwardRectCopier, MirrorRectCopier> copyFrame;
	copyFrame(in, out, pFrame, m_videoMode, m_cropping, m_mirroring);
}

void ColorKinectStream::copyFrameYUV422(OniFrame* pFrame, NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect)
{
	// I YUV422, two pixels are packed into a 4-byte sequence.
	// We need a little tricky mapping implementation for mirror mode.

	OniYUV422DoublePixel* in = reinterpret_cast<OniYUV422DoublePixel*>(LockedRect.pBits);
	OniYUV422DoublePixel* out = reinterpret_cast<OniYUV422DoublePixel*>(pFrame->data);

	int resolutionXInPackets = m_videoMode.resolutionX / 2;

	struct SwappedYUV422PixelCopier
	{
		void operator()(const OniYUV422DoublePixel* const in, OniYUV422DoublePixel* const out)
		{
			out->u = in->u;
			out->y1 = in->y2;
			out->v = in->v;
			out->y2 = in->y1;
		}
	};

	typedef RectCopier<OniYUV422DoublePixel, OniYUV422DoublePixel, PixelCopier<OniYUV422DoublePixel, OniYUV422DoublePixel>,
		ForwardMover<OniYUV422DoublePixel>, ForwardMover<OniYUV422DoublePixel> > ForwardRectCopier;
	typedef RectCopier<OniYUV422DoublePixel, OniYUV422DoublePixel, SwappedYUV422PixelCopier,
		BackwardMover<OniYUV422DoublePixel>, ForwardMover<OniYUV422DoublePixel> > MirrorRectCopier;

	if (!m_cropping.enabled)
	{
		pFrame->dataSize = LockedRect.size;
		pFrame->stride = m_videoMode.resolutionX * 2;

		if (!m_mirroring)
		{
			// optimized copy
			xnOSMemCopy(pFrame->data, LockedRect.pBits, LockedRect.size);
		} else {
			MirrorRectCopier copyRect;
			copyRect(in + resolutionXInPackets - 1, out,
				resolutionXInPackets, resolutionXInPackets, m_videoMode.resolutionY);
		}
	}
	else
	{
		int pixelCount = m_cropping.height * m_cropping.width;
		pFrame->dataSize = pixelCount * 2;
		pFrame->stride = m_cropping.width * 2;

		int originXInPackets = m_cropping.originX / 2;
		int copyWidthInPackets = m_cropping.width / 2;

		if (!m_mirroring)
		{
			ForwardRectCopier copyRect;
			OniYUV422DoublePixel* origin = in + m_cropping.originY * resolutionXInPackets + originXInPackets;
			copyRect(origin, out, copyWidthInPackets, resolutionXInPackets, m_cropping.height);
		}
		else
		{
			MirrorRectCopier copyRect;
			OniYUV422DoublePixel* origin = in + (m_cropping.originY+1) * resolutionXInPackets - originXInPackets - 1;
			copyRect(origin, out, copyWidthInPackets, resolutionXInPackets, m_cropping.height);
		}
	}
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
