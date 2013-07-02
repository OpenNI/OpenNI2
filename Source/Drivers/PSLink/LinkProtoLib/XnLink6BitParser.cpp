#include "XnLink6BitParser.h"
#include "XnLinkProtoUtils.h"
#include <XnLog.h>

namespace xn
{

Link6BitParser::Link6BitParser() :
	m_nState(0),
	m_nShift(0)
{
}

Link6BitParser::~Link6BitParser()
{
}

XnStatus Link6BitParser::ParsePacketImpl(XnLinkFragmentation fragmentation,
											 const XnUInt8* pSrc, 
											 const XnUInt8* pSrcEnd, 
											 XnUInt8*& pDst, 
											 const XnUInt8* pDstEnd)
{
	OniDepthPixel*& pDstPixel = reinterpret_cast<OniDepthPixel*&>(pDst);
	const OniDepthPixel* pDstPixelEnd = reinterpret_cast<const OniDepthPixel*>(pDstEnd);
	XnSizeT nPacketBits = 0;
	XnSizeT nPacketDstWords = 0;

	XnSizeT nPacketDataSize = pSrcEnd - pSrc;

	if ((fragmentation & XN_LINK_FRAG_BEGIN) != 0)
	{
		//Reset state for new frame
		m_nState = 0;
	}

	nPacketBits = (nPacketDataSize * 8);
	nPacketDstWords = (nPacketBits / 6);
	if ((nPacketBits % 6) != 0)
	{
		nPacketDstWords++;
	}

	if ((pDstPixel + nPacketDstWords) > pDstPixelEnd) //Do we have enough room for this packet?
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	while (pSrc < pSrcEnd)
	{
		XN_ASSERT(pDstPixel < pDstPixelEnd);
		if (pSrc + 1 == pSrcEnd && (m_nState != 0 || m_nState != 3))
			break;
		switch (m_nState)
		{
			case 0:
				*pDstPixel++ = (*pSrc & 0x3f); //6 first bits
				m_nState++;
				break;
			case 1:
				*pDstPixel++ = 
					(*pSrc >> 6) | //last 2 bits from prev frame
					((pSrc[1] & 0xf) << 2); //4 first bits put at the end of frame
				pSrc++;
				m_nState++;
				break;
			case 2:
				*pDstPixel++ = 
					(*pSrc >> 4) | //last 4 bits, for start new frame
					((pSrc[1] & 0x3f) << 2); //first 2 bits, put at the end of frame
				pSrc++;
				m_nState++;
				break;
			case 3:
				*pDstPixel++ = (*pSrc >> 6); //last 6 bits it is the frame
				pSrc++;
				m_nState = 0;
				break;
			default:
				XN_ASSERT(FALSE);
		}//switch
	}// while

	return XN_STATUS_OK;
}

}