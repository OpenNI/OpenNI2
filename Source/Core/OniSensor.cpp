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
#include "OniSensor.h"
#include <OniCAPI.h>

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

Sensor::Sensor(xnl::ErrorLogger& errorLogger, FrameManager& frameManager, const DriverHandler& driverHandler) : 
	m_streamCount(0),
	m_startedStreamCount(0),
	m_errorLogger(errorLogger),
	m_frameManager(frameManager),
	m_driverHandler(driverHandler),
	m_streamHandle(NULL),
	m_requiredFrameSize(0)
{
	resetFrameAllocator();

	OniStreamServices::streamServices = this;
	OniStreamServices::getDefaultRequiredFrameSize = getDefaultRequiredFrameSizeCallback;
	OniStreamServices::acquireFrame = acquireFrameCallback;
	OniStreamServices::addFrameRef = addFrameRefCallback;
	OniStreamServices::releaseFrame = releaseFrameCallback;
}

Sensor::~Sensor()
{
	releaseAllFrames();
}

void Sensor::setDriverStream(void* streamHandle)
{
	m_streamHandle = streamHandle;
	m_driverHandler.streamSetServices(m_streamHandle, this);
	m_driverHandler.streamSetNewFrameCallback(m_streamHandle, newFrameCallback, this);
}

OniStatus Sensor::setFrameBufferAllocator(OniFrameAllocBufferCallback alloc, OniFrameFreeBufferCallback free, void* pCookie)
{
	xnl::AutoCSLocker lock(m_refCountCS);
	if (m_startedStreamCount > 0)
	{
		m_errorLogger.Append("Cannot set frame buffer allocator while stream is running");
		return ONI_STATUS_OUT_OF_FLOW;
	}

	if (alloc == NULL && free == NULL)
	{
		resetFrameAllocator();
	}
	else if (alloc == NULL || free == NULL)
	{
		m_errorLogger.Append("Cannot set only alloc or only free function. Both must be supplied.");
		return ONI_STATUS_BAD_PARAMETER;
	}
	else
	{
		m_allocFrameBufferCallback = alloc;
		m_freeFrameBufferCallback = free;
		m_frameBufferAllocatorCookie = pCookie;
	}

	return ONI_STATUS_OK;
}

void Sensor::setRequiredFrameSize(int requiredFrameSize)
{
	if (m_requiredFrameSize != requiredFrameSize)
	{
		// release all previous frames. They can't be used anymore
		releaseAllFrames();
	}

	m_requiredFrameSize = requiredFrameSize;
}

void Sensor::resetFrameAllocator()
{
	m_allocFrameBufferCallback = allocFrameBufferFromPoolCallback;
	m_freeFrameBufferCallback = releaseFrameBufferToPoolCallback;
	m_frameBufferAllocatorCookie = this;
}

OniFrame* Sensor::acquireFrame()
{
	OniFrameInternal* pResult = m_frameManager.acquireFrame();
	if (pResult == NULL)
	{
		return NULL;
	}

	pResult->data = m_allocFrameBufferCallback(m_requiredFrameSize, m_frameBufferAllocatorCookie);
	if (pResult->data == NULL)
	{
		m_frameManager.release(pResult);
		return NULL;
	}

	pResult->dataSize = m_requiredFrameSize;
	pResult->backToPoolFunc = frameBackToPoolCallback;
	pResult->backToPoolFuncCookie = this;
	pResult->freeBufferFunc = m_freeFrameBufferCallback;
	pResult->freeBufferFuncCookie = m_frameBufferAllocatorCookie;

	xnl::AutoCSLocker lock(m_framesCS);
	m_currentStreamFrames.AddLast(pResult);

	return pResult;
}

void* Sensor::allocFrameBufferFromPool(int size)
{
	XN_ASSERT(size == m_requiredFrameSize);
	void* pResult = NULL;
	xnl::AutoCSLocker lock(m_framesCS);
	if (m_availableFrameBuffers.IsEmpty())
	{
		// create a new one
		pResult = xnOSMallocAligned(size, XN_DEFAULT_MEM_ALIGN);
		m_allFrameBuffers.AddLast(pResult);
	}
	else
	{
		xnl::List<void*>::Iterator it = m_availableFrameBuffers.Begin();
		pResult = *it;
		m_availableFrameBuffers.Remove(it);
	}
	return pResult;
}

void Sensor::releaseFrameBufferToPool(void* pBuffer)
{
	xnl::AutoCSLocker lock(m_framesCS);
	m_availableFrameBuffers.AddLast(pBuffer);
}

void* ONI_CALLBACK_TYPE Sensor::allocFrameBufferFromPoolCallback(int size, void* pCookie)
{
	Sensor* pThis = (Sensor*)pCookie;
	return pThis->allocFrameBufferFromPool(size);
}

void ONI_CALLBACK_TYPE Sensor::releaseFrameBufferToPoolCallback(void* pBuffer, void* pCookie)
{
	Sensor* pThis = (Sensor*)pCookie;
	pThis->releaseFrameBufferToPool(pBuffer);
}

void ONI_CALLBACK_TYPE Sensor::freeFrameBufferMemoryCallback(void* pBuffer, void* /*pCookie*/)
{
	xnOSFreeAligned(pBuffer);
}

void Sensor::releaseAllFrames()
{
	xnl::AutoCSLocker lock(m_framesCS);
	// change release method of current frames
	for (xnl::List<OniFrameInternal*>::Iterator it = m_currentStreamFrames.Begin(); it != m_currentStreamFrames.End(); ++it)
	{
		// don't return frame buffer to pool, instead just free it
		if ((*it)->freeBufferFunc == releaseFrameBufferToPoolCallback)
		{
			(*it)->freeBufferFunc = freeFrameBufferMemoryCallback;
		}

		// mark that this frame does not belong to this stream anymore
		(*it)->backToPoolFuncCookie = NULL;
	}

	m_currentStreamFrames.Clear();

	// delete all available frames
	for (xnl::List<void*>::Iterator it = m_availableFrameBuffers.Begin(); it != m_availableFrameBuffers.End(); ++it)
	{
		xnOSFreeAligned(*it);
	}
	m_availableFrameBuffers.Clear();
}

void ONI_CALLBACK_TYPE Sensor::frameBackToPoolCallback(OniFrameInternal* pFrame, void* pCookie)
{
	// release the data
	if (pFrame->data != NULL)
	{
		// this can happen if allocation of data failed
		pFrame->freeBufferFunc(pFrame->data, pFrame->freeBufferFuncCookie);
		pFrame->data = NULL;
	}

	if (pCookie != NULL)
	{
		Sensor* pThis = (Sensor*)pCookie;
		xnl::AutoCSLocker lock(pThis->m_framesCS);
		pThis->m_currentStreamFrames.Remove(pFrame);
	}
}

int Sensor::getDefaultRequiredFrameSize()
{
	OniStatus nRetVal = ONI_STATUS_OK;

	OniVideoMode videoMode;
	int size = sizeof(videoMode);
	nRetVal = m_driverHandler.streamGetProperty(m_streamHandle, ONI_STREAM_PROPERTY_VIDEO_MODE, &videoMode, &size);
	XN_ASSERT(nRetVal == ONI_STATUS_OK);

	int stride;
	size = sizeof(stride);
	nRetVal = m_driverHandler.streamGetProperty(m_streamHandle, ONI_STREAM_PROPERTY_STRIDE, &stride, &size);
	if (nRetVal != ONI_STATUS_OK)
	{
		stride = videoMode.resolutionX * oniFormatBytesPerPixel(videoMode.pixelFormat);
	}

	return stride * videoMode.resolutionY;
}

void ONI_CALLBACK_TYPE Sensor::newFrameCallback(void* /*streamHandle*/, OniFrame* pFrame, void* pCookie)
{
	Sensor* pThis = (Sensor*)pCookie;
	pThis->m_newFrameEvent.Raise(pFrame);
}

int ONI_CALLBACK_TYPE Sensor::getDefaultRequiredFrameSizeCallback(void* streamServices)
{
	Sensor* pThis = (Sensor*)streamServices;
	return pThis->getDefaultRequiredFrameSize();
}

OniFrame* ONI_CALLBACK_TYPE Sensor::acquireFrameCallback(void* streamServices)
{
	Sensor* pThis = (Sensor*)streamServices;
	return pThis->acquireFrame();
}

void ONI_CALLBACK_TYPE Sensor::addFrameRefCallback(void* streamServices, OniFrame* pFrame)
{
	Sensor* pThis = (Sensor*)streamServices;
	return pThis->m_frameManager.addRef(pFrame);
}

void ONI_CALLBACK_TYPE Sensor::releaseFrameCallback(void* streamServices, OniFrame* pFrame)
{
	Sensor* pThis = (Sensor*)streamServices;
	return pThis->m_frameManager.release(pFrame);
}

ONI_NAMESPACE_IMPLEMENTATION_END