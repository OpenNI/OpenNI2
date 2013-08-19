#ifndef _BASE_KINECT_STREAM_H_
#define _BASE_KINECT_STREAM_H_

#include "Driver\OniDriverAPI.h"
#include "XnLib.h"
#include <Shlobj.h>
#include "NuiApi.h"

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

class BaseKinectStream : public oni::driver::StreamBase
{
public:
	BaseKinectStream(KinectStreamImpl* pStreamImpl);

	virtual ~BaseKinectStream();

	virtual OniStatus start();

	virtual void stop();

	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);

	virtual OniBool isPropertySupported(int propertyId);

	virtual OniStatus SetVideoMode(OniVideoMode* pVideoMode);

	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode);
	
	virtual OniStatus SetCropping(OniCropping* cropping);
	
	virtual OniStatus GetCropping(OniCropping* cropping);
	
	bool isRunning() { return m_running; }

	virtual void frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect) = 0;
	
	
protected:
	KinectStreamImpl *m_pStreamImpl;
	OniVideoMode m_videoMode;
	OniCropping m_cropping;
	OniBool m_mirroring;
	bool m_running;
	
private:
	void destroy();

};
} // namespace kinect_device

// Macro to ensure the property size.
// To be used in getProperty/setProperty implementation.
// Borrowed from Drivers/PSLink/PrimeClientDefs.h and modified a bit.
#define EXACT_PROP_SIZE_OR_DO(size, type) if ((size_t)(size) != sizeof(type))

#define EXACT_PROP_SIZE_OR_RETURN(size, type) \
	EXACT_PROP_SIZE_OR_DO(size, type) \
	{ \
		printf("Unexpected size: %d != %d\n", (size), sizeof(type)); /* TODO: Better to use log */ \
		return ONI_STATUS_ERROR; \
	}

#endif //_BASE_KINECT_STREAM_H_
