#ifndef _FREENECT_DEPTH_STREAM_H_
#define _FREENECT_DEPTH_STREAM_H_

#include "FreenectStream.h"


class FreenectDepthStream : public FreenectStream
{	
private:
	static const OniSensorType sensor_type = ONI_SENSOR_DEPTH;
	static const OniVideoMode supported_video_modes[];
	virtual void buildFrame(void* data, OniDriverFrame* pFrame);
	
protected:
	OniVideoMode video_mode;
	static OniSensorType getSensorType() { return sensor_type; }
	static OniVideoMode* getSupportedVideoModes();
	const OniVideoMode getVideoMode() const { return video_mode; }
	OniStatus setVideoMode(OniVideoMode requested_mode);
	
public:
	FreenectDepthStream(Freenect::FreenectDevice* pDevice);
	~FreenectDepthStream() { }	

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

#endif // _FREENECT_DEPTH_STREAM_H_
