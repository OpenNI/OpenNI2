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
#ifndef XNAUDIOSTREAM_H
#define XNAUDIOSTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnStreamingStream.h>
#include <XnCore.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

/**
* Represents a default base implementation of an audio stream.
*/
class XnAudioStream : public XnStreamingStream
{
public:
	XnAudioStream(const XnChar* csName, XnUInt32 nMaxNumberOfChannels);
	~XnAudioStream() { Free(); }

	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	XnStatus Init();

	//---------------------------------------------------------------------------
	// Getters
	//---------------------------------------------------------------------------
	inline XnSampleRate GetSampleRate() const { return (XnSampleRate)m_SampleRate.GetValue(); }
	inline XnUInt32 GetNumberOfChannels() const { return (XnUInt32)m_NumberOfChannels.GetValue(); }

protected:
	//---------------------------------------------------------------------------
	// Properties Getters
	//---------------------------------------------------------------------------
	inline XnActualIntProperty& SampleRateProperty() { return m_SampleRate; }
	inline XnActualIntProperty& NumberOfChannelsProperty() { return m_NumberOfChannels; }

	//---------------------------------------------------------------------------
	// Setters
	//---------------------------------------------------------------------------
	virtual XnStatus SetSampleRate(XnSampleRate nSampleRate);
	virtual XnStatus SetNumberOfChannels(XnUInt32 nNumberOfChannels);

	XnStatus CalcRequiredSize(XnUInt32* pnRequiredSize) const;

private:

	static XnStatus XN_CALLBACK_TYPE SetSampleRateCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetNumberOfChannelsCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);

	//---------------------------------------------------------------------------
	// Members
	//---------------------------------------------------------------------------
	XnActualIntProperty m_SampleRate;
	XnActualIntProperty m_NumberOfChannels;

	XnUInt32 m_nMaxNumberOfChannels;
};

#endif // XNAUDIOSTREAM_H
