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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnFrameBufferManager.h"
#include <XnLog.h>
#include <XnDDK.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnFrameBufferManager::XnFrameBufferManager() :
	m_pBufferPool(NULL),
	m_pWorkingBuffer(NULL),
	m_nStableFrameID(0),
	m_hLock(NULL)
{
}

XnFrameBufferManager::~XnFrameBufferManager()
{
	Free();
}

XnStatus XnFrameBufferManager::Init(XnUInt32 nBufferSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_NEW(m_pBufferPool, XnOniFramePool);
	m_pBufferPool->SetFrameSize(nBufferSize);
	int numFrames = 6; // user, synced frame holder (last+synced), XnSensor frame-sync(last+incoming), working
	if (!m_pBufferPool->Initialize(numFrames))
	{
		return XN_STATUS_ALLOC_FAILED;
	}
	
	nRetVal = xnOSCreateCriticalSection(&m_hLock);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = Reallocate(nBufferSize);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

void XnFrameBufferManager::Free()
{
	if (m_hLock != NULL)
	{
		xnOSCloseCriticalSection(&m_hLock);
		m_hLock = NULL;
	}

	// Release the working buffer.
	if (m_pWorkingBuffer != NULL)
	{
		m_pBufferPool->DecRef(m_pWorkingBuffer);
		m_pWorkingBuffer = NULL;
	}

	// Delete the buffer pool.
	if (m_pBufferPool != NULL)
	{
		XN_DELETE(m_pBufferPool);
		m_pBufferPool = NULL;
	}
}

XnStatus XnFrameBufferManager::Reallocate(XnUInt32 nBufferSize)
{
	xnOSEnterCriticalSection(&m_hLock);

	// release current ones
	if (m_pWorkingBuffer != NULL)
	{
		m_pBufferPool->DecRef(m_pWorkingBuffer);
		m_pWorkingBuffer = NULL;
	}

	// Change the buffer size.
	m_pBufferPool->SetFrameSize(nBufferSize);

	// TODO: validate all is OK
	/*if (nRetVal != XN_STATUS_OK)
	{
		xnOSLeaveCriticalSection(&m_hLock);
		return (nRetVal);
	}*/

	// and take one
	if (nBufferSize == 0)
	{
		m_pWorkingBuffer = NULL;
	}
	else
	{
		// take working buffer
		m_pWorkingBuffer = m_pBufferPool->Acquire();
		if (m_pWorkingBuffer == NULL)
		{
			XN_ASSERT(FALSE);
			return XN_STATUS_ERROR;
		}
		m_writeBuffer.SetExternalBuffer((XnUChar*)m_pWorkingBuffer->data, nBufferSize);
	}

	xnOSLeaveCriticalSection(&m_hLock);

	return (XN_STATUS_OK);
}

void XnFrameBufferManager::MarkWriteBufferAsStable(XnUInt32* pnFrameID)
{
	xnOSEnterCriticalSection(&m_hLock);

	OniFrame* pStableBuffer = m_pWorkingBuffer;
	pStableBuffer->dataSize = m_writeBuffer.GetSize();

	// lock buffer pool (for rollback option)
	m_pBufferPool->Lock();

	// mark working as stable
	m_nStableFrameID++;
	*pnFrameID = m_nStableFrameID;
	pStableBuffer->frameIndex = m_nStableFrameID;

	// take a new working buffer
	m_pWorkingBuffer = m_pBufferPool->Acquire();
	if (m_pWorkingBuffer == NULL)
	{
		xnLogError(XN_MASK_DDK, "Failed to get new working buffer!");

		// we'll return back to our old working one
		m_pWorkingBuffer = pStableBuffer;
		m_pWorkingBuffer->dataSize = 0;

		m_pBufferPool->Unlock();

		XN_ASSERT(FALSE);
		return;
	}

	m_writeBuffer.SetExternalBuffer((XnUChar*)m_pWorkingBuffer->data, m_pBufferPool->GetFrameSize());
	
	m_pBufferPool->Unlock();
	xnOSLeaveCriticalSection(&m_hLock);

	// reset new working
	m_pWorkingBuffer->dataSize = 0;

	// notify stream that new data is available
	NewFrameEventArgs args;
	args.pFrame = pStableBuffer;
	m_NewFrameEvent.Raise(args);
}

void XnFrameBufferManager::AddRefToFrame(OniFrame* pFrame)
{
	m_pBufferPool->AddRef(pFrame);
}

void XnFrameBufferManager::ReleaseFrame(OniFrame* pFrame)
{
	m_pBufferPool->Release(pFrame);
}

