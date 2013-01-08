#include "FreenectColorStream.h"


const OniVideoMode FreenectColorStream::default_video_mode = makeOniVideoMode(ONI_PIXEL_FORMAT_RGB888, 640, 480, 30);

// Add video modes here as you implement them
FreenectVideoStream::FreenectVideoModeMap FreenectColorStream::getSupportedVideoModes()
{
	FreenectVideoModeMap modes;
	//										pixelFormat, resolutionX, resolutionY, fps		freenect_video_format, freenect_resolution											
	modes[makeOniVideoMode(ONI_PIXEL_FORMAT_RGB888, 640, 480, 30)] = { FREENECT_VIDEO_RGB, FREENECT_RESOLUTION_MEDIUM };

	
	return modes;	

	/* working format possiblities
	FREENECT_VIDEO_RGB
	FREENECT_VIDEO_YUV_RGB
	FREENECT_VIDEO_YUV_RAW
	*/
}
void FreenectColorStream::populateFrame(void* data, OniDriverFrame* pFrame) const
{
	pFrame->frame.sensorType = sensor_type;
	pFrame->frame.stride = video_mode.resolutionX*3;
	pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
	pFrame->frame.croppingEnabled = FALSE;
	pFrame->frame.dataSize = device->getVideoBufferSize();
	pFrame->frame.data = xnOSMallocAligned(pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);
	if (pFrame->frame.data == NULL)
	{
		XN_ASSERT(FALSE);
		return;
	}
	// copy stream buffer from freenect
	switch (video_mode.pixelFormat)
	{
		default:
			printf("pixelFormat %s not supported by populateFrame\n", video_mode.pixelFormat);
			return;  
		case ONI_PIXEL_FORMAT_RGB888:
			unsigned char* _data = static_cast<unsigned char*>(data);
			unsigned char* frame_data = static_cast<unsigned char*>(pFrame->frame.data);
			if (mirroring)
			{
				for (unsigned int i = 0; i < pFrame->frame.dataSize; i += 3)
				{
					// find corresponding mirrored pixel
					unsigned int pixel = i / 3;
					unsigned int row = pixel / video_mode.resolutionX;
					unsigned int col = video_mode.resolutionX - (pixel % video_mode.resolutionX);
					unsigned int target = 3 * (row * video_mode.resolutionX + col);
					// copy it to this pixel
					frame_data[i] = _data[target];
					frame_data[i+1] = _data[target+1];
					frame_data[i+2] = _data[target+2];
				}
			}
			else
				std::copy(_data, _data+pFrame->frame.dataSize, frame_data);
			return;
	}
}

// for StreamBase
OniStatus FreenectColorStream::setProperty(int propertyId, const void* data, int dataSize)
{
	switch (propertyId)
	{
		default:
			return FreenectVideoStream::setProperty(propertyId, data, dataSize);
		case ONI_STREAM_PROPERTY_MIRRORING:		// OniBool
			if (dataSize != sizeof(OniBool))
			{
				printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniBool));
				return ONI_STATUS_ERROR;
			}
			mirroring = *(static_cast<const OniBool*>(data));
			return ONI_STATUS_OK;
	}
}


/* color video modes reference

FREENECT_VIDEO_RGB             = 0, //< Decompressed RGB mode (demosaicing done by libfreenect)
FREENECT_VIDEO_BAYER           = 1, //< Bayer compressed mode (raw information from camera)
FREENECT_VIDEO_YUV_RGB         = 5, //< YUV RGB mode
FREENECT_VIDEO_YUV_RAW         = 6, //< YUV Raw mode

ONI_PIXEL_FORMAT_RGB888 = 200,
ONI_PIXEL_FORMAT_YUV422 = 201,
ONI_PIXEL_FORMAT_JPEG = 204,
*/
