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
/// @file
/// Contains the declaration of Device class that implements a virtual OpenNI
/// device, capable of reading data from a *.ONI file.

#ifndef PLAYERDEVICE_H
#define PLAYERDEVICE_H

#include "Driver/OniDriverAPI.h"
#include "XnString.h"
#include "XnList.h"
#include "XnOSCpp.h"
#include "PlayerNode.h"
#include "PlayerProperties.h"
#include "PlayerStream.h"

namespace oni_file {

class PlayerSource;

/// Implements a virtual OpenNI device, which reads is adata from a *.ONI file.
class PlayerDevice : public oni::driver::DeviceBase
{
public:
    /// Constructs a device from the given file path.
    /// @param[in] filePath The path to a *.ONI file.
    PlayerDevice(const xnl::String& filePath);
	~PlayerDevice();

    /// Initialize the device object.
    OniStatus Initialize();

    /// @copydoc OniDeviceBase::GetStreamSourceInfoList(OniSourceInfo**, int*)
    virtual OniStatus getSensorInfoList(OniSensorInfo** pSources, int* numSources);

    /// @copydoc OniDeviceBase::CreateStream(OniStreamSource)
    virtual oni::driver::StreamBase* createStream(OniSensorType);

	virtual void destroyStream(oni::driver::StreamBase* pStream);

	/// @copydoc OniDeviceBase::TryManualTrigger()
	virtual OniStatus tryManualTrigger();

	/// Get property.
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);

	/// Set property.
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);

	virtual OniBool isPropertySupported(int propertyId);

	/// @copydoc OniDeviceBase::Invoke(int, void*, int)
	virtual OniStatus invoke(int commandId, void* data, int dataSize);
	virtual OniBool isCommandSupported(int commandId);

	OniBool isPlayerEOF() { return m_player.IsEOF(); };

	typedef void (XN_CALLBACK_TYPE *DriverEOFCallback)(void* pCookie, const char* uri);
	void SetEOFEventCallback(DriverEOFCallback pFunc, void* pDriverCookie) 
	{
		m_driverEOFCallback = pFunc; 
		m_driverCookie      = pDriverCookie; 
	};
	void TriggerDriverEOFCallback() { if(m_driverEOFCallback) (m_driverEOFCallback)(m_driverCookie, m_filePath.Data()); };

	const char* getOriginalDevice() {return m_originalDevice;}
protected:
	PlayerSource* FindSource(const XnChar* strNodeName);

	// Wake up when timestamp is valid.
	void SleepToTimestamp(XnUInt64 nTimeStamp);

	void LoadConfigurationFromIniFile();

private:
	void close();

	typedef struct 
	{
		int frameId;
		PlayerStream* pStream;
	} Seek;

	void MainLoop();

	static XN_THREAD_PROC ThreadProc(XN_THREAD_PARAM pThreadParam);

	static void     ONI_CALLBACK_TYPE ReadyForDataCallback(const PlayerStream::ReadyForDataEventArgs& newDataEventArgs, void* pCookie);
	static void     ONI_CALLBACK_TYPE StreamDestroyCallback(const PlayerStream::DestroyEventArgs& destroyEventArgs, void* pCookie);

	static XnStatus XN_CALLBACK_TYPE OnNodeAdded(void* pCookie, const XnChar* strNodeName, XnProductionNodeType type, XnCodecID compression, XnUInt32 nNumberOfFrames);
	static XnStatus XN_CALLBACK_TYPE OnNodeRemoved(void* pCookie, const XnChar* strNodeName);
	static XnStatus XN_CALLBACK_TYPE OnNodeIntPropChanged(void* pCookie, const XnChar* strNodeName, const XnChar* strPropName, XnUInt64 nValue);
	static XnStatus XN_CALLBACK_TYPE OnNodeRealPropChanged(void* pCookie, const XnChar* strNodeName, const XnChar* strPropName, XnDouble dValue);
	static XnStatus XN_CALLBACK_TYPE OnNodeStringPropChanged(void* pCookie, const XnChar* strNodeName, const XnChar* strPropName, const XnChar* strValue);
	static XnStatus XN_CALLBACK_TYPE OnNodeGeneralPropChanged(void* pCookie, const XnChar* strNodeName, const XnChar* strPropName, XnUInt32 nBufferSize, const void* pBuffer);
	static XnStatus XN_CALLBACK_TYPE OnNodeStateReady(void* pCookie, const XnChar* strNodeName);
	static XnStatus XN_CALLBACK_TYPE OnNodeNewData(void* pCookie, const XnChar* strNodeName, XnUInt64 nTimeStamp, XnUInt32 nFrame, const void* pData, XnUInt32 nSize);
	static void		XN_CALLBACK_TYPE OnEndOfFileReached(void* pCookie);
	XnStatus AddPrivateProperty(PlayerSource* pSource, const XnChar* strPropName, XnUInt32 nBufferSize, const void* pBuffer);
	XnStatus AddPrivateProperty_PS1080(PlayerSource* pSource, const XnChar* strPropName, XnUInt32 nBufferSize, const void* pBuffer);
	XnStatus AddPrivateProperty_PSLink(PlayerSource* pSource, const XnChar* strPropName, XnUInt32 nBufferSize, const void* pBuffer);

	static XnStatus XN_CALLBACK_TYPE FileOpen(void* pCookie);
	static XnStatus XN_CALLBACK_TYPE FileRead(void* pCookie, void* pBuffer, XnUInt32 nSize, XnUInt32* pnBytesRead);
	static XnStatus XN_CALLBACK_TYPE FileSeek(void* pCookie, XnOSSeekType seekType, const XnInt32 nOffset);
	static XnUInt32 XN_CALLBACK_TYPE FileTell(void* pCookie);
	static void     XN_CALLBACK_TYPE FileClose(void* pCookie);
	static XnStatus XN_CALLBACK_TYPE FileSeek64(void* pCookie, XnOSSeekType seekType, const XnInt64 nOffset);
	static XnUInt64 XN_CALLBACK_TYPE FileTell64(void* pCookie);

	static XnStatus XN_CALLBACK_TYPE CodecCreate(void* pCookie, const char* strNodeName, XnCodecID nCodecId, XnCodec** ppCodec);
	static void     XN_CALLBACK_TYPE CodecDestroy(void* pCookie, XnCodec* pCodec);
	
	static XnStatus ResolveGlobalConfigFileName(XnChar* strConfigFile, XnUInt32 nBufSize, const XnChar* strConfigDir);

	// Name of the node (used for identifying the device in the callbacks).
	xnl::String m_nodeName;

	// The path to a *.ONI file which is mounted by this device.
    const xnl::String m_filePath;

	// Handle to the opened file.
	XN_FILE_HANDLE m_fileHandle;

	// Thread handle.
	XN_THREAD_HANDLE m_threadHandle;

	// Running flag.
	OniBool m_running;

	// Seek frame.
	Seek m_seek;
	OniBool m_isSeeking;

	// Speed of playback.
	XnDouble m_dPlaybackSpeed;

	// Timestamps.
	XnUInt64 m_nStartTimestamp;
	XnUInt64 m_nStartTime;
	XnBool m_bHasTimeReference;

	// Repeat recording in loop.
	OniBool m_bRepeat;

	// Player object.
	PlayerNode m_player;

	// Driver EOF callback
	DriverEOFCallback m_driverEOFCallback;
	void *m_driverCookie;

	// Properties.
	PlayerProperties m_properties;

	// List of sources.
	typedef xnl::List<PlayerSource*> SourceList;
	SourceList m_sources;

	// List of streams.
	typedef xnl::List<PlayerStream*> StreamList;
	StreamList m_streams;

	// Internal event for stream ready for data.
	xnl::OSEvent m_readyForDataInternalEvent;

	// Internal event for manual trigger (more frames requested).
	xnl::OSEvent m_manualTriggerInternalEvent;

	// Internal event for seek complete.
	xnl::OSEvent m_SeekCompleteInternalEvent;

	// Critical section.
	xnl::CriticalSection m_cs;

	char m_originalDevice[ONI_MAX_STR];

	char m_iniFilePath[XN_FILE_MAX_PATH];

};

} // namespace oni_files_player

#endif // PLAYERDEVICE_H
