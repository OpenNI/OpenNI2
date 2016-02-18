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
#include "XnLink16zParser.h"
#include "XnShiftToDepth.h"

#define MAX_SMALL_DIFF_NIBBLE	0x0C
#define SMALL_DIFF_OFFSET 6
#define BIG_DIFF_OFFSET 64

namespace xn
{

template<bool TS2D>
Link16zParser<TS2D>::Link16zParser(const XnShiftToDepthTables& shiftToDepthTables) :
	m_pShiftToDepth(shiftToDepthTables.pShiftToDepthTable),
	m_nShift(0),
	m_nState(STATE_OPCODE),
	m_nBigDiff(0),
	m_nMaxShift((XnUInt16)(shiftToDepthTables.nShiftsCount - 1))
{
}

template<bool TS2D>
Link16zParser<TS2D>::~Link16zParser()
{

}

template<>
inline OniDepthPixel Link16zParser<true>::TranslatePixel(XnUInt32 nShift)
{
	return m_pShiftToDepth[nShift];
}

template<>
inline OniDepthPixel Link16zParser<false>::TranslatePixel(XnUInt32 nShift)
{
	return static_cast<OniDepthPixel>(nShift);
}


template<bool TS2D>
XnStatus Link16zParser<TS2D>::ParsePacketImpl(XnLinkFragmentation fragmentation,
										const XnUInt8* pSrc, 
										const XnUInt8* pSrcEnd, 
										XnUInt8*& pDst, 
										const XnUInt8* pDstEnd)
{
	OniDepthPixel*& pDstPixel = reinterpret_cast<OniDepthPixel*&>(pDst); //Reference to pointer - moving pDstPixel affects pDst also.
	const OniDepthPixel* pDstPixelEnd = reinterpret_cast<const OniDepthPixel*>(pDstEnd);
	XnUInt32 nRLERepeats = 0;
	XnBool bReadHigh = TRUE; //TRUE when reading high part of byte, FALSE when reading low part
	XnUInt32 nNibble = 0;

	if ((fragmentation & XN_LINK_FRAG_BEGIN) != 0)
	{
		//We purposely give m_nShift a value that will mess up diff calculations if it's in the beginning by mistake
		m_nShift = (m_nMaxShift + BIG_DIFF_OFFSET + 1); 
		m_nBigDiff = 0;
		m_nState = STATE_OPCODE;
		bReadHigh = TRUE;
	}

	////////////////////////////////////////////
	while ((pSrc < pSrcEnd) && (pDstPixel < pDstPixelEnd))
	{
		//Read next nibble
		if (bReadHigh) 
		{ 
			//Read high nibble of this byte
			nNibble = (*pSrc >> 4); 
			bReadHigh = FALSE;
		} 
		else 
		{ 
			//Read low nibble of this byte and skip to next byte
			nNibble = (*pSrc & 0x0F); 
			pSrc++; 
			bReadHigh = TRUE; 
		}

		switch (m_nState)
		{
			case STATE_OPCODE:
				if (nNibble <= MAX_SMALL_DIFF_NIBBLE)
				{
					//Read value is small diff - calculate shift and write it.
					m_nShift += nNibble - SMALL_DIFF_OFFSET;
					if (m_nShift > m_nMaxShift)
					{
//						XN_ASSERT(FALSE);
						m_nState = STATE_BAD_FRAME;
						return XN_STATUS_LINK_BAD_PACKET_FORMAT;
					}
					*pDstPixel++ = TranslatePixel(m_nShift);
					//Stay in STATE_OPCODE
				}
				else
				{
					m_nState = State(nNibble); //Next state is determined by opcode nibble
				}
				break;
			case STATE_RLE:				
				if (m_nShift > m_nMaxShift)
				{
//					XN_ASSERT(FALSE);
					m_nState = STATE_BAD_FRAME;
					return XN_STATUS_LINK_BAD_PACKET_FORMAT;
				}

				//Write as many values as needed or just fill remaining space.
				nRLERepeats = XN_MIN(nNibble + 1, XnUInt32(pDstPixelEnd - pDstPixel));
				while (nRLERepeats > 0)
				{
					*pDstPixel++ = TranslatePixel(m_nShift);
					nRLERepeats--;
				}
				m_nState = STATE_OPCODE;
				break;
			case STATE_FULL_ENC1:
				if (nNibble & 0x08)
				{
					//Take 3 lower bits from nibble, make room for 4 bits more
					m_nBigDiff = (nNibble & 0x07) << 4;
					m_nState = STATE_FULL_ENC_BIG_DIFF;
				}
				else
				{
					//Take 3 lower bits from nibble, make room for 12 bits more
					//m_nShift = (nNibble & 0x07) << 12;					
					//There's no point doing this since max shift is 12 bits
					m_nState = State(m_nState + 1); //To STATE_FULL_ENC2
				}
				break;
			case STATE_FULL_ENC2:		
				//Take 4 bits from nibble, make room for 8 more
				//m_nShift |= (nNibble << 8);
				//There's no point saving anything from the previous shift value since max shift is 12 bits
				m_nShift = (nNibble << 8);
				m_nState = State(m_nState + 1); //To STATE_FULL_ENC3
				break;
			case STATE_FULL_ENC3:			
				//Take 4 bits from nibble, make room for 4 more
				m_nShift |= (nNibble << 4);
				m_nState = State(m_nState + 1); //To STATE_FULL_ENC4
				break;
			case STATE_FULL_ENC4:
				//Take 4 bits from nibble, write value to destination
				m_nShift |= nNibble;
				if (m_nShift > m_nMaxShift)
				{
					XN_ASSERT(FALSE);
					return XN_STATUS_LINK_RESP_CORRUPT_PACKET;
				}
				*pDstPixel++ = TranslatePixel(m_nShift);
				m_nState = STATE_OPCODE;
				break;
			case STATE_FULL_ENC_BIG_DIFF:
				//Take 4 bits from nibble, write value with difference to destination
				m_nBigDiff |= nNibble;
				m_nShift += (m_nBigDiff - BIG_DIFF_OFFSET);
				if (m_nShift > m_nMaxShift)
				{
//					XN_ASSERT(FALSE);
					m_nState = STATE_BAD_FRAME;
					return XN_STATUS_LINK_BAD_PACKET_FORMAT;
				}
				*pDstPixel++ = TranslatePixel(m_nShift);
				m_nState = STATE_OPCODE;
				break;
			case STATE_BAD_FRAME:
				//We stay in this state until the next frame beginning arrives
				return XN_STATUS_LINK_BAD_PACKET_FORMAT;
			default:
				XN_ASSERT(FALSE);
				return XN_STATUS_ERROR;
		} //switch
	} //while
	////////////////////////////////////////////

/*	if (pSrc < pSrcEnd)
	{
		//We had more bytes in input but not enough space in output to write them
		XN_ASSERT(FALSE);
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}
*/

	return XN_STATUS_OK;
}

//---------------------------------------------------------------------------
// Explicit instantiations
//---------------------------------------------------------------------------
template class Link16zParser<true>;
template class Link16zParser<false>;

}