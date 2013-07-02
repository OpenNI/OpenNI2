#include "KinectDevice.h"
#include "DepthKinectStream.h"
#include "ColorKinectStream.h"
#include "IRKinectStream.h"
#include <Shlobj.h>
#include "NuiApi.h"
#include "XnLog.h"

using namespace kinect_device;
using namespace oni::driver;
#define DEFAULT_FPS 30
KinectDevice::KinectDevice(INuiSensor * pNuiSensor):m_pDepthStream(NULL), m_pColorStream(NULL),m_pNuiSensor(pNuiSensor)
{
	m_numSensors = 3;

	m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
	m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
	m_sensors[0].numSupportedVideoModes = 3;
	m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_sensors[0].pSupportedVideoModes[0].fps = DEFAULT_FPS;
	m_sensors[0].pSupportedVideoModes[0].resolutionX = 640;
	m_sensors[0].pSupportedVideoModes[0].resolutionY = 480;

	m_sensors[0].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_sensors[0].pSupportedVideoModes[1].fps = DEFAULT_FPS;
	m_sensors[0].pSupportedVideoModes[1].resolutionX = 320;
	m_sensors[0].pSupportedVideoModes[1].resolutionY = 240;

	m_sensors[0].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_sensors[0].pSupportedVideoModes[2].fps = DEFAULT_FPS;
	m_sensors[0].pSupportedVideoModes[2].resolutionX = 80;
	m_sensors[0].pSupportedVideoModes[2].resolutionY = 60;

	m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
	m_sensors[1].sensorType = ONI_SENSOR_COLOR;
	m_sensors[1].numSupportedVideoModes = 3;
	m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_sensors[1].pSupportedVideoModes[0].fps         = 12;
	m_sensors[1].pSupportedVideoModes[0].resolutionX = 1280;
	m_sensors[1].pSupportedVideoModes[0].resolutionY = 960;

	m_sensors[1].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_sensors[1].pSupportedVideoModes[1].fps = DEFAULT_FPS;
	m_sensors[1].pSupportedVideoModes[1].resolutionX = 640;
	m_sensors[1].pSupportedVideoModes[1].resolutionY = 480;

	m_sensors[1].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_YUV422;
	m_sensors[1].pSupportedVideoModes[2].fps         = 15;
	m_sensors[1].pSupportedVideoModes[2].resolutionX = 640;
	m_sensors[1].pSupportedVideoModes[2].resolutionY = 480;

	m_sensors[2].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 3);
	m_sensors[2].sensorType = ONI_SENSOR_IR;
	m_sensors[2].numSupportedVideoModes = 3;
	m_sensors[2].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_sensors[2].pSupportedVideoModes[0].fps = DEFAULT_FPS;
	m_sensors[2].pSupportedVideoModes[0].resolutionX = 640;
	m_sensors[2].pSupportedVideoModes[0].resolutionY = 480;

	m_sensors[2].pSupportedVideoModes[1].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
	m_sensors[2].pSupportedVideoModes[1].fps = DEFAULT_FPS;
	m_sensors[2].pSupportedVideoModes[1].resolutionX = 640;
	m_sensors[2].pSupportedVideoModes[1].resolutionY = 480;

	m_sensors[2].pSupportedVideoModes[2].pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
	m_sensors[2].pSupportedVideoModes[2].fps = DEFAULT_FPS;
	m_sensors[2].pSupportedVideoModes[2].resolutionX = 640;
	m_sensors[2].pSupportedVideoModes[2].resolutionY = 480;	
}

KinectDevice::~KinectDevice()
{
	if (m_pNuiSensor)
		m_pNuiSensor->NuiShutdown();
	
	if (m_pDepthStream != NULL)
		XN_DELETE(m_pDepthStream);
	
	if (m_pColorStream!= NULL)
		XN_DELETE(m_pColorStream);
	
	if (m_pNuiSensor)
		m_pNuiSensor->Release();
}

OniStatus KinectDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
	*numSensors = m_numSensors;
	*pSensors = m_sensors;
	return ONI_STATUS_OK;
}

StreamBase* KinectDevice::createStream(OniSensorType sensorType)
{
	BaseKinectStream* pImage = NULL;
	
	if (sensorType ==  ONI_SENSOR_COLOR )
	{
		if (m_pColorStream == NULL)
		{
			m_pColorStream = XN_NEW(KinectStreamImpl, m_pNuiSensor, sensorType);
			
		}	
		pImage = XN_NEW(ColorKinectStream, m_pColorStream);
	}
	else if (sensorType == ONI_SENSOR_DEPTH )
	{
		if (m_pDepthStream == NULL)
		{
			m_pDepthStream = XN_NEW(KinectStreamImpl, m_pNuiSensor, sensorType);			
		}
		pImage = XN_NEW(DepthKinectStream, m_pDepthStream); 
	}
	if (sensorType ==  ONI_SENSOR_IR )
	{
		if (m_pColorStream == NULL)
		{
			m_pColorStream = XN_NEW(KinectStreamImpl, m_pNuiSensor, sensorType);
		}		
		pImage = XN_NEW(IRKinectStream, m_pColorStream);
		
	}
	return pImage;
}

void kinect_device::KinectDevice::destroyStream(oni::driver::StreamBase* pStream)
{
	XN_DELETE(pStream);
}

OniStatus KinectDevice::setProperty(int propertyId, const void* data, int dataSize)
{
	switch (propertyId) {
	case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
		{
			// FIXME data size validation
			if (m_pDepthStream) {
				OniImageRegistrationMode* pMode = (OniImageRegistrationMode*)data;
				m_pDepthStream->setImageRegistrationMode(*pMode);
				return ONI_STATUS_OK;
			} else {
				return ONI_STATUS_ERROR;
			}
		}
	}

	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus KinectDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
	switch (propertyId) {
	case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
		{
			// FIXME data size validation
			if (m_pDepthStream) {
				OniImageRegistrationMode* pMode = (OniImageRegistrationMode*)data;
				*pMode = m_pDepthStream->getImageRegistrationMode();
				return ONI_STATUS_OK;
			} else {
				return ONI_STATUS_ERROR;
			}
		}
	}

	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool KinectDevice::isPropertySupported(int propertyId)
{
	return (propertyId == ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION);
}

OniBool KinectDevice::isCommandSupported(int commandId)
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus KinectDevice::tryManualTrigger()
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool KinectDevice::isImageRegistrationModeSupported(OniImageRegistrationMode mode)
{
	return (mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR || mode == ONI_IMAGE_REGISTRATION_OFF);
}
