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
#include "XnLinkOutputStreamsMgr.h"
#include "ILinkOutputStream.h"
#include "XnLinkOutputStream.h"
#include <XnLog.h>

namespace xn
{

const XnUInt16 LinkOutputStreamsMgr::INITIAL_PACKET_ID = 1;

LinkOutputStreamsMgr::LinkOutputStreamsMgr()
{
}

LinkOutputStreamsMgr::~LinkOutputStreamsMgr()
{

}

XnStatus LinkOutputStreamsMgr::Init()
{
	return XN_STATUS_OK;
}

void LinkOutputStreamsMgr::Shutdown()
{
	for (XnUInt16 nStreamID = 0; nStreamID < m_outputStreams.GetSize(); nStreamID++)
	{
		ShutdownOutputStream(nStreamID);
	}
	m_outputStreams.Clear();
}

XnStatus LinkOutputStreamsMgr::InitOutputStream(XnUInt16 nStreamID, 
												XnUInt32 nMaxMsgSize, 
												XnUInt16 nMaxPacketSize,
												XnLinkCompressionType compression, 
												XnStreamFragLevel streamFragLevel, 
												LinkOutputDataEndpoint* pOutputDataEndpoint)
{
	XnStatus nRetVal = XN_STATUS_OK;
	ILinkOutputStream* pLinkOutputStream = NULL;
	if (nStreamID < m_outputStreams.GetSize())
	{
		XN_DELETE(m_outputStreams[nStreamID]);
		m_outputStreams[nStreamID] = NULL;
	}
	
	switch (streamFragLevel)
	{
		case XN_LINK_STREAM_FRAG_LEVEL_FRAMES:
			pLinkOutputStream = XN_NEW(LinkOutputStream);
			break;
		default:
			xnLogError(XN_MASK_LINK, "Bad stream fragmentation level %u", streamFragLevel);
			XN_ASSERT(FALSE);
			return XN_STATUS_ERROR;
	}

	XN_VALIDATE_ALLOC_PTR(pLinkOutputStream);

	nRetVal = pLinkOutputStream->Init(nStreamID, 
	                                  nMaxMsgSize, 
									  nMaxPacketSize, 
									  compression, 
									  INITIAL_PACKET_ID,
									  pOutputDataEndpoint);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(pLinkOutputStream);
		xnLogError(XN_MASK_LINK, "Failed to initialize link output stream %u: %s", nStreamID, xnGetStatusString(nRetVal));
		XN_ASSERT(FALSE);
		return nRetVal;
	}

	nRetVal = m_outputStreams.Set(nStreamID, pLinkOutputStream, NULL);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(pLinkOutputStream);
		xnLogError(XN_MASK_LINK, "Failed to add to output streams array: %s", xnGetStatusString(nRetVal));
		XN_ASSERT(FALSE);
		return nRetVal;
	}

	return XN_STATUS_OK;
}

void LinkOutputStreamsMgr::ShutdownOutputStream(XnUInt16 nStreamID)
{
	if (nStreamID > m_outputStreams.GetSize())
	{
		xnLogWarning(XN_MASK_LINK, "Stream ID %u is not in array", nStreamID);
		XN_ASSERT(FALSE);
		return;
	}

	if (m_outputStreams[nStreamID] != NULL)
	{
		m_outputStreams[nStreamID]->Shutdown();
		XN_DELETE(m_outputStreams[nStreamID]);
		m_outputStreams[nStreamID] = NULL;
	}
}

XnStatus LinkOutputStreamsMgr::SendData(XnUInt16 nStreamID, 
										XnUInt16 nMsgType, 
										XnUInt16 nCID, 
										XnLinkFragmentation fragmentation,
										const void* pData, 
										XnUInt32 nDataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	if ((nStreamID >= m_outputStreams.GetSize()) || (m_outputStreams[nStreamID] == NULL) || 
		!m_outputStreams[nStreamID]->IsInitialized())
	{
		xnLogError(XN_MASK_LINK, "Stream %u is not initialized", nStreamID);
		XN_ASSERT(FALSE);
		return XN_STATUS_NOT_INIT;
	}

	nRetVal = m_outputStreams[nStreamID]->SendData(nMsgType, nCID, fragmentation, pData, nDataSize);
	XN_IS_STATUS_OK_LOG_ERROR("Send data on output stream", nRetVal);
	return XN_STATUS_OK;
}

XnBool LinkOutputStreamsMgr::IsStreamInitialized( XnUInt16 nStreamID ) const
{
	return (
		nStreamID < m_outputStreams.GetSize() &&
		m_outputStreams[nStreamID] != NULL &&
		m_outputStreams[nStreamID]->IsInitialized());
}

}