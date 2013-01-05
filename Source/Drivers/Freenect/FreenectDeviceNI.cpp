#include "FreenectDeviceNI.h"
#include "XnLib.h"


FreenectDeviceNI::FreenectDeviceNI(freenect_context *_ctx, int _index) : Freenect::FreenectDevice(_ctx, _index)
{
	depth_stream = NULL;
	video_stream = NULL;
}
FreenectDeviceNI::~FreenectDeviceNI()
{
	destroyStream(depth_stream);
	destroyStream(video_stream);
}

// for DeviceBase
OniStatus FreenectDeviceNI::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
	*numSensors = 2;
	OniSensorInfo * sensors = new OniSensorInfo[*numSensors];
	sensors[0] = depth_stream->getSensorInfo();
	sensors[1] = video_stream->getSensorInfo();
	*pSensors = sensors;
	return ONI_STATUS_OK;
}
StreamBase* FreenectDeviceNI::createStream(OniSensorType sensorType)
{
	switch(sensorType)
	{
		case ONI_SENSOR_DEPTH:
			Freenect::FreenectDevice::startDepth();
			if (depth_stream == NULL)
				depth_stream = XN_NEW(FreenectDepthStream, this);
			return depth_stream;
		case ONI_SENSOR_COLOR:
			Freenect::FreenectDevice::startVideo();
			if (video_stream == NULL)
				video_stream = XN_NEW(FreenectVideoStream, this);
			return video_stream;
		// todo: IR
		default:
			//m_driverServices.errorLoggerAppend("FreenectDeviceNI: Can't create a stream of type %d", sensorType);
			return NULL;
	}
}
void FreenectDeviceNI::destroyStream(StreamBase* pStream)
{
	if (pStream == depth_stream)
	{
		Freenect::FreenectDevice::stopDepth();
		depth_stream = NULL;
	}
	if (pStream == video_stream)
	{
		Freenect::FreenectDevice::stopVideo();
		video_stream = NULL;
	}
	
	XN_DELETE(pStream);
}
