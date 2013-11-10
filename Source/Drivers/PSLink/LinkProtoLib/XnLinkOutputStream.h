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
#ifndef XNLINKOUTPUTSTREAM_H
#define XNLINKOUTPUTSTREAM_H

#include "ILinkOutputStream.h"
#include "XnLinkProtoUtils.h"

namespace xn
{

class LinkMsgEncoder;

class LinkOutputStream : public ILinkOutputStream
{
public:
	LinkOutputStream();
	virtual ~LinkOutputStream();

	virtual XnStatus Init(XnUInt16 nStreamID, 
	                      XnUInt32 nMaxMsgSize, 
						  XnUInt16 nMaxPacketSize, 
						  XnLinkCompressionType compression, 
						  XnUInt16 nInitialPacketID,
						  LinkOutputDataEndpoint* pOutputDataEndpoint);

	virtual XnBool IsInitialized() const;
	virtual void Shutdown();
	virtual XnLinkCompressionType GetCompression() const;

	virtual XnStatus SendData(XnUInt16 nMsgType, 
							  XnUInt16 nCID, 
							  XnLinkFragmentation fragmentation,
							  const void* pData, 
							  XnUInt32 nDataSize) const;

protected:
	virtual XnStatus CreateLinkMsgEncoder(LinkMsgEncoder*& pLinkMsgEncoder);

private:
	XnBool m_bInitialized;
	XnUInt16 m_nStreamID;
	XnLinkCompressionType m_compression;
	LinkMsgEncoder* m_pLinkMsgEncoder;
	LinkOutputDataEndpoint* m_pOutputDataEndpoint;
	mutable XnUInt16 m_nPacketID;
};
}

#endif // XNLINKOUTPUTSTREAM_H
