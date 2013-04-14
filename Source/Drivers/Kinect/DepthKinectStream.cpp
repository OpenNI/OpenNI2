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
	m_videoMode.resolutionX = KINECT_RESOLUTION_X_640;
	m_videoMode.resolutionY = KINECT_RESOLUTION_Y_480;
}

void DepthKinectStream::frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT& LockedRect)
{
	OniFrame* pFrame = getServices().acquireFrame();
	const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);
	const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (m_videoMode.resolutionY * m_videoMode.resolutionX);
	// Get the min and max reliable depth for the current frame
	unsigned short * data = (unsigned short *)pFrame->data;
	if (!m_cropping.enabled)
	{
		while (pBufferRun < pBufferEnd)
		{
			// discard the portion of the depth that contains only the player index
			*(data++) = (pBufferRun->depth > 0 && pBufferRun->depth < DEVICE_MAX_DEPTH_VAL)?pBufferRun->depth:0;
			++pBufferRun;
		}
		pFrame->stride = m_videoMode.resolutionX * 2;
		pFrame->height = pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
		pFrame->width  = pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
	}
	else
	{
		int cropX = m_cropping.originX;
		int cropY = m_cropping.originY;
		while (cropY < m_cropping.originY + m_cropping.height)
		{
			while (cropX < m_cropping.originX + m_cropping.width)
			{
				const NUI_DEPTH_IMAGE_PIXEL *iter = pBufferRun + (m_videoMode.resolutionX * cropY + cropX++);
				*(data++)= (iter->depth > 0 && iter->depth < DEVICE_MAX_DEPTH_VAL) ? iter->depth:0;
			}
			cropY++;
			cropX = m_cropping.originX;
		}
		pFrame->stride = m_cropping.width * 2;
		pFrame->height = m_cropping.height;
		pFrame->width  = m_cropping.width;
		pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
		pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
		pFrame->cropOriginX = m_cropping.originX; 
		pFrame->cropOriginY = m_cropping.originY;
		pFrame->croppingEnabled = TRUE;
	}
	pFrame->videoMode.pixelFormat = m_videoMode.pixelFormat;
	pFrame->videoMode.fps = m_videoMode.fps;
	pFrame->sensorType = ONI_SENSOR_DEPTH;
	pFrame->frameIndex = imageFrame.dwFrameNumber;
	pFrame->timestamp = imageFrame.liTimeStamp.QuadPart*1000;
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
