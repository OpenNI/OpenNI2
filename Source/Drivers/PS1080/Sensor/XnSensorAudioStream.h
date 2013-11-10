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
#ifndef XNSENSORAUDIOSTREAM_H
#define XNSENSORAUDIOSTREAM_H

#if 0 // Audio support

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnAudioStream.h>
#include "XnSensorStreamHelper.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_AUDIO_STREAM_DEFAULT_VOLUME					12
#define XN_AUDIO_STREAM_DEFAULT_SAMPLE_RATE				48000
#define XN_AUDIO_STREAM_DEFAULT_NUMBER_OF_CHANNELS		2
#define XN_AUDIO_STREAM_DEFAULT_OUTPUT_FORMAT			ONI_PIXEL_FORMAT_PCM
#define XN_AUDIO_STREAM_DEFAULT_CHUNK_SIZE				2120

//---------------------------------------------------------------------------
// XnSensorAudioStream class
//---------------------------------------------------------------------------
class XnSensorAudioStream : public XnAudioStream, public IXnSensorStream
{
public:
	XnSensorAudioStream(const XnChar* strDeviceName, const XnChar* StreamName, XnSensorObjects* pObjects, XnBool bAllowOtherUsers);
	~XnSensorAudioStream() { Free(); }

	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	XnStatus Init();
	XnStatus Free();
	XnStatus BatchConfig(const XnActualPropertiesHash& props) { return m_Helper.BatchConfig(props); }

	inline XnSensorStreamHelper* GetHelper() { return &m_Helper; }

	friend class XnAudioProcessor;
protected:
	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	XnStatus Open() { return m_Helper.Open(); }
	XnStatus Close() { return m_Helper.Close(); }
	XnStatus ConfigureStreamImpl();
	XnStatus OpenStreamImpl();
	XnStatus CloseStreamImpl();
	XnStatus CreateDataProcessor(XnDataProcessor** ppProcessor);
	XnStatus MapPropertiesToFirmware();
	void GetFirmwareStreamConfig(XnResolutions* pnRes, XnUInt32* pnFPS) { *pnRes = XN_RESOLUTION_CUSTOM; *pnFPS = 0; }

	XnStatus Mirror(OniFrame* /*pFrame*/) const { return XN_STATUS_OK; }

	//---------------------------------------------------------------------------
	// Setters
	//---------------------------------------------------------------------------
	XnStatus SetOutputFormat(OniFormat nOutputFormat);
	XnStatus SetLeftChannelVolume(XnUInt32 nVolume);
	XnStatus SetRightChannelVolume(XnUInt32 nVolume);
	XnStatus SetSampleRate(XnSampleRate nSampleRate);
	XnStatus SetNumberOfChannels(XnUInt32 nNumberOfChannels);
	XnStatus SetActualRead(XnBool bRead);

private:
	XnStatus NewData();
	XnStatus ReallocBuffer();

	inline XnSensorFirmwareParams* GetFirmwareParams() const { return m_Helper.GetFirmware()->GetParams(); }

	static XnStatus ConvertNumberOfChannelsToStereo(XnUInt64 nSource, XnUInt64* pnDest);
	static XnStatus ConvertStereoToNumberOfChannels(XnUInt64 nSource, XnUInt64* pnDest);
	static XnStatus ConvertSampleRateToFirmwareRate(XnUInt64 nSource, XnUInt64* pnDest);
	static XnStatus ConvertFirmwareRateToSampleRate(XnUInt64 nSource, XnUInt64* pnDest);

	static XnStatus XN_CALLBACK_TYPE SetLeftChannelVolumeCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetRightChannelVolumeCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetActualReadCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE NewDataCallback(void* pCookie);

	//---------------------------------------------------------------------------
	// Members
	//---------------------------------------------------------------------------
	XnSensorStreamHelper m_Helper;

	XnAudioSharedBuffer m_header;
	XnDeviceAudioBuffer m_buffer;

	const XnChar* m_strDeviceName;
	XnBool m_bAllowOtherUsers;
	XnActualIntProperty m_LeftChannelVolume;
	XnActualIntProperty m_RightChannelVolume;

	XnActualIntProperty m_ActualRead;

	XnUInt32 m_nOrigAudioPacketSize;

	XnUInt32 m_nFrameID;
};

#endif // Audio support
#endif // XNSENSORAUDIOSTREAM_H
