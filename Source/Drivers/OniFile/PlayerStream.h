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
/// Contains the declaration of Stream class that implements a stream from 
/// a virtual OpenNI device.

#ifndef PLAYERSTREAM_H
#define PLAYERSTREAM_H

#include "Driver/OniDriverAPI.h"
#include "PlayerProperties.h"
#include "PlayerSource.h"
#include "XnOSCpp.h"

namespace oni_file {

class Decoder;
class PlayerDevice;

/// Implements a stream from a virtual OpenNI device.
class PlayerStream : public oni::driver::StreamBase
{
public:

	// General stream event.
	typedef struct 
	{
		PlayerStream* pStream;
	} StreamEventArgs;
	typedef xnl::Event<StreamEventArgs> StreamEvent;
	typedef void (ONI_CALLBACK_TYPE* StreamCallback)(const StreamEventArgs& streamEventArgs, void* pCookie);

	// Ready for data event.
	typedef StreamEventArgs ReadyForDataEventArgs;
	typedef StreamEvent		ReadyForDataEvent;
	typedef StreamCallback  ReadyForDataCallback;

	// Destroy event.
	typedef StreamEventArgs DestroyEventArgs;
	typedef StreamEvent		DestroyEvent;
	typedef StreamCallback  DestroyCallback;

public:
	/// Constructor.
    PlayerStream(PlayerDevice* pDevice, PlayerSource* pSource);

	/// Destructor.
	virtual ~PlayerStream();

    /// Initialize the stream object.
    OniStatus Initialize();

	virtual int getRequiredFrameSize();
	
    /// @copydoc OniStreamBase::start()
    virtual OniStatus start();

	/// has start() been called on this stream already?
	bool isStreamStarted() { return m_isStarted; }

    /// @copydoc OniStreamBase::stop()
    virtual void stop();

	// Return the player source the stream was created on.
	PlayerSource* GetSource();

    /// @copydoc OniStreamBase::getProperty(int,void*,int*)
    virtual OniStatus getProperty(int propertyId, void* pData, int* pDataSize);

	/// @copydoc OniStreamBase::setProperty(int,void*,int*)
	virtual OniStatus setProperty(int propertyId, const void* pData, int dataSize);

	// Register for 'ready for data' event.
	OniStatus RegisterReadyForDataEvent(ReadyForDataCallback callback, void* pCookie, OniCallbackHandle& handle);

	// Unregister from 'ready for data' event.
	void UnregisterReadyForDataEvent(OniCallbackHandle handle);

	// Register for 'destroy' event.
	OniStatus RegisterDestroyEvent(DestroyCallback callback, void* pCookie, OniCallbackHandle& handle);

	// Unregister from 'destroy' event.
	void UnregisterDestroyEvent(OniCallbackHandle handle);

	void notifyAllProperties();
private:
	void destroy();

    void MainLoop();
	static XN_THREAD_PROC ThreadProc(XN_THREAD_PARAM pThreadParam);

	// Callback to be called when new data is available.
	static void ONI_CALLBACK_TYPE OnNewDataCallback(const PlayerSource::NewDataEventArgs& newDataEventArgs, void* pCookie);

// Data members
private:

	// Source the stream was created on.
	PlayerSource* m_pSource;

	// Stream properties.
    PlayerProperties m_properties;

	// Handle to new data callback.
	OniCallbackHandle m_newDataHandle;

	// Ready for data event.
	ReadyForDataEvent m_readyForDataEvent;

	// Destroy event.
	DestroyEvent m_destroyEvent;

	// Critical section.
	xnl::CriticalSection m_cs;

	// Are we streaming right now?
	bool m_isStarted;

	int m_requiredFrameSize;

	PlayerDevice* m_pDevice;
};

} // namespace oni_files_player

#endif // PLAYERSTREAM_H
