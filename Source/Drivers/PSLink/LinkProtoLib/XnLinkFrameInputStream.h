#ifndef __XNLINKFRAMEINPUTSTREAM_H__
#define __XNLINKFRAMEINPUTSTREAM_H__

#include "XnLinkMsgParser.h"
#include "XnLinkInputStream.h"
#include <XnEvent.h>
#include <XnStatus.h>
#include <OpenNI.h>

#include <Driver/OniDriverAPI.h>

struct XnDumpFile;

namespace xn
{

class LinkControlEndpoint;

typedef struct NewFrameEventArgs
{
	OniFrame* pFrame;
} NewFrameEventArgs;

typedef xnl::Event<NewFrameEventArgs> NewFrameEvent;

class LinkFrameInputStream : public LinkInputStream
{
public:
	LinkFrameInputStream();
	virtual ~LinkFrameInputStream();
	virtual XnStatus Init(LinkControlEndpoint* pLinkControlEndpoint,
                          XnStreamType streamType,
                          XnUInt16 nStreamID, 
                          IConnection* pConnection);

	virtual void SetStreamServices(oni::driver::StreamServices* pServices) { m_pServices = pServices; }

	XnUInt32 GetRequiredFrameSize() const { return CalcBufferSize(); }

	virtual void Reset();

	virtual XnBool IsInitialized() const;
	virtual void Shutdown();
	virtual XnStatus HandlePacket(const LinkPacketHeader& header, const XnUInt8* pData, XnBool& bPacketLoss);
	
	virtual void SetDumpName(const XnChar* strDumpName);
    virtual void SetDumpOn(XnBool bDumpOn);
    
	virtual XnStreamFragLevel GetStreamFragLevel() const { return XN_LINK_STREAM_FRAG_LEVEL_FRAMES; }

	typedef void (XN_CALLBACK_TYPE* NewFrameEventHandler)(const NewFrameEventArgs& args, void* pCookie);
	NewFrameEvent::EventInterface& GetNewFrameEvent() { return m_newFrameEvent; }

	virtual XnBool IsOutputFormatSupported(OniPixelFormat format) const;

	virtual const xnl::Array<XnFwStreamVideoMode>& GetSupportedVideoModes() const;
	virtual const XnFwStreamVideoMode& GetVideoMode() const;
	virtual XnStatus SetVideoMode(const XnFwStreamVideoMode& videoMode);

	virtual const XnShiftToDepthConfig& GetShiftToDepthConfig() const;
	virtual XnStatus GetShiftToDepthTables(const XnShiftToDepthTables*& pTables) const;
	virtual XnStatus SetDepthScale(XnDouble dDepthScale);

	virtual const XnLinkCameraIntrinsics& GetCameraIntrinsics() const { return m_cameraIntrinsics; }

	virtual const OniCropping& GetCropping() const;
	virtual XnStatus SetCropping(OniCropping cropping);

	virtual void GetFieldOfView(XnFloat* pHFOV, XnFloat* pVFOV) const { if (pHFOV) *pHFOV = m_fHFOV; if (pVFOV) *pVFOV = m_fVFOV; }

protected:
	virtual XnStatus StartImpl();
	virtual XnStatus StopImpl();

	virtual LinkMsgParser* CreateLinkMsgParser();

private:
	class DefaultStreamServices : public oni::driver::StreamServices
	{
	public:
		DefaultStreamServices();
		void setStream(LinkFrameInputStream* pStream);
		static int ONI_CALLBACK_TYPE getDefaultRequiredFrameSizeCallback(void* streamServices);
		static OniFrame* ONI_CALLBACK_TYPE acquireFrameCallback(void* streamServices);
		static void ONI_CALLBACK_TYPE releaseFrameCallback(void* streamServices, OniFrame* pFrame);
		static void ONI_CALLBACK_TYPE addFrameRefCallback(void* streamServices, OniFrame* pFrame);
	};

	struct LinkOniFrame : public OniFrame
	{
		int refCount;
	};

	static void Swap(XnUInt32& nVal1, XnUInt32& nVal2);
	XnUInt32 GetOutputBytesPerPixel() const;
    virtual XnUInt32 CalcBufferSize() const;
	XnUInt32 CalcExpectedSize() const;
	XnStatus UpdateCameraIntrinsics();

	DefaultStreamServices m_defaultServices;

	oni::driver::StreamServices* m_pServices;

	volatile XnBool m_bInitialized;

	NewFrameEvent m_newFrameEvent;
	OniFrame* m_pCurrFrame;

	XnBool m_currentFrameCorrupt;
	mutable XN_CRITICAL_SECTION_HANDLE m_hCriticalSection; //Protects buffers info

	XnUInt32 m_nBufferSize;
	LinkMsgParser* m_pLinkMsgParser;

	XnDumpFile* m_pDumpFile;
	XnChar m_strDumpName[XN_FILE_MAX_PATH];
	XnUInt32 m_nDumpFrameID;

	xnl::Array<XnFwStreamVideoMode> m_supportedVideoModes;
	XnFwStreamVideoMode m_videoMode;

	int m_frameIndex;
	OniCropping m_cropping;

	XnShiftToDepthConfig m_shiftToDepthConfig;
	XnShiftToDepthTables m_shiftToDepthTables;

	XnLinkCameraIntrinsics m_cameraIntrinsics;

	// Field of View
	XnFloat m_fHFOV; 
	XnFloat m_fVFOV;
};

}

#endif // __XNLINKFRAMEINPUTSTREAM_H__
