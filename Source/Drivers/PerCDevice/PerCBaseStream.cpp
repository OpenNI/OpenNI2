#include "PerCDeviceEnumerator.h"
#include "PerCBaseStream.h"

namespace perc_device
{
PerCBaseStream::PerCBaseStream(XnUInt32 idxDeviceInternal, XnUInt32 idxStream, const OniVideoMode &videoMode)
    : m_idxDeviceInternal(idxDeviceInternal)
    , m_pxcDevice(NULL)
    , m_idxStream(idxStream)
    , m_pxcStream(NULL)
    , m_running(false)
    , m_mirror(false)
    , m_videoMode(videoMode)
{
    m_cropping.enabled = 0;
    createStream();
}

PerCBaseStream::~PerCBaseStream()
{
	stop();
}

OniStatus PerCBaseStream::start()
{
    if (m_running)
        return ONI_STATUS_OK;

    if (!m_pxcStream.IsValid())
        return ONI_STATUS_ERROR;

    PXCCapture::VideoStream::ProfileInfo pinfo;
    for (int idxProfile = 0; ; idxProfile++) 
    {
        if (PXC_STATUS_NO_ERROR > m_pxcStream->QueryProfile(idxProfile, &pinfo))
            return ONI_STATUS_ERROR;
        if ((m_videoMode.resolutionX == (int)pinfo.imageInfo.width) && (m_videoMode.resolutionY == (int)pinfo.imageInfo.height) &&
            (pinfo.frameRateMin.denominator * m_videoMode.fps >= pinfo.frameRateMin.numerator) &&
            (pinfo.frameRateMax.denominator * m_videoMode.fps <= pinfo.frameRateMax.numerator))
            break;
    }
    if (PXC_STATUS_NO_ERROR > m_pxcStream->SetProfile(&pinfo))
        return ONI_STATUS_ERROR;

    xnOSCreateThread(threadFunc, this, &m_threadHandle);

	return ONI_STATUS_OK;
}

void PerCBaseStream::stop()
{
    if (!m_running)
        return;
	m_running = false;
    if (m_threadHandle)
        xnOSWaitForThreadExit(m_threadHandle, 10000);    

    // Is there another way to stop stream?
    // If we don't recreate device, a error may appears (or may not), then we change
    // a stream profile on the running stream and then read a frame.
    // Another way - set Sleep(1000) in start() after SetProfile() before start the thread
    m_pxcStream.ReleaseRef();
    m_pxcDevice.ReleaseRef();
    createStream();
}

OniBool PerCBaseStream::isPropertySupported(int propertyId)
{
    return ((ONI_STREAM_PROPERTY_VIDEO_MODE == propertyId)  ||
            (ONI_STREAM_PROPERTY_MIRRORING  == propertyId)  ||
            (ONI_STREAM_PROPERTY_CROPPING   == propertyId));
}
OniStatus PerCBaseStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	if (ONI_STREAM_PROPERTY_VIDEO_MODE == propertyId)
	{
		if (*pDataSize != sizeof(OniVideoMode))
		{
			printf("Unexpected size: %d != %d\n", *pDataSize, (int)sizeof(OniVideoMode));
			return ONI_STATUS_ERROR;
		}
		return GetVideoMode((OniVideoMode*)data);
	}
    else if (ONI_STREAM_PROPERTY_MIRRORING == propertyId)
    {
        if (*pDataSize != sizeof(OniBool))
		{
			printf("Unexpected size: %d != %d\n", *pDataSize, (int)sizeof(OniBool));
			return ONI_STATUS_ERROR;
		}
        *(OniBool *)data = m_mirror;
        return ONI_STATUS_OK;
    }
    else if (ONI_STREAM_PROPERTY_CROPPING == propertyId)
    {
		if (*pDataSize != sizeof(OniCropping))
		{
			printf("Unexpected size: %d != %d\n", *pDataSize, (int)sizeof(OniCropping));
			return ONI_STATUS_ERROR;
		}
        *(OniCropping *)data = m_cropping;
        return ONI_STATUS_OK;
    }
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus PerCBaseStream::setProperty(int propertyId, const void* data, int dataSize)
{
	if (ONI_STREAM_PROPERTY_VIDEO_MODE == propertyId)
	{
		if (dataSize != sizeof(OniVideoMode))
		{
			printf("Unexpected size: %d != %d\n", dataSize, (int)sizeof(OniVideoMode));
			return ONI_STATUS_ERROR;
		}
		return SetVideoMode((OniVideoMode*)data);
	}
    else if (ONI_STREAM_PROPERTY_MIRRORING == propertyId)
    {
        if (dataSize != sizeof(OniBool))
		{
			printf("Unexpected size: %d != %d\n", dataSize, (int)sizeof(OniBool));
			return ONI_STATUS_ERROR;
		}
        m_mirror  = *(const OniBool *)data;
        return ONI_STATUS_OK;
    }
    else if (ONI_STREAM_PROPERTY_CROPPING == propertyId)
	{
		if (dataSize != sizeof(OniCropping))
		{
			printf("Unexpected size: %d != %d\n", dataSize, (int)sizeof(OniCropping));
			return ONI_STATUS_ERROR;
		}
        m_cropping = *(OniCropping *)data;
        return ONI_STATUS_OK;
	}
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool PerCBaseStream::getFrameRect(int &left, int &top, int &right, int &bottom)
{
    left = 0; right = m_videoMode.resolutionX - 1;
    top = 0; bottom = m_videoMode.resolutionY - 1;
    if (0 == m_cropping.enabled)
        return 1;

    top = min(m_cropping.originY, m_cropping.originY + m_cropping.height - 1);
    bottom = max(m_cropping.originY, m_cropping.originY + m_cropping.height - 1);
    if ((bottom < 0) || (m_videoMode.resolutionY <= top))
        return 0;
    if (top < 0) 
        top = 0;
    if (m_videoMode.resolutionY <= bottom) 
        bottom = m_videoMode.resolutionY - 1;

    left = min(m_cropping.originX, m_cropping.originX + m_cropping.width - 1);
    right = max(m_cropping.originX, m_cropping.originX + m_cropping.width - 1);
    if ((right < 0) || (m_videoMode.resolutionX <= left))
        return 0;
    if (left < 0) 
        left = 0;
    if (m_videoMode.resolutionX <= right) 
        right = m_videoMode.resolutionX - 1;

    return 1;
}

void PerCBaseStream::createStream()
{
    m_pxcDevice = deviceEnumerator().getDevice(m_idxDeviceInternal);
    if (!m_pxcDevice.IsValid())
    {
        //TODO log
        return;
    }
    if (PXC_STATUS_NO_ERROR > m_pxcDevice->CreateStream<PXCCapture::VideoStream>(m_idxStream, &m_pxcStream)) 
    {
        //TODO log
        return;
    }
}

XN_THREAD_PROC PerCBaseStream::threadFunc(XN_THREAD_PARAM pThreadParam)
{
	PerCBaseStream* pStream = (PerCBaseStream*)pThreadParam;
	pStream->m_running = true;
	pStream->Mainloop();

	XN_THREAD_PROC_RETURN(XN_STATUS_OK);
}


}//namespace perc_device