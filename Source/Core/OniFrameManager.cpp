#include "OniFrameManager.h"
#include <XnOSCpp.h>

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

FrameManager::FrameManager()
{
}

FrameManager::~FrameManager()
{
}

OniFrameInternal* FrameManager::acquireFrame()
{
	OniFrameInternal* pFrame = m_frames.Acquire();
    xnOSMemSet(pFrame, 0, sizeof(OniFrameInternal));
	pFrame->refCount = 1;
	return pFrame;
}

void FrameManager::addRef(OniFrame* pFrame)
{
	OniFrameInternal* pInternal = (OniFrameInternal*)pFrame;
	m_frames.Lock();
	++pInternal->refCount;
	m_frames.Unlock();
}

void FrameManager::release(OniFrame* pFrame)
{
	OniFrameInternal* pInternal = (OniFrameInternal*)pFrame;
	m_frames.Lock();
	if (--pInternal->refCount == 0)
	{
		// notify frame is back to pool
        if (pInternal->backToPoolFunc != NULL)
        {
            pInternal->backToPoolFunc(pInternal, pInternal->backToPoolFuncCookie);
        }
        
		// and return frame to pool
		m_frames.Release(pInternal);
	}
	m_frames.Unlock();
}

ONI_NAMESPACE_IMPLEMENTATION_END
