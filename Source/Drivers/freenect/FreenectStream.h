#ifndef _FREENECT_STREAM_H_
#define _FREENECT_STREAM_H_

#include "Drivers/OniDriverAPI.h"
#include "XnLib.h"

namespace kinect_device {

class KinectStreamImpl;

static const int KINECT_RESOLUTION_X_80 = 80;
static const int KINECT_RESOLUTION_Y_60 = 60;
static const int KINECT_RESOLUTION_X_320 = 320;
static const int KINECT_RESOLUTION_Y_240 = 240;
static const int KINECT_RESOLUTION_X_640 = 640;
static const int KINECT_RESOLUTION_Y_480 = 480;
static const int KINECT_RESOLUTION_X_1280 = 1280;
static const int KINECT_RESOLUTION_Y_960 = 960;

typedef struct  
{
	int refCount;
} KinectStreamFrameCookie;

class BaseKinectStream : public oni::driver::StreamBase
{
public:
	BaseKinectStream(KinectStreamImpl* pStreamImpl);

	virtual ~BaseKinectStream();

	OniStatus BaseKinectStream::start();

	void BaseKinectStream::stop();

	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);

	virtual OniBool isPropertySupported(int propertyId);

	virtual void notifyAllProperties();

	virtual OniStatus SetVideoMode(OniVideoMode* pVideoMode);

	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode);

	void addRefToFrame(OniDriverFrame* pFrame);
	
	void releaseFrame(OniDriverFrame* pFrame);

	bool isRunning() { return m_running; }

	virtual void frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect) = 0;
	
	
protected:
	KinectStreamImpl *m_pStreamImpl;
	OniVideoMode m_videoMode;
	
	bool m_running;
	
private:
	void destroy();

};
} // namespace kinect_device
#endif //_FREENECT_STREAM_H_
