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
#ifndef XNLINKLOGPARSER_H
#define XNLINKLOGPARSER_H

#include "XnLinkMsgParser.h"
#include <XnHash.h>

namespace xn
{

class LinkLogParser : public LinkMsgParser
{
public:
	LinkLogParser();
	virtual ~LinkLogParser();

	void GenerateOutputBuffer(bool toCreate);

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);
	
	XnStatus WriteToLogFile( XnUInt8 fileID, const void* pData, XnUInt32 dataLength );
	XnStatus CloseLogFile( XnUInt8 fileID );
	XnStatus OpenLogFile( XnUInt8 fileID, const XnChar* fileName );
private:
	xnl::Hash<XnUInt8, XnDumpFile*> m_activeLogs;
	bool m_copyDataToOutput;
};

}

#endif // XNLINKLOGPARSER_H
