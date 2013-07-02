#ifndef _XNLINK6BITPARSER_H_
#define _XNLINK6BITPARSER_H_

#include "XnLinkMsgParser.h"

namespace xn
{

class Link6BitParser : public LinkMsgParser
{
public:
	Link6BitParser();
	virtual ~Link6BitParser();

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);

private:
	XnUInt32 m_nState;
	XnUInt16 m_nShift;
};

}

#endif // _XNLINK6BITPARSER_H_
