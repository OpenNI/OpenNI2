#ifndef __XNLINKINPUTSTREAM_H__
#define __XNLINKINPUTSTREAM_H__

#include "XnBitSet.h"
#include "XnLinkProtoLibDefs.h"
#include "XnLinkProtoUtils.h"

namespace xn
{

class LinkControlEndpoint;
class LinkMsgParser;
class IConnection;

class LinkInputStream
{
public:
    LinkInputStream();
    virtual ~LinkInputStream();

    virtual XnStatus Init(LinkControlEndpoint* pLinkControlEndpoint, 
                          XnStreamType streamType,
                          XnUInt16 nStreamID, 
                          IConnection* pConnection);

	virtual XnBool IsInitialized() const = 0;
    virtual void Shutdown();
    virtual void Reset();

	virtual XnStatus Start();
	virtual XnStatus Stop();
	virtual XnBool IsStreaming() const;
    virtual XnUInt16 GetStreamID() const;

	typedef void (XN_CALLBACK_TYPE* NewDataAvailableHandler)(void* pCookie);

	virtual XnStatus HandlePacket(const LinkPacketHeader& header, const XnUInt8* pData, XnBool& bPacketLoss) = 0;

	virtual void SetDumpName(const XnChar* strDumpName) = 0;
	virtual void SetDumpOn(XnBool bDumpOn) = 0;

    /* Stream Properties */
	virtual XnBool IsOutputFormatSupported(OniPixelFormat format) const;
	virtual OniPixelFormat GetOutputFormat() const;
	virtual XnStatus SetOutputFormat(OniPixelFormat format);

    virtual XnBool IsInterfaceSupported(XnUInt8 nInterfaceID) const;

    virtual XnStreamFragLevel GetStreamFragLevel() const = 0;

    virtual XnBool GetMirror() const;
    virtual XnStatus SetMirror(XnBool bMirror);

protected:
	virtual XnStatus StartImpl() = 0;
	virtual XnStatus StopImpl() = 0;

	virtual LinkMsgParser* CreateLinkMsgParser();
	LinkControlEndpoint* m_pLinkControlEndpoint;
    IConnection* m_pConnection;
    XnStreamType m_streamType;
    XnUInt16 m_nStreamID;
	OniPixelFormat m_outputFormat;
	volatile XnBool m_bStreaming;
	int m_streamingRefcount;

	xnl::BitSet m_supportedInterfaces;
	XnBool m_bMirror;
};

}

#endif // __XNLINKINPUTSTREAM_H__
