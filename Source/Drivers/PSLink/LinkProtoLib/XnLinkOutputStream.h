#ifndef __XNLINKOUTPUTSTREAM_H__
#define __XNLINKOUTPUTSTREAM_H__

#include "ILinkOutputStream.h"
#include "XnLinkProtoUtils.h"

namespace xn
{

class LinkMsgEncoder;

class LinkOutputStream : public ILinkOutputStream
{
public:
	LinkOutputStream();
	virtual ~LinkOutputStream();

	virtual XnStatus Init(XnUInt16 nStreamID, 
	                      XnUInt32 nMaxMsgSize, 
						  XnUInt16 nMaxPacketSize, 
						  XnLinkCompressionType compression, 
						  XnUInt16 nInitialPacketID,
						  LinkOutputDataEndpoint* pOutputDataEndpoint);

	virtual XnBool IsInitialized() const;
	virtual void Shutdown();
	virtual XnLinkCompressionType GetCompression() const;

	virtual XnStatus SendData(XnUInt16 nMsgType, 
							  XnUInt16 nCID, 
							  XnLinkFragmentation fragmentation,
							  const void* pData, 
							  XnUInt32 nDataSize) const;

protected:
	virtual XnStatus CreateLinkMsgEncoder(LinkMsgEncoder*& pLinkMsgEncoder);

private:
	XnBool m_bInitialized;
	XnUInt16 m_nStreamID;
	XnLinkCompressionType m_compression;
	LinkMsgEncoder* m_pLinkMsgEncoder;
	LinkOutputDataEndpoint* m_pOutputDataEndpoint;
	mutable XnUInt16 m_nPacketID;
};
}

#endif // __XNLINKOUTPUTSTREAM_H__
