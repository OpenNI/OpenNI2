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
#include "XnDataProcessorHolder.h"
#include <XnOS.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnDataProcessorHolder::XnDataProcessorHolder() :
	m_hLock(NULL),
	m_pProcessor(NULL)
{
}

XnDataProcessorHolder::~XnDataProcessorHolder()
{
	xnOSCloseCriticalSection(&m_hLock);
	XN_DELETE(m_pProcessor);
}

XnStatus XnDataProcessorHolder::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = xnOSCreateCriticalSection(&m_hLock);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

void XnDataProcessorHolder::Lock()
{
	xnOSEnterCriticalSection(&m_hLock);
}

void XnDataProcessorHolder::Unlock()
{
	xnOSLeaveCriticalSection(&m_hLock);
}

void XnDataProcessorHolder::Replace(XnDataProcessor* pNew)
{
	// lock first, to make sure processor is not in use right now
	Lock();

	if (m_pProcessor != NULL)
	{
		XN_DELETE(m_pProcessor);
	}

	m_pProcessor = pNew;

	Unlock();
}

void XnDataProcessorHolder::ProcessData(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize)
{
	if (m_pProcessor == NULL)
		return;

	// lock first
	Lock();

	// check again (it could have been replaced while we waited to lock)
	if (m_pProcessor != NULL)
	{
		m_pProcessor->ProcessData(pHeader, pData, nDataOffset, nDataSize);
	}

	Unlock();
}

