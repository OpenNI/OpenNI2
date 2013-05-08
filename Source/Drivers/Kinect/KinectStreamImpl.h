#ifndef _KINECT_STREAM_IMPL_H_
#define _KINECT_STREAM_IMPL_H_

#include "BaseKinectStream.h"
#include <Shlobj.h>
#include "NuiApi.h"
#include "XnList.h"

struct INuiSensor;

namespace kinect_device {
class KinectStreamImpl
{
public:
	KinectStreamImpl(INuiSensor * pNuiSensor, OniSensorType sensorType);
	
	virtual ~KinectStreamImpl();
	
	void addStream(BaseKinectStream* stream);
	
	void removeStream(BaseKinectStream* stream);	

	unsigned int getStreamCount();

	void setVideoMode(OniVideoMode* videoMode);
	
	OniStatus virtual start();

	void virtual stop();

	bool	isRunning() { return m_running; }

	OniSensorType getSensorType () { return m_sensorType; }

	void setSensorType(OniSensorType sensorType);
	
	void mainLoop();

	OniStatus setAutoWhiteBalance(BOOL val);

	OniStatus getAutoWhitBalance(BOOL *val);

	OniStatus setAutoExposure(BOOL val);

	OniStatus getAutoExposure(BOOL *val);
	
	OniStatus convertDepthToColorCoordinates(oni::driver::StreamBase* colorStream, 
								int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY);

	static NUI_IMAGE_RESOLUTION getNuiImagResolution(int resolutionX, int resolutionY);
	
private:
		
	void setDefaultVideoMode();
	
	NUI_IMAGE_TYPE getNuiImageType();

	static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam);

	xnl::List<BaseKinectStream*> m_streamList;
	// Thread
	INuiSensor* m_pNuiSensor;
	OniSensorType m_sensorType;
	bool		 m_running;
	HANDLE       m_hStreamHandle;
	HANDLE       m_hNextFrameEvent;
	OniVideoMode m_videoMode;
	
	XN_THREAD_HANDLE m_threadHandle;	
};
} // namespace kinect_device

#endif //_KINECT_STREAM_IMPL_H_
