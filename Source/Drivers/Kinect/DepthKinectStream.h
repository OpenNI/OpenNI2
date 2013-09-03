#ifndef _DEPTH_KINECT_STREAM_H_
#define _DEPTH_KINECT_STREAM_H_

#include "BaseKinectStream.h"
#include "XnArray.h"

struct INuiSensor;
namespace kinect_device {

class DepthKinectStream : public BaseKinectStream
{
public:
	DepthKinectStream(KinectStreamImpl* pStreamImpl);

	virtual void frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect);

	virtual OniStatus convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY);
	
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);

	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);

	virtual OniBool isPropertySupported(int propertyId);

	virtual void notifyAllProperties();

private:
	xnl::Array<int> m_mappedCoordsBuffer;

	void populateFrameImageMetadata(OniFrame* pFrame, int dataUnitSize);
	void copyDepthPixelsStraight(const NUI_DEPTH_IMAGE_PIXEL* source, int numPoints, OniFrame* pFrame);
	void copyDepthPixelsWithImageRegistration(const NUI_DEPTH_IMAGE_PIXEL* source, int numPoints, OniFrame* pFrame);

	OniStatus setNearMode(OniBool value);
	OniStatus getNearMode(OniBool* pValue);
};
} // namespace kinect_device
#endif //_DEPTH_KINECT_STREAM_H_
