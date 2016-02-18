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
#ifndef XNLINKOUTPUTSTREAMSMGR_H
#define XNLINKOUTPUTSTREAMSMGR_H

#include "XnLinkProtoLibDefs.h"
#include "XnLinkProtoUtils.h"
#include <XnArray.h>
#include <XnStatus.h>
#include <XnPlatform.h>

namespace xn
{

class ILinkMsgEncoderFactory;
class ILinkOutputStream;
class LinkOutputDataEndpoint;

class LinkOutputStreamsMgr
{
public:
	LinkOutputStreamsMgr();
	~LinkOutputStreamsMgr();
	XnStatus Init();
	void Shutdown();

	XnStatus InitOutputStream(XnUInt16 nStreamID, 
							  XnUInt32 nMaxMsgSize, 
							  XnUInt16 nMaxPacketSize,
							  XnLinkCompressionType compression, 
							  XnStreamFragLevel streamFragLevel, 
							  LinkOutputDataEndpoint* pOutputDataEndpoint);

	XnBool IsStreamInitialized(XnUInt16 nStreamID) const;
	
	void ShutdownOutputStream(XnUInt16 nStreamID);
	XnStatus SendData(XnUInt16 nStreamID, 
	                  XnUInt16 nMsgType, 
					  XnUInt16 nCID, 
					  XnLinkFragmentation fragmentation,
					  const void* pData, 
					  XnUInt32 nDataSize);

private:
	static const XnUInt16 INITIAL_PACKET_ID;
	xnl::Array<ILinkOutputStream*> m_outputStreams;
};

}

#endif // XNLINKOUTPUTSTREAMSMGR_H
