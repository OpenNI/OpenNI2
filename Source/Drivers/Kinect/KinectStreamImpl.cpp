#include "KinectStreamImpl.h"
#include "BaseKinectStream.h"

#include "NuiApi.h"

using namespace oni::driver;
using namespace kinect_device;
using namespace xnl;

#define DEFAULT_FPS 30

KinectStreamImpl::KinectStreamImpl(INuiSensor *pNuiSensor, OniSensorType sensorType):
									m_pNuiSensor(pNuiSensor), m_sensorType(sensorType),
									m_running(FALSE), m_hStreamHandle(INVALID_HANDLE_VALUE),
									m_hNextFrameEvent(CreateEvent(NULL, TRUE, FALSE, NULL))
															
{
	setDefaultVideoMode();
}

KinectStreamImpl::~KinectStreamImpl()
{
	if (m_running)
	{
		m_running = FALSE;
		xnOSWaitForThreadExit(m_threadHandle, INFINITE);
		xnOSCloseThread(&m_threadHandle);
	}

	if (m_hNextFrameEvent != INVALID_HANDLE_VALUE)
		CloseHandle(m_hNextFrameEvent);
}

void KinectStreamImpl::addStream(BaseKinectStream* stream)
{
	m_streamList.AddLast(stream);
}

void KinectStreamImpl::removeStream(BaseKinectStream* stream)
{
	m_streamList.Remove(stream);
}

unsigned int KinectStreamImpl::getStreamCount()
{
	return m_streamList.Size();
}

void KinectStreamImpl::setVideoMode(OniVideoMode* videoMode)
{
	m_videoMode.fps = videoMode->fps;
	m_videoMode.pixelFormat = videoMode->pixelFormat;
	m_videoMode.resolutionX = videoMode->resolutionX;
	m_videoMode.resolutionY = videoMode->resolutionY;
}

OniStatus KinectStreamImpl::start()
{
	if (m_running != TRUE)
	{
		// Open a color image stream to receive frames
		HRESULT hr = m_pNuiSensor->NuiImageStreamOpen(
			getNuiImageType(),
			getNuiImagResolution(m_videoMode.resolutionX, m_videoMode.resolutionY),
			0,
			2,
			m_hNextFrameEvent,
			&m_hStreamHandle);

		if (FAILED(hr))
		{
			return ONI_STATUS_ERROR;
		}

		//m_pNuiSensor->NuiImageStreamSetImageFrameFlags(m_pStreamHandle, NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE);

		XnStatus nRetVal = xnOSCreateThread(threadFunc, this, &m_threadHandle);
		if (nRetVal != XN_STATUS_OK)
		{
			return ONI_STATUS_ERROR;
		}
		return ONI_STATUS_OK;
	}
	else
	{
		return ONI_STATUS_OK;
	}
}

void KinectStreamImpl::stop()
{
	if (m_running == true)
	{
		List<BaseKinectStream*>::Iterator iter = m_streamList.Begin();
		while( iter != m_streamList.End())
		{
			if (((BaseKinectStream*)(*iter))->isRunning())
				return;
			++iter;
		}
		m_running = false;
		xnOSWaitForThreadExit(m_threadHandle, INFINITE);
		xnOSCloseThread(&m_threadHandle);
	}
}

void KinectStreamImpl::setSensorType(OniSensorType sensorType)
{ 
	if( m_sensorType != sensorType)
	{
		m_sensorType = sensorType; 
		setDefaultVideoMode();
	}	
}

static const unsigned int LOOP_TIMEOUT = 10;
void KinectStreamImpl::mainLoop()
{
	m_running = TRUE;
	while (m_running)
	{
		if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextFrameEvent, LOOP_TIMEOUT) && m_running)
		{
			HRESULT hr;
			NUI_IMAGE_FRAME imageFrame;
			hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_hStreamHandle, 0, &imageFrame);
			if (FAILED(hr))
			{
				continue;
			}
			INuiFrameTexture * pTexture;
			if (m_sensorType == ONI_SENSOR_DEPTH)
			{
				BOOL nearMode;
				// Get the depth image pixel texture
				hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
					m_hStreamHandle, &imageFrame, &nearMode, &pTexture);
			}
			else
			{
				pTexture = imageFrame.pFrameTexture;	
			}

			NUI_LOCKED_RECT LockedRect;

			// Lock the frame data so the Kinect knows not to modify it while we're reading it
			pTexture->LockRect(0, &LockedRect, NULL, 0);
			if (LockedRect.Pitch != 0)
			{
				List<BaseKinectStream*>::ConstIterator iter = m_streamList.Begin();
				while( iter != m_streamList.End())
				{
					if (((BaseKinectStream*)(*iter))->isRunning())
						((BaseKinectStream*)(*iter))->frameReceived(imageFrame, LockedRect);
					++iter;
				}
			}
			// We're done with the texture so unlock it
			pTexture->UnlockRect(0);

			// Release the frame
			m_pNuiSensor->NuiImageStreamReleaseFrame(m_hStreamHandle, &imageFrame);
		}		
	}
	return;
}

OniStatus KinectStreamImpl::setAutoWhiteBalance(BOOL val)
{
	INuiColorCameraSettings *pCameraSettings;
	HRESULT hr = m_pNuiSensor->NuiGetColorCameraSettings(&pCameraSettings);
	if (FAILED(hr))
	{
		return ONI_STATUS_ERROR;
	}
	hr = pCameraSettings->SetAutoWhiteBalance(val);
	OniStatus status = FAILED(hr)? ONI_STATUS_ERROR : ONI_STATUS_OK;
	pCameraSettings->Release();
	return status;
}

OniStatus KinectStreamImpl::getAutoWhitBalance(BOOL *val)
{
	INuiColorCameraSettings *pCameraSettings;
	HRESULT hr = m_pNuiSensor->NuiGetColorCameraSettings(&pCameraSettings);
	if (FAILED(hr))
	{
		return ONI_STATUS_ERROR;
	}
	hr = pCameraSettings->GetAutoWhiteBalance(val);
	OniStatus status = FAILED(hr) ? ONI_STATUS_ERROR : ONI_STATUS_OK;
	pCameraSettings->Release();
	return status;
}

OniStatus KinectStreamImpl::setAutoExposure(BOOL val)
{
	INuiColorCameraSettings *pCameraSettings;
	HRESULT hr = m_pNuiSensor->NuiGetColorCameraSettings(&pCameraSettings);
	if (FAILED(hr))
	{
		return ONI_STATUS_ERROR;
	}
	hr = pCameraSettings->SetAutoExposure(val);
	OniStatus status = FAILED(hr) ? ONI_STATUS_ERROR : ONI_STATUS_OK;
	pCameraSettings->Release();
	return status;
}

OniStatus KinectStreamImpl::getAutoExposure(BOOL *val)
{
	INuiColorCameraSettings *pCameraSettings;
	HRESULT hr = m_pNuiSensor->NuiGetColorCameraSettings(&pCameraSettings);
	if (FAILED(hr))
	{
		return ONI_STATUS_ERROR;
	}
	hr = pCameraSettings->GetAutoExposure(val);
	OniStatus status = FAILED(hr) ? ONI_STATUS_ERROR : ONI_STATUS_OK;
	pCameraSettings->Release();
	return status;
}

OniStatus KinectStreamImpl::convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY)
{
	OniVideoMode videoMode;
	int size = sizeof(videoMode);
	 if (ONI_STATUS_OK != colorStream->getProperty(ONI_STREAM_PROPERTY_VIDEO_MODE, &videoMode, &size))
		 return ONI_STATUS_ERROR;
	 HRESULT hr = m_pNuiSensor->NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
				getNuiImagResolution(videoMode.resolutionX, videoMode.resolutionY),
				getNuiImagResolution(m_videoMode.resolutionX, m_videoMode.resolutionY),
				NULL, depthX, depthY, depthZ << 3, (LONG*)pColorX, (LONG*)pColorY);
	 if (FAILED(hr))
		 return ONI_STATUS_ERROR;
	return ONI_STATUS_OK;
}

void KinectStreamImpl::setDefaultVideoMode()
{
	switch (m_sensorType)
	{
	case ONI_SENSOR_COLOR:
		m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_videoMode.fps         = DEFAULT_FPS;
		m_videoMode.resolutionX = KINECT_RESOLUTION_X_640;
		m_videoMode.resolutionY = KINECT_RESOLUTION_Y_480;
		break;

	case ONI_SENSOR_DEPTH:
		m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		m_videoMode.fps         = DEFAULT_FPS;
		m_videoMode.resolutionX = KINECT_RESOLUTION_X_640;
		m_videoMode.resolutionY = KINECT_RESOLUTION_Y_480;
		break;

	case ONI_SENSOR_IR:
		m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
		m_videoMode.fps         = DEFAULT_FPS;
		m_videoMode.resolutionX = KINECT_RESOLUTION_X_640;
		m_videoMode.resolutionY = KINECT_RESOLUTION_Y_480;
		break;
	default:
		;
	}
}

NUI_IMAGE_RESOLUTION KinectStreamImpl::getNuiImagResolution(int resolutionX, int resolutionY)
{
	NUI_IMAGE_RESOLUTION imgResolution = NUI_IMAGE_RESOLUTION_320x240;
	if (resolutionX == KINECT_RESOLUTION_X_80 && resolutionY == KINECT_RESOLUTION_Y_60 )
	{
		imgResolution = NUI_IMAGE_RESOLUTION_80x60;
	} 
	else if (resolutionX == KINECT_RESOLUTION_X_320 && resolutionY == KINECT_RESOLUTION_Y_240 )
	{
		imgResolution = NUI_IMAGE_RESOLUTION_320x240;
	}
	else if (resolutionX == KINECT_RESOLUTION_X_640 && resolutionY == KINECT_RESOLUTION_Y_480 )
	{
		imgResolution = NUI_IMAGE_RESOLUTION_640x480;
	}
	else if (resolutionX == KINECT_RESOLUTION_X_1280 && resolutionY == KINECT_RESOLUTION_Y_960 )
	{
		imgResolution = NUI_IMAGE_RESOLUTION_1280x960;
	}
	return imgResolution;
}

NUI_IMAGE_TYPE KinectStreamImpl::getNuiImageType()
{
	NUI_IMAGE_TYPE imgType;
	switch (m_sensorType)
	{
	case ONI_SENSOR_IR:
		imgType = NUI_IMAGE_TYPE_COLOR_INFRARED;
		break;
	case ONI_SENSOR_COLOR:
		if (m_videoMode.pixelFormat == ONI_PIXEL_FORMAT_YUV422)
			imgType = NUI_IMAGE_TYPE_COLOR_RAW_YUV;
		else
			imgType = NUI_IMAGE_TYPE_COLOR;
		break;
	case ONI_SENSOR_DEPTH:
		imgType =  NUI_IMAGE_TYPE_DEPTH;
		break;
	default:
		imgType = NUI_IMAGE_TYPE_COLOR;
		break;
	}
	return imgType;
}

XN_THREAD_PROC KinectStreamImpl::threadFunc(XN_THREAD_PARAM pThreadParam)
{
	KinectStreamImpl* pStream = (KinectStreamImpl*)pThreadParam;
	pStream->mainLoop();
	XN_THREAD_PROC_RETURN(XN_STATUS_OK);
}
