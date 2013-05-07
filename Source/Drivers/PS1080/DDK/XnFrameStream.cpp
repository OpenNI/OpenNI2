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

	nRetVal = m_bufferManager.Init();
	XN_IS_STATUS_OK(nRetVal);

	// register for new data events
	m_bufferManager.SetNewFrameCallback(OnTripleBufferNewData, this);

	XN_VALIDATE_ADD_PROPERTIES(this, &m_IsFrameStream, &m_FPS );

	return (XN_STATUS_OK);
}

XnStatus XnFrameStream::StartBufferManager(XnFrameBufferManager** pBufferManager)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_bufferManager.Start(GetServices());
	XN_IS_STATUS_OK(nRetVal);

	*pBufferManager = &m_bufferManager;

	return (XN_STATUS_OK);
}

XnStatus XnFrameStream::Free()
{
	m_bufferManager.Free();
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

XnStatus XN_CALLBACK_TYPE XnFrameStream::SetFPSCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnFrameStream* pThis = (XnFrameStream*)pCookie;
	return pThis->SetFPS((XnUInt32)nValue);
}

void XN_CALLBACK_TYPE XnFrameStream::OnTripleBufferNewData(OniFrame* pFrame, void* pCookie)
{
	XnFrameStream* pThis = (XnFrameStream*)pCookie;
	pThis->NewDataAvailable(pFrame);
}

XnStatus XnFrameStream::Close()
{
	m_bufferManager.Stop();

	return XnDeviceStream::Close();
}

