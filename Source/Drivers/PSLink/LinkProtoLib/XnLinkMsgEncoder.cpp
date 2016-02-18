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
#include "XnLinkMsgEncoder.h"
#include "XnLinkProtoUtils.h"
#include <XnOS.h>
#include <XnLog.h>

namespace xn
{

LinkMsgEncoder::LinkMsgEncoder()
{
	m_nMaxMsgSize = 0;
	m_nMaxPacketSize = 0;
	m_nMaxNumPackets = 0;
	m_nBufferSize = 0;
	m_pCurrPacket = NULL;
	m_nEncodedSize = 0;
	m_pOutputBuffer = NULL;
	xnOSMemSet(&m_packetHeader, 0, sizeof(m_packetHeader));

	//Set constant packet header values - for all messages
	m_packetHeader.SetMagic();
	m_packetHeader.SetSize(sizeof(m_packetHeader)); //this is the minimum size, we'll add to this.
}

LinkMsgEncoder::~LinkMsgEncoder()
{
	Shutdown();
}

XnStatus LinkMsgEncoder::Init(XnUInt32 nMaxMsgSize, XnUInt16 nMaxPacketSize)
{
	if (nMaxPacketSize == 0)
	{
		xnLogError(XN_MASK_LINK, "Got max packet size of 0 in link msg encoder init :(");
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	m_nMaxMsgSize = nMaxMsgSize;
	m_nMaxPacketSize = nMaxPacketSize;

	XnUInt16 nMaxPacketDataSize = m_nMaxPacketSize - sizeof(LinkPacketHeader);
	m_nMaxNumPackets = m_nMaxMsgSize / nMaxPacketDataSize;
	if (m_nMaxPacketSize % nMaxPacketDataSize != 0)
	{
		//We need one more packet for remainder of data
		m_nMaxNumPackets++;
	}
	m_nBufferSize = m_nMaxNumPackets * m_nMaxPacketSize;
	m_pOutputBuffer = reinterpret_cast<XnUInt8*>(xnOSMallocAligned(m_nBufferSize, XN_DEFAULT_MEM_ALIGN));
	XN_VALIDATE_ALLOC_PTR(m_pOutputBuffer);

	return XN_STATUS_OK;
}

void LinkMsgEncoder::Shutdown()
{
	xnOSFreeAligned(m_pOutputBuffer);
	m_pOutputBuffer = NULL;
	m_nMaxMsgSize = 0;
}

void LinkMsgEncoder::BeginEncoding(XnUInt16 nMsgType, 
								   XnUInt16 nBasePacketID, 
								   XnUInt16 nStreamID, 
								   XnLinkFragmentation firstPacketFrag /*= XN_LINK_FRAG_BEGIN*/,
								   XnUInt16 nCID /*= 0*/)
{
	//Initialize m_packetHeader values for all packets in this message.
	m_packetHeader.SetMsgType(nMsgType);
	m_packetHeader.SetPacketID(nBasePacketID); //We'll increment this every time we advance m_pCurrPacket
	m_packetHeader.SetStreamID(nStreamID);		//This is the same for all packets
	m_packetHeader.SetFragmentationFlags(XN_LINK_FRAG_MIDDLE); //(for MOST packets)
	m_packetHeader.SetCID(nCID);

	//Point to first packet - this also sets m_pCurrPacket.
	m_pCurrPacketBuffer = m_pOutputBuffer;

	//Copy prepared packet header into first destination packet
	xnOSMemCopy(m_pCurrPacket, &m_packetHeader, sizeof(m_packetHeader));

	//Set values specific to first packet
	m_pCurrPacket->SetFragmentationFlags(firstPacketFrag);

	//Minimum size is always one packet header
	m_nEncodedSize = sizeof(LinkPacketHeader);
}

void LinkMsgEncoder::EncodeData(const void* pSourceData, XnUInt32 nSize)
{
	XnUInt16 nPacketRemainingSpace = 0; //Remaining space in current packet in each iteration
	XnUInt16 nPacketBytesToCopy = 0; //Number of bytes to copy to current packet in each iteration
	XnUInt32 nBytesLeftToCopy = nSize; //Total number of bytes left to copy
	const XnUInt8* pCurrData = reinterpret_cast<const XnUInt8*>(pSourceData); //Current source data pointer
	while (nBytesLeftToCopy > 0)
	{
		if (m_pCurrPacket->GetSize() == m_nMaxPacketSize)
		{
			//Current packet is full. Move to next packet (this also advances m_pCurrPacket).
			m_pCurrPacketBuffer += m_nMaxPacketSize;
			if (m_pCurrPacketBuffer >= m_pOutputBuffer + m_nBufferSize)
			{
				xnLogError(XN_MASK_LINK, "Msg encoder buffer overrun :( Was about to write to position %u, but buffer size is only %u",
					(m_pCurrPacketBuffer - m_pOutputBuffer), m_nBufferSize);
				XN_ASSERT(FALSE);
				return;
			}
			//Advance packet ID
			m_packetHeader.SetPacketID(m_packetHeader.GetPacketID() + 1);
			/*Copy prepared packet header into destination packet. This also sets m_pCurrPacket->m_nSize to minimum
			  and m_pCurrPacket->m_nFragmentation to XN_LINK_FRAG_MIDDLE.*/
			xnOSMemCopy(m_pCurrPacket, &m_packetHeader, sizeof(m_packetHeader));
			//Increase encoded size for packet header
			m_nEncodedSize += sizeof(m_packetHeader);
		}
		//Calculate remaining space in current packet
		nPacketRemainingSpace = m_nMaxPacketSize - m_pCurrPacket->GetSize();
		//Calculate how many bytes we're copying to the current packet
		nPacketBytesToCopy = static_cast<XnUInt16>(XN_MIN(nPacketRemainingSpace, nBytesLeftToCopy));
		
		/************ Copy data to current packet ********************/
		xnOSMemCopy(m_pCurrPacketBuffer + m_pCurrPacket->GetSize(), 
		            pCurrData,
					nPacketBytesToCopy);
		/*************************************************************/

		//Advance current source data pointer
		pCurrData += nPacketBytesToCopy;
		//Increase encoded size for packet data
		m_nEncodedSize += nPacketBytesToCopy;
		//Increase size of current packet
		m_pCurrPacket->SetSize(m_pCurrPacket->GetSize() + nPacketBytesToCopy);
		//Decrease number of bytes we have left to copy
		nBytesLeftToCopy -= nPacketBytesToCopy;
	}
}

void LinkMsgEncoder::EndEncoding(XnLinkFragmentation lastPacketFrag /*= XN_LINK_FRAG_END*/)
{
	//Add END flag to fragmentation flags if it's set
	m_pCurrPacket->SetFragmentationFlags(
		XnLinkFragmentation(m_pCurrPacket->GetFragmentationFlags() | (lastPacketFrag & XN_LINK_FRAG_END)));
}

const void* LinkMsgEncoder::GetEncodedData() const
{
	return m_pOutputBuffer;
}

XnUInt32 LinkMsgEncoder::GetEncodedSize() const
{
	return m_nEncodedSize;
}


XnUInt16 LinkMsgEncoder::GetMaxPacketSize() const
{
	return m_nMaxPacketSize;
}

XnUInt16 LinkMsgEncoder::GetPacketID() const
{
	return m_pCurrPacket->GetPacketID();
}

XnUInt32 LinkMsgEncoder::GetMaxMsgSize() const
{
	return m_nMaxMsgSize;
}

void LinkMsgEncoder::SetPacketID(XnUInt16 nPacketID)
{
	m_pCurrPacket->SetPacketID(nPacketID);
}
}