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
#if 0 // Audio support

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnAudioProcessor.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnAudioProcessor::XnAudioProcessor(XnSensorAudioStream* pStream, XnSensorStreamHelper* pHelper, XnDeviceAudioBuffer* pBuffer, XnUInt32 nInputPacketSize) :
	XnWholePacketProcessor(pHelper->GetPrivateData(), pStream->GetType(), nInputPacketSize),
	m_AudioInDump(NULL),
	m_pStream(pStream),
	m_pBuffer(pBuffer),
	m_pHelper(pHelper)
{
	m_AudioInDump = xnDumpFileOpen(XN_DUMP_AUDIO_IN, "AudioIn.pcm");
}

XnAudioProcessor::~XnAudioProcessor()
{
	xnDumpFileClose(m_AudioInDump);
	GetStream()->NumberOfChannelsProperty().OnChangeEvent().Unregister(m_hNumChannelsCallback);
}

XnStatus XnAudioProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnWholePacketProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = GetStream()->NumberOfChannelsProperty().OnChangeEvent().Register(DeleteChannelChangedCallback, this, m_hNumChannelsCallback);
	XN_IS_STATUS_OK(nRetVal);

	CalcDeleteChannel();
	
	return (XN_STATUS_OK);
}

void XnAudioProcessor::ProcessWholePacket(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData)
{
	xnOSEnterCriticalSection(&m_pBuffer->hLock);

	// take write packet
	XnUChar* pWritePacket = m_pBuffer->pAudioBuffer + (m_pBuffer->nAudioWriteIndex * m_pBuffer->nAudioPacketSize);

	if (m_bDeleteChannel)
	{
		XnUInt16* pSamples = (XnUInt16*)pData;
		XnUInt16* pSamplesEnd = (XnUInt16*)(pData + pHeader->nBufSize);
		XnUInt16* pOutput = (XnUInt16*)pWritePacket;

		while (pSamples < pSamplesEnd)
		{
			*pOutput = *pSamples;

			pOutput++;
			// skip a sample
			pSamples += 2;
		}
	}
	else
	{
		// copy data
		xnOSMemCopy(pWritePacket, pData, pHeader->nBufSize);
	}

	// mark timestamp
	if (ShouldUseHostTimestamps())
	{
		m_pBuffer->pAudioPacketsTimestamps[m_pBuffer->nAudioWriteIndex] = GetHostTimestamp();
	}
	else
	{
		m_pBuffer->pAudioPacketsTimestamps[m_pBuffer->nAudioWriteIndex] = CreateTimestampFromDevice(pHeader->nTimeStamp);
	}

	if (m_nLastPacketID % 10 == 0)
	{
		XnUInt64 nSysTime;
		xnOSGetTimeStamp(&nSysTime);

		xnDumpFileWriteString(m_pDevicePrivateData->BandwidthDump, "%llu,%s,%d,%d\n",
			nSysTime, "Audio", -1, m_nBytesReceived);

		m_nBytesReceived = 0;
	}

	// move write index forward
	m_pBuffer->nAudioWriteIndex = (m_pBuffer->nAudioWriteIndex + 1) % m_pBuffer->nAudioBufferNumOfPackets;

	// if write index got to read index (end of buffer), move read index forward (and loose a packet)
	if (m_pBuffer->nAudioWriteIndex == m_pBuffer->nAudioReadIndex)
	{
		m_pBuffer->nAudioReadIndex = (m_pBuffer->nAudioReadIndex + 1) % m_pBuffer->nAudioBufferNumOfPackets;
	}

	xnOSLeaveCriticalSection(&m_pBuffer->hLock);

	xnDumpFileWriteBuffer(m_AudioInDump, pData, pHeader->nBufSize);

	if (m_pBuffer->pAudioCallback != NULL)
	{
		m_pBuffer->pAudioCallback(m_pBuffer->pAudioCallbackCookie);
	}
}

void XnAudioProcessor::CalcDeleteChannel()
{
	m_bDeleteChannel = (m_pHelper->GetFirmwareVersion() >= XN_SENSOR_FW_VER_5_2 && GetStream()->GetNumberOfChannels() == 1);
}

XnStatus XnAudioProcessor::DeleteChannelChangedCallback(const XnProperty* /*pSender*/, void* pCookie)
{
	XnAudioProcessor* pThis = (XnAudioProcessor*)pCookie;
	pThis->CalcDeleteChannel();
	return XN_STATUS_OK;
}

#endif // Audio support