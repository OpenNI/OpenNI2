#ifndef _FREENECT_STREAM_H_
#define _FREENECT_STREAM_H_

#include "libfreenect.h"
#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"
#include "PS1080.h"
#include "XnMath.h"

#include "XnPair.h"

typedef struct  
{
	int refCount;
} FreenectStreamFrameCookie;


class FreenectStream : public oni::driver::StreamBase
{
private:
	virtual int getBytesPerPixel() = 0;
	virtual OniStatus getVideoMode(OniVideoMode* pVideoMode) = 0;

protected:
	Freenect::FreenectDevice* device;
	int frameId; // number frames starting with 1
	bool running;
	
public:
	FreenectStream(Freenect::FreenectDevice* pDevice)
	{
		device = pDevice;
		frameId = 1;
	}
	~FreenectStream()
	{
		stop();
	}
	
	virtual void buildFrame(void* data, uint32_t timestamp) = 0;
	
	OniStatus start()
	{
		running = true;
		return ONI_STATUS_OK;		
	}
	void stop()
	{
		running = false;
	}	
	
	
	/*
	// Stream properties
	enum
	{
		ONI_STREAM_PROPERTY_CROPPING			= 0, // OniCropping*
		ONI_STREAM_PROPERTY_HORIZONTAL_FOV		= 1, // float: radians
		ONI_STREAM_PROPERTY_VERTICAL_FOV		= 2, // float: radians
		ONI_STREAM_PROPERTY_VIDEO_MODE			= 3, // OniVideoMode*
	
		ONI_STREAM_PROPERTY_MAX_VALUE			= 4, // int
		ONI_STREAM_PROPERTY_MIN_VALUE			= 5, // int
	
		ONI_STREAM_PROPERTY_STRIDE			= 6, // int
		ONI_STREAM_PROPERTY_MIRRORING			= 7, // OniBool
	
		ONI_STREAM_PROPERTY_NUMBER_OF_FRAMES		= 8, // int
	
		// Camera
		ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE		= 100, // OniBool
		ONI_STREAM_PROPERTY_AUTO_EXPOSURE			= 101, // OniBool
	};
	*/	
	OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		switch(propertyId)
		{
			default:
				return ONI_STATUS_NOT_SUPPORTED;
			case ONI_STREAM_PROPERTY_VIDEO_MODE:
				if (*pDataSize != sizeof(OniVideoMode))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVideoMode));
					return ONI_STATUS_ERROR;
				}
				return getVideoMode((OniVideoMode*)data);			
		}
	}

	// freenect_resolution (libfreenect.h) to (x,y)
	//int* resolution_convert(freenect_resolution resolution) {
	xnl::Pair<int, int> resolution_convert(freenect_resolution resolution)
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
	
	
	void addRefToFrame(OniDriverFrame* pFrame)
	{
		++((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount;
	}
	void releaseFrame(OniDriverFrame* pFrame)
	{
		if (0 == --((FreenectStreamFrameCookie*)pFrame->pDriverCookie)->refCount)
		{
			xnOSFree(pFrame->pDriverCookie);
			xnOSFreeAligned(pFrame->frame.data);
			xnOSFree(pFrame);
		}
	}
};



// PARTIAL REFERENCE FROM TestDevice.cpp
/*
class TestStream : public oni::driver::StreamBase
{
public:
	TestStream() : oni::driver::StreamBase()
	{
		m_osEvent.Create(TRUE);
		m_sendCount = 0;
	}

	virtual OniStatus SetVideoMode(OniVideoMode*) = 0;
	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode) = 0;


	OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
		{
			if (dataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}
			return SetVideoMode((OniVideoMode*)data);
		}
		else if (propertyId == 666)
		{
			if (dataSize != sizeof(int))
			{
				printf("Unexpected size: %d != %d\n", dataSize, sizeof(int));
				return ONI_STATUS_ERROR;
			}

			// Increment the send count.
			m_cs.Lock();
			m_sendCount += *((int*)data);
			m_cs.Unlock();

			// Raise the OS event, to allow thread to start working.
			m_osEvent.Set();
		}

		return ONI_STATUS_NOT_IMPLEMENTED;
	}

protected:

	int singleRes(int x, int y) {return y*TEST_RESOLUTION_X+x;}
	int m_sendCount;

	xnl::CriticalSection m_cs;
	xnl::OSEvent m_osEvent;
};
*/

// END REFERENCE



#endif // _FREENECT_STREAM_H_
