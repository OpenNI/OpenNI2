#ifndef _FREENECT_STREAM_H_
#define _FREENECT_STREAM_H_

#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
//#include "XnHash.h"
//#include "XnEvent.h"
//#include "XnPlatform.h"
//#include "PS1080.h"
//#include "XnMath.h"

#include "XnPair.h"


#define SIZE(array) sizeof array / sizeof 0[array]

typedef struct  
{
	int refCount;
} FreenectStreamFrameCookie;

static bool operator==(const OniVideoMode& left, const OniVideoMode& right)
{
	return (left.pixelFormat == right.pixelFormat && left.resolutionX == right.resolutionX
					&& left.resolutionY == right.resolutionY && left.fps == right.fps) ? true : false;
}



class FreenectStream : public oni::driver::StreamBase
{
private:
	//virtual int getBytesPerPixel() = 0;

protected:
	Freenect::FreenectDevice* device;
	OniVideoMode video_mode; // derived classes should set this up in constructor
	static const OniVideoMode* supported_modes[];
	int frame_id; // number each frame
	bool running;
	
public:
	FreenectStream(Freenect::FreenectDevice* pDevice)
	{
		device = pDevice;
		frame_id = 1;
	}
	~FreenectStream()
	{
		stop();
	}
	
	//virtual void buildFrame(void* data, OniDriverFrame* pFrame) = 0;
	virtual void buildFrame(void* data, OniDriverFrame* pFrame) {};
	
	
	OniStatus start() {	running = true;	return ONI_STATUS_OK;	}
	void stop() { running = false; }
	
	OniBool isPropertySupported(int propertyId) {	return (getProperty(propertyId, NULL, NULL) == ONI_STATUS_NOT_SUPPORTED) ? false : true; }
	OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		switch(propertyId)
		{
			default:
			case ONI_STREAM_PROPERTY_CROPPING:						// OniCropping*
			case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:			// float: radians
			case ONI_STREAM_PROPERTY_VERTICAL_FOV:				// float: radians
			case ONI_STREAM_PROPERTY_MAX_VALUE:						// int
			case ONI_STREAM_PROPERTY_MIN_VALUE:						// int
			case ONI_STREAM_PROPERTY_STRIDE:							// int
			case ONI_STREAM_PROPERTY_MIRRORING:						// OniBool
			case ONI_STREAM_PROPERTY_NUMBER_OF_FRAMES:		// int
			// camera
			case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:	// OniBool
			case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:				// OniBool
				return ONI_STATUS_NOT_SUPPORTED;
				
			case ONI_STREAM_PROPERTY_VIDEO_MODE:					// OniVideoMode*
				if (*pDataSize != sizeof(OniVideoMode))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVideoMode));
					return ONI_STATUS_ERROR;
				}				
				*(static_cast<OniVideoMode*>(data)) = video_mode;
				return ONI_STATUS_OK;
		}
	}
	
	
	static xnl::Pair<int, int> convertResolution(freenect_resolution resolution)
	{
		switch(resolution)
		{
			case FREENECT_RESOLUTION_LOW:
				return xnl::Pair<int, int>(320, 240);
			case FREENECT_RESOLUTION_MEDIUM:
				return xnl::Pair<int, int>(640, 480);
			case FREENECT_RESOLUTION_HIGH:
				return xnl::Pair<int, int>(1280, 1024);
		}
	}


	
	
	void acquireFrame(void* data, uint32_t timestamp)
	{
		if (!running)
			return;
			
		//if (frame_id == 1)
			
		
		OniDriverFrame* pFrame = (OniDriverFrame*)xnOSCalloc(1, sizeof(OniDriverFrame));
		if (pFrame == NULL)
		{
			XN_ASSERT(FALSE);
			return;
		}
		pFrame->pDriverCookie = xnOSMalloc(sizeof(FreenectStreamFrameCookie));
		((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount = 1;

		pFrame->frame.frameIndex = frame_id++;
		pFrame->frame.timestamp = timestamp;
		pFrame->frame.videoMode = video_mode;
		pFrame->frame.width = video_mode.resolutionX;
		pFrame->frame.height = video_mode.resolutionY;
		pFrame->frame.cropOriginX = pFrame->frame.cropOriginY = 0;
		pFrame->frame.croppingEnabled = FALSE;
		
		buildFrame(data, pFrame);
		raiseNewFrame(pFrame);
	}
	
	void addRefToFrame(OniDriverFrame* pFrame) { ++((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount; }
	void releaseFrame(OniDriverFrame* pFrame)
	{
		if (0 == --((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount)
		{
			xnOSFree(pFrame->pDriverCookie);
			xnOSFreeAligned(pFrame->frame.data);
			xnOSFree(pFrame);
		}
	}
	
	
	/* unimplemented from StreamBase
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize) {return ONI_STATUS_NOT_IMPLEMENTED;}	
	virtual OniStatus invoke(int commandId, const void* data, int dataSize) {return ONI_STATUS_NOT_IMPLEMENTED;}
	virtual OniBool isCommandSupported(int commandId) {return FALSE;}
	virtual void setNewFrameCallback(NewFrameCallback handler, void* pCookie) { m_newFrameCallback = handler; m_newFrameCallbackCookie = pCookie; }
	virtual void setPropertyChangedCallback(PropertyChangedCallback handler, void* pCookie) { m_propertyChangedCallback = handler; m_propertyChangedCookie = pCookie; }
	virtual void notifyAllProperties() { return; }
	virtual OniStatus convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY) { return ONI_STATUS_NOT_SUPPORTED; }	
	*/
};

#endif // _FREENECT_STREAM_H_
