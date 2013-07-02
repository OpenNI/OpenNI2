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
	m_pServices(NULL),
	m_pWorkingBuffer(NULL),
	m_nStableFrameID(0),
	m_newFrameCallback(NULL),
	m_newFrameCallbackCookie(NULL),
	m_hLock(NULL)
{
}

XnFrameBufferManager::~XnFrameBufferManager()
{
	Free();
}

void XnFrameBufferManager::SetNewFrameCallback(NewFrameCallback func, void* pCookie)
{
	m_newFrameCallback = func;
	m_newFrameCallbackCookie = pCookie;
}

XnStatus XnFrameBufferManager::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = xnOSCreateCriticalSection(&m_hLock);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

void XnFrameBufferManager::Free()
{
	Stop();

	if (m_hLock != NULL)
	{
		xnOSCloseCriticalSection(&m_hLock);
		m_hLock = NULL;
	}
}

XnStatus XnFrameBufferManager::Start(oni::driver::StreamServices& services)
{
	m_pServices = &services;

	// take working buffer
	m_pWorkingBuffer = m_pServices->acquireFrame();
	if (m_pWorkingBuffer == NULL)
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	m_writeBuffer.SetExternalBuffer((XnUChar*)m_pWorkingBuffer->data, m_pWorkingBuffer->dataSize);

	return (XN_STATUS_OK);
}

void XnFrameBufferManager::Stop()
{
	if (m_pWorkingBuffer != NULL)
	{
		m_pServices->releaseFrame(m_pWorkingBuffer);
		m_pWorkingBuffer = NULL;
	}

	m_pServices = NULL;
}

void XnFrameBufferManager::MarkWriteBufferAsStable(XnUInt32* pnFrameID)
{
	xnOSEnterCriticalSection(&m_hLock);

	OniFrame* pStableBuffer = m_pWorkingBuffer;
	pStableBuffer->dataSize = m_writeBuffer.GetSize();

	// mark working as stable
	m_nStableFrameID++;
	*pnFrameID = m_nStableFrameID;
	pStableBuffer->frameIndex = m_nStableFrameID;

	// take a new working buffer
	m_pWorkingBuffer = m_pServices->acquireFrame();
	if (m_pWorkingBuffer == NULL)
	{
		xnLogError(XN_MASK_DDK, "Failed to get new working buffer!");

		// we'll return back to our old working one
		m_pWorkingBuffer = pStableBuffer;
		m_pWorkingBuffer->dataSize = 0;

		XN_ASSERT(FALSE);
		return;
	}

	m_writeBuffer.SetExternalBuffer((XnUChar*)m_pWorkingBuffer->data, m_pWorkingBuffer->dataSize);
	
	xnOSLeaveCriticalSection(&m_hLock);

	// reset new working
	m_pWorkingBuffer->dataSize = 0;

	// notify stream that new data is available
	if (m_newFrameCallback != NULL)
	{
		m_newFrameCallback(pStableBuffer, m_newFrameCallbackCookie);
	}

	// and release our reference
	m_pServices->releaseFrame(pStableBuffer);
}

