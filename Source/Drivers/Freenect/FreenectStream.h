#ifndef _FREENECT_STREAM_H_
#define _FREENECT_STREAM_H_

#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include "XnLib.h"


#define SIZE(array) sizeof array / sizeof 0[array]

struct RetrieveKey
{
	template <typename T>
	typename T::first_type operator()(T pair) const
	{
		return pair.first;
	}
};

typedef struct  
{
	int refCount;
} FreenectStreamFrameCookie;


using namespace oni::driver;

class FreenectStream : public StreamBase
{
private:
	int frame_id; // number each frame
	virtual void buildFrame(void* data, OniDriverFrame* pFrame) const = 0;
	
protected:
	static const OniSensorType sensor_type;
	Freenect::FreenectDevice* device;
	bool running; // acquireFrame() does something iff true

public:
	FreenectStream(Freenect::FreenectDevice* pDevice)
	{
		device = pDevice;
		frame_id = 1;
	}
	~FreenectStream() { stop();	}

	virtual void acquireFrame(void* data, uint32_t timestamp)
	{
		if (!running)
			return;			
		
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
		
		buildFrame(data, pFrame);
		raiseNewFrame(pFrame);
	}

	// from StreamBase
	OniStatus start() {	running = true;	return ONI_STATUS_OK;	}
	void stop() { running = false; }
	virtual void addRefToFrame(OniDriverFrame* pFrame) { ++((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount; }
	virtual void releaseFrame(OniDriverFrame* pFrame)
	{
		if (0 == --((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount)
		{
			xnOSFree(pFrame->pDriverCookie);
			xnOSFreeAligned(pFrame->frame.data);
			xnOSFree(pFrame);
		}
	}
	// property handlers are empty skeletons by default
	// only add here if the property is generic to all children
	// otherwise, implement in child and call these in default case (see FreenectDepthStream.h)
	OniBool isPropertySupported(int propertyId) { return (getProperty(propertyId, NULL, NULL) != ONI_STATUS_NOT_SUPPORTED); }
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		switch(propertyId)
		{
			default:
			case ONI_STREAM_PROPERTY_CROPPING:						// OniCropping*
			case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:			// float: radians
			case ONI_STREAM_PROPERTY_VERTICAL_FOV:				// float: radians
			case ONI_STREAM_PROPERTY_VIDEO_MODE:					// OniVideoMode*
			case ONI_STREAM_PROPERTY_MAX_VALUE:						// int
			case ONI_STREAM_PROPERTY_MIN_VALUE:						// int
			case ONI_STREAM_PROPERTY_STRIDE:							// int
			case ONI_STREAM_PROPERTY_MIRRORING:						// OniBool
			case ONI_STREAM_PROPERTY_NUMBER_OF_FRAMES:		// int
			// camera
			case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:	// OniBool
			case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:				// OniBool
				return ONI_STATUS_NOT_SUPPORTED;
		}
	}
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		switch(propertyId)
		{
			default:
			case ONI_STREAM_PROPERTY_CROPPING:						// OniCropping*
			case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:			// float: radians
			case ONI_STREAM_PROPERTY_VERTICAL_FOV:				// float: radians
			case ONI_STREAM_PROPERTY_VIDEO_MODE:					// OniVideoMode*
			case ONI_STREAM_PROPERTY_MAX_VALUE:						// int
			case ONI_STREAM_PROPERTY_MIN_VALUE:						// int
			case ONI_STREAM_PROPERTY_STRIDE:							// int
			case ONI_STREAM_PROPERTY_MIRRORING:						// OniBool
			case ONI_STREAM_PROPERTY_NUMBER_OF_FRAMES:		// int
			// camera
			case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:	// OniBool
			case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:				// OniBool
				return ONI_STATUS_NOT_SUPPORTED;
		}
	}
	

	/* todo : from StreamBase
	virtual OniStatus convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY) { return ONI_STATUS_NOT_SUPPORTED; }	
	*/
};


#endif // _FREENECT_STREAM_H_
