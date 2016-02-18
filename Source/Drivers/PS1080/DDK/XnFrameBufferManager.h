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
#ifndef XNFRAMEBUFFERMANAGER_H
#define XNFRAMEBUFFERMANAGER_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <XnEvent.h>
#include <Core/XnBuffer.h>
#include <Driver/OniDriverAPI.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

class XnFrameBufferManager
{
public:
	typedef void (XN_CALLBACK_TYPE* NewFrameCallback)(OniFrame* pFrame, void* pCookie);

	XnFrameBufferManager();
	~XnFrameBufferManager();

	void SetNewFrameCallback(NewFrameCallback func, void* pCookie);

	XnStatus Init();
	void Free();

	XnStatus Start(oni::driver::StreamServices& services);
	void Stop();

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

	void MarkWriteBufferAsStable(XnUInt32* pnFrameID);

	inline XnUInt32 GetLastFrameID() const { return m_nStableFrameID; }

private:
	XN_DISABLE_COPY_AND_ASSIGN(XnFrameBufferManager);

	oni::driver::StreamServices* m_pServices;
	OniFrame* m_pWorkingBuffer;
	XnUInt32 m_nStableFrameID;
	NewFrameCallback m_newFrameCallback;
	void* m_newFrameCallbackCookie;
	XN_CRITICAL_SECTION_HANDLE m_hLock;
	XnBuffer m_writeBuffer;
};

#endif // XNFRAMEBUFFERMANAGER_H
