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
#ifndef PLAYERNODE_H
#define PLAYERNODE_H

#include "DataRecords.h"
#include "XnPlayerTypes.h"
#include "Formats/XnCodecIDs.h"
#include "Formats/XnStreamFormats.h"
#include "XnHash.h"
#include "XnEvent.h"

class XnCodec;

namespace oni_file {

class PlayerNode
{
public:

	/** Prototype for state change callback function. **/
	typedef void (XN_CALLBACK_TYPE* EndOfFileReachedHandler)(void* pCookie);

	typedef struct  
	{
		XnStatus (XN_CALLBACK_TYPE* Create)(void* pCookie, const char* strNodeName, XnCodecID nCodecId, XnCodec** ppCodec);
		void     (XN_CALLBACK_TYPE* Destroy)(void* pCookie, XnCodec* pCodec);
	} CodecFactory;

public:
	PlayerNode(const XnChar* strName);
	virtual ~PlayerNode();

	//public functions
	virtual XnStatus Init();
	virtual XnStatus Destroy();

	//xn::ModulePlayer implementation
	virtual XnStatus SetInputStream(void* pStreamCookie, XnPlayerInputStreamInterface* pStream);
	virtual XnStatus ReadNext();
	virtual XnStatus SetNodeNotifications(void* pNotificationsCookie, XnNodeNotifications* pNodeNotifications);
	virtual XnStatus SetNodeCodecFactory(void* pFactoryCookie, PlayerNode::CodecFactory* pPlayerNodeCodecFactory);
	virtual XnStatus SetRepeat(XnBool bRepeat);
	virtual XnStatus SeekToTimeStamp(XnInt64 nTimeOffset, XnPlayerSeekOrigin origin);

	virtual XnStatus SeekToFrame(const XnChar* strNodeName, XnInt32 nFrameOffset, XnPlayerSeekOrigin origin);
	virtual XnStatus TellTimestamp(XnUInt64& nTimestamp);
	virtual XnStatus TellFrame(const XnChar* strNodeName, XnUInt32& nFrameNumber);
	virtual XnUInt32 GetNumFrames(const XnChar* strNodeName, XnUInt32& nFrames);

	virtual const XnChar* GetSupportedFormat();
	virtual XnBool IsEOF();
	virtual XnStatus RegisterToEndOfFileReached(EndOfFileReachedHandler handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual void UnregisterFromEndOfFileReached(XnCallbackHandle hCallback);

	static XnStatus ValidateStream(void *pStreamCookie, XnPlayerInputStreamInterface* pInputStream);

private:
	struct RecordUndoInfo
	{
		RecordUndoInfo() { Reset(); }
		void Reset() { nRecordPos = 0; nUndoRecordPos = 0; }
		XnUInt64 nRecordPos;
		XnUInt64 nUndoRecordPos;
	};

	typedef xnl::StringsHash<RecordUndoInfo> RecordUndoInfoMap;

	struct PlayerNodeInfo
	{
		PlayerNodeInfo();
		~PlayerNodeInfo();

		void Reset();

		XnBool bValid;
		XnChar strName[XN_MAX_NAME_LENGTH];
		XnUInt64 nLastDataPos;
		XnCodecID compression;
		XnUInt32 nFrames;
		XnUInt32 nCurFrame;
		XnUInt64 nMaxTimeStamp;
		XnBool bStateReady;
		XnBool bIsGenerator;
		XnCodec* pCodec;
		RecordUndoInfoMap recordUndoInfoMap;
		RecordUndoInfo newDataUndoInfo;
		DataIndexEntry* pDataIndex;
	};

	XnStatus ProcessRecord(XnBool bProcessPayload);
	XnStatus SeekToTimeStampAbsolute(XnUInt64 nDestTimeStamp);
	XnStatus SeekToTimeStampRelative(XnInt64 nOffset);
	XnStatus UndoRecord(PlayerNode::RecordUndoInfo& undoInfo, XnUInt64 nDestPos, XnBool& nUndone);
	XnStatus SeekToFrameAbsolute(XnUInt32 nNodeID, XnUInt32 nFrameNumber);
	XnStatus ProcessEachNodeLastData(XnUInt32 nIDToProcessLast);

	static XnInt32 CompareVersions(const XnVersion* pV0, const XnVersion* pV1);
	XnStatus OpenStream();
	XnStatus Read(void* pData, XnUInt32 nSize, XnUInt32& nBytesRead);
	XnStatus ReadRecordHeader(Record& record);
	XnStatus ReadRecordFields(Record& record);
	//ReadRecord reads just the fields of the record, not the payload.
	XnStatus ReadRecord(Record& record);
	XnStatus SeekStream(XnOSSeekType seekType, XnInt64 nOffset);
	XnUInt64 TellStream();
	XnStatus CloseStream();

	XnBool IsTypeGenerator(XnProductionNodeType type);

	XnStatus HandleRecord(Record& record, XnBool bHandleRecord);
	XnStatus HandleNodeAddedImpl(XnUInt32 nNodeID, XnProductionNodeType type, const XnChar* strName, XnCodecID compression, XnUInt32 nNumberOfFrames, XnUInt64 nMinTimestamp, XnUInt64 nMaxTimestamp);
	XnStatus HandleNodeAddedRecord(NodeAddedRecord record);
	XnStatus HandleGeneralPropRecord(GeneralPropRecord record);
	XnStatus HandleIntPropRecord(IntPropRecord record);
	XnStatus HandleRealPropRecord(RealPropRecord record);
	XnStatus HandleStringPropRecord(StringPropRecord record);
	XnStatus HandleNodeRemovedRecord(NodeRemovedRecord record);
	XnStatus HandleNodeStateReadyRecord(NodeStateReadyRecord record);
	XnStatus HandleNodeDataBeginRecord(NodeDataBeginRecord record);
	XnStatus HandleNewDataRecord(NewDataRecordHeader record, XnBool bHandleRecord);
	XnStatus HandleDataIndexRecord(DataIndexRecordHeader record, XnBool bReadPayload);
	XnStatus HandleEndRecord(EndRecord record);
	XnStatus Rewind();
	XnStatus ProcessUntilFirstData();
	PlayerNodeInfo* GetPlayerNodeInfo(XnUInt32 nNodeID);
	XnStatus RemovePlayerNodeInfo(XnUInt32 nNodeID);
	XnUInt32 GetPlayerNodeIDByName(const XnChar* strNodeName);
	PlayerNodeInfo* GetPlayerNodeInfoByName(const XnChar* strNodeName);
	XnStatus SaveRecordUndoInfo(PlayerNodeInfo* pPlayerNodeInfo, const XnChar* strPropName, XnUInt64 nRecordPos, XnUInt64 nUndoRecordPos);
	XnStatus GetRecordUndoInfo(PlayerNodeInfo* pPlayerNodeInfo, const XnChar* strPropName, XnUInt64& nRecordPos, XnUInt64& nUndoRecordPos);
	XnStatus SkipRecordPayload(Record record);
	XnStatus SeekToRecordByType(XnUInt32 nNodeID, RecordType type);
	DataIndexEntry* FindTimestampInDataIndex(XnUInt32 nNodeID, XnUInt64 nTimestamp);
	DataIndexEntry** GetSeekLocationsFromDataIndex(XnUInt32 nNodeID, XnUInt32 nDestFrame);

	// BC functions
	XnStatus HandleNodeAdded_1_0_0_5_Record(NodeAdded_1_0_0_5_Record record);
	XnStatus HandleNodeAdded_1_0_0_4_Record(NodeAdded_1_0_0_4_Record record);

	static const XnUInt64 DATA_MAX_SIZE;
	static const XnUInt64 RECORD_MAX_SIZE;
	static const XnVersion OLDEST_SUPPORTED_FILE_FORMAT_VERSION;
	static const XnVersion FIRST_FILESIZE64BIT_FILE_FORMAT_VERSION;

	XnVersion m_fileVersion;
	XnChar m_strName[XN_MAX_NAME_LENGTH];
	XnBool m_bOpen;
	XnBool m_bIs32bitFileFormat;
	XnUInt8* m_pRecordBuffer;
	XnUInt8* m_pUncompressedData;
	void* m_pStreamCookie;
	XnPlayerInputStreamInterface* m_pInputStream;
	void* m_pNotificationsCookie;
	XnNodeNotifications* m_pNodeNotifications;
	void* m_pNodeCodecFactoryCookie;
	PlayerNode::CodecFactory* m_pNodeCodecFactory;
	XnBool m_bRepeat;
	XnBool m_bDataBegun;
	XnBool m_bEOF;
	
	XnUInt64 m_nTimeStamp;
	XnUInt64 m_nGlobalMaxTimeStamp;

	xnl::EventNoArgs m_eofReachedEvent;

	PlayerNodeInfo* m_pNodeInfoMap;
	XnUInt32 m_nMaxNodes;

	DataIndexEntry** m_aSeekTempArray;

	XnMapOutputMode m_lastOutputMode;
};

}

#endif // PLAYERNODE_H
