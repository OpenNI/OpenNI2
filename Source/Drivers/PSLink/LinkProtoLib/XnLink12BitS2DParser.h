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
#ifndef XNLINK12BITS2DPARSER_H
#define XNLINK12BITS2DPARSER_H

#include "XnLinkMsgParser.h"
#include "XnShiftToDepth.h"

/* The size of an input element in the stream. */
#define XN_INPUT_ELEMENT_SIZE 24
/* The size of an output element in the stream. */
#define XN_OUTPUT_ELEMENT_SIZE 32

namespace xn
{

class Link12BitS2DParser : public LinkMsgParser
{
public:
	Link12BitS2DParser(const XnShiftToDepthTables& shiftToDepthTables);
	virtual ~Link12BitS2DParser();

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);

private:
	XnStatus Unpack12to16(const XnUInt8* pcInput,XnUInt8* pDest, const XnUInt32 nInputSize, XnUInt32* pnActualRead, XnUInt32* pnActualWritten);
	XnUInt32 ProcessFramePacketChunk(const XnUInt8* pData,XnUInt8* pDest, XnUInt32 nDataSize);

	const OniDepthPixel* m_pShiftToDepth;
	XnUInt32 m_ContinuousBufferSize;
	XnUInt8 m_ContinuousBuffer[XN_INPUT_ELEMENT_SIZE];
};

}

#endif // XNLINK12BITS2DPARSER_H
