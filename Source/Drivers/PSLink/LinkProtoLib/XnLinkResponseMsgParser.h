#ifndef __XNLINKRESPONSEMSGPARSER_H__
#define __XNLINKRESPONSEMSGPARSER_H__

#include "XnLinkMsgParser.h"

namespace xn
{

class LinkResponseMsgParser : public LinkMsgParser
{
protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);
};

}

#endif // __XNLINKRESPONSEMSGPARSER_H__

