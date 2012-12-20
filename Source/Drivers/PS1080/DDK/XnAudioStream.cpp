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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnAudioStream.h"
#include <XnOS.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_AUDIO_STREAM_BUFFER_SIZE_IN_SECONDS	1.5

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnAudioStream::XnAudioStream(const XnChar* csName, XnUInt32 nMaxNumberOfChannels) :
	XnStreamingStream(XN_STREAM_TYPE_AUDIO, csName),
	m_SampleRate(XN_STREAM_PROPERTY_SAMPLE_RATE, "SampleRate", XN_SAMPLE_RATE_48K),
	m_NumberOfChannels(XN_STREAM_PROPERTY_NUMBER_OF_CHANNELS, "NumChannels", 2),
	m_nMaxNumberOfChannels(nMaxNumberOfChannels)
{
}

XnStatus XnAudioStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// init base
	nRetVal = XnStreamingStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	m_SampleRate.UpdateSetCallback(SetSampleRateCallback, this);
	m_NumberOfChannels.UpdateSetCallback(SetNumberOfChannelsCallback, this);

	XN_VALIDATE_ADD_PROPERTIES(this, &m_SampleRate, &m_NumberOfChannels);

	// required size 
	nRetVal = RegisterRequiredSizeProperty(&m_SampleRate);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnAudioStream::SetSampleRate(XnSampleRate nSampleRate)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_SampleRate.UnsafeUpdateValue(nSampleRate);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnAudioStream::SetNumberOfChannels(XnUInt32 nNumberOfChannels)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_NumberOfChannels.UnsafeUpdateValue(nNumberOfChannels);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnAudioStream::CalcRequiredSize(XnUInt32* pnRequiredSize) const
{
	XnUInt32 nSampleSize = 2 * m_nMaxNumberOfChannels; // 16-bit per channel (2 bytes)
	XnUInt32 nSamples = (XnUInt32)(GetSampleRate() * XN_AUDIO_STREAM_BUFFER_SIZE_IN_SECONDS);

	*pnRequiredSize = nSamples * nSampleSize;
	
	return (XN_STATUS_OK);
}

XnStatus XnAudioStream::SetSampleRateCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnAudioStream* pStream = (XnAudioStream*)pCookie;
	return pStream->SetSampleRate((XnSampleRate)nValue);
}

XnStatus XnAudioStream::SetNumberOfChannelsCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnAudioStream* pStream = (XnAudioStream*)pCookie;
	return pStream->SetNumberOfChannels((XnUInt32)nValue);
}
