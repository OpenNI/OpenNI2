#include "PerCColorStream.h"

namespace perc_device
{
PerCColorStream::PerCColorStream(XnUInt32 idxDeviceInternal, XnUInt32 idxStream, const OniVideoMode &videoMode)
    : PerCBaseStream(idxDeviceInternal, idxStream, videoMode)
{
}

OniStatus PerCColorStream::SetVideoMode(OniVideoMode* pVideoMode) 
{
    if (m_running)
        return ONI_STATUS_NOT_SUPPORTED;
    m_videoMode.pixelFormat = pVideoMode->pixelFormat;
    m_videoMode.fps         = pVideoMode->fps;
    m_videoMode.resolutionX = pVideoMode->resolutionX;
    m_videoMode.resolutionY = pVideoMode->resolutionY;    
    return ONI_STATUS_OK;
}
OniStatus PerCColorStream::GetVideoMode(OniVideoMode* pVideoMode)
{
    pVideoMode->pixelFormat = m_videoMode.pixelFormat;
    pVideoMode->fps         = m_videoMode.fps;
    pVideoMode->resolutionX = m_videoMode.resolutionX;
    pVideoMode->resolutionY = m_videoMode.resolutionY;
	return ONI_STATUS_OK;
}

OniBool PerCColorStream::isPropertySupported(int propertyId)
{
    return PerCBaseStream::isPropertySupported(propertyId);
}

OniStatus PerCColorStream::getProperty(int propertyId, void* data, int* pDataSize)
{
    return PerCBaseStream::getProperty(propertyId, data, pDataSize);
}
OniStatus PerCColorStream::setProperty(int propertyId, const void* data, int dataSize)
{
    return PerCBaseStream::setProperty(propertyId, data, dataSize);
}

void PerCColorStream::Mainloop()
{
    if (!m_pxcStream.IsValid())
    {
        m_running = false;
        return;
    }
    
    int frameId = 1;
    pxcU64 startTimeStamp = 0;    
    while (m_running)
    {
        PXCSmartPtr<PXCImage> image; PXCSmartSP sp;
        if (PXC_STATUS_NO_ERROR > m_pxcStream->ReadStreamAsync(&image, &sp))
            continue;
        if (PXC_STATUS_NO_ERROR > sp->Synchronize())
            continue;
        if (!image.IsValid())
            continue;

        int left, top, right, bottom;
        if (0 == getFrameRect(left, top, right, bottom))
            continue;

        OniFrame* pFrame = getServices().acquireFrame();
		pFrame->frameIndex = frameId;
        pFrame->videoMode = m_videoMode;
        if (m_cropping.enabled)
        {
            pFrame->croppingEnabled = 1;
            pFrame->cropOriginX = m_cropping.originX;
            pFrame->cropOriginY = m_cropping.originY;
        }
        else
        {
            pFrame->croppingEnabled = 0;
            pFrame->cropOriginX = 0;
            pFrame->cropOriginY = 0;
        }

		pFrame->sensorType = ONI_SENSOR_COLOR;
        if (0 == startTimeStamp)
        {
            startTimeStamp = image->QueryTimeStamp();
            pFrame->timestamp = 0;
        }
        else
        {
            pFrame->timestamp = (uint64_t)((image->QueryTimeStamp() - startTimeStamp) / 1000000);
        }

        pFrame->width = right - left + 1;
        pFrame->height = bottom - top + 1;
        pFrame->stride = 3 * pFrame->width;

        PXCImage::ImageData data;
        image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::COLOR_FORMAT_RGB24, &data);
        //note: DepthSense throught PCSDK return color frame in RGB format
        // we have to optimize this, if some change will appears in PCSDK

        if (m_mirror)
        {
            for (int row = 0; row < pFrame->height; row++)
            {
                pxcBYTE *pSrc = data.planes[0] + data.pitches[0] * (row + top) +  3 * (m_videoMode.resolutionX - 1 - left);
                pxcBYTE *pDst = (pxcBYTE *)(pFrame->data) + pFrame->stride * row;
                for (int col = 0; col < pFrame->width; col++, pSrc-=3, pDst+=3)
                {
                    pDst[0] = pSrc[2]; // red
                    pDst[1] = pSrc[1]; // green
                    pDst[2] = pSrc[0]; // blue
                }
            }
        }
        else
        {
            for (int row = 0; row < pFrame->height; row++)
            {
                pxcBYTE *pSrc = data.planes[0] + data.pitches[0] * (row + top) + 3 * left;
                pxcBYTE *pDst = (pxcBYTE *)(pFrame->data) + pFrame->stride * row;
                for (int col = 0; col < pFrame->width; col++, pSrc+=3, pDst+=3)
                {
                    pDst[0] = pSrc[2]; // red
                    pDst[1] = pSrc[1]; // green
                    pDst[2] = pSrc[0]; // blue
                }
            }
        }
        image->ReleaseAccess(&data);

        raiseNewFrame(pFrame);
		getServices().releaseFrame(pFrame);

		frameId++;
    }
}

}// namespace perc_device
