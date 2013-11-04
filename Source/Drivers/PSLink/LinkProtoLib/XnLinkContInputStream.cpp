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
#include "XnLinkContInputStream.h"
#include "XnLinkProtoUtils.h"
#include "XnLinkProtoLibDefs.h"
#include "XnCyclicBuffer.h"
#include "XnLinkControlEndpoint.h"
#include <XnLog.h>
#include <XnEvent.h>

#define XN_MASK_INPUT_STREAM "xnInputStream"

namespace xn
{


const XnUInt32 LinkContInputStream::CONT_STREAM_PREDEFINED_BUFFER_SIZE = 0x40000;

LinkContInputStream::LinkContInputStream()
{
	m_bInitialized = FALSE;
    m_bStreaming = FALSE;
	m_bNewDataAvailable = FALSE;
	m_hCriticalSection = NULL;
	m_nUserBufferMaxSize = 0;
	m_pUserBuffer = NULL;
	m_nUserBufferCurrentSize = 0;
	xnOSCreateCriticalSection(&m_hCriticalSection);
	m_pDumpFile = NULL;
	xnOSMemSet(m_strDumpName, 0, sizeof(m_strDumpName));
}

LinkContInputStream::~LinkContInputStream()
{
	Shutdown();
	xnOSCloseCriticalSection(&m_hCriticalSection);
}

XnStatus LinkContInputStream::Init(LinkControlEndpoint* pLinkControlEndpoint,
                                   XnStreamType streamType,
                                   XnUInt16 nStreamID, 
                                   IConnection* pConnection)
{
	XnStatus nRetVal = XN_STATUS_OK;
	if (m_hCriticalSection == NULL)
	{
		xnLogError(XN_MASK_INPUT_STREAM, "Cannot initialize - critical section was not created successfully");
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	xnl::AutoCSLocker csLock(m_hCriticalSection);
	if (m_bInitialized)
	{
		//We shutdown first so we can re-initialize.
		Shutdown();
	}

    nRetVal = LinkInputStream::Init(pLinkControlEndpoint, streamType, nStreamID, pConnection);
    XN_IS_STATUS_OK_LOG_ERROR("Init base input stream", nRetVal);

	m_nStreamID = nStreamID;
	m_nUserBufferMaxSize = CONT_STREAM_PREDEFINED_BUFFER_SIZE;
	m_nUserBufferCurrentSize = m_nWorkingBufferCurrentSize = 0;
	//Allocate buffers
	m_pUserBuffer = reinterpret_cast<XnUInt8*>(xnOSCallocAligned(1, m_nUserBufferMaxSize, XN_DEFAULT_MEM_ALIGN));
	if (m_pUserBuffer == NULL)
	{
		Shutdown();
		xnLogError(XN_MASK_INPUT_STREAM, "Failed to allocate buffer of size %u", m_nUserBufferMaxSize);
		XN_ASSERT(FALSE);
		return XN_STATUS_ALLOC_FAILED;
	}
	m_pWorkingBuffer = reinterpret_cast<XnUInt8*>(xnOSCallocAligned(1, CONT_STREAM_PREDEFINED_BUFFER_SIZE, XN_DEFAULT_MEM_ALIGN));
	if (m_pWorkingBuffer == NULL)
	{
		Shutdown();
		xnLogError(XN_MASK_INPUT_STREAM, "Failed to allocate buffer of size %u", m_nUserBufferMaxSize);
		XN_ASSERT(FALSE);
		return XN_STATUS_ALLOC_FAILED;
	}
	
    nRetVal = xnLinkGetStreamDumpName(m_nStreamID, m_strDumpName, sizeof(m_strDumpName));
    if (nRetVal != XN_STATUS_OK)
    {
        xnLogWarning(XN_MASK_INPUT_STREAM, "Failed to get stream dump name: %s", xnGetStatusString(nRetVal));
        XN_ASSERT(FALSE);
    }

    m_bInitialized = TRUE;
	return XN_STATUS_OK;
}

XnBool LinkContInputStream::IsInitialized() const
{
	return m_bInitialized;
}

void LinkContInputStream::Shutdown()
{
	if (!m_bInitialized)
		return;

	xnOSEnterCriticalSection(&m_hCriticalSection);

	XN_ALIGNED_FREE_AND_NULL(m_pUserBuffer);
	XN_ALIGNED_FREE_AND_NULL(m_pWorkingBuffer);

	m_bInitialized = FALSE;
	m_bNewDataAvailable = FALSE;
    LinkInputStream::Shutdown();

	xnOSLeaveCriticalSection(&m_hCriticalSection);
}

XnStatus LinkContInputStream::HandlePacket(const LinkPacketHeader& header, const XnUInt8* pData, XnBool& bPacketLoss)
{
	XnStatus nRetVal = XN_STATUS_OK;
	xnl::AutoCSLocker csLock(m_hCriticalSection);
	if (!m_bInitialized)
	{
		return XN_STATUS_NOT_INIT;
	}

	// TODO: handle packet loss!
	bPacketLoss = FALSE;

	if(m_streamType == XN_LINK_STREAM_TYPE_LOG)
	{
		// begin parsing frame
		nRetVal = m_logParser.BeginParsing(m_pWorkingBuffer, CONT_STREAM_PREDEFINED_BUFFER_SIZE);
		XN_IS_STATUS_OK_LOG_ERROR("Begin parsing link log msg", nRetVal);

		nRetVal = m_logParser.ParsePacket(header, pData);

		if (nRetVal != XN_STATUS_OK)
			XN_IS_STATUS_OK_LOG_ERROR("Parse data from stream", nRetVal);
	}
	
	//Write new data to dump (if it's on)
	xnDumpFileWriteBuffer(m_pDumpFile, 
		reinterpret_cast<const XnUInt8*>(m_logParser.GetParsedData()), 
		m_logParser.GetParsedSize());

	if (header.GetFragmentationFlags() & XN_LINK_FRAG_END)
	{
		//Notify that we have new data available
		m_bNewDataAvailable = TRUE;
		nRetVal = m_newDataAvailableEvent.Raise();
		XN_IS_STATUS_OK_LOG_ERROR("Raise new data available event", nRetVal);
	}

	return XN_STATUS_OK;
}

const void* LinkContInputStream::GetData() const
{
	return m_pUserBuffer;
}

XnUInt32 LinkContInputStream::GetDataSize() const
{
	return m_nUserBufferCurrentSize;
}

const void* LinkContInputStream::GetNextData() const
{
	static const XnUInt64 nDummy = 0;
	//TODO: Implement this properly for timestamps...
	XN_ASSERT(FALSE);
	return &nDummy;
}

XnUInt32 LinkContInputStream::GetNextDataSize() const
{
	return 0;
}

XnBool LinkContInputStream::IsNewDataAvailable() const
{
	xnOSEnterCriticalSection(&m_hCriticalSection);
	if (!m_bInitialized)
	{
		return FALSE;
	}
	XnBool bNewDataAvailable = m_bNewDataAvailable;
	xnOSLeaveCriticalSection(&m_hCriticalSection);

	return bNewDataAvailable;
}

XnStatus LinkContInputStream::StartImpl()
{
    XnStatus nRetVal = XN_STATUS_OK;
    if (m_bStreaming)
    {
        return XN_STATUS_OK;
    }
	
    m_pDumpFile = xnDumpFileOpen(m_strDumpName, "%s", m_strDumpName);
	
	//We only need log buffer output if dumping is on
	m_logParser.GenerateOutputBuffer(m_pDumpFile != NULL);

	//We must set the streaming flag first cuz the data handler checks it
	m_bStreaming = TRUE;

    nRetVal = m_pConnection->Connect();
    XN_IS_STATUS_OK_LOG_ERROR("Connect stream's input connection", nRetVal);
    nRetVal = m_pLinkControlEndpoint->StartStreaming(m_nStreamID);
    XN_IS_STATUS_OK_LOG_ERROR("Start streaming", nRetVal);

    return XN_STATUS_OK;
}

XnStatus LinkContInputStream::StopImpl()
{
    XnStatus nRetVal = XN_STATUS_OK;
    if (!m_bStreaming)
    {
        return XN_STATUS_OK;
    }

    nRetVal = m_pLinkControlEndpoint->StopStreaming(m_nStreamID);
    XN_IS_STATUS_OK_LOG_ERROR("Stop streaming", nRetVal);
    m_pConnection->Disconnect();
    m_bStreaming = FALSE;
    xnDumpFileClose(m_pDumpFile);

    return XN_STATUS_OK;
}

XnBool LinkContInputStream::IsStreaming() const
{
    return m_bStreaming;
}

XnStatus LinkContInputStream::UpdateData()
{
	xnl::AutoCSLocker csLock(m_hCriticalSection);
	if (!m_bInitialized)
	{
		xnLogError(XN_MASK_INPUT_STREAM, "Attempted to update data from stream %u which is not initialized", m_nStreamID);
		XN_ASSERT(FALSE);
		return XN_STATUS_NOT_INIT;
	}

	if (m_bNewDataAvailable)
	{
		//Copy working buffer to user buffer
		xnOSMemCopy(m_pUserBuffer, m_pWorkingBuffer, m_nUserBufferMaxSize);
		m_nUserBufferCurrentSize = m_nWorkingBufferCurrentSize;
		m_bNewDataAvailable = FALSE;
	}

	return XN_STATUS_OK;
}

XnStatus LinkContInputStream::RegisterToNewDataAvailable(NewDataAvailableHandler pHandler, void* pCookie, XnCallbackHandle& hCallback)
{
	return m_newDataAvailableEvent.Register(pHandler, pCookie, hCallback);
}

void LinkContInputStream::UnregisterFromNewDataAvailable(XnCallbackHandle hCallback)
{
	m_newDataAvailableEvent.Unregister(hCallback);
}

void LinkContInputStream::SetDumpName(const XnChar* strDumpName)
{
    XnStatus nRetVal = XN_STATUS_OK;
    (void)nRetVal;
    nRetVal = xnOSStrCopy(m_strDumpName, strDumpName, sizeof(m_strDumpName));
    if (nRetVal != XN_STATUS_OK)
    {
        xnLogWarning(XN_MASK_INPUT_STREAM, "Failed to set dump name: %s", xnGetStatusString(nRetVal));
        XN_ASSERT(FALSE);
    }
}

void LinkContInputStream::SetDumpOn(XnBool bDumpOn)
{
    XnStatus nRetVal = XN_STATUS_OK;
    (void)nRetVal;
    
    nRetVal = xnDumpSetMaskState(m_strDumpName, bDumpOn);
    if (nRetVal != XN_STATUS_OK)
    {
        xnLogWarning(XN_MASK_INPUT_STREAM, "Failed to set dump state: %s", xnGetStatusString(nRetVal));
        XN_ASSERT(FALSE);
    }
}
}

