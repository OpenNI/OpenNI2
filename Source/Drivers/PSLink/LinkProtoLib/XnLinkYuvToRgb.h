#ifndef _XN_LINK_YUV_TO_RGB_H_
#define _XN_LINK_YUV_TO_RGB_H_

namespace xn
{

class LinkYuvToRgb
{
public:
	enum {
		YUV_422_BYTES_PER_PIXEL = 2,
		RGB_888_BYTES_PER_PIXEL = 3
	};

	static XnStatus Yuv422ToRgb888(const XnUInt8* pSrc, XnSizeT srcBytes, XnUInt8* pDst, XnSizeT& dstSize);
};

}

#endif //_XN_LINK_YUV_TO_RGB_H_