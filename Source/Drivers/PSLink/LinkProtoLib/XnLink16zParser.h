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
#ifndef XNLINK16ZPARSER_H
#define XNLINK16ZPARSER_H

#include "XnLinkMsgParser.h"

struct XnShiftToDepthTables;

namespace xn
{

template<bool TS2D>
class Link16zParser : public LinkMsgParser
{
public:
	Link16zParser(const XnShiftToDepthTables& shiftToDepthTables);
	virtual ~Link16zParser();

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);
private:
	inline OniDepthPixel TranslatePixel(XnUInt32 nShift);

	const OniDepthPixel* m_pShiftToDepth;
	XnUInt32 m_nShift;

	enum State 
	{
		STATE_OPCODE			= 0x0D,
		STATE_RLE				= 0x0E,
		STATE_FULL_ENC1			= 0x0F,
		STATE_FULL_ENC2			= 0x10,
		STATE_FULL_ENC3			= 0x11,
		STATE_FULL_ENC4			= 0x12,
		STATE_FULL_ENC_BIG_DIFF = 0x13,
		STATE_BAD_FRAME			= 0xFF,
	};

	State m_nState;
	XnUInt32 m_nBigDiff;
	XnUInt16 m_nMaxShift;
};

}

#endif // XNLINK16ZPARSER_H
