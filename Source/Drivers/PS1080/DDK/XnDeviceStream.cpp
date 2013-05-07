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
#include "XnDeviceStream.h"
#include <XnOS.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnDeviceStream::XnDeviceStream(const XnChar* csType, const XnChar* csName) :
	XnDeviceModule(csName),
	m_pServices(NULL),
	m_IsStream(XN_STREAM_PROPERTY_IS_STREAM, "IsStream", TRUE),
	m_Type(XN_STREAM_PROPERTY_TYPE, "Type", csType),
	m_IsOpen(XN_STREAM_PROPERTY_STATE, "State", FALSE),
	m_RequiredSize(XN_STREAM_PROPERTY_REQUIRED_DATA_SIZE, "RequiredDataSize"),
	m_OutputFormat(XN_STREAM_PROPERTY_OUTPUT_FORMAT, "OutputFormat"),
	m_IsMirrored(XN_MODULE_PROPERTY_MIRROR, "Mirror"),
	m_pNewDataCallback(NULL),
	m_nRefCount(1),
	m_nOpenRefCount(0),
	m_hCriticalSection(NULL),
	m_hOpenLock(NULL)
{
}

XnStatus XnDeviceStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// init module
	nRetVal = XnDeviceModule::Init();
	XN_IS_STATUS_OK(nRetVal);

	// cs
	nRetVal = xnOSCreateCriticalSection(&m_hCriticalSection);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSCreateCriticalSection(&m_hOpenLock);
	XN_IS_STATUS_OK(nRetVal);

	m_IsOpen.UpdateSetCallback(SetIsOpenCallback, this);
	m_OutputFormat.UpdateSetCallback(SetOutputFormatCallback, this);
	m_IsMirrored.UpdateSetCallback(SetIsMirrorCallback, this);

	XN_VALIDATE_ADD_PROPERTIES(this, &m_IsStream, &m_Type, &m_IsOpen, &m_OutputFormat, &m_RequiredSize, &m_IsMirrored);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceStream::Free()
{
	xnOSCloseCriticalSection(&m_hCriticalSection);
	xnOSCloseCriticalSection(&m_hOpenLock);
	XnDeviceModule::Free();

	return (XN_STATUS_OK);
}

XnStatus XnDeviceStream::Open()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_IsOpen.UnsafeUpdateValue(TRUE);
	XN_IS_STATUS_OK(nRetVal);

	m_nOpenRefCount = 1;
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceStream::Close()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_IsOpen.UnsafeUpdateValue(FALSE);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceStream::SetOutputFormat(OniPixelFormat nOutputFormat)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_OutputFormat.UnsafeUpdateValue(nOutputFormat);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceStream::SetMirror(XnBool bIsMirrored)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_IsMirrored.UnsafeUpdateValue(bIsMirrored);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

void XnDeviceStream::NewDataAvailable(OniFrame* pFrame)
{
	xnOSEnterCriticalSection(&m_hCriticalSection);
	XnBool bMirror = IsMirrored();
	xnOSLeaveCriticalSection(&m_hCriticalSection);

	// mirror it if needed
	if (bMirror)
	{
		Mirror(pFrame);
	}

	// make sure someone is listening (otherwise, no one will ever free this frame!)
	XN_ASSERT(m_pNewDataCallback != NULL);
	m_pNewDataCallback(this, pFrame, m_pNewDataCallbackCookie);
}

void XnDeviceStream::SetNewDataCallback(NewDataCallbackPtr pFunc, void* pCookie)
{
	m_pNewDataCallback = pFunc;
	m_pNewDataCallbackCookie = pCookie;
}

XnStatus XnDeviceStream::RegisterRequiredSizeProperty(XnProperty* pProperty)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// make sure the property belongs to this module (because we never unregister from it)
	XN_ASSERT(strcmp(pProperty->GetModule(), GetName()) == 0);

	XnCallbackHandle hCallbackDummy;
	nRetVal = pProperty->OnChangeEvent().Register(UpdateRequiredSizeCallback, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	// recalculate it
	nRetVal = UpdateRequiredSize();
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceStream::UpdateRequiredSize()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnUInt32 nRequiredSize;
	nRetVal = CalcRequiredSize(&nRequiredSize);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_RequiredSize.UnsafeUpdateValue(nRequiredSize);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XN_CALLBACK_TYPE XnDeviceStream::UpdateRequiredSizeCallback(const XnProperty* /*pSenser*/, void* pCookie)
{
	XnDeviceStream* pStream = (XnDeviceStream*)pCookie;
	return pStream->UpdateRequiredSize();
}

XnStatus XN_CALLBACK_TYPE XnDeviceStream::SetIsOpenCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnDeviceStream* pStream = (XnDeviceStream*)pCookie;
	if (nValue == TRUE)
	{
		return pStream->Open();
	}
	else
	{
		return pStream->Close();
	}
}

XnStatus XN_CALLBACK_TYPE XnDeviceStream::SetOutputFormatCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnDeviceStream* pStream = (XnDeviceStream*)pCookie;
	return pStream->SetOutputFormat((OniPixelFormat)nValue);
}

XnStatus XN_CALLBACK_TYPE XnDeviceStream::SetIsMirrorCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnDeviceStream* pStream = (XnDeviceStream*)pCookie;
	return pStream->SetMirror((XnBool)nValue);
}

void XnDeviceStream::AddRef()
{
	xnl::AutoCSLocker lock(m_hCriticalSection);
	++m_nRefCount;
}

XnUInt32 XnDeviceStream::DecRef()
{
	xnl::AutoCSLocker lock(m_hCriticalSection);
	return --m_nRefCount;
}

void XnDeviceStream::OpenAddRef()
{
	xnl::AutoCSLocker lock(m_hOpenLock);
	++m_nOpenRefCount;
}

XnUInt32 XnDeviceStream::OpenDecRef()
{
	xnl::AutoCSLocker lock(m_hOpenLock);
	return --m_nOpenRefCount;
}
