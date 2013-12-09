#ifndef _PERC_BASE_STREAM_H_
#define _PERC_BASE_STREAM_H_

#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"

#include "pxcsmartptr.h"
#include "pxccapture.h"

namespace perc_device
{
class PerCBaseStream 
    : public oni::driver::StreamBase
{
public:
    PerCBaseStream(XnUInt32 idxDeviceInternal, XnUInt32 idxStream, const OniVideoMode &videoMode);
	virtual ~PerCBaseStream();

    bool isValid()
    {
        return (m_pxcDevice.IsValid() && m_pxcStream.IsValid());
    }

	virtual OniStatus start();
	virtual void stop();

	virtual OniStatus SetVideoMode(OniVideoMode*) = 0;
	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode) = 0;
	
    virtual OniBool isPropertySupported(int /*propertyId*/);
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);

	virtual void Mainloop() = 0;
protected:
	static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam);
protected:
    XnUInt32 m_idxDeviceInternal;
    PXCSmartPtr<PXCCapture::Device>         m_pxcDevice;
    XnUInt32 m_idxStream;
    PXCSmartPtr<PXCCapture::VideoStream>    m_pxcStream;

    void createStream();

	bool m_running;
	XN_THREAD_HANDLE m_threadHandle;
protected:
    OniVideoMode m_videoMode;

    OniBool m_mirror;
    OniCropping m_cropping;
    OniBool getFrameRect(int &left, int &top, int &right, int &bottom);
private:
    PerCBaseStream(const PerCBaseStream &);
    PerCBaseStream &operator =(const PerCBaseStream &);
};
}// namespace perc_device

#endif //_PERC_BASE_STREAM_H_