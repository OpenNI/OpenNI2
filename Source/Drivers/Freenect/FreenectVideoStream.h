#ifndef _FREENECT_IMAGE_STREAM_H_
#define _FREENECT_IMAGE_STREAM_H_

#include "FreenectStream.h"


class FreenectVideoStream : public FreenectStream
{	
private:
	static const OniSensorType sensor_type = ONI_SENSOR_COLOR;
	static const OniVideoMode supported_video_modes[];
	void buildFrame(void* data, OniDriverFrame* pFrame);
	
protected:	
	OniVideoMode video_mode;
	static OniSensorType getSensorType() { return sensor_type; }
	static OniVideoMode* getSupportedVideoModes();
	const OniVideoMode getVideoMode() const { return video_mode; }
	OniStatus setVideoMode(OniVideoMode requested_mode);
	
public:
	FreenectVideoStream(Freenect::FreenectDevice* pDevice);
	~FreenectVideoStream() { }

	static OniSensorInfo getSensorInfo()
	{
		OniVideoMode* modes = getSupportedVideoModes();
		//       sensorType, numSupportedVideoModes, pSupportedVideoModes
		return { getSensorType(), SIZE(modes), modes };
	}
	
	// from StreamBase
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
};


#endif // _FREENECT_IMAGE_STREAM_H_
