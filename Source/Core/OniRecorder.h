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
#ifndef ONIRECORDER_H
#define ONIRECORDER_H 1

#include "OniStream.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

class VideoStream;

class Recorder
{
public:
    /**
     * Constructs a recorder, given an ErrorLogger and a handle.
     * @note The newly constructed recorder becomes the owner of the handle.
     * @note The handle might be NULL.
     */
	Recorder(OniRecorderHandle handle = NULL);

    /**
     * Destroys the recorder and stops recording if needed.
     */
	virtual ~Recorder();

    /**
     * Initializes the recorder.
     * @param fileName The file that will store the recording.
     */
	virtual OniStatus initialize(const char* path);

    /**
     * Attaches a stream to the recorder. Can not be done if Start() has been
     * called at least once.
     */
	virtual OniStatus attachStream(VideoStream& stream, OniBool allowLossyCompression);

    /**
     * Detaches a stream from the recorder.
     */
	virtual OniStatus detachStream(VideoStream& stream);

    /**
     * Detaches all streams from the recorder.
     */
    OniStatus detachAllStreams();

    /**
     * Starts recording.
     * @note There's a known side effect related to AttachStream(Stream&).
     * @see AttachStream(Stream&)
     */
	virtual OniStatus start();

    /**
     * Stops recording.
     */
	virtual void stop();

    /**
     * Records a frame into the given stream.
     */
	virtual OniStatus record(VideoStream& stream, OniFrame& aFrame) = 0;

    /**
     *
     */
	virtual OniStatus recordStreamProperty(
		VideoStream&     stream,
		int         propertyId,
		const void* pData, 
		int         dataSize) = 0;

protected:
	// There's a 1:1 mapping between a Recorder object and a handle to that object.
	// The Recorder object owns its handle.
	OniRecorderHandle m_handle;

	// A map of stream -> stream information.
	typedef xnl::Lockable< xnl::Hash<VideoStream*, XnUInt32> > StreamFrameIDList;
	StreamFrameIDList m_frameIds;

	XnBool           m_running;     //< TRUE whenever the threadMain is running.
	XnBool           m_started;     //< TRUE whenever the recorder has started.
	XnBool           m_wasStarted;  //< TRUE if the recorder has been started once.

};

ONI_NAMESPACE_IMPLEMENTATION_END

#endif // ONIRECORDER_H
