#ifndef _FREENECT_COLOR_STREAM_H_
#define _FREENECT_COLOR_STREAM_H_

#include "FreenectVideoStream.h"
#include "Driver/OniDriverAPI.h"
#include "libfreenect.hpp"


typedef std::map< OniVideoMode, std::pair<freenect_video_format, freenect_resolution> > FreenectVideoModeMap;

class FreenectColorStream : public FreenectVideoStream
{
private:
	static const OniSensorType sensor_type = ONI_SENSOR_COLOR;
	static const OniVideoMode default_video_mode;
	static FreenectVideoModeMap getSupportedVideoModes();
	virtual void populateFrame(void* image, OniDriverFrame* pFrame) const;
	OniStatus setVideoMode(OniVideoMode requested_mode)
	{
		FreenectVideoModeMap::const_iterator matched_mode_iter = getSupportedVideoModes().find(requested_mode);
		if (matched_mode_iter == getSupportedVideoModes().end())
			return ONI_STATUS_NOT_SUPPORTED;
		try { device->setVideoFormat(matched_mode_iter->second.first, matched_mode_iter->second.second); }
		catch (std::runtime_error e)
		{
			printf("format-resolution combination not supported by libfreenect: %d-%d\n", matched_mode_iter->second.first, matched_mode_iter->second.second);
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
