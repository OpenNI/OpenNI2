/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#ifndef DEPTHKINECTSTREAM_H
#define DEPTHKINECTSTREAM_H

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
#endif // DEPTHKINECTSTREAM_H
