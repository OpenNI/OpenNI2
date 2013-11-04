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
#include "XnLinkOutputStream.h"
#include "XnLinkMsgEncoder.h"
#include "XnLinkOutputDataEndpoint.h"
#include <XnLog.h>

namespace xn
{

LinkOutputStream::LinkOutputStream()
{
	m_bInitialized = FALSE;
	m_nStreamID = XN_LINK_STREAM_ID_INVALID;
	m_compression = XN_LINK_COMPRESSION_NONE;
	m_pLinkMsgEncoder = NULL;
	m_pOutputDataEndpoint = NULL;
	m_nPacketID = 0;
}

LinkOutputStream::~LinkOutputStream()
{

}

XnStatus LinkOutputStream::Init(XnUInt16 nStreamID, 
								XnUInt32 nMaxMsgSize, 
								XnUInt16 nMaxPacketSize, 
								XnLinkCompressionType compression, 
								XnUInt16 nInitialPacketID,
								LinkOutputDataEndpoint* pOutputDataEndpoint)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XN_VALIDATE_INPUT_PTR(pOutputDataEndpoint);

	if (m_bInitialized)
	{
		//We shutdown first so we can re-initialize.
		Shutdown();
	}

	m_nStreamID = nStreamID;
	m_compression = compression;
	m_nPacketID = nInitialPacketID;
	m_pOutputDataEndpoint = pOutputDataEndpoint;
	nRetVal = CreateLinkMsgEncoder(m_pLinkMsgEncoder);
	XN_IS_STATUS_OK_LOG_ERROR("Create link msg encoder", nRetVal);
	nRetVal = m_pLinkMsgEncoder->Init(nMaxMsgSize, nMaxPacketSize);
	XN_IS_STATUS_OK_LOG_ERROR("Init link msg encoder", nRetVal);
	m_bInitialized = TRUE;
	return XN_STATUS_OK;
}

XnBool LinkOutputStream::IsInitialized() const
{
	return m_bInitialized;
}

void LinkOutputStream::Shutdown()
{
	m_pLinkMsgEncoder->Shutdown();
	XN_DELETE(m_pLinkMsgEncoder);
	m_pLinkMsgEncoder = NULL;
	m_nStreamID = XN_LINK_STREAM_ID_INVALID;
	m_bInitialized = FALSE;
}


XnLinkCompressionType LinkOutputStream::GetCompression() const
{
	return m_compression;	
}

XnStatus LinkOutputStream::SendData(XnUInt16 nMsgType, 
									XnUInt16 nCID, 
									XnLinkFragmentation fragmentation,
									const void* pData, 
									XnUInt32 nDataSize) const
{
	XnStatus nRetVal = XN_STATUS_OK;
	m_pLinkMsgEncoder->BeginEncoding(nMsgType, m_nPacketID, m_nStreamID, 
		XnLinkFragmentation(fragmentation & XN_LINK_FRAG_BEGIN), nCID);
	m_pLinkMsgEncoder->EncodeData(pData, nDataSize);
	m_pLinkMsgEncoder->EndEncoding(XnLinkFragmentation(fragmentation & XN_LINK_FRAG_END));
	nRetVal = m_pOutputDataEndpoint->SendData(m_pLinkMsgEncoder->GetEncodedData(), 
		m_pLinkMsgEncoder->GetEncodedSize());
	XN_IS_STATUS_OK_LOG_ERROR("Send data in output data endpoint", nRetVal);

	// update packet ID
	m_nPacketID = m_pLinkMsgEncoder->GetPacketID() + 1;

	return XN_STATUS_OK;
}

XnStatus LinkOutputStream::CreateLinkMsgEncoder(LinkMsgEncoder*& pLinkMsgEncoder)
{
	switch (m_compression)
	{
	case XN_LINK_COMPRESSION_NONE:
		{
			pLinkMsgEncoder = XN_NEW(LinkMsgEncoder);
			break;
		}
	default:
		{
			xnLogError(XN_MASK_LINK, "Unknown compression type: %u", m_compression);
			XN_ASSERT(FALSE);
			return XN_STATUS_ERROR;
		}
	}

	XN_VALIDATE_ALLOC_PTR(pLinkMsgEncoder);
	return XN_STATUS_OK;
}

}
