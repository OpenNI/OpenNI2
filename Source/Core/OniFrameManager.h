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
#ifndef ONIFRAMEMANAGER_H
#define ONIFRAMEMANAGER_H

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

#endif // ONIFRAMEMANAGER_H
