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
#ifndef ONIDATARECORDS_H
#define ONIDATARECORDS_H 1

#include "XnOS.h"
#include "XnList.h"

#include "OniCommon.h"
#include "OniCTypes.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN
#if (ONI_PLATFORM != ONI_PLATFORM_ARC)
#   pragma pack(push, 1)
#endif

#define ONI_CODEC_ID(c1, c2, c3, c4) XnUInt32((c4 << 24) | (c3 << 16) | (c2 << 8) | c1)

#define ONI_CODEC_UNCOMPRESSED      ONI_CODEC_ID('N', 'O', 'N', 'E')
#define ONI_CODEC_16Z_EMB_TABLES    ONI_CODEC_ID('1', '6', 'z', 'T')
#define ONI_CODEC_JPEG              ONI_CODEC_ID('J', 'P', 'E', 'G')

static const XnSizeT IDENTITY_SIZE = 4;

/// The structure of ONI file's file header.
struct FileHeaderData
{
    XnUInt8 identity[IDENTITY_SIZE];
    struct Version
    {
        XnUInt8  major;
        XnUInt8  minor;
        XnUInt16 maintenance;
        XnUInt32 build;
    } version;
    XnUInt64 maxTimeStamp;
    XnUInt32 maxNodeId;
};

/// The structure of ONI file's record header.
struct RecordHeaderData
{
    XnUInt32 magic;
    XnUInt32 recordType;
    XnUInt32 nodeId;
    XnUInt32 fieldsSize;
    XnUInt32 payloadSize;
    XnUInt64 undoRecordPos;
};

struct VideoModeData
{
    XnUInt32 width;
    XnUInt32 height;
    XnUInt32 fps;
};

/// An entry for a frame in the SeekTable
typedef struct DataIndexEntry
{
	XnUInt64 nTimestamp;
	XnUInt32 nConfigurationID;
	XnUInt64 nSeekPos;
} DataIndexEntry;

typedef xnl::List<DataIndexEntry> DataIndexEntryList;

/// Enumerates known record types.
enum RecordType
{
    RECORD_NODE_ADDED_1_0_0_4		= 0x02,
    RECORD_INT_PROPERTY				= 0x03,
    RECORD_REAL_PROPERTY			= 0x04,
    RECORD_STRING_PROPERTY			= 0x05,
    RECORD_GENERAL_PROPERTY			= 0x06,
    RECORD_NODE_REMOVED				= 0x07,
    RECORD_NODE_DATA_BEGIN			= 0x08,
    RECORD_NODE_STATE_READY			= 0x09,
    RECORD_NEW_DATA					= 0x0A,
    RECORD_END						= 0x0B,
    RECORD_NODE_ADDED_1_0_0_5		= 0x0C,
    RECORD_NODE_ADDED				= 0x0D,
    RECORD_SEEK_TABLE               = 0x0E,
};

/// Enumerates known node types.
enum NodeType
{
    NODE_TYPE_INVALID = -1,
    NODE_TYPE_DEVICE  =  1,
    NODE_TYPE_DEPTH   =  2,
    NODE_TYPE_IMAGE   =  3,
    NODE_TYPE_IR      =  5,
};

/// Return a NodeType that matches the given OniSourceType.
static inline NodeType AsNodeType(OniSensorType sensorType)
{
    NodeType res = NODE_TYPE_INVALID;
    switch (sensorType)
    {
    case ONI_SENSOR_COLOR:
        res = NODE_TYPE_IMAGE;
        break;
    case ONI_SENSOR_DEPTH:
        res = NODE_TYPE_DEPTH;
        break;
    case ONI_SENSOR_IR:
        res = NODE_TYPE_IR;
        break;
    default:
        ;
    }
    return res;
}

///
class RecordAssembler
{
public:
    ///
    RecordAssembler();

    ///
    ~RecordAssembler();

    ///
    void initialize();

    ///
    OniStatus serialize(XN_FILE_HANDLE file);

    ///
    OniStatus emit_RECORD_NODE_ADDED_1_0_0_5(
            XnUInt32 nodeType, 
            XnUInt32 nodeId,
            XnUInt32 codecId,
            XnUInt32 numberOfFrames,
            XnUInt64 minTimeStamp,
            XnUInt64 maxTimeStamp);
    ///
    OniStatus emit_RECORD_NODE_ADDED(
            XnUInt32 nodeType, 
            XnUInt32 nodeId,
            XnUInt32 codecId,
            XnUInt32 numberOfFrames,
            XnUInt64 minTimeStamp,
            XnUInt64 maxTimeStamp,
            XnUInt64 seekTablePosition);

    /// 
    OniStatus emit_RECORD_NODE_STATE_READY(XnUInt32 nodeId);

    ///
    OniStatus emit_RECORD_NODE_REMOVED(XnUInt32 nodeId, XnUInt64 nodeAddedPos);

    ///
    OniStatus emit_RECORD_SEEK_TABLE(
            XnUInt32 nodeId, 
	    XnUInt32 numFrames, 
	    DataIndexEntryList dataIndexEntryList);

    ///
    OniStatus emit_RECORD_END();

    ///
    OniStatus emit_RECORD_NODE_DATA_BEGIN(
            XnUInt32 nodeId,
            XnUInt32 framesCount,
            XnUInt64 maxTimeStamp);

    ///
    OniStatus emit_RECORD_NEW_DATA(
            XnUInt32    nodeId,
            XnUInt64    undoRecordPos,
            XnUInt64    timeStamp,
            XnUInt32    frameId, 
            const void* data, 
            XnSizeT     dataSize_bytes);

    ///
    OniStatus emit_RECORD_GENERAL_PROPERTY(
            XnUInt32    nodeId,
            XnUInt64    undoRecordPos,
            const char* propertyName,
            const void* data,
            XnSizeT     dataSize_bytes);

    ///
    OniStatus emit_RECORD_INT_PROPERTY(
            XnUInt32    nodeId,
            XnUInt64    undoRecordPos,
            const char* propertyName,
            XnUInt64    data);
	OniStatus emit_RECORD_REAL_PROPERTY(
		XnUInt32    nodeId,
		XnUInt64    undoRecordPos,
		const char* propertyName,
		XnDouble    data);

private:
    //
    void emitCommonHeader(XnUInt32 recordType, XnUInt32 nodeId, XnUInt64 undoRecordPos);
    //
    OniStatus emitString(const XnChar* pStr, XnSizeT& totalFieldsSize_bytes);
    //
    OniStatus emitData(const void* pData, XnSizeT dataSize_bytes);
    //
    template<typename T>
    OniStatus emit(const T& field, XnSizeT& totalFieldsSize_bytes);

    // Union of a buffer and a record header.
    union 
    {
        XnUInt8*          m_pBuffer;
        RecordHeaderData* m_header;
    };
    // Size of the buffer in bytes.
    XnSizeT m_bufferSize_bytes;
    // Pointer into the buffer for emit operations.
    XnUInt8* m_pEmitPtr;
};

#if (ONI_PLATFORM != ONI_PLATFORM_ARC)
#   pragma pack(pop)
#endif
ONI_NAMESPACE_IMPLEMENTATION_END

#endif // ONIDATARECORDS_H
