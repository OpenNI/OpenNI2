#include "FreenectVideoStream.h"
#include "XnLib.h"
#include "libfreenect.hpp"


//	pixelFormat, resolutionX, resolutionY, fps
const OniVideoMode FreenectVideoStream::supported_video_modes[] = {
	{ ONI_PIXEL_FORMAT_RGB888, 640, 480, 30 },
};

FreenectVideoStream::FreenectVideoStream(Freenect::FreenectDevice* pDevice) : FreenectStream(pDevice)
{
	setVideoMode(getSupportedVideoModes()[0]);
}

OniVideoMode* FreenectVideoStream::getSupportedVideoModes()
{
	// make a copy to avoid need for const
	OniVideoMode* modes = new OniVideoMode[SIZE(supported_video_modes)];
	std::copy(supported_video_modes, supported_video_modes+SIZE(supported_video_modes), modes);
	return modes;
}
OniStatus FreenectVideoStream::setVideoMode(OniVideoMode requested_mode)
{
	OniVideoMode* supported_modes = getSupportedVideoModes();
	freenect_video_format format;
	freenect_resolution resolution;
	
	if (requested_mode == supported_modes[0])
	{
		format = FREENECT_VIDEO_RGB;
		resolution = FREENECT_RESOLUTION_MEDIUM;
	
		/* working format possiblities
		FREENECT_VIDEO_RGB
		FREENECT_VIDEO_YUV_RGB
		FREENECT_VIDEO_YUV_RAW
		*/
	}
	else
	{
		return ONI_STATUS_NOT_SUPPORTED;
	}
	
	try { device->setVideoFormat(format, resolution); }
	catch (std::runtime_error e)
	{
		printf("format-resolution combination not supported: %d-%d\n", format, resolution);
		return ONI_STATUS_NOT_SUPPORTED;
	}
	
	video_mode = requested_mode;
	return ONI_STATUS_OK;
}
void FreenectVideoStream::buildFrame(void* image, OniDriverFrame* pFrame)
{
	pFrame->frame.sensorType = ONI_SENSOR_COLOR;
	pFrame->frame.videoMode = video_mode;
	pFrame->frame.dataSize = device->getVideoBufferSize();
	pFrame->frame.width = video_mode.resolutionX;
	pFrame->frame.height = video_mode.resolutionY;
	pFrame->frame.stride = video_mode.resolutionX*3;
	pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
	pFrame->frame.croppingEnabled = FALSE;	
	
	// copy stream buffer from freenect
	pFrame->frame.data = xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);
	if (pFrame->frame.data == NULL)
	{
		XN_ASSERT(FALSE);
		return;
	}
	unsigned char* _data = static_cast<unsigned char*>(image);
	unsigned char* frame_data = static_cast<unsigned char*>(pFrame->frame.data);
	std::copy(_data, _data+pFrame->frame.dataSize, frame_data);		
}

// for StreamBase
OniStatus FreenectVideoStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	switch(propertyId)
	{
		default:
			return FreenectStream::getProperty(propertyId, data, pDataSize);

		case ONI_STREAM_PROPERTY_VIDEO_MODE:					// OniVideoMode*
			if (*pDataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}				
			*(static_cast<OniVideoMode*>(data)) = video_mode;
			return ONI_STATUS_OK;
	}
}
OniStatus FreenectVideoStream::setProperty(int propertyId, const void* data, int dataSize)
{
	switch(propertyId)
	{
		default:
			return FreenectStream::setProperty(propertyId, data, dataSize);

		case ONI_STREAM_PROPERTY_VIDEO_MODE:					// OniVideoMode*
			if (dataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}
			raisePropertyChanged(propertyId, data, dataSize);
			return setVideoMode(*(reinterpret_cast<const OniVideoMode*>(data)));
	}
}


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
