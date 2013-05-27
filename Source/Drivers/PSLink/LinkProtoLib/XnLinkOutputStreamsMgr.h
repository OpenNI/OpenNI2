#ifndef __XNLINKOUTPUTSTREAMSMGR_H__
#define __XNLINKOUTPUTSTREAMSMGR_H__

#include "XnLinkProtoLibDefs.h"
#include "XnLinkProtoUtils.h"
#include <XnArray.h>
#include <XnStatus.h>
#include <XnPlatform.h>

namespace xn
{

class ILinkMsgEncoderFactory;
class ILinkOutputStream;
class LinkOutputDataEndpoint;

class LinkOutputStreamsMgr
{
public:
	LinkOutputStreamsMgr();
	~LinkOutputStreamsMgr();
	XnStatus Init();
	void Shutdown();

	XnStatus InitOutputStream(XnUInt16 nStreamID, 
							  XnUInt32 nMaxMsgSize, 
							  XnUInt16 nMaxPacketSize,
							  XnLinkCompressionType compression, 
							  XnStreamFragLevel streamFragLevel, 
							  LinkOutputDataEndpoint* pOutputDataEndpoint);

	XnBool IsStreamInitialized(XnUInt16 nStreamID) const;
	
	void ShutdownOutputStream(XnUInt16 nStreamID);
	XnStatus SendData(XnUInt16 nStreamID, 
	                  XnUInt16 nMsgType, 
					  XnUInt16 nCID, 
					  XnLinkFragmentation fragmentation,
					  const void* pData, 
					  XnUInt32 nDataSize);

private:
	static const XnUInt16 INITIAL_PACKET_ID;
	xnl::Array<ILinkOutputStream*> m_outputStreams;
};

}

#endif // __XNLINKOUTPUTSTREAMSMGR_H__
