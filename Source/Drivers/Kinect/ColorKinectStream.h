#ifndef _COLOR_KINECT_STREAM_H_
#define _COLOR_KINECT_STREAM_H_

#include "BaseKinectStream.h"

struct INuiSensor;
namespace kinect_device {


class ColorKinectStream : public BaseKinectStream
{
public:
	ColorKinectStream(KinectStreamImpl* pStreamImpl);
	
	virtual OniStatus start();

	virtual void frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect);

	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);

	virtual OniStatus setProperty(int propertyId, const void* data, int pDataSize);
	
	virtual OniBool isPropertySupported(int propertyId);
};
} // namespace kinect_device

#endif //_COLOR_KINECT_STREAM_H_
