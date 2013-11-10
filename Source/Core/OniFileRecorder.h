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
#ifndef ONIFILERECORDER_H
#define ONIFILERECORDER_H 1

#include "XnString.h"
#include "XnPriorityQueue.h"

// These come from OniFile/Formats
#include "Xn16zEmbTablesCodec.h"
#include "XnJpegCodec.h"
#include "XnUncompressedCodec.h"

#include "OniRecorder.h"
#include "OniDataRecords.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

class FileRecorder : public Recorder
{
public:
    FileRecorder(FrameManager& frameManager, xnl::ErrorLogger& errorLogger, OniRecorderHandle handle = NULL);
	~FileRecorder();

    OniStatus initialize(const char* fileName);

    OniStatus attachStream(VideoStream& stream, OniBool allowLossyCompression);
    OniStatus detachStream(VideoStream& stream);
	
    virtual OniStatus start();

    virtual OniStatus record(VideoStream& stream, OniFrame& aFrame);

    virtual OniStatus recordStreamProperty(
            VideoStream&     stream,
            int         propertyId,
            const void* pData, 
            int         dataSize);
    
private:
    XN_DISABLE_COPY_AND_ASSIGN(FileRecorder)

    // Messages are sent to Recorder's message loop and executed asynchronously.
    // Please note: not all the fields are valid for every message. For example,
    // Message::MESSAGE_DETACH relies only upon the nodeId field of the message, other
    // fields are invalid and irrelevant for that message.
    struct Message
    {
        enum Type
        {
            MESSAGE_NO_OPERATION,  ///< Does not use any of Message fields. Does nothing.
            MESSAGE_INITIALIZE,    ///< Does not use any of Message fields.
            MESSAGE_TERMINATE,     ///< Does not use any of Message fields.
            MESSAGE_ATTACH,        ///< Uses: nodeId, pStream
            MESSAGE_DETACH,        ///< Uses: nodeId
            MESSAGE_START,         ///< Does not use any of Message fields.
            MESSAGE_RECORD,        ///< Uses: nodeId, pFrame
            MESSAGE_RECORDPROPERTY,
        }
        type;

        XnUInt32    nodeId;
        VideoStream*     pStream;
        union {
            const void*     pData;
            OniFrame*       pFrame;
        };
        XnUInt32    propertyId;
        XnSizeT     dataSize;
        
    };

    // Used for undo functionality.
           class Memento;
    friend class Memento;

    // The main function of Recorder's thread.
    static XN_THREAD_PROC threadMain(XN_THREAD_PARAM pThreadParam);

    // Obtains the next message from the queue of messages and executes an
    // action associated with that message.
    void messagePump();

    // Sends a message to the threadMain.
    void send(
            Message::Type type, 
            VideoStream*     pStream    = NULL, 
            const void* pData      = NULL,
            XnUInt32    propertyId = 0u,
            XnSizeT     dataSize   = 0u,
			int priority = ms_priorityNormal);

    // Message handlers:
    void onInitialize();
    void onTerminate();
    void onAttach(XnUInt32 nodeId, VideoStream* pStream);
    void onDetach(XnUInt32 nodeId);
    void onStart (XnUInt32 nodeId);
    void onRecord(XnUInt32 nodeId, XnCodecBase* pCodec, const OniFrame* pFrame, XnUInt32 frameId, XnUInt64 timestamp);
    void onRecordProperty(
            XnUInt32    nodeId, 
            XnUInt32    propertyId,
            const void* pData,
            XnSizeT     dataSize);

	FrameManager& m_frameManager;

    // Error logger.
    xnl::ErrorLogger& m_errorLogger;

    // Stores utility information about a stream. Currently the structure is
    // pretty simple, but it might evolve in future.
    struct AttachedStreamInfo
    {
        XnUInt32       nodeId;
        XnUInt32       frameId;
        XnCodecBase*   pCodec;
        OniBool        allowLossyCompression;
        XnUInt64       lastInputTimestamp;
        XnUInt64       lastOutputTimestamp;

        // needed for overriding the NODE_ADDED record when detaching the stream
        XnUInt64       nodeAddedRecordPosition;
        XnUInt32       nodeType; 
        XnUInt32       codecId;

        // needed for keeping track of undoRecordPos field
        XnUInt64       lastNewDataRecordPosition;
        xnl::Hash<const char *, XnUInt64> 
                       lastPropertyRecordPosition;

        // needed for generating the SeekTable in the end
        DataIndexEntryList dataIndex;
    };

    // A map of stream -> stream information.
    typedef xnl::Lockable< xnl::Hash<VideoStream*, AttachedStreamInfo> > AttachedStreams;
    AttachedStreams m_streams;

    // A helper function for the properties' undoRecordPos
    XnUInt64 getLastPropertyRecordPos(XnUInt32 nodeId, const char *propName, XnUInt64 newRecordPos);

    // The maximum ID of a stream. This number grows with the every next stream
    // attached to this Recorder and never decreases.
    XnUInt32 m_maxId;

    // increased any time when a property changes in ANY attached stream,
    // to avoid the player seeking with the tables when a configuration has changed.
    XnUInt32 m_configurationId;

    // A message queue, used by threadMain and Send().
    typedef xnl::Lockable<xnl::PriorityQueue<Message, 3> > MessageQueue;
    MessageQueue m_queue;
	int m_propertyPriority;
	static const int ms_priorityLow = 2;
	static const int ms_priorityNormal = 1;
	static const int ms_priorityHigh = 0;

    // The Recorder uses RecordAssembler to assemble records correctly and to
    // serialize them to a file.
    RecordAssembler m_assembler;

    XN_THREAD_HANDLE m_thread;
    FileHeaderData   m_fileHeader;  //< Will be patched during termination.
    xnl::String      m_fileName;
    XN_FILE_HANDLE   m_file;
};

ONI_NAMESPACE_IMPLEMENTATION_END

#endif // ONIFILERECORDER_H
