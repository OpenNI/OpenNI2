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
#ifndef ONIFRAMEHOLDER_H
#define ONIFRAMEHOLDER_H

#include "OniCommon.h"
#include "OniFrameManager.h"
#include "OniCTypes.h"
#include "XnOSCpp.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

class VideoStream;

class FrameHolder
{
public:

	// Constructor.
	FrameHolder(FrameManager& frameManager) : m_frameManager(frameManager), m_enabled(TRUE) {}

	// Destructor.
	virtual ~FrameHolder() {}

	// Get the next frame belonging to a stream.
	virtual OniStatus readFrame(VideoStream* pStream, OniFrame** pFrame) = 0;

	// Process a newly received frame.
	virtual OniStatus processNewFrame(VideoStream* pStream, OniFrame* pFrame) = 0;
	
	// Peek at next frame.
	virtual OniFrame* peekFrame(VideoStream* pStream) = 0;

	// Clear all the frame in the holder.
	virtual void clear() = 0;

	// Set whether stream is enabled.
	virtual void setStreamEnabled(VideoStream* /*pStream*/, OniBool /*enabled*/) {}

	// Return list of streams which are members of the stream group.
	virtual void getStreams(VideoStream** ppStreams, int* pNumStreams) = 0;

	// Return number of streams which are members of the stream group.
	virtual int getNumStreams() = 0;

	// Critical section lock (usually used with peek).
	void lock() { m_cs.Lock(); }

	// Critical section unlock (usually used with peek).
	void unlock() { m_cs.Unlock(); }

	// Set whether frame holder is enabled.
	void setEnabled(OniBool enabled) { m_enabled = enabled; }

// Data members:
protected:
	FrameManager& m_frameManager;
	OniBool m_enabled;

private:

	xnl::CriticalSection m_cs;

};

ONI_NAMESPACE_IMPLEMENTATION_END

#endif // ONIFRAMEHOLDER_H
