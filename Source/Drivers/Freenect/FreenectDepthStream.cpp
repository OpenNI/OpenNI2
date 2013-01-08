#include "FreenectDepthStream.h"


const OniVideoMode FreenectDepthStream::default_video_mode = makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30);

// Add video modes here as you implement them
FreenectDepthStream::FreenectDepthModeMap FreenectDepthStream::getSupportedVideoModes()
{
	FreenectDepthModeMap modes;
	//										pixelFormat, resolutionX, resolutionY, fps		freenect_video_format, freenect_resolution											
	modes[makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30)] = { FREENECT_DEPTH_REGISTERED, FREENECT_RESOLUTION_MEDIUM };
		
	
	return modes;
}
void FreenectDepthStream::populateFrame(void* data, OniDriverFrame* pFrame) const
{	
	pFrame->frame.sensorType = sensor_type;
	pFrame->frame.stride = video_mode.resolutionX*sizeof(uint16_t);
	pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
	pFrame->frame.croppingEnabled = FALSE;	
	pFrame->frame.dataSize = device->getDepthBufferSize();
	pFrame->frame.data = xnOSMallocAligned(sizeof(uint16_t)*pFrame->frame.dataSize, XN_DEFAULT_MEM_ALIGN);
	if (pFrame->frame.data == NULL)
	{
		XN_ASSERT(FALSE);
		return;
	}
	
	// copy stream buffer from freenect
	uint16_t* _data = static_cast<uint16_t*>(data);
	uint16_t* frame_data = static_cast<uint16_t*>(pFrame->frame.data);
	if (mirroring)
	{
		for (unsigned int i = 0; i < pFrame->frame.dataSize; i++)
		{
			// find corresponding mirrored pixel
			unsigned int row = i / video_mode.resolutionX;
			unsigned int col = video_mode.resolutionX - (i % video_mode.resolutionX);
			unsigned int target = (row * video_mode.resolutionX + col);
			// copy it to this pixel
			frame_data[i] = _data[target];
		}
	}
	else
		std::copy(_data, _data+pFrame->frame.dataSize, frame_data);
}

// for StreamBase
OniStatus FreenectDepthStream::setProperty(int propertyId, const void* data, int dataSize)
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
