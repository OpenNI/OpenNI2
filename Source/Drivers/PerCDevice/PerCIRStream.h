#ifndef _PERC_IR_STREAM_H_
#define _PERC_IR_STREAM_H_

#include "PerCBaseStream.h"

namespace perc_device
{
class PerCIRStream 
    : public PerCBaseStream
{
public:
    PerCIRStream(XnUInt32 idxDeviceInternal, XnUInt32 idxStream, const OniVideoMode &videoMode);

	OniStatus SetVideoMode(OniVideoMode* pVideoMode);
	OniStatus GetVideoMode(OniVideoMode* pVideoMode);
private:
	void Mainloop();
};
}// namespace perc_device

#endif //_PERC_IR_STREAM_H_