#ifndef _PERC_COLOR_STREAM_H_
#define _PERC_COLOR_STREAM_H_

#include "PerCBaseStream.h"

namespace perc_device
{
class PerCColorStream 
    : public PerCBaseStream
{
public:
    PerCColorStream(XnUInt32 idxDeviceInternal, XnUInt32 idxStream, const OniVideoMode &videoMode);

	virtual OniStatus SetVideoMode(OniVideoMode* pVideoMode);
	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode);

    virtual OniBool isPropertySupported(int propertyId);
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
private:
	void Mainloop();
};

}// namespace perc_device

#endif //_PERC_COLOR_STREAM_H_