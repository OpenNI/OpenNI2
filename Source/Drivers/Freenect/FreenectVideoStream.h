#ifndef _FREENECT_IMAGE_STREAM_H_
#define _FREENECT_IMAGE_STREAM_H_

#include "FreenectStream.h"


typedef std::map< OniVideoMode, std::pair<freenect_video_format, freenect_resolution> > FreenectVideoModeMap;

class FreenectVideoStream : public FreenectStream
{	
private:
	static const FreenectVideoModeMap supported_video_modes;
	static FreenectVideoModeMap getSupportedVideoModes();
	virtual void buildFrame(void* data, OniDriverFrame* pFrame);
	
protected:
	static const OniSensorType sensor_type = ONI_SENSOR_COLOR;
	OniVideoMode video_mode;
	OniStatus setVideoMode(OniVideoMode requested_mode);
	
public:
	FreenectVideoStream(Freenect::FreenectDevice* pDevice);
	~FreenectVideoStream() { }

	static OniSensorInfo getSensorInfo();
	static OniVideoMode makeOniVideoMode(OniPixelFormat pixel_format, int resolution_x, int resolution_y, int frames_per_second)
	{
		OniVideoMode mode;
		mode.pixelFormat = pixel_format;
		mode.resolutionX = resolution_x;
		mode.resolutionY = resolution_y;
		mode.fps = frames_per_second;
		return mode;
	}
	

	// from StreamBase
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
};


#endif // _FREENECT_IMAGE_STREAM_H_
