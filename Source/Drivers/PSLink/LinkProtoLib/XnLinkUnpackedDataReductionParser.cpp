/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
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