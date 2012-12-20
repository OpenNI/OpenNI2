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
#ifndef _ONI_IMPL_FRAME_POOL_H_
#define _ONI_IMPL_FRAME_POOL_H_

#include "XnPool.h"
#include "Driver/OniDriverAPI.h"

typedef struct
{
	// The original OniFrame structure.
	OniDriverFrame frame;
	
	/* Internal */

	// Frame awaiting resize.
	OniBool resize;

	// Cookie
	void* cookie;

} OniFrameInPool;

class XnOniFramePool : public xnl::Pool<OniFrame>
{
public:
	XnOniFramePool() : Pool(true), m_frameSize(0)
	{
	}

	~XnOniFramePool()
	{
		Clear();
	}

	using xnl::Pool<OniFrame>::Initialize;

	virtual XnStatus Release(OniFrame* pFrame)
	{
		Lock();

		// Try to release the frame.
		XnStatus nRetVal = Pool::Release(pFrame);
		if (nRetVal == XN_STATUS_OK)
		{
			// Check if frame requires resize.
			OniFrameInPool* pFrameInPool = (OniFrameInPool*)pFrame;
			if (pFrameInPool->resize)
			{
				pFrameInPool->resize = FALSE;
				DestroyData(pFrameInPool);
				CreateData(pFrameInPool);
			}
		}

		Unlock();

		return nRetVal;
	}

	OniFrame* Create()
	{
		// Create the OniFrame in the pool (contains the OniFrame and internal data).
		OniFrameInPool* pFrame = XN_NEW(OniFrameInPool);
		XN_VALIDATE_PTR(pFrame, NULL);

		// Allocate the data.
		CreateData(pFrame);

		// Set internal fields.
		pFrame->resize = FALSE;
		pFrame->cookie = NULL;

		return (OniFrame*)pFrame;
	}

	void Destroy(OniFrame* pFrame)
	{
		DestroyData((OniFrameInPool*)pFrame);
		XN_DELETE(pFrame);
	}

	int GetFrameSize() { return m_frameSize; }

	void SetFrameSize(int frameSize)
	{
		if (m_frameSize == frameSize)
			return;

		Lock();

		// Store the new frame size.
		m_frameSize = frameSize;

		// Parse all the available frames and resize them.
		for (xnl::List<OniFrame*>::Iterator availableIterator = m_available.Begin(); 
			 availableIterator != m_available.End(); ++availableIterator)
		{
			OniFrameInPool* pFrame = (OniFrameInPool*)(*availableIterator);
			DestroyData(pFrame);
			CreateData(pFrame);
		}

		for (xnl::Hash<OniFrame*, int>::Iterator usedIterator = m_used.Begin();
			 usedIterator != m_used.End(); ++usedIterator)
		{
			OniFrameInPool* pFrame = (OniFrameInPool*)((*usedIterator).Key());
			pFrame->resize = TRUE;
		}

		Unlock();
	}

	void* GetCookie(OniFrame* pFrame)
	{
		return ((OniFrameInPool*)pFrame)->cookie;
	}

	void SetCookie(OniFrame* pFrame, void* cookie)
	{
		((OniFrameInPool*)pFrame)->cookie = cookie;
	}

protected:

	inline void CreateData(OniFrameInPool* pFrame)
	{
		// Create buffer used by frame.
		pFrame->frame.frame.dataSize = m_frameSize;
		pFrame->frame.frame.data = xnOSCallocAligned(m_frameSize, 1, XN_DEFAULT_MEM_ALIGN);
	}

	inline void DestroyData(OniFrameInPool* pFrame)
	{
		// Destroy buffer used by frame.
		xnOSFreeAligned(pFrame->frame.frame.data);
		pFrame->frame.frame.data = NULL;
		pFrame->frame.frame.dataSize = 0;
	}

private:
	int m_frameSize;
};

#endif // ONI_IMPL_FRAME_POOL_H_
