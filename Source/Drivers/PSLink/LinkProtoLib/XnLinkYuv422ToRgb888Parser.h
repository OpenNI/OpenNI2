#ifndef _XN_LINK_YUV_422_TO_RGB_888_PARSER_H_
#define _XN_LINK_YUV_422_TO_RGB_888_PARSER_H_

#include "XnLinkMsgParser.h"

namespace xn
{

class LinkYuv422ToRgb888Parser : public LinkMsgParser
{
public:
	LinkYuv422ToRgb888Parser();
	virtual ~LinkYuv422ToRgb888Parser();

protected:
	virtual XnStatus ParsePacketImpl(
		XnLinkFragmentation fragmentation,
		const XnUInt8* pSrc, 
		const XnUInt8* pSrcEnd, 
		XnUInt8*& pDst, 
		const XnUInt8* pDstEnd);
};

}

#endif //_XN_LINK_YUV_422_TO_RGB_888_PARSER_H_