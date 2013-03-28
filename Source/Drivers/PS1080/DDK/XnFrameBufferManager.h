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
#ifndef __XN_MULTI_FRAME_BUFFER_H__
#define __XN_MULTI_FRAME_BUFFER_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <XnEvent.h>
#include "XnOniFramePool.h"
#include <Core/XnBuffer.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

class XnFrameBufferManager
{
public:
	XnFrameBufferManager();
	~XnFrameBufferManager();

	XnStatus Init(XnUInt32 nBufferSize);
	XnStatus Reallocate(XnUInt32 nBufferSize);
	void Free();

	inline XnBuffer* GetWriteBuffer() 
	{ 
		// NOTE: no need to lock buffer, as we assume the same thread is the one that is responsible
		// for marking working buffer as stable.
		return &m_writeBuffer; 
	}

	inline OniFrame* GetWriteFrame()
	{
		return m_pWorkingBuffer;
	}

	void AddRefToFrame(OniFrame* pFrame);
	void ReleaseFrame(OniFrame* pFrame);

	void MarkWriteBufferAsStable(XnUInt32* pnFrameID);

	inline XnUInt32 GetLastFrameID() const { return m_nStableFrameID; }

	typedef struct NewFrameEventArgs
	{
		OniFrame* pFrame;
	} NewFrameEventArgs;

	typedef xnl::Event<NewFrameEventArgs> NewFrameEvent;
	NewFrameEvent/* ::EventInterface */& OnNewFrameEvent() { return m_NewFrameEvent; }

private:
	XnOniFramePool* m_pBufferPool;
	OniFrame* m_pWorkingBuffer;
	XnUInt32 m_nStableFrameID;
	NewFrameEvent m_NewFrameEvent;
	XN_CRITICAL_SECTION_HANDLE m_hLock;
	XnBuffer m_writeBuffer;
};

#endif //__XN_MULTI_FRAME_BUFFER_H__
