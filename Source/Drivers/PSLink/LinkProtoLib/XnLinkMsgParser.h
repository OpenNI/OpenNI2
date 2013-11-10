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
#ifndef XNLINKMSGPARSER_H
#define XNLINKMSGPARSER_H

#include <XnStatus.h>
#include "XnLinkProtoUtils.h"

namespace xn
{

class LinkMsgParser 
{
public:
	LinkMsgParser();
	virtual ~LinkMsgParser();
	virtual XnStatus Init();
	virtual void Shutdown();
	
	XnStatus BeginParsing(void* pDestBuffer, XnUInt32 nDestBufferSize);
	XnStatus ParsePacket(const LinkPacketHeader& header, const XnUInt8* pData);

	const void* GetParsedData() const;
	XnUInt32 GetParsedSize() const;
	XnUInt32 GetBufferSize() const;

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation, 
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);

private:
	XnUInt8* m_pDestBuffer;
	XnUInt8* m_pCurrDest;
	XnUInt8* m_pDestEnd;
};

}

#endif // XNLINKMSGPARSER_H
