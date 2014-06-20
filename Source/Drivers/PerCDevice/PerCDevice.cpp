#include "PerCDepthStream.h"
#include "PerCColorStream.h"
#include "PerCIRStream.h"
#include "PerCDevice.h"
#include "PerCDeviceEnumerator.h"

#include "pxcsession.h"
#include "pxcsmartptr.h"
#include "pxccapture.h"

namespace perc_device
{
PerCDevice::PerCDevice(const char *uri, oni::driver::DriverServices& driverServices) 
    : m_driverServices(driverServices)
    , m_idxDeviceInternal(deviceEnumerator().getDeviceIndex(uri))
{
    fillStreamArray();
}

PerCDevice::~PerCDevice() 
{
    clearSensorsList();
}


OniStatus PerCDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
    *numSensors = m_sensors.GetSize();
    *pSensors = m_sensors.GetData();
	return ONI_STATUS_OK;
}

oni::driver::StreamBase* PerCDevice::createStream(OniSensorType sensorType)
{
    switch (sensorType)
    {
    case ONI_SENSOR_COLOR:
	    {
            if (-1 == m_streamColor.idxStream)
                break;

            PerCColorStream* pColor = XN_NEW(PerCColorStream, m_idxDeviceInternal, m_streamColor.idxStream, m_streamColor.videoModeDef);
            if ((NULL != pColor) && (!pColor->isValid()))
            {
                XN_DELETE(pColor);
                pColor = NULL;
            }
		    return pColor;
	    }
    case ONI_SENSOR_DEPTH:
        {
            if (-1 == m_streamDepth.idxStream)
                break;

            PerCDepthStream* pDepth = XN_NEW(PerCDepthStream, m_idxDeviceInternal, m_streamDepth.idxStream, m_streamDepth.videoModeDef);
            if ((NULL != pDepth) && (!pDepth->isValid()))
            {
                XN_DELETE(pDepth);
                pDepth = NULL;
            }
		    return pDepth;
        }
    case ONI_SENSOR_IR:
        {
            if (-1 == m_streamIR.idxStream)
                break;

            PerCIRStream* pIR = XN_NEW(PerCIRStream, m_idxDeviceInternal, m_streamIR.idxStream, m_streamIR.videoModeDef);
            if ((NULL != pIR) && (!pIR->isValid()))
            {
                XN_DELETE(pIR);
                pIR = NULL;
            }
		    return pIR;
        }
    }

	m_driverServices.errorLoggerAppend("PerCDevice: Can't create a stream of type %d", sensorType);
	return NULL;
}

void PerCDevice::destroyStream(oni::driver::StreamBase* pStream)
{
	XN_DELETE(pStream);
}

OniStatus PerCDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
	OniStatus rc = ONI_STATUS_OK;
    propertyId;
    data;
    pDataSize;

	//switch (propertyId)
	//{
	//case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
	//	{
	//		if (*pDataSize == sizeof(OniVersion))
	//		{
	//			OniVersion* version = (OniVersion*)data;
	//			version->major = version->minor = version->maintenance = version->build = 2;
	//		}
	//		else
	//		{
	//			m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
	//			rc = ONI_STATUS_ERROR;
	//		}
	//	}
	//	break;
	//default:
	//	m_driverServices.errorLoggerAppend("Unknown property: %d\n", propertyId);
	//	rc = ONI_STATUS_ERROR;
	//}
	return rc;
}

bool PerCDevice::fillColorSensorVideoMode(OniSensorInfo &sensor, PXCCapture::Device *device, int idxStream)
{
    sensor.numSupportedVideoModes = 0;
    sensor.pSupportedVideoModes = NULL;
    if ((ONI_SENSOR_COLOR != sensor.sensorType) || (NULL == device))
        return false;

    xnl::Array<OniVideoMode > modeSet;

    PXCSmartPtr<PXCCapture::VideoStream> vstream = 0;
    if (PXC_STATUS_NO_ERROR > device->CreateStream<PXCCapture::VideoStream>(idxStream, &vstream)) 
        return false;
    for (int idxProfile = 0; ;idxProfile++) 
    {
        PXCCapture::VideoStream::ProfileInfo pinfo;
        if (PXC_STATUS_NO_ERROR > vstream->QueryProfile(idxProfile, &pinfo))
            break;

        bool validMode = true;
        OniVideoMode mode = {ONI_PIXEL_FORMAT_RGB888, 0, 0, 0};// make MSVS happy
        switch (pinfo.imageInfo.format)
        {
        case PXCImage::COLOR_FORMAT_RGB32:
        case PXCImage::COLOR_FORMAT_RGB24:
            mode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
            break;
        case PXCImage::COLOR_FORMAT_YUY2:
        case PXCImage::COLOR_FORMAT_NV12:
        case PXCImage::COLOR_FORMAT_GRAY:
            //TODO
            validMode = false;
            break;
        default:
            validMode = false;
            break;
        }
        if (!validMode)
            continue;
        mode.resolutionX = pinfo.imageInfo.width;
        mode.resolutionY = pinfo.imageInfo.height;
        if (0 == pinfo.frameRateMin.denominator)
            continue;
        mode.fps = (int)(pinfo.frameRateMin.numerator / pinfo.frameRateMin.denominator);

        for (XnUInt32 idxMode = 0; idxMode < modeSet.GetSize(); idxMode++)
        {
            if ((mode.fps           == modeSet[idxMode].fps)            &&
                (mode.pixelFormat   == modeSet[idxMode].pixelFormat)    &&
                (mode.resolutionX   == modeSet[idxMode].resolutionX)    &&
                (mode.resolutionY   == modeSet[idxMode].resolutionY))
            {
                validMode = false;
                break;
            }
        }
        if (validMode)
            modeSet.AddLast(mode);
    }

    if (0 == modeSet.GetSize())
        return false;
    
    sensor.pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, modeSet.GetSize());
    if (NULL == sensor.pSupportedVideoModes)
        return false;
    sensor.numSupportedVideoModes = modeSet.GetSize();
    memcpy(sensor.pSupportedVideoModes, modeSet.GetData(), modeSet.GetSize() * sizeof(OniVideoMode));
    return true;
}

bool PerCDevice::fillDepthSensorVideoMode(OniSensorInfo &sensor, PXCCapture::Device *device, int idxStream)
{
    sensor.numSupportedVideoModes = 0;
    sensor.pSupportedVideoModes = NULL;
    if ((ONI_SENSOR_DEPTH != sensor.sensorType) || (NULL == device))
        return false;

    pxcF32 fDepthUnit;
    device->QueryProperty(PXCCapture::Device::PROPERTY_DEPTH_UNIT, &fDepthUnit);
    OniPixelFormat pixFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
    if ((99.f < fDepthUnit) && (101.f > fDepthUnit))
        pixFormat = ONI_PIXEL_FORMAT_DEPTH_100_UM;
    else if ((999.f < fDepthUnit) && (1001.f > fDepthUnit))
        pixFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
    else
    {
        m_driverServices.errorLoggerAppend("PerCDevice: the unit of depth values is invalid %f", fDepthUnit);
        return false;
    }

    xnl::Array<OniVideoMode> modeSet;

    PXCSmartPtr<PXCCapture::VideoStream> vstream = 0;
    if (PXC_STATUS_NO_ERROR > device->CreateStream<PXCCapture::VideoStream>(idxStream, &vstream)) 
        return false;
    for (int idxProfile = 0; ;idxProfile++) 
    {
        PXCCapture::VideoStream::ProfileInfo pinfo;
        if (PXC_STATUS_NO_ERROR > vstream->QueryProfile(idxProfile, &pinfo))
            break;

        bool validMode = true;
        OniVideoMode mode = {ONI_PIXEL_FORMAT_DEPTH_1_MM, 0, 0, 0};// make MSVS happy
        switch (pinfo.imageInfo.format)
        {
        case PXCImage::COLOR_FORMAT_DEPTH:
            mode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
            break;
        case PXCImage::COLOR_FORMAT_VERTICES:
        default:
            validMode = false;
            break;
        }
        if (!validMode)
            continue;
        mode.resolutionX = pinfo.imageInfo.width;
        mode.resolutionY = pinfo.imageInfo.height;
        if (0 == pinfo.frameRateMin.denominator)
            continue;
        mode.fps = (int)(pinfo.frameRateMin.numerator / pinfo.frameRateMin.denominator);

        for (XnUInt32 idxMode = 0; idxMode < modeSet.GetSize(); idxMode++)
        {
            if ((mode.fps           == modeSet[idxMode].fps)            &&
                (mode.pixelFormat   == modeSet[idxMode].pixelFormat)    &&
                (mode.resolutionX   == modeSet[idxMode].resolutionX)    &&
                (mode.resolutionY   == modeSet[idxMode].resolutionY))
            {
                validMode = false;
                break;
            }
        }
        if (validMode)
            modeSet.AddLast(mode);
    }

    if (0 == modeSet.GetSize())
        return false;
    
    sensor.pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, modeSet.GetSize());
    if (NULL == sensor.pSupportedVideoModes)
        return false;
    sensor.numSupportedVideoModes = modeSet.GetSize();
    memcpy(sensor.pSupportedVideoModes, modeSet.GetData(), modeSet.GetSize() * sizeof(OniVideoMode));
    return true;
}

void PerCDevice::fillStreamArray()
{
    m_sensors.Clear();
    PXCSmartPtr<PXCCapture::Device> device = deviceEnumerator().getDevice(m_idxDeviceInternal);
    if (!device.IsValid())
    {
        m_driverServices.errorLoggerAppend("PerCDevice: device is invalid");
        return;
    }

    m_streamColor.idxStream = m_streamDepth.idxStream = m_streamIR.idxStream = -1;

    for (int idxStream = 0; ;idxStream++) 
    {
        PXCCapture::Device::StreamInfo sinfo;
        if (PXC_STATUS_NO_ERROR > device->QueryStream(idxStream, &sinfo)) 
            break;
        if (PXCCapture::VideoStream::CUID != sinfo.cuid) 
            continue;
        m_sensors.SetSize(m_sensors.GetSize() + 1);
        OniSensorInfo &sensor = m_sensors[m_sensors.GetSize() - 1];
        bool validSensor = false;
        switch (sinfo.imageType) 
        {
        case PXCImage::IMAGE_TYPE_COLOR: 
            sensor.sensorType = ONI_SENSOR_COLOR;
            validSensor = fillColorSensorVideoMode(sensor, device, idxStream);
            if ((-1 == m_streamColor.idxStream) && validSensor)
            {
                m_streamColor.idxStream = idxStream;
                m_streamColor.videoModeDef = sensor.pSupportedVideoModes[0];
            }
            break;
        case PXCImage::IMAGE_TYPE_DEPTH:
            sensor.sensorType = ONI_SENSOR_DEPTH;
            validSensor = fillDepthSensorVideoMode(sensor, device, idxStream);
            if (validSensor)
            {
                if (-1 == m_streamDepth.idxStream)
                {
                    m_streamDepth.idxStream = idxStream;
                    m_streamDepth.videoModeDef = sensor.pSupportedVideoModes[0];
                }
                else if (-1 == m_streamIR.idxStream)
                {
                    sensor.sensorType = ONI_SENSOR_IR;
                    for (int idxMode = 0; idxMode < sensor.numSupportedVideoModes; idxMode++)
                    {
                        sensor.pSupportedVideoModes[idxMode].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
                    }
                    m_streamIR.idxStream = idxStream;
                    m_streamIR.videoModeDef = sensor.pSupportedVideoModes[0];
                }
            }
            break;
        default:
            break;
        }
        if (!validSensor)
            m_sensors.SetSize(m_sensors.GetSize() - 1);
    }
}

void PerCDevice::clearSensorsList()
{
    for(XnUInt32 i = 0; i < m_sensors.GetSize(); ++i)
	{
		XN_DELETE_ARR(m_sensors[i].pSupportedVideoModes);
	}
    m_sensors.Clear();
}

}//namespace perc_device