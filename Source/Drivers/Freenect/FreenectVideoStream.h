#ifndef _FREENECT_VIDEO_STREAM_H_
#define _FREENECT_VIDEO_STREAM_H_

#include "FreenectStream.h"
#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include <algorithm>


static OniVideoMode makeOniVideoMode(OniPixelFormat pixel_format, int resolution_x, int resolution_y, int frames_per_second)
{
	OniVideoMode mode;
	mode.pixelFormat = pixel_format;
	mode.resolutionX = resolution_x;
	mode.resolutionY = resolution_y;
	mode.fps = frames_per_second;
	return mode;
}
static bool operator==(const OniVideoMode& left, const OniVideoMode& right)
{
	return (left.pixelFormat == right.pixelFormat && left.resolutionX == right.resolutionX
					&& left.resolutionY == right.resolutionY && left.fps == right.fps);
}
static bool operator<(const OniVideoMode& left, const OniVideoMode& right)
{
	return (left.resolutionX*left.resolutionY < right.resolutionX*right.resolutionY);
}


class FreenectVideoStream : public FreenectStream
{	
private:
	virtual OniStatus setVideoMode(OniVideoMode requested_mode) = 0;
	virtual void populateFrame(void* data, OniDriverFrame* pFrame) const = 0;
	virtual void buildFrame(void* data, OniDriverFrame* pFrame) const
	{
		pFrame->frame.videoMode = video_mode;
		pFrame->frame.width = video_mode.resolutionX;
		pFrame->frame.height = video_mode.resolutionY;
		populateFrame(data, pFrame);
	}
	
protected:
	typedef std::map< OniVideoMode, std::pair<freenect_video_format, freenect_resolution> > FreenectVideoModeMap;
	OniVideoMode video_mode;
	bool mirroring;
	
public:
	FreenectVideoStream(Freenect::FreenectDevice* pDevice) : FreenectStream(pDevice) { }
	~FreenectVideoStream() { }

	// from StreamBase
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		switch (propertyId)
		{
			default:
				return FreenectStream::getProperty(propertyId, data, pDataSize);
			case ONI_STREAM_PROPERTY_VIDEO_MODE:	// OniVideoMode*
				if (*pDataSize != sizeof(OniVideoMode))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVideoMode));
					return ONI_STATUS_ERROR;
				}				
				*(static_cast<OniVideoMode*>(data)) = video_mode;
				return ONI_STATUS_OK;
			case ONI_STREAM_PROPERTY_MIRRORING:		// OniBool
				if (*pDataSize != sizeof(OniBool))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniBool));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<OniBool*>(data)) = mirroring;						
				return ONI_STATUS_OK;
		}
	}
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		switch (propertyId)
		{
			default:
				return FreenectStream::setProperty(propertyId, data, dataSize);
			case ONI_STREAM_PROPERTY_VIDEO_MODE:	// OniVideoMode*
				if (dataSize != sizeof(OniVideoMode))
				{
					printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniVideoMode));
					return ONI_STATUS_ERROR;
				}
				if (ONI_STATUS_OK != setVideoMode(*(static_cast<const OniVideoMode*>(data))))
				{
					return ONI_STATUS_NOT_SUPPORTED;
				}
				raisePropertyChanged(propertyId, data, dataSize);
				return ONI_STATUS_OK;
		}
	}
};


/* image video modes reference

FREENECT_VIDEO_RGB             = 0, //< Decompressed RGB mode (demosaicing done by libfreenect)
FREENECT_VIDEO_BAYER           = 1, //< Bayer compressed mode (raw information from camera)
FREENECT_VIDEO_IR_8BIT         = 2, //< 8-bit IR mode
FREENECT_VIDEO_IR_10BIT        = 3, //< 10-bit IR mode
FREENECT_VIDEO_IR_10BIT_PACKED = 4, //< 10-bit packed IR mode
FREENECT_VIDEO_YUV_RGB         = 5, //< YUV RGB mode
FREENECT_VIDEO_YUV_RAW         = 6, //< YUV Raw mode
FREENECT_VIDEO_DUMMY           = 2147483647, //< Dummy value to force enum to be 32 bits wide

ONI_PIXEL_FORMAT_RGB888 = 200,
ONI_PIXEL_FORMAT_YUV422 = 201,
ONI_PIXEL_FORMAT_GRAY8 = 202,
ONI_PIXEL_FORMAT_GRAY16 = 203,
ONI_PIXEL_FORMAT_JPEG = 204,
*/


#endif // _FREENECT_VIDEO_STREAM_H_
