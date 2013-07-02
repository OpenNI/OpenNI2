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
#include "OniSyncedStreamsFrameHolder.h"
#include "Driver/OniDriverTypes.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN
	
// Constructor.
SyncedStreamsFrameHolder::SyncedStreamsFrameHolder(FrameManager& frameManager, VideoStream** ppStreams, int numStreams) : 
	FrameHolder(frameManager), m_FrameSyncedStreams(numStreams)
{
	m_FrameSyncedStreams.SetSize(numStreams);
	m_FrameSyncedStreams.Zero();

	lock();

	// Set the frame sync group for all the streams and store their streams.
	for (int i = 0; i < numStreams; ++i)
	{
		m_FrameSyncedStreams[i].pStream = ppStreams[i];
		setStreamEnabled(ppStreams[i], ppStreams[i]->isStarted());
	}

	unlock();
}

// Destructor.
SyncedStreamsFrameHolder::~SyncedStreamsFrameHolder()
{
	clear();
}

// Get the next frame belonging to a stream.
OniStatus SyncedStreamsFrameHolder::readFrame(VideoStream* pStream, OniFrame** pFrame)
{
	// Make sure frame holder is enabled.
	if (!m_enabled)
	{
		*pFrame = NULL;
		return ONI_STATUS_ERROR;
	}

	lock();

	// Parse all the streams.
	int frameId = (m_FrameSyncedStreams[0].pLastFrame != NULL) ? 
					m_FrameSyncedStreams[0].pLastFrame->frameIndex : -1;
	int minSyncedFrameId = -1;
	XnUInt32 validFrameCount = 0;
	OniBool syncedFramesExist = FALSE;
	XnUInt32 numFrameSyncStreams = m_FrameSyncedStreams.GetSize();
	for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
	{
		// Is this the stream frame was received on?
		if (m_FrameSyncedStreams[i].pStream == pStream)
		{
			// If no frames exist, wait for a new frame event.
			if ((m_FrameSyncedStreams[i].pSyncedFrame == NULL) &&
				(m_FrameSyncedStreams[i].pLastFrame == NULL))
			{
				unlock();
				pStream->waitForNewFrameEvent();
					
				// Algorithm needs to be restarted, so call function again 
				// NOTE: this is not a real recursion as next call will surely succeed (new frame event received).
				return readFrame(pStream, pFrame);
			}

			// Check if synced frame exists.
			if (m_FrameSyncedStreams[i].pSyncedFrame != NULL)
			{
				// Copy frame and clear synced frame.
				*pFrame = m_FrameSyncedStreams[i].pSyncedFrame;
				m_FrameSyncedStreams[i].pSyncedFrame = NULL;
			}
			else // if (m_FameSyncedStreams[i].pLastFrame != NULL)
			{
				// Copy frame and clear last frame.
				*pFrame = m_FrameSyncedStreams[i].pLastFrame;
				m_FrameSyncedStreams[i].pLastFrame = NULL;

				// Increment valid frame count, to make sure that after invalidation last frames may still be copied to synced.
				++validFrameCount;
			}
		}
		// Check if other synced frames exist.
		else if (m_FrameSyncedStreams[i].pSyncedFrame != NULL)
		{
			if ((minSyncedFrameId == -1) || (minSyncedFrameId < m_FrameSyncedStreams[i].pSyncedFrame->frameIndex))
			{
				minSyncedFrameId = m_FrameSyncedStreams[i].pSyncedFrame->frameIndex;
			}
			syncedFramesExist = TRUE;
		}

		// Check if stream has a last frame frame.
		if ((m_FrameSyncedStreams[i].pLastFrame != NULL) && 
			(m_FrameSyncedStreams[i].pLastFrame->frameIndex == frameId))
		{
			++validFrameCount;
		}
	}

	// Check if there is are synced frames which have lower ID from the the frame being returned.
	if ((minSyncedFrameId != -1) && ((*pFrame)->frameIndex != minSyncedFrameId))
	{
		// Invalidate the synced frames.
		for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
		{
			// Release the stored synced frame.
			if (m_FrameSyncedStreams[i].pSyncedFrame != NULL)
			{
				m_frameManager.release(m_FrameSyncedStreams[i].pSyncedFrame);
			}
			m_FrameSyncedStreams[i].pSyncedFrame = NULL;
		}
		syncedFramesExist = FALSE;
	}

	// Check if all the streams have valid frames but no synced ones.
	if (!syncedFramesExist && (validFrameCount == numFrameSyncStreams))
	{
		// 'Latch' the valid frames (move them to 'synced').
		for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
		{
			// Replace synced frame with last frame.
			m_FrameSyncedStreams[i].pSyncedFrame = m_FrameSyncedStreams[i].pLastFrame;
			m_FrameSyncedStreams[i].pLastFrame = NULL;
		}

		// Send the raise event to all streams.
		for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
		{
			// Raise internal new frame event.
			m_FrameSyncedStreams[i].pStream->raiseNewFrameEvent();
		}
	}

	unlock();

	return ONI_STATUS_OK;
}

// Process a newly received frame.
OniStatus SyncedStreamsFrameHolder::processNewFrame(VideoStream* pStream, OniFrame* pFrame)
{
	// Make sure frame holder is enabled.
	if (!m_enabled)
	{
		return ONI_STATUS_OK;
	}

	lock();

	// Parse all the streams.
	int frameId = pFrame->frameIndex;
	XnUInt32 validFrameCount = 1;
	XnUInt32 syncedFramesCount = 0;
	XnUInt32 numFrameSyncStreams = m_FrameSyncedStreams.GetSize();
	for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
	{
		// Is this the stream frame was received on?
		if (m_FrameSyncedStreams[i].pStream == pStream)
		{
			// Release any old frame and store the new one.
			if (m_FrameSyncedStreams[i].pLastFrame != NULL)
			{
				m_frameManager.release(m_FrameSyncedStreams[i].pLastFrame);
				m_FrameSyncedStreams[i].pLastFrame = NULL;
			}

			// Copy the frame only if stream is enabled.
			if (m_FrameSyncedStreams[i].enabled)
			{
				m_FrameSyncedStreams[i].pLastFrame = pFrame;
				m_frameManager.addRef(pFrame);
			}
			else
			{
				--validFrameCount;
			}
		}
		// Check if stored frame in other streams has same ID.
		else if ((m_FrameSyncedStreams[i].pLastFrame != NULL) &&
					(m_FrameSyncedStreams[i].pLastFrame->frameIndex == frameId))
		{
			++validFrameCount;
		}

		// Check if stream has synced frame.
		if (m_FrameSyncedStreams[i].pSyncedFrame != NULL)
		{
			++syncedFramesCount;
		}
	}

	// Check if all the streams have valid frame, but no synced ones or no synced frames retrieved.
	if (((syncedFramesCount == 0) || (syncedFramesCount == numFrameSyncStreams)) &&
		(validFrameCount == numFrameSyncStreams))
	{
		// 'Latch' the valid frames (move them to 'synced').
		for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
		{
			// Release the stored synced frame.
			if (m_FrameSyncedStreams[i].pSyncedFrame != NULL)
			{
				m_frameManager.release(m_FrameSyncedStreams[i].pSyncedFrame);
			}

			// Replace synced frame with last frame.
			m_FrameSyncedStreams[i].pSyncedFrame = m_FrameSyncedStreams[i].pLastFrame;
			m_FrameSyncedStreams[i].pLastFrame = NULL;
		}

		// Send the raise event to all streams.
		for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
		{
			// Raise internal new frame event.
			m_FrameSyncedStreams[i].pStream->raiseNewFrameEvent();
		}
	}

	unlock();

	return ONI_STATUS_OK;
}

// Peek at next frame.
OniFrame* SyncedStreamsFrameHolder::peekFrame(VideoStream* pStream)
{
	// Make sure frame holder is enabled.
	if (!m_enabled)
	{
		return NULL;
	}

	lock();

	// Parse all the streams.
	XnUInt32 numFrameSyncStreams = m_FrameSyncedStreams.GetSize();
	for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
	{
		// Is this the stream frame was received on?
		if (m_FrameSyncedStreams[i].pStream == pStream)
		{
			OniFrame* pRetVal = m_FrameSyncedStreams[i].pSyncedFrame;
			unlock();
			return pRetVal;
		}
	}

	unlock();
	return NULL;
}

// Clear all the frame in the holder.
void SyncedStreamsFrameHolder::clear()
{
	lock();

	// Release all the streams from the frame sync group and release the stored frames.
	XnUInt32 numFrameSyncStreams = m_FrameSyncedStreams.GetSize();
	for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
	{
		if (m_FrameSyncedStreams[i].pLastFrame != NULL)
		{
			m_frameManager.release(m_FrameSyncedStreams[i].pLastFrame);
			m_FrameSyncedStreams[i].pLastFrame = NULL;
		}
		if (m_FrameSyncedStreams[i].pSyncedFrame != NULL)
		{
			m_frameManager.release(m_FrameSyncedStreams[i].pSyncedFrame);
			m_FrameSyncedStreams[i].pSyncedFrame = NULL;
		}
	}

	unlock();
}

// Set whether stream is enabled.
void SyncedStreamsFrameHolder::setStreamEnabled(VideoStream* pStream, OniBool enabled)
{
	lock();

	// Parse all the streams.
	XnUInt32 numEnabled = 0;
	XnUInt32 numFrameSyncStreams = m_FrameSyncedStreams.GetSize();
	for (XnUInt32 i = 0; i < numFrameSyncStreams; ++i)
	{
		// Is this the stream being updated?
		if (m_FrameSyncedStreams[i].pStream == pStream)
		{
			// Set enable flag.
			m_FrameSyncedStreams[i].enabled = enabled;

			// If not enabled, release the stored frames.
			if (!enabled)
			{
				if (m_FrameSyncedStreams[i].pLastFrame != NULL)
				{
					m_frameManager.release(m_FrameSyncedStreams[i].pLastFrame);
					m_FrameSyncedStreams[i].pLastFrame = NULL;
				}
				if (m_FrameSyncedStreams[i].pSyncedFrame != NULL)
				{
					m_frameManager.release(m_FrameSyncedStreams[i].pSyncedFrame);
					m_FrameSyncedStreams[i].pSyncedFrame = NULL;
				}
			}
		}

		// Count enabled streams.
		if (m_FrameSyncedStreams[i].enabled)
		{
			numEnabled++;
		}
	}

	// Update global enabled flag.
	// NOTE: do NOT change m_enabled flag for now, to make sure no frames are received.
	//m_enabled = (numEnabled == numFrameSyncStreams) ? TRUE : FALSE;

	unlock();
}

// Return list of streams which are members of the stream group.
void SyncedStreamsFrameHolder::getStreams(VideoStream** ppStreams, int* pNumStreams)
{
	lock();

	// Copy stream list.
	int numFrameSyncStreams = m_FrameSyncedStreams.GetSize();
	*pNumStreams = (*pNumStreams > numFrameSyncStreams) ? numFrameSyncStreams : *pNumStreams;
	for (int i = 0; i < *pNumStreams; ++i)
	{
		ppStreams[i] = m_FrameSyncedStreams[i].pStream;
	}

	unlock();
}

// Return number of streams which are members of the stream group.
int SyncedStreamsFrameHolder::getNumStreams()
{
	return m_FrameSyncedStreams.GetSize();
}

ONI_NAMESPACE_IMPLEMENTATION_END
