#include "PerCIRStream.h"

namespace perc_device
{
PerCIRStream::PerCIRStream(XnUInt32 idxDeviceInternal, XnUInt32 idxStream, const OniVideoMode &videoMode)
    : PerCBaseStream(idxDeviceInternal, idxStream, videoMode)
{
}

OniStatus PerCIRStream::SetVideoMode(OniVideoMode *pVideoMode)
{
    if (m_running)
        return ONI_STATUS_NOT_SUPPORTED;
    m_videoMode.pixelFormat = pVideoMode->pixelFormat;
    m_videoMode.fps         = pVideoMode->fps;
    m_videoMode.resolutionX = pVideoMode->resolutionX;
    m_videoMode.resolutionY = pVideoMode->resolutionY;    
    return ONI_STATUS_OK;
}
OniStatus PerCIRStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	pVideoMode->pixelFormat = m_videoMode.pixelFormat;
    pVideoMode->fps         = m_videoMode.fps;
	pVideoMode->resolutionX = m_videoMode.resolutionX;
	pVideoMode->resolutionY = m_videoMode.resolutionY;
	return ONI_STATUS_OK;
}

void PerCIRStream::Mainloop()
{
    if (NULL == m_pxcStream)
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

        PXCImage::ImageData data;
        image->AcquireAccess(PXCImage::ACCESS_READ, &data);
        if (PXCImage::COLOR_FORMAT_DEPTH != data.format)
        {
            image->ReleaseAccess(&data);
            continue;
        }

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
        pFrame->sensorType = ONI_SENSOR_DEPTH;
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
        pFrame->stride = sizeof(short) * pFrame->width;

        if (m_mirror)
        {
            for (int row = 0; row < pFrame->height; row++)
            {
                pxcI16 *pSrc = (pxcI16 *)(data.planes[1] + data.pitches[1] * (row + top) + sizeof(short) * (m_videoMode.resolutionX - 1 - left));
                pxcI16 *pDst = (pxcI16 *)((pxcBYTE *)(pFrame->data) + pFrame->stride * row);
                for (int col = 0; col < pFrame->width; col++, pSrc--, pDst++)
                {
                    *pDst = *pSrc;
                }
            }
        }
        else
        {
            for (int row = 0; row < pFrame->height; row++)
            {
                pxcI16 *pSrc = (pxcI16 *)(data.planes[1] + data.pitches[1] * (row + top) + sizeof(short) * left);
                pxcI16 *pDst = (pxcI16 *)((pxcBYTE *)(pFrame->data) + pFrame->stride * row);
                memcpy(pDst, pSrc, pFrame->stride);
            }
        }

        image->ReleaseAccess(&data);

        raiseNewFrame(pFrame);
		getServices().releaseFrame(pFrame);

		frameId++;
    }
}

} //namespace perc_device