#include "FreenectDepthStream.h"


const OniVideoMode FreenectDepthStream::default_video_mode = makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30);

// Add video modes here as you implement them
FreenectDepthModeMap FreenectDepthStream::getSupportedVideoModes()
{
	FreenectDepthModeMap modes;
	//										pixelFormat, resolutionX, resolutionY, fps		freenect_video_format, freenect_resolution											
	modes[makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30)] = { FREENECT_DEPTH_11BIT, FREENECT_RESOLUTION_MEDIUM };
	
	
	return modes;
}
void FreenectDepthStream::populateFrame(void* depth, OniDriverFrame* pFrame) const
{
	pFrame->frame.sensorType = sensor_type;
	pFrame->frame.stride = video_mode.resolutionX*sizeof(OniDepthPixel);
	pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
	pFrame->frame.croppingEnabled = FALSE;	
	// copy stream buffer from freenect
	uint8_t* _data = static_cast<uint8_t*>(depth);
	uint8_t* frame_data = static_cast<uint8_t*>(pFrame->frame.data);
	std::copy(_data, _data+pFrame->frame.dataSize, frame_data);
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
