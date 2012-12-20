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
#include "XnStreamingStream.h"
#include <XnOS.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnStreamingStream::XnStreamingStream(const XnChar* csType, const XnChar* csName) :
	XnDeviceStream(csType, csName),
	m_IsStreamingStream(XN_STREAM_PROPERTY_IS_STREAMING, "IsStreaming", TRUE),
	m_ReadChunkSize(XN_STREAM_PROPERTY_READ_CHUNK_SIZE, "ReadChunkSize")
{
}

XnStatus XnStreamingStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// init base
	nRetVal = XnDeviceStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	m_ReadChunkSize.UpdateSetCallback(SetReadChunkSizeCallback, this);

	XN_VALIDATE_ADD_PROPERTIES(this, &m_IsStreamingStream, &m_ReadChunkSize);

	return (XN_STATUS_OK);
}

XnStatus XnStreamingStream::SetReadChunkSize(XnUInt32 nChunkSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_ReadChunkSize.UnsafeUpdateValue(nChunkSize);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XN_CALLBACK_TYPE XnStreamingStream::SetReadChunkSizeCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnStreamingStream* pStream = (XnStreamingStream*)pCookie;
	return pStream->SetReadChunkSize((XnUInt32)nValue);
}
