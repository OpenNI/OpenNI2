#ifndef _PERC_DEPTH_STREAM_H_
#define _PERC_DEPTH_STREAM_H_

#include "PerCBaseStream.h"
#include "pxcprojection.h"
#include "pxcmetadata.h"

namespace perc_device
{
class PerCDepthStream 
    : public PerCBaseStream
{
public:
    PerCDepthStream(XnUInt32 idxDeviceInternal, XnUInt32 idxStream, const OniVideoMode &videoMode);

	OniStatus SetVideoMode(OniVideoMode* pVideoMode);
	OniStatus GetVideoMode(OniVideoMode* pVideoMode);

	virtual OniStatus convertDepthToColorCoordinates(StreamBase* /*colorStream*/, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY);
private:
     PXCSmartPtr<PXCProjection> m_pxcProjection;
private:
	void Mainloop();
};
}// namespace perc_device

#endif //_PERC_DEPTH_STREAM_H_