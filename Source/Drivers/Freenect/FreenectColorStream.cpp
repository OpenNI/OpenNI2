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
	// copy stream buffer from freenect
	unsigned char* _data = static_cast<unsigned char*>(data);
	unsigned char* frame_data = static_cast<unsigned char*>(pFrame->frame.data);
	std::copy(_data, _data+pFrame->frame.dataSize, frame_data);
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
