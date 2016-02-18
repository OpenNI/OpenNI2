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
#ifndef XNLINKINPUTSTREAM_H
#define XNLINKINPUTSTREAM_H

#include "XnBitSet.h"
#include "XnLinkProtoLibDefs.h"
#include "XnLinkProtoUtils.h"

namespace xn
{

class LinkControlEndpoint;
class LinkMsgParser;
class IConnection;

class LinkInputStream
{
public:
    LinkInputStream();
    virtual ~LinkInputStream();

    virtual XnStatus Init(LinkControlEndpoint* pLinkControlEndpoint, 
                          XnStreamType streamType,
                          XnUInt16 nStreamID, 
                          IConnection* pConnection);

	virtual XnBool IsInitialized() const = 0;
    virtual void Shutdown();
    virtual void Reset();

	virtual XnStatus Start();
	virtual XnStatus Stop();
	virtual XnBool IsStreaming() const;
    virtual XnUInt16 GetStreamID() const;

	typedef void (XN_CALLBACK_TYPE* NewDataAvailableHandler)(void* pCookie);

	virtual XnStatus HandlePacket(const LinkPacketHeader& header, const XnUInt8* pData, XnBool& bPacketLoss) = 0;

	virtual void SetDumpName(const XnChar* strDumpName) = 0;
	virtual void SetDumpOn(XnBool bDumpOn) = 0;

    /* Stream Properties */
	virtual XnBool IsOutputFormatSupported(OniPixelFormat format) const;
	virtual OniPixelFormat GetOutputFormat() const;
	virtual XnStatus SetOutputFormat(OniPixelFormat format);

    virtual XnBool IsInterfaceSupported(XnUInt8 nInterfaceID) const;

    virtual XnStreamFragLevel GetStreamFragLevel() const = 0;

    virtual XnBool GetMirror() const;
    virtual XnStatus SetMirror(XnBool bMirror);

	virtual XnStatus SetGain(XnUInt16 gain);
	virtual XnStatus GetGain(XnUInt16& gain);

protected:
	virtual XnStatus StartImpl() = 0;
	virtual XnStatus StopImpl() = 0;

	virtual LinkMsgParser* CreateLinkMsgParser();
	LinkControlEndpoint* m_pLinkControlEndpoint;
    IConnection* m_pConnection;
    XnStreamType m_streamType;
    XnUInt16 m_nStreamID;
	OniPixelFormat m_outputFormat;
	volatile XnBool m_bStreaming;
	int m_streamingRefcount;

	xnl::BitSet m_supportedInterfaces;
	XnBool m_bMirror;
};

}

#endif // XNLINKINPUTSTREAM_H
