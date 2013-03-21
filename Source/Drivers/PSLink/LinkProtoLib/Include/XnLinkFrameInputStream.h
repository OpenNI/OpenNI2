#ifndef __XNLINKFRAMEINPUTSTREAM_H__
#define __XNLINKFRAMEINPUTSTREAM_H__

#include "XnLinkMsgParser.h"
#include "XnLinkInputStream.h"
#include <XnEvent.h>
#include <XnStatus.h>
#include <OpenNI.h>

#include <Driver/OniDriverTypes.h>

struct XnDumpFile;

namespace xn
{

class LinkControlEndpoint;

typedef struct NewFrameEventArgs
{
	OniDriverFrame* pFrame;
} NewFrameEventArgs;

typedef xnl::Event<NewFrameEventArgs> NewFrameEvent;

class MyFrameDealer
{
public:
	typedef struct  
	{
		int refCount;
	} MyFrameDealerCookie;

	OniDriverFrame* AcquireFrame(int dataSize)
	{
		OniDriverFrame* pFrame = (OniDriverFrame*)xnOSCalloc(1, sizeof(OniDriverFrame));
		if (pFrame == NULL)
		{
			XN_ASSERT(FALSE);
			return NULL;
		}

		pFrame->frame.data = xnOSMallocAligned(dataSize, XN_DEFAULT_MEM_ALIGN);
		if (pFrame->frame.data == NULL)
		{
			XN_ASSERT(FALSE);
			return NULL;
		}

		pFrame->pDriverCookie = xnOSMalloc(sizeof(MyFrameDealerCookie));
		((MyFrameDealerCookie*)pFrame->pDriverCookie)->refCount = 0;

		pFrame->frame.dataSize = dataSize;
		return pFrame;
	}

	void addRefToFrame(OniDriverFrame* pFrame)
	{
		++((MyFrameDealerCookie*)pFrame->pDriverCookie)->refCount;
	}

	void releaseFrame(OniDriverFrame* pFrame)
	{
		if (0 == --((MyFrameDealerCookie*)pFrame->pDriverCookie)->refCount)
		{
			xnOSFree(pFrame->pDriverCookie);
			xnOSFreeAligned(pFrame->frame.data);
			xnOSFree(pFrame);
		}
	}
};

class LinkFrameInputStream : public LinkInputStream
{
public:
	LinkFrameInputStream();
	virtual ~LinkFrameInputStream();
	virtual XnStatus Init(LinkControlEndpoint* pLinkControlEndpoint,
                          XnStreamType streamType,
                          XnUInt16 nStreamID, 
                          IConnection* pConnection);

	virtual void Reset();

	virtual XnBool IsInitialized() const;
	virtual void Shutdown();
	virtual XnStatus HandlePacket(const LinkPacketHeader& header, const XnUInt8* pData, XnBool& bPacketLoss);
	
	virtual void SetDumpName(const XnChar* strDumpName);
    virtual void SetDumpOn(XnBool bDumpOn);
    
	virtual XnStreamFragLevel GetStreamFragLevel() const { return XN_LINK_STREAM_FRAG_LEVEL_FRAMES; }

	typedef void (XN_CALLBACK_TYPE* NewFrameEventHandler)(const NewFrameEventArgs& args, void* pCookie);
	NewFrameEvent::EventInterface& GetNewFrameEvent() { return m_newFrameEvent; }

	void AddRefToFrame(OniDriverFrame* pFrame);
	void ReleaseFrame(OniDriverFrame* pFrame);
	
	virtual XnBool IsOutputFormatSupported(OniPixelFormat format) const;

	virtual const xnl::Array<XnStreamVideoMode>& GetSupportedVideoModes() const;
	virtual const XnStreamVideoMode& GetVideoMode() const;
	virtual XnStatus SetVideoMode(const XnStreamVideoMode& videoMode);

	virtual const XnShiftToDepthConfig& GetShiftToDepthConfig() const;
	virtual XnStatus GetShiftToDepthTables(const XnShiftToDepthTables*& pTables) const;
	virtual XnStatus SetDepthScale(XnDouble dDepthScale);

	virtual const XnLinkCameraIntrinsics& GetCameraIntrinsics() const { return m_cameraIntrinsics; }

	virtual const OniCropping& GetCropping() const;
	virtual XnStatus SetCropping(OniCropping cropping);

	virtual void GetFieldOfView(XnFloat* pHFOV, XnFloat* pVFOV) const { if (pHFOV) *pHFOV = m_fVFOV; if (pVFOV) *pVFOV = m_fVFOV; }

protected:
	virtual XnStatus StartImpl();
	virtual XnStatus StopImpl();

	virtual LinkMsgParser* CreateLinkMsgParser();

private:
    void DeallocateBuffers(XnUInt32 nBuffers);
	const void* GetDataImpl(XnUInt32 nBufferIndex) const;
	XnUInt32 GetDataSizeImpl(XnUInt32 nBufferIndex) const;
	XnUInt64 GetTimestampImpl(XnUInt32 nBufferIndex) const;

	static void Swap(XnUInt32& nVal1, XnUInt32& nVal2);
	XnUInt32 GetOutputBytesPerPixel() const;
    virtual XnUInt32 CalcBufferSize() const;
	XnUInt32 CalcExpectedSize() const;
	XnStatus UpdateCameraIntrinsics();

    enum {NUM_BUFFERS = 2};
	volatile XnBool m_bInitialized;

	struct BufferInfo
	{
		XnUInt64 m_nTimestamp;
		void* m_pData;
		XnUInt32 m_nSize;
	} m_buffersInfo[NUM_BUFFERS];


	NewFrameEvent m_newFrameEvent;

	MyFrameDealer* m_pBufferManager;

	XnUInt32 m_nStableBufferIdx;
	XnUInt32 m_nWorkingBufferIdx;
	
	XnBool m_currentFrameCorrupt;
	mutable XN_CRITICAL_SECTION_HANDLE m_hCriticalSection; //Protects buffers info

	XnUInt32 m_nBufferSize;
	LinkMsgParser* m_pLinkMsgParser;

	XnDumpFile* m_pDumpFile;
	XnChar m_strDumpName[XN_FILE_MAX_PATH];
	XnUInt32 m_nDumpFrameID;

	xnl::Array<XnStreamVideoMode> m_supportedVideoModes;
	XnStreamVideoMode m_videoMode;

	int m_frameIndex;
	OniCropping m_cropping;

	XnShiftToDepthConfig m_shiftToDepthConfig;
	XnShiftToDepthTables m_shiftToDepthTables;

	XnLinkCameraIntrinsics m_cameraIntrinsics;

	// Field of View
	XnFloat m_fHFOV, m_fVFOV;
};

}

#endif // __XNLINKFRAMEINPUTSTREAM_H__
