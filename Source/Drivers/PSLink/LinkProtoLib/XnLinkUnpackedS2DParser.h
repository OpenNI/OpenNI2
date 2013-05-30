#ifndef __XNLINKUNPACKEDS2DPARSER_H__
#define __XNLINKUNPACKEDS2DPARSER_H__

#include "XnLinkMsgParser.h"
#include "XnShiftToDepth.h"

namespace xn
{

class LinkUnpackedS2DParser : public LinkMsgParser
{
public:
	LinkUnpackedS2DParser(const XnShiftToDepthTables& shiftToDepthTables);
	virtual ~LinkUnpackedS2DParser();

	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);
private:
	LinkUnpackedS2DParser& operator=(const LinkUnpackedS2DParser&);
	const XnShiftToDepthTables& m_shiftToDepthTables;
};

}

#endif // __XNLINKUNPACKEDS2DPARSER_H__
