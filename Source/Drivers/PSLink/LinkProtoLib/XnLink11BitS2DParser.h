#ifndef __XNLINK11BITS2DPARSER_H__
#define __XNLINK11BITS2DPARSER_H__

#include "XnLinkMsgParser.h"
#include "XnShiftToDepth.h"

namespace xn
{

class Link11BitS2DParser : public LinkMsgParser
{
public:
	Link11BitS2DParser(const XnShiftToDepthTables& shiftToDepthTables);
	virtual ~Link11BitS2DParser();

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);

private:
	XnUInt32 m_nState;
	XnUInt16 m_nShift;
	const OniDepthPixel* m_pShiftToDepth;
};

}

#endif // __XNLINK11BITS2DPARSER_H__
