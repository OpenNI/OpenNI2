#ifndef _XN_LINK_24Z_YUV422_PARSER_H_
#define _XN_LINK_24Z_YUV422_PARSER_H_

#include "XnLinkMsgParser.h"

namespace xn
{

class Link24zYuv422Parser : public LinkMsgParser
{
public:
	Link24zYuv422Parser(XnUInt32 xRes, XnUInt32 yRes, XnBool transformToRGB);
	virtual ~Link24zYuv422Parser();

	virtual XnStatus Init();

protected:
	virtual XnStatus ParsePacketImpl(
		XnLinkFragmentation fragmentation,
		const XnUInt8* pSrc, 
		const XnUInt8* pSrcEnd, 
		XnUInt8*& pDst, 
		const XnUInt8* pDstEnd);

private:
	XnStatus Uncompress24z(
		const XnUInt8* pInput, XnSizeT nInputSize,
		XnUInt8* pOutput, XnSizeT* pnOutputSize, XnUInt32 nLineSize,
		XnSizeT* pnActualRead, XnBool bLastPart);

	XnUInt8* m_dataFromPrevPacket;
	XnSizeT m_dataFromPrevPacketBytes;
	XnUInt32 m_lineWidthBytes;
	XnUInt32 m_rgbFrameSize;
	XnUInt32 m_expectedFrameSize;
	XnBool m_transformToRGB;
	XnUInt8* m_tempYuvImage; // hold Yuv Image, when transform is required
	XnUInt32 m_tempYuvImageBytes;
};

}


#endif //_XN_LINK_24Z_YUV422_PARSER_H_