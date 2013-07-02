#ifndef _ONI_FRAME_MANAGER_H_
#define _ONI_FRAME_MANAGER_H_

#include <OniCTypes.h>
#include "OniCommon.h"
#include <XnOS.h>
#include <XnPool.h>

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

struct OniFrameInternal;
typedef void (ONI_CALLBACK_TYPE* BackToPoolFuncPtr)(struct OniFrameInternal* pBuffer, void* pCookie);
typedef void (ONI_CALLBACK_TYPE* FreeBufferFuncPtr)(void* data, void* pCookie);

struct OniFrameInternal : public OniFrame
{
	int refCount;
	BackToPoolFuncPtr backToPoolFunc; // callback function to be called when frame reached zero refs and returned to pool
	void* backToPoolFuncCookie;
	FreeBufferFuncPtr freeBufferFunc; // callback function for freeing the frame buffer
	void* freeBufferFuncCookie;
};

class FrameManager
{
public:
	FrameManager();
	~FrameManager();

	OniFrameInternal* acquireFrame();
	void addRef(OniFrame* pFrame);
	void release(OniFrame* pFrame);

private:
	xnl::Pool<OniFrameInternal> m_frames;
};

ONI_NAMESPACE_IMPLEMENTATION_END

#endif //_ONI_FRAME_MANAGER_H_