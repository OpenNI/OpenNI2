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
#ifndef __XN_FRAME_STREAM_PROCESSOR_H__
#define __XN_FRAME_STREAM_PROCESSOR_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnStreamProcessor.h"
#include <DDK/XnFrameStream.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

/*
* A processor for streams that are frame-based.
*/
class XnFrameStreamProcessor : public XnStreamProcessor
{
public:
	/*
	* Initializes a new frame-stream-processor.
	*
	* @param	pDevicePrivateData	[in]	A pointer to the device.
	* @param	csName				[in]	Name of this stream.
	* @param	nTypeSOF			[in]	The packet type that signifies start-of-frame.
	* @param	nTypeEOF			[in]	The packet type that signifies end-of-frame.
	*/
	XnFrameStreamProcessor(XnFrameStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager, XnUInt16 nTypeSOF, XnUInt16 nTypeEOF);

	/** 
	* Destroys a frame-based stream processor
	*/
	virtual ~XnFrameStreamProcessor();

protected:
	//---------------------------------------------------------------------------
	// Overridden Functions
	//---------------------------------------------------------------------------
	virtual void ProcessPacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize);
	virtual void OnPacketLost();

	//---------------------------------------------------------------------------
	// New Virtual Functions
	//---------------------------------------------------------------------------

	/*
	* Called when a frame starts.
	*
	* @param	pHeader		[in]	Header for current packet.
	*/
	virtual void OnStartOfFrame(const XnSensorProtocolResponseHeader* pHeader);

	/*
	* Called for every chunk received
	*
	* @param	pHeader		[in]	A pointer to current packet header.
	* @param	pData		[in]	A pointer to the data.
	* @param	nDataOffset	[in]	The offset of this data chunk inside current packet.
	* @param	nDataSize	[in]	Size of the data in bytes.
	*/
	virtual void ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize) = 0;

	/*
	* Called when a frame ends.
	*
	* @param	pHeader		[in]	Header for current packet.
	*/
	virtual void OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader);

	/*
	* Called when a frame is ready for reading.
	*
	* @param	nFrameID	[in]	ID of this frame.
	* @param	nFrameTS	[in]	Timestamp of this frame.
	*/
	virtual void OnFrameReady(XnUInt32 /*nFrameID*/, XnUInt64 /*nFrameTS*/) {}

	//---------------------------------------------------------------------------
	// Utility Functions
	//---------------------------------------------------------------------------

	inline XnFrameStream* GetStream()
	{
		return (XnFrameStream*)XnStreamProcessor::GetStream();
	}

	/*
	* Gets the expected output size.
	*/
	inline XnUInt32 GetExpectedOutputSize()
	{
		return GetStream()->GetRequiredDataSize();
	}

	/* 
	* Gets current write buffer.
	*/
	inline XnBuffer* GetWriteBuffer()
	{
		return m_pTripleBuffer->GetWriteBuffer();
	}

	inline OniFrame* GetWriteFrame()
	{
		return m_pTripleBuffer->GetWriteFrame();
	}

	/*
	* Gets current frame ID (for logging purposes mainly).
	*/
	inline XnUInt32 GetCurrentFrameID()
	{
		return m_pTripleBuffer->GetLastFrameID();
	}

	/*
	* Notifies that write buffer has overflowed, logging a warning and reseting it.
	*/
	void WriteBufferOverflowed();

	/* 
	* Checks if write buffer has overflowed, if so, a log will be issued and buffer will reset.
	*/
	inline XnBool CheckWriteBufferForOverflow(XnUInt32 nWriteSize)
	{
		if (GetWriteBuffer()->GetFreeSpaceInBuffer() < nWriteSize)
		{
			WriteBufferOverflowed();
			return FALSE;
		}

		return TRUE;
	}

	/*
	* Marks current frame as corrupted.
	*/
	void FrameIsCorrupted();

	void SetAllowDoubleSOFPackets(XnBool bAllow) { m_bAllowDoubleSOF = bAllow; }

private:
	//---------------------------------------------------------------------------
	// Class Members
	//---------------------------------------------------------------------------
	/* The type of start-of-frame packet. */
	XnUInt16 m_nTypeSOF;
	/* The type of end-of-frame packet. */
	XnUInt16 m_nTypeEOF;
	/* A pointer to the triple frame buffer of this stream. */
	XnFrameBufferManager* m_pTripleBuffer;

	XnChar m_csInDumpMask[100];
	XnChar m_csInternalDumpMask[100];
	XnDumpFile* m_InDump;
	XnDumpFile* m_InternalDump;
	XnBool m_bFrameCorrupted;
	XnBool m_bAllowDoubleSOF;
	XnUInt16 m_nLastSOFPacketID;
	XnUInt64 m_nFirstPacketTimestamp;
};

#endif //__XN_FRAME_STREAM_PROCESSOR_H__
