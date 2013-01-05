#include "FreenectDepthStream.h"
#include "XnLib.h"
#include "libfreenect.hpp"


//	pixelFormat, resolutionX, resolutionY, fps
const OniVideoMode FreenectDepthStream::supported_video_modes[] = {
	{ ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30 },
};

FreenectDepthStream::FreenectDepthStream(Freenect::FreenectDevice* pDevice) : FreenectStream(pDevice)
{	
	setVideoMode(getSupportedVideoModes()[0]);
}

OniVideoMode* FreenectDepthStream::getSupportedVideoModes()
{
	// make a copy to avoid need for const
	OniVideoMode* modes = new OniVideoMode[SIZE(supported_video_modes)];
	std::copy(supported_video_modes, supported_video_modes+SIZE(supported_video_modes), modes);
	return modes;
}
OniStatus FreenectDepthStream::setVideoMode(OniVideoMode requested_mode)
{	
	freenect_depth_format format;
	freenect_resolution resolution;
	
	if (requested_mode == supported_video_modes[0])
	{
		format = FREENECT_DEPTH_11BIT;
		resolution = FREENECT_RESOLUTION_MEDIUM;
		
		/* working format possiblities
		FREENECT_DEPTH_10BIT
		FREENECT_DEPTH_REGISTERED
		FREENECT_DEPTH_MM
		*/
	}
	else
	{
		return ONI_STATUS_NOT_SUPPORTED;
	}
	
	try { device->setDepthFormat(format, resolution); }
	catch (std::runtime_error e)
	{
		printf("format-resolution combination not supported: %d-%d\n", format, resolution);
		return ONI_STATUS_NOT_SUPPORTED;
	}
	
	video_mode = requested_mode;
	return ONI_STATUS_OK;
}
void FreenectDepthStream::buildFrame(void* depth, OniDriverFrame* pFrame)
{
	pFrame->frame.sensorType = ONI_SENSOR_DEPTH;
	pFrame->frame.videoMode = video_mode;
	pFrame->frame.dataSize = device->getDepthBufferSize();
	pFrame->frame.width = video_mode.resolutionX;
	pFrame->frame.height = video_mode.resolutionY;
	pFrame->frame.stride = video_mode.resolutionX*sizeof(OniDepthPixel);
	pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
	pFrame->frame.croppingEnabled = FALSE;	

	// copy stream buffer from freenect
	pFrame->frame.data = xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);
	if (pFrame->frame.data == NULL)
	{
		XN_ASSERT(FALSE);
		return;
	}
	uint8_t* _data = static_cast<uint8_t*>(depth);
	uint8_t* frame_data = static_cast<uint8_t*>(pFrame->frame.data);
	std::copy(_data, _data+pFrame->frame.dataSize, frame_data);
}

// for StreamBase
OniStatus FreenectDepthStream::getProperty(int propertyId, void* data, int* pDataSize)
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
OniStatus FreenectDepthStream::setProperty(int propertyId, const void* data, int dataSize)
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


/* depth video modes reference

FREENECT_DEPTH_11BIT        = 0, //< 11 bit depth information in one uint16_t/pixel
FREENECT_DEPTH_10BIT        = 1, //< 10 bit depth information in one uint16_t/pixel
FREENECT_DEPTH_11BIT_PACKED = 2, //< 11 bit packed depth information
FREENECT_DEPTH_10BIT_PACKED = 3, //< 10 bit packed depth information
FREENECT_DEPTH_REGISTERED   = 4, //< processed depth data in mm, aligned to 640x480 RGB
FREENECT_DEPTH_MM           = 5, //< depth to each pixel in mm, but left unaligned to RGB image
FREENECT_DEPTH_DUMMY        = 2147483647, //< Dummy value to force enum to be 32 bits wide

ONI_PIXEL_FORMAT_DEPTH_1_MM = 100,
ONI_PIXEL_FORMAT_DEPTH_100_UM = 101,
ONI_PIXEL_FORMAT_SHIFT_9_2 = 102,
ONI_PIXEL_FORMAT_SHIFT_9_3 = 103,
*/
