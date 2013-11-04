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