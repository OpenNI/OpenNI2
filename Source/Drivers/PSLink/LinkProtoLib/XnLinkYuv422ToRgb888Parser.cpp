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
#include "XnLinkYuv422ToRgb888Parser.h"
#include "XnLinkYuvToRgb.h"

namespace xn
{

LinkYuv422ToRgb888Parser::LinkYuv422ToRgb888Parser()
{

}

LinkYuv422ToRgb888Parser::~LinkYuv422ToRgb888Parser()
{

}

XnStatus LinkYuv422ToRgb888Parser::ParsePacketImpl(XnLinkFragmentation /*fragmentation*/, const XnUInt8* pSrc, const XnUInt8* pSrcEnd, XnUInt8*& pDst, const XnUInt8* pDstEnd)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnSizeT dstSize = pDstEnd - pDst;
	nRetVal = LinkYuvToRgb::Yuv422ToRgb888(pSrc, pSrcEnd - pSrc, pDst, dstSize);
	XN_IS_STATUS_OK(nRetVal);

	pDst += dstSize;

	return (XN_STATUS_OK);
}

}


