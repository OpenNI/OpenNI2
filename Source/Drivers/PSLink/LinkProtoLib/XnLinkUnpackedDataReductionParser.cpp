#include "XnLinkUnpackedDataReductionParser.h"
#include "XnLinkProtoUtils.h"
#include "XnLinkDefs.h"
#include <XnOS.h>
#include <XnLog.h>

namespace xn
{


const XnUInt16 LinkUnpackedDataReductionParser::FACTOR = 200;


LinkUnpackedDataReductionParser::LinkUnpackedDataReductionParser()
{

}

LinkUnpackedDataReductionParser::~LinkUnpackedDataReductionParser()
{

}

XnStatus LinkUnpackedDataReductionParser::ParsePacketImpl(XnLinkFragmentation /*fragmentation*/,
														  const XnUInt8* pSrc, 
														  const XnUInt8* pSrcEnd, 
														  XnUInt8*& pDst, 
														  const XnUInt8* pDstEnd)
{
	XnSizeT nPacketDataSize = pSrcEnd - pSrc;

	if ((pDst + nPacketDataSize) > pDstEnd)
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	////////////////////////////////////////////
	while (pSrc < pSrcEnd)
	{
		*((XnUInt16*)pDst) = *((XnUInt16*)pSrc) * FACTOR;
		pDst += sizeof(XnUInt16);
		pSrc += sizeof(XnUInt16);
	}
	////////////////////////////////////////////

	return XN_STATUS_OK;
}

}