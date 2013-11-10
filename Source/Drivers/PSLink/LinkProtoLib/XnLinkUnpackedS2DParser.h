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
#ifndef XNLINKUNPACKEDS2DPARSER_H
#define XNLINKUNPACKEDS2DPARSER_H

#include "XnLinkMsgParser.h"
#include "XnShiftToDepth.h"

namespace xn
{

class LinkUnpackedS2DParser : public LinkMsgParser
{
public:
	LinkUnpackedS2DParser(const XnShiftToDepthTables& shiftToDepthTables);
	virtual ~LinkUnpackedS2DParser();

	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);
private:
	LinkUnpackedS2DParser& operator=(const LinkUnpackedS2DParser&);
	const XnShiftToDepthTables& m_shiftToDepthTables;
};

}

#endif // XNLINKUNPACKEDS2DPARSER_H
