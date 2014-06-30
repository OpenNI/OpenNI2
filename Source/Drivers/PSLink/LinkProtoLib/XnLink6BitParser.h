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
protected:
	XnUInt16 m_nShift; //if private and field not used, it clashes with clang due to [-Werror,-Wunused-private-field]
};

}

#endif // _XNLINK6BITPARSER_H_
