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
#include "OniStreamFrameHolder.h"
#include "OniStream.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

StreamFrameHolder::StreamFrameHolder(FrameManager& frameManager, VideoStream* pStream) : 
	FrameHolder(frameManager), m_pStream(pStream), m_pLastFrame(NULL)
{
}

StreamFrameHolder::~StreamFrameHolder()
{
	clear();
}

OniStatus StreamFrameHolder::readFrame(VideoStream* pStream, OniFrame** pFrame)
{
	// Verify called with relevant stream.
	if (pStream != m_pStream)
	{
		return ONI_STATUS_BAD_PARAMETER;
	}

	// Make sure frame holder is enabled.
	if (!m_enabled)
	{
		*pFrame = NULL;
		return ONI_STATUS_ERROR;
	}

	// If frame already exists, wait() will return immidiately.
	m_pStream->waitForNewFrameEvent();

	// Return the last frame and set it to NULL.
	lock();
	*pFrame = m_pLastFrame;
	m_pLastFrame = NULL;
	unlock();

	return ONI_STATUS_OK;
}

OniStatus StreamFrameHolder::processNewFrame(VideoStream* pStream, OniFrame* pFrame)
{
	// Verify called with relevant stream.
	if (pStream != m_pStream)
	{
		return ONI_STATUS_BAD_PARAMETER;
	}

	// Make sure frame holder is enabled.
	if (!m_enabled)
	{
		return ONI_STATUS_OK;
	}

	// Release the last stored frame and store received frame.
	lock();
	if (m_pLastFrame != NULL)
	{
		m_frameManager.release(m_pLastFrame);
	}
	m_pLastFrame = pFrame;
	m_frameManager.addRef(m_pLastFrame);
	unlock();

	// Raise the new frame event.
	m_pStream->raiseNewFrameEvent();

	return ONI_STATUS_OK;
}

OniFrame* StreamFrameHolder::peekFrame(VideoStream* pStream)
{
	// Verify called with relevant stream.
	if (pStream != m_pStream)
	{
		return NULL;
	}

	// Make sure frame holder is enabled.
	if (!m_enabled)
	{
		return NULL;
	}

	return m_pLastFrame;
}

// Clear all the frame in the holder.
void StreamFrameHolder::clear()
{
	// Release the last stored frame.
	lock();
	if (m_pLastFrame != NULL)
	{
		m_frameManager.release(m_pLastFrame);
	}
	m_pLastFrame = NULL;
	unlock();
}

// Return list of streams which are members of the stream group.
void StreamFrameHolder::getStreams(VideoStream** ppStreams, int* pNumStreams)
{
	*pNumStreams = 1;
	*ppStreams = m_pStream;
}

// Return number of streams which are members of the stream group.
int StreamFrameHolder::getNumStreams()
{
	return 1;
}

void StreamFrameHolder::setStreamEnabled(VideoStream* pStream, OniBool enabled)
{
	if (pStream != m_pStream)
		return;

	if (enabled == FALSE)
	{
		clear();
	}
}

ONI_NAMESPACE_IMPLEMENTATION_END
