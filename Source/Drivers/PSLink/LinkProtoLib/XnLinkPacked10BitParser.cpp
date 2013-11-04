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
#include "XnLinkPacked10BitParser.h"
#include "XnLinkProtoUtils.h"
#include <XnOS.h>

namespace xn
{

LinkPacked10BitParser::LinkPacked10BitParser()
{
	m_nState = 0;
}

LinkPacked10BitParser::~LinkPacked10BitParser()
{

}

XnStatus LinkPacked10BitParser::ParsePacketImpl(XnLinkFragmentation fragmentation,
												const XnUInt8* pSrc, 
												const XnUInt8* pSrcEnd, 
												XnUInt8*& pDst, 
												const XnUInt8* pDstEnd)
{
	//pDstWord always points to same address as pDst.
	XnUInt16*& pDstWord = reinterpret_cast<XnUInt16*&>(pDst);
	const XnUInt16* pDstWordEnd = reinterpret_cast<const XnUInt16*>(pDstEnd);
	XnSizeT nPacketBits = 0;
	XnSizeT nPacketDstWords = 0;
	
	XnSizeT nPacketDataSize = pSrcEnd - pSrc;

	if ((fragmentation & XN_LINK_FRAG_BEGIN) != 0)
	{
		//Reset state for new frame
		m_nState = 0;
	}

	//Calculate needed space for this packet when it's unpacked
	nPacketBits = (nPacketDataSize * 8);
	nPacketDstWords = (nPacketBits / 10);
	if (nPacketBits % 10 != 0)
	{
		nPacketDstWords++;
	}

	if ((pDstWord + nPacketDstWords) > pDstWordEnd) //Do we have enough room for this packet?
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	while (pSrc < pSrcEnd)
	{
		switch (m_nState)
		{
			case 0:
				*pDstWord = (*pSrc << 2); //8 first bits, make room for 2 more
				m_nState++;
				break;
			case 1:
				*pDstWord++ |= ((*pSrc >> 6) & 0x03); //2 more bits - got 1 whole word 
				*pDstWord = ((*pSrc & 0x3F) << 4); //6 first bits, make room for 4 more
				m_nState++;
				break;
			case 2:
				*pDstWord++ |= ((*pSrc >> 4) & 0x0F); //4 more bits - got 2 whole words
				*pDstWord = ((*pSrc & 0x0F) << 6); //4 first bits, make room for 6 more
				m_nState++;
				break;
			case 3:
				*pDstWord++ |= ((*pSrc >> 2) & 0x3F); //6 more bits - got 3 whole words
				*pDstWord = ((*pSrc & 0x03) << 8); //2 first bits, make room for 8 more
				m_nState++;
				break;
			case 4:
				*pDstWord++ |= *pSrc; //8 more bits - got 4 whole words
				m_nState = 0;
				break;
			default:
				XN_ASSERT(FALSE);
		}
		pSrc++;
	}

	return XN_STATUS_OK;
}

}