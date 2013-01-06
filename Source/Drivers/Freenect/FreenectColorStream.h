#ifndef _FREENECT_COLOR_STREAM_H_
#define _FREENECT_COLOR_STREAM_H_

#include "FreenectVideoStream.h"
#include "Driver/OniDriverAPI.h"
#include "libfreenect.hpp"


class FreenectColorStream : public FreenectVideoStream
{
private:
	static const OniSensorType sensor_type = ONI_SENSOR_COLOR;
	static const OniVideoMode default_video_mode;
	static FreenectVideoModeMap getSupportedVideoModes();
	virtual void populateFrame(void* data, OniDriverFrame* pFrame) const;
	OniStatus setVideoMode(OniVideoMode requested_mode)
	{
		FreenectVideoModeMap supported_video_modes = getSupportedVideoModes();
		FreenectVideoModeMap::const_iterator matched_mode_iter = supported_video_modes.find(requested_mode);
		if (matched_mode_iter == supported_video_modes.end())
			return ONI_STATUS_NOT_SUPPORTED;			
		
		freenect_video_format format = matched_mode_iter->second.first;
		freenect_resolution resolution = matched_mode_iter->second.second;
		
		try { device->setVideoFormat(format, resolution); }
		catch (std::runtime_error e)
		{
			printf("format-resolution combination not supported by libfreenect: %d-%d\n", format, resolution);
			return ONI_STATUS_NOT_SUPPORTED;
		}
		video_mode = requested_mode;
		return ONI_STATUS_OK;
	}

public:
	FreenectColorStream(Freenect::FreenectDevice* pDevice) : FreenectVideoStream(pDevice) { setVideoMode(default_video_mode); }
	~FreenectColorStream() { }
	
	static OniSensorInfo getSensorInfo()
	{
		FreenectVideoModeMap supported_modes = getSupportedVideoModes();
		OniVideoMode* modes = new OniVideoMode[supported_modes.size()];
		std::transform(supported_modes.begin(), supported_modes.end(), modes, RetrieveKey());
		return { sensor_type, SIZE(modes), modes }; // sensorType, numSupportedVideoModes, pSupportedVideoModes
	}
};


#endif // _FREENECT_COLOR_STREAM_H_
