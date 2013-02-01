#ifndef _IR_KINECT_STREAM_H_
#define _IR_KINECT_STREAM_H_

#include "BaseKinectStream.h"

struct INuiSensor;
namespace kinect_device {


class IRKinectStream : public BaseKinectStream
{
public:
	IRKinectStream(KinectStreamImpl* pStreamImpl);

	virtual OniStatus start();

	virtual void frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect);

};
} // namespace kinect_device
#endif //_IR_KINECT_STREAM_H_
