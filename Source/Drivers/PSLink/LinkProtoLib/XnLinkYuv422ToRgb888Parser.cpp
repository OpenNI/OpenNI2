#include "XnLinkYuv422ToRgb888Parser.h"
#include "XnLinkYuvToRgb.h"

namespace xn
{

LinkYuv422ToRgb888Parser::LinkYuv422ToRgb888Parser()
{

}

LinkYuv422ToRgb888Parser::~LinkYuv422ToRgb888Parser()
{

}

XnStatus LinkYuv422ToRgb888Parser::ParsePacketImpl(XnLinkFragmentation /*fragmentation*/, const XnUInt8* pSrc, const XnUInt8* pSrcEnd, XnUInt8*& pDst, const XnUInt8* pDstEnd)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnSizeT dstSize = pDstEnd - pDst;
	nRetVal = LinkYuvToRgb::Yuv422ToRgb888(pSrc, pSrcEnd - pSrc, pDst, dstSize);
	XN_IS_STATUS_OK(nRetVal);

	pDst += dstSize;

	return (XN_STATUS_OK);
}

}


