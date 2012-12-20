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
#ifndef __XN_STREAMING_STREAM_H__
#define __XN_STREAMING_STREAM_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnDeviceStream.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

/** Represents a base class for streams which are not frame based, by streaming. */
class XnStreamingStream : public XnDeviceStream
{
public:
	XnStreamingStream(const XnChar* csType, const XnChar* csName);
	~XnStreamingStream() { Free(); }

	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	XnStatus Init();

protected:
	//---------------------------------------------------------------------------
	// Properties Getters
	//---------------------------------------------------------------------------
	inline XnActualIntProperty& ReadChunkSizeProperty() { return m_ReadChunkSize; }

	//---------------------------------------------------------------------------
	// Getters
	//---------------------------------------------------------------------------
	inline XnUInt32 GetReadChunkSize() { return (XnUInt32)m_ReadChunkSize.GetValue(); }

	//---------------------------------------------------------------------------
	// Setters
	//---------------------------------------------------------------------------
	virtual XnStatus SetReadChunkSize(XnUInt32 nChunkSize);

private:
	static XnStatus XN_CALLBACK_TYPE SetReadChunkSizeCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);

	//---------------------------------------------------------------------------
	// Members
	//---------------------------------------------------------------------------
	XnActualIntProperty m_IsStreamingStream;
	XnActualIntProperty m_ReadChunkSize;
};

#endif //__XN_STREAMING_STREAM_H__