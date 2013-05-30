#ifndef __XNLINKPACKED10BITPARSER_H__
#define __XNLINKPACKED10BITPARSER_H__

#include "XnLinkMsgParser.h"

namespace xn
{

class LinkPacked10BitParser : public LinkMsgParser
{
public:
	LinkPacked10BitParser();
	virtual ~LinkPacked10BitParser();

	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);

private:
	XnUInt32 m_nState;
};

}

#endif // __XNLINKPACKED10BITPARSER_H__