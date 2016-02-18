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
#include "DepthKinectStream.h"

#include <Shlobj.h>
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"
#include "NuiApi.h"
#include "PS1080.h"
#include "D2S.h.h"
#include "S2D.h.h"
#include "KinectStreamImpl.h"
#include "KinectProperties.h"

using namespace oni::driver;
using namespace kinect_device;


static const XnInt MAX_SHIFT_VAL = 2047;
static const XnInt PARAM_COEFF_VAL = 4;
static const XnInt SHIFT_SCALE_VAL = 10;
static const XnInt GAIN_VAL = 42;
static const XnInt ZPD_VAL = 120;
static const XnInt CONST_SHIFT_VAL = 200;
static const XnInt DEVICE_MAX_DEPTH_VAL = 10000;
static const XnDouble ZPPS_VAL = 0.10520000010728836;
static const XnDouble LDDIS_VAL = 7.5;

#define DEFAULT_FPS 30

DepthKinectStream::DepthKinectStream(KinectStreamImpl* pStreamImpl):
					BaseKinectStream(pStreamImpl)
{
	m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_videoMode.fps = DEFAULT_FPS;
	m_videoMode.resolutionX = KINECT_RESOLUTION_X_640;
	m_videoMode.resolutionY = KINECT_RESOLUTION_Y_480;
}

// Discard the depth value equal or greater than the max value.
inline unsigned short filterReliableDepthValue(unsigned short value)
{
	return value < DEVICE_MAX_DEPTH_VAL ? value : 0;
}

// Populate the image-related metadata (resolution, cropping, etc.) to OniDriverFrame.
void DepthKinectStream::populateFrameImageMetadata(OniFrame* pFrame, int dataUnitSize)
{
	if (!m_cropping.enabled)
	{
		pFrame->height = m_videoMode.resolutionY;
		pFrame->width  = m_videoMode.resolutionX;
		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
	} else {
		pFrame->height = m_cropping.height;
		pFrame->width  = m_cropping.width;
		pFrame->cropOriginX = m_cropping.originX;
		pFrame->cropOriginY = m_cropping.originY;
		pFrame->croppingEnabled = TRUE;
	}
	pFrame->dataSize = pFrame->width * pFrame->height * dataUnitSize;
	pFrame->stride = pFrame->width * dataUnitSize;
	pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
	pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
	pFrame->videoMode.pixelFormat = m_videoMode.pixelFormat;
	pFrame->videoMode.fps = m_videoMode.fps;
}

// FIXME: For preliminary benchmarking purpose. Should not belong here.
static void recordAverageProcessTime(const char* message, LARGE_INTEGER* pAccTime, LARGE_INTEGER* pAccCount, const LARGE_INTEGER& startTime)
{
#if 0 // set to 1 to display the process time
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);
	pAccTime->QuadPart += endTime.QuadPart - startTime.QuadPart;
	pAccCount->QuadPart++;
	printf("%s: %lld\n", message, pAccTime->QuadPart / pAccCount->QuadPart);
#endif
}

// Copy the depth pixels (NUI_DEPTH_IMAGE_PIXEL) to OniDriverFrame
// with applying cropping but NO depth-to-image registration.
void DepthKinectStream::copyDepthPixelsStraight(const NUI_DEPTH_IMAGE_PIXEL* in, int numPoints, OniFrame* pFrame)
{
	// FIXME: for preliminary benchmarking purpose
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);

	struct DepthPixelCopier
	{
		void operator()(const NUI_DEPTH_IMAGE_PIXEL* const in, OniDepthPixel* const out)
		{
			*out = filterReliableDepthValue(in->depth);
		}
	};

	typedef LineCopier<NUI_DEPTH_IMAGE_PIXEL, OniDepthPixel, DepthPixelCopier, ForwardMover<NUI_DEPTH_IMAGE_PIXEL> > ForwardLineCopier;
	typedef RectCopier<NUI_DEPTH_IMAGE_PIXEL, OniDepthPixel, DepthPixelCopier, ForwardMover<NUI_DEPTH_IMAGE_PIXEL>, ForwardMover<NUI_DEPTH_IMAGE_PIXEL> > ForwardRectCopier;
	typedef RectCopier<NUI_DEPTH_IMAGE_PIXEL, OniDepthPixel, DepthPixelCopier, BackwardMover<NUI_DEPTH_IMAGE_PIXEL>, ForwardMover<NUI_DEPTH_IMAGE_PIXEL> > MirrorRectCopier;

	OniDepthPixel* out = reinterpret_cast<OniDepthPixel*>(pFrame->data);

	FrameCopier<NUI_DEPTH_IMAGE_PIXEL, OniDepthPixel, ForwardLineCopier, ForwardRectCopier, MirrorRectCopier> copyFrame;
	copyFrame(in, out, pFrame, m_videoMode, m_cropping, m_mirroring);

	// FIXME: for preliminary benchmarking purpose
	static LARGE_INTEGER accTime, accCount;
	recordAverageProcessTime("No Image Reg", &accTime, &accCount, startTime);
}

//
// Copy the depth pixels (NUI_DEPTH_IMAGE_PIXEL) to OniDriverFrame
// with applying cropping and depth-to-image registration.
//
// Note: The local variable assignments and const qualifiers are carefully designed to generate
// a high performance code with VC 2010. We recommend check the performance when changing the code
// even if it was a trivial change.
//

// Helper template
template<typename XTransformer>
struct DepthMapper
{
	void operator()(const NUI_DEPTH_IMAGE_PIXEL* const in, OniDepthPixel* const out,
		const int* mappedCoordsIter, const unsigned int numPoints, OniFrame* pFrame, XTransformer transformX)
	{
		const unsigned int width = pFrame->width;
		const unsigned int height = pFrame->height;
		const unsigned int minX = pFrame->cropOriginX;
		const unsigned int minY = pFrame->cropOriginY;

		for (unsigned int i = 0; i < numPoints; i++)
		{
			const unsigned int x = transformX(*mappedCoordsIter++) - minX;
			const unsigned int y = *mappedCoordsIter++ - minY;
			if (x < width - 1 && y < height) {
				const unsigned short d = filterReliableDepthValue((in+i)->depth);
				OniDepthPixel* p = out + x + y * width;
				if (*p == 0 || *p > d) *p = d;
				p++;
				if (*p == 0 || *p > d) *p = d;
			}
		}
	}
};

// Function body
void DepthKinectStream::copyDepthPixelsWithImageRegistration(const NUI_DEPTH_IMAGE_PIXEL* source, int numPoints, OniFrame* pFrame)
{
	// Note: We evaluated another possible implementation using INuiCoordinateMapper*::MapColorFrameToDepthFrame,
	// but, counterintuitively, it turned out to be slower than this implementation. We reverted it back.

	// FIXME: for preliminary benchmarking purpose
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);

	OniDepthPixel* const target = (OniDepthPixel*) pFrame->data;
	xnOSMemSet(target, 0, pFrame->dataSize);

	m_mappedCoordsBuffer.SetSize(numPoints * 2);

	HRESULT hr = m_pStreamImpl->convertDepthFrameToColorCoordinates(
		m_videoMode, // assume the target image with the same resolution as the source.
		source,
		numPoints,
		m_mappedCoordsBuffer.GetData()
		);
	XN_ASSERT(SUCCEEDED(hr));

	if (!m_mirroring) {
		struct ForwardXTransformer
		{
			unsigned int operator()(const unsigned int x) { return x; }
		};

		DepthMapper<ForwardXTransformer> mapDepth;
		mapDepth(source, target, m_mappedCoordsBuffer.GetData(), numPoints, pFrame, ForwardXTransformer());
	} else {
		struct BackwardXTransformer
		{
			const unsigned int m_width;

			BackwardXTransformer(unsigned int width) : m_width(width) {}

			// Note the range of x is 0 <= x <= width - 2
			unsigned int operator()(const unsigned int x) { return m_width - x - 2; }
		};

		DepthMapper<BackwardXTransformer> mapDepth;
		mapDepth(source, target, m_mappedCoordsBuffer.GetData(), numPoints, pFrame, BackwardXTransformer(pFrame->videoMode.resolutionX));
	}

	// FIXME: for preliminary benchmarking purpose
	static LARGE_INTEGER accTime, accCount;
	recordAverageProcessTime("With Image Reg", &accTime, &accCount, startTime);
}


void DepthKinectStream::frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT& LockedRect)
{
	OniFrame* pFrame = getServices().acquireFrame();

	// populate the video-related metadata
	populateFrameImageMetadata(pFrame, sizeof(unsigned short));

	// populate other frame info
	pFrame->sensorType = ONI_SENSOR_DEPTH;
	pFrame->frameIndex = imageFrame.dwFrameNumber;
	pFrame->timestamp = imageFrame.liTimeStamp.QuadPart*1000;

	// populate the pixel data
	int numPoints = m_videoMode.resolutionY * m_videoMode.resolutionX;
	if (m_pStreamImpl->getImageRegistrationMode() == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) {
		copyDepthPixelsWithImageRegistration((NUI_DEPTH_IMAGE_PIXEL*)LockedRect.pBits, numPoints, pFrame);
	} else {
		copyDepthPixelsStraight((NUI_DEPTH_IMAGE_PIXEL*)LockedRect.pBits, numPoints, pFrame);
	}

	raiseNewFrame(pFrame);
	getServices().releaseFrame(pFrame);
}

OniStatus DepthKinectStream::convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY)
{
	return m_pStreamImpl->convertDepthToColorCoordinates(colorStream, depthX, depthY, depthZ, pColorX, pColorY);
}

OniStatus DepthKinectStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	OniStatus status = ONI_STATUS_NOT_SUPPORTED;
	switch (propertyId)
	{
	case ONI_STREAM_PROPERTY_MAX_VALUE:
		{
			XnInt * val = (XnInt *)data;
			*val = DEVICE_MAX_DEPTH_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_CLOSE_RANGE:
	case XN_STREAM_PROPERTY_INPUT_FORMAT:
	case XN_STREAM_PROPERTY_CROPPING_MODE:
	case XN_STREAM_PROPERTY_PIXEL_REGISTRATION:
	case XN_STREAM_PROPERTY_WHITE_BALANCE_ENABLED:
		status = ONI_STATUS_NOT_SUPPORTED;
		break;
	case XN_STREAM_PROPERTY_GAIN:
		{
			XnInt* val = (XnInt*)data;
			*val = GAIN_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_HOLE_FILTER:
	case XN_STREAM_PROPERTY_REGISTRATION_TYPE:
	case XN_STREAM_PROPERTY_AGC_BIN:
		return ONI_STATUS_NOT_SUPPORTED;
	case XN_STREAM_PROPERTY_CONST_SHIFT:
		{
			XnInt* val = (XnInt*)data;
			*val = CONST_SHIFT_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_PIXEL_SIZE_FACTOR:
		status = ONI_STATUS_NOT_SUPPORTED;
		break;
	case XN_STREAM_PROPERTY_MAX_SHIFT:
		{
			XnInt* val = (XnInt*)data;
			*val = MAX_SHIFT_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_PARAM_COEFF:
		{
			XnInt* val = (XnInt*)data;
			*val = PARAM_COEFF_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_SHIFT_SCALE:
		{
			XnInt* val = (XnInt*)data;
			*val = SHIFT_SCALE_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE:
		{
			XnInt* val = (XnInt*)data;
			*val = ZPD_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE:
		{
			XnDouble* val = (XnDouble*)data;
			*val = ZPPS_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE:
		{
			XnDouble* val = (XnDouble*)data;
			*val = LDDIS_VAL;
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_DCMOS_RCMOS_DISTANCE:
		status = ONI_STATUS_NOT_SUPPORTED;
		break;
	case XN_STREAM_PROPERTY_S2D_TABLE:
		{
			*pDataSize = sizeof(S2D);
			xnOSMemCopy(data, S2D, sizeof(S2D));
			status = ONI_STATUS_OK;
			break;
		}
	case XN_STREAM_PROPERTY_D2S_TABLE:
		{
			*pDataSize = sizeof(D2S);
			xnOSMemCopy(data, D2S, sizeof(D2S));
			status = ONI_STATUS_OK;
			break;
		}
	case KINECT_DEPTH_PROPERTY_CLOSE_RANGE:
		{
			EXACT_PROP_SIZE_OR_RETURN(*pDataSize, OniBool);
			status = getNearMode((OniBool*)data);
			break;
		}
	default:
		status = BaseKinectStream::getProperty(propertyId, data, pDataSize);
		break;
	}

	return status;
}

OniStatus DepthKinectStream::setProperty(int propertyId, const void* data, int dataSize)
{
	OniStatus status = ONI_STATUS_NOT_SUPPORTED;
	switch (propertyId)
	{
	case KINECT_DEPTH_PROPERTY_CLOSE_RANGE:
		{
			EXACT_PROP_SIZE_OR_RETURN(dataSize, OniBool);
			status = setNearMode(*(OniBool*)data);
			break;
		}
	default:
		status = BaseKinectStream::setProperty(propertyId, data, dataSize);
		break;
	}

	return status;
}

OniBool DepthKinectStream::isPropertySupported(int propertyId)
{
	OniBool status = FALSE;
	switch (propertyId)
	{
	case ONI_STREAM_PROPERTY_MAX_VALUE:
	case ONI_STREAM_PROPERTY_MIRRORING:
	case XN_STREAM_PROPERTY_GAIN:
	case XN_STREAM_PROPERTY_CONST_SHIFT:
	case XN_STREAM_PROPERTY_MAX_SHIFT:
	case XN_STREAM_PROPERTY_PARAM_COEFF:
	case XN_STREAM_PROPERTY_SHIFT_SCALE:
	case XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE:
	case XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE:
	case XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE:
	case XN_STREAM_PROPERTY_S2D_TABLE:
	case XN_STREAM_PROPERTY_D2S_TABLE:
	case KINECT_DEPTH_PROPERTY_CLOSE_RANGE:
		status = TRUE;
	default:
		status = BaseKinectStream::isPropertySupported(propertyId);
		break;
	}
	return status;
}

void DepthKinectStream::notifyAllProperties()
{
	XnDouble nDouble;
	int size = sizeof(nDouble);
	getProperty(XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE, &nDouble, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE, &nDouble, size);

	getProperty(XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE, &nDouble, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE, &nDouble, size);

	XnInt nInt;
	size = sizeof(nInt);
	getProperty(XN_STREAM_PROPERTY_GAIN, &nInt, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_GAIN, &nInt, size);

	getProperty(XN_STREAM_PROPERTY_CONST_SHIFT, &nInt, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_CONST_SHIFT, &nInt, size);

	getProperty(XN_STREAM_PROPERTY_MAX_SHIFT, &nInt, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_MAX_SHIFT, &nInt, size);

	getProperty(XN_STREAM_PROPERTY_SHIFT_SCALE, &nInt, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_SHIFT_SCALE, &nInt, size);

	getProperty(XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE, &nInt, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE, &nInt, size);

	getProperty(ONI_STREAM_PROPERTY_MAX_VALUE, &nInt, &size);
	raisePropertyChanged(ONI_STREAM_PROPERTY_MAX_VALUE, &nInt, size);

	unsigned short nBuff[10001];
	size = sizeof(S2D);
	getProperty(XN_STREAM_PROPERTY_S2D_TABLE, nBuff, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_S2D_TABLE, nBuff, size);
	
	size = sizeof(D2S);
	getProperty(XN_STREAM_PROPERTY_D2S_TABLE, nBuff, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_D2S_TABLE, nBuff, size);

	size = sizeof(PARAM_COEFF_VAL);
	getProperty(XN_STREAM_PROPERTY_PARAM_COEFF, nBuff, &size);
	raisePropertyChanged(XN_STREAM_PROPERTY_PARAM_COEFF, nBuff, size);

	// Is this really necessary to save? Just in case.
	OniBool nearMode;
	size = sizeof(nearMode);
	getProperty(KINECT_DEPTH_PROPERTY_CLOSE_RANGE, &nearMode, &size);
	raisePropertyChanged(KINECT_DEPTH_PROPERTY_CLOSE_RANGE, &nearMode, size);

	BaseKinectStream::notifyAllProperties();
}

OniStatus DepthKinectStream::setNearMode(OniBool value)
{
	return m_pStreamImpl->setImageFrameFlags(NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE, value);
}

OniStatus DepthKinectStream::getNearMode(OniBool* pValue)
{
	*pValue = m_pStreamImpl->getImageFrameFlags(NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);
	return ONI_STATUS_OK;
}
