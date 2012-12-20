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
#include "XnFrameStream.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnFrameStream::XnFrameStream(const XnChar* csType, const XnChar* csName) :
	XnDeviceStream(csType, csName),
	m_pBufferManager(NULL),
	m_nLastReadFrame(0),
	m_IsFrameStream(XN_STREAM_PROPERTY_IS_FRAME_BASED, "IsFrameBased", TRUE),
	m_FPS(XN_STREAM_PROPERTY_FPS, "FPS", 0)
{
	m_FPS.UpdateSetCallback(SetFPSCallback, this);
}

XnStatus XnFrameStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// init base
	nRetVal = XnDeviceStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_ADD_PROPERTIES(this, &m_IsFrameStream, &m_FPS );

	XnCallbackHandle hDummy;

	// be notified when required size changes
	nRetVal = RequiredSizeProperty().OnChangeEvent().Register(RequiredSizeChangedCallback, this, hDummy);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnFrameStream::GetTripleBuffer(XnFrameBufferManager** pBufferManager)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// lazy initialization (this allows us to set buffer pool after initialization of the stream
	// and before data actually arrives (or stream data is allocated)
	if (m_pBufferManager == NULL)
	{
		// allocate buffer manager
		XN_VALIDATE_NEW(m_pBufferManager, XnFrameBufferManager);

		nRetVal = m_pBufferManager->Init(GetRequiredDataSize());
		XN_IS_STATUS_OK(nRetVal);

		// register for new data events
		XnCallbackHandle hDummy;
		nRetVal = m_pBufferManager->OnNewFrameEvent().Register(OnTripleBufferNewData, this, hDummy);
		XN_IS_STATUS_OK(nRetVal);
	}

	*pBufferManager = m_pBufferManager;

	return (XN_STATUS_OK);
}

XnStatus XnFrameStream::Free()
{
	if (m_pBufferManager != NULL)
	{
		XN_DELETE(m_pBufferManager);
		m_pBufferManager = NULL;
	}

	XnDeviceStream::Free();
	return (XN_STATUS_OK);
}

XnStatus XnFrameStream::SetFPS(XnUInt32 nFPS)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_FPS.UnsafeUpdateValue(nFPS);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnFrameStream::OnRequiredSizeChanging()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = ReallocTripleFrameBuffer();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);	
}

XnStatus XnFrameStream::ReallocTripleFrameBuffer()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_pBufferManager != NULL)
	{
		nRetVal = m_pBufferManager->Reallocate(GetRequiredDataSize());
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

void XnFrameStream::AddRefToFrame(OniFrame* pFrame)
{
	m_pBufferManager->AddRefToFrame(pFrame);
}

void XnFrameStream::ReleaseFrame(OniFrame* pFrame)
{
	m_pBufferManager->ReleaseFrame(pFrame);
}

XnStatus XN_CALLBACK_TYPE XnFrameStream::SetFPSCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnFrameStream* pThis = (XnFrameStream*)pCookie;
	return pThis->SetFPS((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnFrameStream::RequiredSizeChangedCallback(const XnProperty* /*pSender*/, void* pCookie)
{
	XnFrameStream* pThis = (XnFrameStream*)pCookie;
	return pThis->OnRequiredSizeChanging();
}

void XN_CALLBACK_TYPE XnFrameStream::OnTripleBufferNewData(const XnFrameBufferManager::NewFrameEventArgs& args, void* pCookie)
{
	XnFrameStream* pThis = (XnFrameStream*)pCookie;
	pThis->NewDataAvailable(args.pFrame);
}

