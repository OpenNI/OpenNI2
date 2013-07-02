#ifndef __LINKINPUTSTREAMSMGR_H__
#define __LINKINPUTSTREAMSMGR_H__

#include "XnLinkDefs.h"
#include "XnLinkProtoLibDefs.h"
#include "XnLinkProtoUtils.h"
#include "XnLinkInputStream.h"
#include <XnStatus.h>
#include <XnHash.h>

namespace xn
{

class LinkControlEndpoint;
class IConnection;

class LinkInputStreamsMgr
{

public:
	LinkInputStreamsMgr();
	~LinkInputStreamsMgr();

	XnStatus Init();
	void Shutdown();

	void RegisterStreamOfType(XnStreamType streamType, const XnChar* strCreationInfo, XnUInt16 nStreamID);
	XnBool UnregisterStream(XnUInt16 nStreamID); // returns true if the unregistered stream was the last one
	XnBool HasStreamOfType(XnStreamType streamType, const XnChar* strCreationInfo, XnUInt16& nStreamID);

	XnStatus InitInputStream(LinkControlEndpoint* pLinkControlEndpoint, 
	                         XnStreamType streamType,
                             XnUInt16 nStreamID, 
                             IConnection* pConnection);

	void ShutdownInputStream(XnUInt16 nStreamID);
	XnStatus HandleData(const void* pData, XnUInt32 nSize);
	const LinkInputStream* GetInputStream(XnUInt16 nStreamID) const;
	LinkInputStream* GetInputStream(XnUInt16 nStreamID);

	XnBool HasStreams() const;

private:
	void HandlePacket(const LinkPacketHeader* pLinkPacketHeader);
	int FindStreamByType(XnStreamType streamType, const XnChar* strCreationInfo); //returns found streamId, or -1

	static const XnUInt32 FRAG_FLAGS_ALLOWED_CHANGES[4][4];
	static const XnUInt16 INITIAL_PACKET_ID;

	struct StreamInfo
	{
		XnUInt16 nNextPacketID;
		XnUInt16 nMsgType;
		XnLinkFragmentation prevFragmentation;
		XnStreamFragLevel streamFragLevel;
		LinkInputStream* pInputStream;
		XnBool packetLoss;

		XnStreamType streamType;
		const XnChar* strCreationInfo;
		int refCount;
	};

	StreamInfo m_streamInfos[XN_LINK_MAX_STREAMS];
};

}

#endif // __LINKINPUTSTREAMSMGR_H__
