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
#ifndef __XN_AUDIO_PROCESSOR_H__
#define __XN_AUDIO_PROCESSOR_H__

#if 0 // Audio support

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnWholePacketProcessor.h"
#include "XnSensorAudioStream.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

class XnAudioProcessor : public XnWholePacketProcessor
{
public:
	XnAudioProcessor(XnSensorAudioStream* pStream, XnSensorStreamHelper* pHelper, XnDeviceAudioBuffer* pBuffer, XnUInt32 nInputPacketSize);
	~XnAudioProcessor();

	XnStatus Init();

protected:
	//---------------------------------------------------------------------------
	// Overridden Functions
	//---------------------------------------------------------------------------
	virtual void ProcessWholePacket(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData);

	inline XnSensorAudioStream* GetStream()
	{
		return m_pStream;
	}

	//---------------------------------------------------------------------------
	// Class Members
	//---------------------------------------------------------------------------
private:
	void CalcDeleteChannel();
	static XnStatus XN_CALLBACK_TYPE DeleteChannelChangedCallback(const XnProperty* pSender, void* pCookie);

	/** Used to dump Audio In data. */
	XnDumpFile* m_AudioInDump;
	XnBool m_bDeleteChannel;
	XnSensorAudioStream* m_pStream;
	XnDeviceAudioBuffer* m_pBuffer;
	XnSensorStreamHelper* m_pHelper;

	XnCallbackHandle m_hNumChannelsCallback;
};

#endif // Audio support
#endif //__XN_AUDIO_PROCESSOR_H__
