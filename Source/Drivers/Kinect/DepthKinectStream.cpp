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
	m_videoMode.resolutionX = KINNECT_RESOLUTION_X_640;
	m_videoMode.resolutionY = KINNECT_RESOLUTION_Y_480;
}

// Discard the depth value equal or greater than the max value.
inline unsigned short filterReliableDepthValue(unsigned short value)
{
	return value < DEVICE_MAX_DEPTH_VAL ? value : 0;
}

// Populate the image-related metadata (resolution, cropping, etc.) to OniDriverFrame.
void DepthKinectStream::populateFrameImageMetadata(OniDriverFrame* pFrame, int dataUnitSize)
{
	if (!m_cropping.enabled)
	{
		pFrame->frame.height = m_videoMode.resolutionY;
		pFrame->frame.width  = m_videoMode.resolutionX;
		pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
		pFrame->frame.croppingEnabled = FALSE;
	} else {
		pFrame->frame.height = m_cropping.height;
		pFrame->frame.width  = m_cropping.width;
		pFrame->frame.cropOriginX = m_cropping.originX;
		pFrame->frame.cropOriginY = m_cropping.originY;
		pFrame->frame.croppingEnabled = TRUE;
	}
	pFrame->frame.stride = pFrame->frame.width * dataUnitSize;
	pFrame->frame.videoMode.resolutionY = m_videoMode.resolutionY;
	pFrame->frame.videoMode.resolutionX = m_videoMode.resolutionX;
	pFrame->frame.videoMode.pixelFormat = m_videoMode.pixelFormat;
	pFrame->frame.videoMode.fps = m_videoMode.fps;
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
void DepthKinectStream::copyDepthPixelsStraight(const NUI_DEPTH_IMAGE_PIXEL* source, int numPoints, OniDriverFrame* pFrame)
{
	// Note: The local variable assignments and const qualifiers are carefully designed to generate
	// a high performance code with VC 2010. We recommend check the performance when changing the code
	// even if it was a trivial change.

	// For benchmarking purpose
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);

	unsigned short* target = (unsigned short*) pFrame->frame.data;

	const unsigned int width = pFrame->frame.width;
	const unsigned int height = pFrame->frame.height;
	const unsigned int skipWidth = m_videoMode.resolutionX - width;

	// Offset the starting position
	source += pFrame->frame.cropOriginX + pFrame->frame.cropOriginY * m_videoMode.resolutionX;

	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			*(target++) = filterReliableDepthValue((source++)->depth);
		}
		source += skipWidth;
	}

	// FIXME: for preliminary benchmarking purpose
	static LARGE_INTEGER accTime, accCount;
	recordAverageProcessTime("No Image Reg", &accTime, &accCount, startTime);
}

// Copy the depth pixels (NUI_DEPTH_IMAGE_PIXEL) to OniDriverFrame
// with applying cropping and depth-to-image registration.
void DepthKinectStream::copyDepthPixelsWithImageRegistration(const NUI_DEPTH_IMAGE_PIXEL* source, int numPoints, OniDriverFrame* pFrame)
{
	// Note: We evaluated another possible implementation using INuiCoordinateMapper*::MapColorFrameToDepthFrame,
	// but, counterintuitively, it turned out to be slower than this implementation. We reverted it back.

	// Note: The local variable assignments and const qualifiers are carefully designed to generate
	// a high performance code with VC 2010. We recommend check the performance when changing the code
	// even if it was a trivial change.

	// For benchmarking purpose
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);

	NUI_IMAGE_RESOLUTION nuiResolution =
		m_pStreamImpl->getNuiImagResolution(pFrame->frame.videoMode.resolutionX, pFrame->frame.videoMode.resolutionY);

	unsigned short* const target = (unsigned short*) pFrame->frame.data;
	xnOSMemSet(target, 0, pFrame->frame.dataSize);

	m_depthValuesBuffer.SetSize(numPoints);
	m_mappedCoordsBuffer.SetSize(numPoints * 2);

	// Pack depth data for NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution
	USHORT* depthValuesIter = m_depthValuesBuffer.GetData();
	for (int i = 0; i < numPoints; i++) {
		*(depthValuesIter++) = (source + i)->depth << 3;
	}

	// Need review: not sure if it is a good idea to directly invoke INuiSensore here.
	m_pStreamImpl->getNuiSensor()->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
		nuiResolution, // assume the target image with the same resolution as the source.
		nuiResolution,
		numPoints,
		m_depthValuesBuffer.GetData(),
		numPoints * 2,
		m_mappedCoordsBuffer.GetData()
		);

	const unsigned int minX = pFrame->frame.cropOriginX;
	const unsigned int minY = pFrame->frame.cropOriginY;
	const unsigned int width = pFrame->frame.width;
	const unsigned int height = pFrame->frame.height;
	const LONG* mappedCoordsIter = m_mappedCoordsBuffer.GetData();

	for (int i = 0; i < numPoints; i++)
	{
		const unsigned int x = *mappedCoordsIter++ - minX;
		const unsigned int y = *mappedCoordsIter++ - minY;
		if (x < width - 1 && y < height) {
			const unsigned short d = filterReliableDepthValue((source+i)->depth);
			unsigned short* p = target + x + y * width;
			if (*p == 0 || *p > d) *p = d;
			p++;
			if (*p == 0 || *p > d) *p = d;
		}
	}

	// FIXME: for preliminary benchmarking purpose
	static LARGE_INTEGER accTime, accCount;
	recordAverageProcessTime("With Image Reg", &accTime, &accCount, startTime);
}


void DepthKinectStream::frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT& LockedRect)
{
	OniDriverFrame* pFrame = NULL;

	// calculate the data size and allocate the buffer
	int numPoints = m_videoMode.resolutionY * m_videoMode.resolutionX;
	int dataSize = numPoints * sizeof(unsigned short);
	unsigned short * data = (unsigned short *) xnOSMallocAligned(dataSize, XN_DEFAULT_MEM_ALIGN);

	// allocate the frame
	pFrame = (OniDriverFrame*)xnOSCalloc(1, sizeof(OniDriverFrame));
	pFrame->pDriverCookie = xnOSMalloc(sizeof(KinectStreamFrameCookie));
	((KinectStreamFrameCookie*)pFrame->pDriverCookie)->refCount = 1;

	// populate the data buffer
	pFrame->frame.dataSize = dataSize;
	pFrame->frame.data = data;

	// populate the video-related metadata
	populateFrameImageMetadata(pFrame, sizeof(unsigned short));

	// populate other frame info
	pFrame->frame.sensorType = ONI_SENSOR_DEPTH;
	pFrame->frame.frameIndex = imageFrame.dwFrameNumber;
	pFrame->frame.timestamp = imageFrame.liTimeStamp.QuadPart*1000;

	// populate the pixel data
	if (m_pStreamImpl->getImageRegistrationMode() == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) {
		copyDepthPixelsWithImageRegistration((NUI_DEPTH_IMAGE_PIXEL*)LockedRect.pBits, numPoints, pFrame);
	} else {
		copyDepthPixelsStraight((NUI_DEPTH_IMAGE_PIXEL*)LockedRect.pBits, numPoints, pFrame);
	}

	raiseNewFrame(pFrame);
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
	case ONI_STREAM_PROPERTY_MIRRORING:
		{
			XnBool * val = (XnBool *)data;
			*val = TRUE;
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
	default:
		status = BaseKinectStream::getProperty(propertyId, data, pDataSize);
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
	BaseKinectStream::notifyAllProperties();
}
