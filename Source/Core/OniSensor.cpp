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
}

Sensor::~Sensor()
{
	releaseAllFrames();
}

void Sensor::setDriverStream(void* streamHandle)
{
	m_streamHandle = streamHandle;
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

void ONI_CALLBACK_TYPE Sensor::newFrameCallback(void* /*streamHandle*/, OniFrame* pFrame, void* pCookie)
{
	Sensor* pThis = (Sensor*)pCookie;
	pThis->m_newFrameEvent.Raise(pFrame);
}

ONI_NAMESPACE_IMPLEMENTATION_END