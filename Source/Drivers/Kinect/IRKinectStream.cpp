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
		struct IRToGray16PixelCopier
		{
			void operator()(const USHORT* const in, OniGrayscale16Pixel* const out)
			{
				*out = (*in) >> 6;
			}
		};

		typedef LineCopier<USHORT, OniGrayscale16Pixel, IRToGray16PixelCopier, ForwardMover<USHORT> > ForwardLineCopier;
		typedef RectCopier<USHORT, OniGrayscale16Pixel, IRToGray16PixelCopier, ForwardMover<USHORT>, ForwardMover<USHORT> > ForwardRectCopier;
		typedef RectCopier<USHORT, OniGrayscale16Pixel, IRToGray16PixelCopier, BackwardMover<USHORT>, ForwardMover<USHORT> > MirrorRectCopier;

		USHORT* in = reinterpret_cast<USHORT*>(LockedRect.pBits);
		OniGrayscale16Pixel* out = reinterpret_cast<OniGrayscale16Pixel*>(pFrame->data);

		FrameCopier<USHORT, OniGrayscale16Pixel, ForwardLineCopier, ForwardRectCopier, MirrorRectCopier> copyFrame;
		copyFrame(in, out, pFrame, m_videoMode, m_cropping, m_mirroring);
	}
	else if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_GRAY8)
	{
		struct IRToGray8PixelCopier
		{
			void operator()(const USHORT* const in, OniGrayscale8Pixel* const out)
			{
				*out = (*in) >> 8;
			}
		};

		typedef LineCopier<USHORT, OniGrayscale8Pixel, IRToGray8PixelCopier, ForwardMover<USHORT> > ForwardLineCopier;
		typedef RectCopier<USHORT, OniGrayscale8Pixel, IRToGray8PixelCopier, ForwardMover<USHORT>, ForwardMover<USHORT> > ForwardRectCopier;
		typedef RectCopier<USHORT, OniGrayscale8Pixel, IRToGray8PixelCopier, BackwardMover<USHORT>, ForwardMover<USHORT> > MirrorRectCopier;

		USHORT* in = reinterpret_cast<USHORT*>(LockedRect.pBits);
		OniGrayscale8Pixel* out = reinterpret_cast<OniGrayscale8Pixel*>(pFrame->data);

		FrameCopier<USHORT, OniGrayscale8Pixel, ForwardLineCopier, ForwardRectCopier, MirrorRectCopier> copyFrame;
		copyFrame(in, out, pFrame, m_videoMode, m_cropping, m_mirroring);
	}
	else if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_RGB888)
	{
		struct IRToRgbPixelCopier
		{
			void operator()(const USHORT* const in, OniRGB888Pixel* const out)
			{
				BYTE intensity = (*in) >> 8;
				out->r = intensity;
				out->g = intensity;
				out->b = intensity;
			}
		};

		typedef LineCopier<USHORT, OniRGB888Pixel, IRToRgbPixelCopier, ForwardMover<USHORT> > ForwardLineCopier;
		typedef RectCopier<USHORT, OniRGB888Pixel, IRToRgbPixelCopier, ForwardMover<USHORT>, ForwardMover<USHORT> > ForwardRectCopier;
		typedef RectCopier<USHORT, OniRGB888Pixel, IRToRgbPixelCopier, BackwardMover<USHORT>, ForwardMover<USHORT> > MirrorRectCopier;

		USHORT* in = reinterpret_cast<USHORT*>(LockedRect.pBits);
		OniRGB888Pixel* out = reinterpret_cast<OniRGB888Pixel*>(pFrame->data);

		FrameCopier<USHORT, OniRGB888Pixel, ForwardLineCopier, ForwardRectCopier, MirrorRectCopier> copyFrame;
		copyFrame(in, out, pFrame, m_videoMode, m_cropping, m_mirroring);
	}
	else
	{
		XN_ASSERT(FALSE); // unsupported format. should not come here.
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


