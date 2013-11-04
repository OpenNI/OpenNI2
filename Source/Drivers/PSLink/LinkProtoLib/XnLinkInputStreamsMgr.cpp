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
#include "XnLinkInputStreamsMgr.h"
#include "XnLinkProtoUtils.h"
#include "XnLinkStatusCodes.h"
#include "XnLinkFrameInputStream.h"
#include "XnLinkProtoLibDefs.h"
#include "XnLinkContInputStream.h"
#include "XnLinkControlEndpoint.h"
#include <XnLog.h>
#include <XnProfiling.h>

namespace xn
{


/* FRAG_FLAGS_ALLOWED_CHANGES[y][x] is 1 if a change from fragmentation flag y to x is allowed, 0 otherwise. */
const XnUInt32 LinkInputStreamsMgr::FRAG_FLAGS_ALLOWED_CHANGES[4][4] = {
/*                                        M, B, E, S */
/* Allowed state changes from MIDDLE: */ {1, 0, 1, 0}, 
/* Allowed state changes from BEGIN:  */ {1, 0, 1, 0}, 
/* Allowed state changes from END:    */ {0, 1, 0, 1}, 
/* Allowed state changes from SINGLE: */ {0, 1, 0, 1}, 
};
const XnUInt16 LinkInputStreamsMgr::INITIAL_PACKET_ID = 1;

LinkInputStreamsMgr::LinkInputStreamsMgr()
{
	xnOSMemSet(&m_streamInfos, 0, sizeof(m_streamInfos));
}

LinkInputStreamsMgr::~LinkInputStreamsMgr()
{
	Shutdown();
}

XnStatus LinkInputStreamsMgr::Init()
{
	return XN_STATUS_OK;
}

void LinkInputStreamsMgr::Shutdown()
{
	for (XnUInt16 nStreamID = 0; nStreamID < XN_LINK_MAX_STREAMS; nStreamID++)
	{
		ShutdownInputStream(nStreamID);
	}
}

void LinkInputStreamsMgr::RegisterStreamOfType(XnStreamType streamType, const XnChar* strCreationInfo, XnUInt16 nStreamID)
{
	if (m_streamInfos[nStreamID].pInputStream == NULL || 
		(m_streamInfos[nStreamID].refCount > 0 && nStreamID != FindStreamByType(streamType,strCreationInfo)))
	{
		xnLogWarning(XN_MASK_LINK, "Trying to register a non existing Input stream %u", nStreamID);
		XN_ASSERT(false);
		return;
	}

	if (m_streamInfos[nStreamID].refCount == 0)
	{
		m_streamInfos[nStreamID].streamType      = streamType;
		m_streamInfos[nStreamID].strCreationInfo = strCreationInfo;
	}
	//increase refcounter
	++m_streamInfos[nStreamID].refCount;
	xnLogVerbose(XN_MASK_LINK, "Input stream %u incref. refcount is %d", nStreamID, m_streamInfos[nStreamID].refCount);
}


XnBool LinkInputStreamsMgr::UnregisterStream(XnUInt16 nStreamID)
{
	XnBool wasLast = false;
	
	if (m_streamInfos[nStreamID].pInputStream == NULL || m_streamInfos[nStreamID].refCount <= 0)
	{
		xnLogWarning(XN_MASK_LINK, "Trying to unregister a non existing Input stream %u", nStreamID);
		XN_ASSERT(false);
		return false;
	}

	//decrease refcounter
	if (--m_streamInfos[nStreamID].refCount == 0) 
	{
		wasLast = true;
	}
	xnLogVerbose(XN_MASK_LINK, "Input stream %u decref. refcount is %d", nStreamID, m_streamInfos[nStreamID].refCount);
	
	return wasLast;
}

XnBool LinkInputStreamsMgr::HasStreamOfType(XnStreamType streamType, const XnChar* strCreationInfo,	XnUInt16& nStreamID)
{
	int i;
	if ((i = FindStreamByType(streamType, strCreationInfo)) >= 0)
	{
		nStreamID = (XnUInt16)i;
		return true;
	}
	return false;
}

int LinkInputStreamsMgr::FindStreamByType(XnStreamType streamType, const XnChar* strCreationInfo)
{
	for (int i = 0; i < XN_LINK_MAX_STREAMS; ++i)
	{
		if (m_streamInfos[i].refCount > 0 && 
			streamType == m_streamInfos[i].streamType && 
			((m_streamInfos[i].strCreationInfo == NULL && strCreationInfo == NULL) || xnOSStrCmp(strCreationInfo, m_streamInfos[i].strCreationInfo) == 0))
		{
			return i;
		}
	}
	return -1;
}

XnStatus LinkInputStreamsMgr::InitInputStream(LinkControlEndpoint* pLinkControlEndpoint, 
                                              XnStreamType streamType,
                                              XnUInt16 nStreamID, 
                                              IConnection* pConnection)
{
	XnStatus nRetVal = XN_STATUS_OK;
    XnStreamFragLevel streamFragLevel = XN_LINK_STREAM_FRAG_LEVEL_NONE;
	if (nStreamID > XN_LINK_MAX_STREAMS)
	{
		xnLogError(XN_MASK_LINK, "Cannot initialize stream of id %u - max stream id is %u",
			nStreamID, XN_LINK_MAX_STREAMS-1);
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_STREAM_ID;
	}

    nRetVal = pLinkControlEndpoint->GetStreamFragLevel(nStreamID, streamFragLevel);
    XN_IS_STATUS_OK_LOG_ERROR("Get stream frag level", nRetVal);

	if (m_streamInfos[nStreamID].pInputStream == NULL)
	{
		//Need to create the input stream for the first time
		switch (streamFragLevel)
		{
			case XN_LINK_STREAM_FRAG_LEVEL_FRAMES:
				m_streamInfos[nStreamID].pInputStream = XN_NEW(LinkFrameInputStream);
				break;
			case XN_LINK_STREAM_FRAG_LEVEL_CONTINUOUS:
				m_streamInfos[nStreamID].pInputStream = XN_NEW(LinkContInputStream);
				break;

			default:
				xnLogError(XN_MASK_LINK, "Bad stream type %u", streamFragLevel);
				XN_ASSERT(FALSE);
				return XN_STATUS_ERROR;
		}
	}
	
	XN_VALIDATE_ALLOC_PTR(m_streamInfos[nStreamID].pInputStream);

	StreamInfo& streamInfo = m_streamInfos[nStreamID];
	if (m_streamInfos[nStreamID].pInputStream->IsInitialized() && (streamInfo.streamFragLevel != streamFragLevel))
	{
		XN_DELETE(m_streamInfos[nStreamID].pInputStream);
		m_streamInfos[nStreamID].pInputStream = NULL;
		xnLogError(XN_MASK_LINK, 
			"Stream %u was already initialized with stream type %u, but now tried to initialize it with stream type %u :(",
			nStreamID, streamInfo.streamFragLevel, streamFragLevel);
		/*Streams may only be re-initialized with the same stream type. This means a frame stream must always
		  remain a frame stream and a continuous stream must always remain a continuous stream. */
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	nRetVal = m_streamInfos[nStreamID].pInputStream->Init(pLinkControlEndpoint, streamType, nStreamID, pConnection);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(m_streamInfos[nStreamID].pInputStream);
		m_streamInfos[nStreamID].pInputStream = NULL;
		xnLogError(XN_MASK_LINK, "Failed to Initialize link input stream: %s", xnGetStatusString(nRetVal));
		XN_ASSERT(FALSE);
		return nRetVal;
	}

	streamInfo.nMsgType = 0;
	streamInfo.nNextPacketID = INITIAL_PACKET_ID;
	streamInfo.streamFragLevel = streamFragLevel;
	streamInfo.prevFragmentation = XN_LINK_FRAG_END; // this means we now expect BEGIN
	streamInfo.packetLoss = FALSE;
	return XN_STATUS_OK;
}

void LinkInputStreamsMgr::ShutdownInputStream(XnUInt16 nStreamID)
{
    LinkInputStream* pLinkInputStream = GetInputStream(nStreamID);
	if (pLinkInputStream != NULL)
	{
		pLinkInputStream->Shutdown();
		XN_DELETE(pLinkInputStream);
		m_streamInfos[nStreamID].pInputStream = NULL;
	}
}

void LinkInputStreamsMgr::HandlePacket(const LinkPacketHeader* pLinkPacketHeader)
{
	//Validate Stream ID
	XnUInt16 nStreamID = pLinkPacketHeader->GetStreamID();
	if (nStreamID >= XN_LINK_MAX_STREAMS)
	{
		xnLogWarning(XN_MASK_LINK, "Got bad Stream ID: %u, max StreamID is %u", nStreamID, XN_LINK_MAX_STREAMS-1);
		XN_ASSERT(FALSE);
		return;
	}

	StreamInfo* pStreamInfo = &m_streamInfos[nStreamID];

	//Validate packet ID
	XnUInt16 nPacketID = pLinkPacketHeader->GetPacketID();
	if (nPacketID != pStreamInfo->nNextPacketID)
	{
		xnLogWarning(XN_MASK_LINK, "Expected packet id of %u but got %u on stream %u.", 
			pStreamInfo->nNextPacketID, nPacketID, nStreamID);
		pStreamInfo->packetLoss = TRUE;
	}

	//We now expect the packet ID to be right after the one we got (even if we lost some packets on the way).
	pStreamInfo->nNextPacketID = (nPacketID + 1);

	XnUInt16 nMsgType = pLinkPacketHeader->GetMsgType();
	XnLinkFragmentation fragmentation = pLinkPacketHeader->GetFragmentationFlags();

	if (!pStreamInfo->packetLoss && !FRAG_FLAGS_ALLOWED_CHANGES[pStreamInfo->prevFragmentation][fragmentation])
	{
		//The transition between the previous fragmentation flags and the current fragmentation flags is not allowed
		xnLogWarning(XN_MASK_LINK, "Packet %u in stream %u has fragmentation flags of %s, but previous packet in this stream was %s",
			nPacketID, nStreamID, 
			xnFragmentationFlagsToStr(fragmentation),
			xnFragmentationFlagsToStr(pStreamInfo->prevFragmentation));
		pStreamInfo->packetLoss = TRUE;
	}

	pStreamInfo->prevFragmentation = fragmentation;

	if (fragmentation & XN_LINK_FRAG_BEGIN)
	{
		//Set message type for new frame
		pStreamInfo->nMsgType = nMsgType;
	} 
	else
	{
		//Validate that message type is consistent with first packet in frame.
		if (!pStreamInfo->packetLoss && nMsgType != pStreamInfo->nMsgType)
		{
			xnLogWarning(XN_MASK_LINK, "Inconsistent msg type for stream %u - expected 0x%04X but got 0x%04X",
				nStreamID, pStreamInfo->nMsgType, nMsgType);
			pStreamInfo->packetLoss = TRUE;
			return;
		}
	}

	if (!pStreamInfo->pInputStream->IsStreaming())
	{
		xnLogWarning(XN_MASK_LINK, "Stream %u got packets but it is not streaming", nStreamID);
		XN_ASSERT(FALSE);
		return;
	}

	// the data is immediately after the header
	const XnUInt8* pPacketData = reinterpret_cast<const XnUInt8*>(pLinkPacketHeader + 1);
	XnStatus nRetVal = pStreamInfo->pInputStream->HandlePacket(*pLinkPacketHeader, pPacketData, pStreamInfo->packetLoss);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogWarning(XN_MASK_LINK, "Failed to handle packet of %u bytes in stream %u: %s", 
			pLinkPacketHeader->GetDataSize(), nStreamID, xnGetStatusString(nRetVal));
		return;
	}
}

XnStatus LinkInputStreamsMgr::HandleData(const void* pData, XnUInt32 nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 nBytesToRead = nSize;

	XN_PROFILING_START_SECTION("LinkInputStreamsMgr::HandleData()");

	const XnUInt8* pRawLinkPacket = reinterpret_cast<const XnUInt8*>(pData);

	while (nBytesToRead > 0)
	{
		const LinkPacketHeader* pLinkPacketHeader = reinterpret_cast<const LinkPacketHeader*>(pRawLinkPacket);

		//Validate basic info in packet header
		nRetVal = pLinkPacketHeader->Validate(nBytesToRead);
		XN_IS_STATUS_OK_LOG_ERROR("Validate packet", nRetVal);

		pRawLinkPacket += pLinkPacketHeader->GetSize();
		nBytesToRead -= pLinkPacketHeader->GetSize();

		HandlePacket(pLinkPacketHeader);
	}

	XN_PROFILING_END_SECTION;

	return XN_STATUS_OK;
}

LinkInputStream* LinkInputStreamsMgr::GetInputStream(XnUInt16 nStreamID)
{
	if (nStreamID >= XN_LINK_MAX_STREAMS)
	{
		XN_ASSERT(FALSE);
		return NULL;
	}

	return m_streamInfos[nStreamID].pInputStream;
}

const LinkInputStream* LinkInputStreamsMgr::GetInputStream(XnUInt16 nStreamID) const
{
	if (nStreamID >= XN_LINK_MAX_STREAMS)
	{
		XN_ASSERT(FALSE);
		return NULL;
	}

	return m_streamInfos[nStreamID].pInputStream;
}

XnBool LinkInputStreamsMgr::HasStreams() const
{
	for (int i = 0; i < XN_LINK_MAX_STREAMS; ++i)
	{
		if (m_streamInfos[i].pInputStream != NULL)
		{
			return TRUE;
		}
	}

	return FALSE;
}

} //namespace xn
