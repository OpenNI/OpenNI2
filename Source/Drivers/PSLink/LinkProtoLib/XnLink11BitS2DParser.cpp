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
#include "XnLink11BitS2DParser.h"
#include "XnShiftToDepth.h"
#include "XnLinkProtoUtils.h"
#include <XnLog.h>

namespace xn
{

Link11BitS2DParser::Link11BitS2DParser(const XnShiftToDepthTables& shiftToDepthTables) :
	m_nState(0),
	m_nShift(0),
	m_pShiftToDepth(shiftToDepthTables.pShiftToDepthTable)
{
}

Link11BitS2DParser::~Link11BitS2DParser()
{
}

XnStatus Link11BitS2DParser::ParsePacketImpl(XnLinkFragmentation fragmentation,
											 const XnUInt8* pSrc, 
											 const XnUInt8* pSrcEnd, 
											 XnUInt8*& pDst, 
											 const XnUInt8* pDstEnd)
{
	XN_ASSERT(m_pShiftToDepth != NULL);
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
	nPacketDstWords = (nPacketBits / 11);
	if ((nPacketBits % 11) != 0)
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
		switch (m_nState)
		{
			case 0:
				m_nShift = (*pSrc << 3); //8 first bits, make room for 3 more
				m_nState++;
				break;
			case 1:
				m_nShift |= ((*pSrc >> 5) & 0x07); //3 more bits, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift]; //Write
				m_nShift = ((*pSrc & 0x1F) << 6); //5 first bits, make room for 6 more
				m_nState++;
				break;
			case 2:
				m_nShift |= ((*pSrc >> 2) & 0x3F); //6 more bits, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift];
				m_nShift = ((*pSrc & 0x03) << 9); //2 first bits, make room for 9 more
				m_nState++;
				break;
			case 3:
				m_nShift |= (*pSrc << 1); //8 more bits, make room for 1 more
				m_nState++;
				break;
			case 4:
				m_nShift |= ((*pSrc >> 7) & 0x01); //1 more bit, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift];
				m_nShift = ((*pSrc & 0x7F) << 4); //7 first bits, make room for 4 more
				m_nState++;
				break;
			case 5:
				m_nShift |= ((*pSrc >> 4) & 0x0F); //4 more bits, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift];
				m_nShift = ((*pSrc & 0x0F) << 7); //4 first bits, make room for 7 more
				m_nState++;
				break;
			case 6:
				m_nShift |= ((*pSrc >> 1) & 0x7F); //7 more bits, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift];
				m_nShift = ((*pSrc & 0x01) << 10); //1 first bit, make room for 10 more
				m_nState++;
				break;
			case 7:
				m_nShift |= (*pSrc << 2); //8 more bits, make room for 2 more
				m_nState++;
				break;
			case 8:
				m_nShift |= ((*pSrc >> 6) & 0x03); //2 more bits, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift];
				m_nShift = ((*pSrc & 0x3F) << 5); //6 first bits, make room for 5 more
				m_nState++;
				break;
			case 9:
				m_nShift |= ((*pSrc >> 3) & 0x1F); //5 more bits, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift];
				m_nShift = ((*pSrc & 0x07) << 8); //3 first bits, make room for 8 more
				m_nState++;
				break;
			case 10:
				m_nShift |= (*pSrc); //8 more bits, we got a whole shift value
				*pDstPixel++ = m_pShiftToDepth[m_nShift];
				m_nState = 0;
				break;
			default:
				XN_ASSERT(FALSE);
		}//switch
		pSrc++;
	}// while

	return XN_STATUS_OK;
}

}