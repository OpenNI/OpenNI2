#include "XnLinkUnpackedS2DParser.h"
#include "XnShiftToDepth.h"
#include "XnLinkProtoUtils.h"
#include <XnLog.h>

namespace xn
{

LinkUnpackedS2DParser::LinkUnpackedS2DParser(const XnShiftToDepthTables& shiftToDepthTables) :
	m_shiftToDepthTables(shiftToDepthTables)
{
}

LinkUnpackedS2DParser::~LinkUnpackedS2DParser()
{
}

XnStatus LinkUnpackedS2DParser::ParsePacketImpl(XnLinkFragmentation /*fragmentation*/,
												const XnUInt8* pSrc, 
												const XnUInt8* pSrcEnd, 
												XnUInt8*& pDst, 
												const XnUInt8* pDstEnd)
{
	XN_ASSERT(m_shiftToDepthTables.bIsInitialized);
	XnStatus nRetVal = XN_STATUS_OK;
	XnSizeT nPacketDataSize = pSrcEnd - pSrc;

	if (pDst + nPacketDataSize > pDstEnd)
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	////////////////////////////////////////////
	nRetVal = XnShiftToDepthConvert(&m_shiftToDepthTables, 
		reinterpret_cast<const XnUInt16*>(pSrc),
		XnUInt32(nPacketDataSize / 2),
		reinterpret_cast<OniDepthPixel*>(pDst));
	XN_IS_STATUS_OK(nRetVal);
	////////////////////////////////////////////

	pDst += nPacketDataSize;

	return XN_STATUS_OK;
}

}