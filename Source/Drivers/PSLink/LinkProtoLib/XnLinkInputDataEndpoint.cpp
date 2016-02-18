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
#include "XnLinkInputDataEndpoint.h"
#include "IConnectionFactory.h"
#include "XnLinkInputStreamsMgr.h"
#include <XnOS.h>
#include <XnOSCpp.h>
#include <XnLog.h>
#include <XnDump.h>

#define XN_MASK_LINK "xnLink"

namespace xn
{

LinkInputDataEndpoint::LinkInputDataEndpoint()
{
	m_pConnection = NULL;
	m_pConnectionFactory = NULL;
	m_bInitialized = FALSE;
	m_nConnected = 0;
	m_pNotifications = NULL;
	m_nEndpointID = 0;
	m_pDumpFile = NULL;
	m_pLinkInputStreamsMgr = NULL;
    m_hCriticalSection = NULL;
}

LinkInputDataEndpoint::~LinkInputDataEndpoint()
{
	Shutdown();
}

XnStatus LinkInputDataEndpoint::Init(XnUInt16 nEndpointID,
									 IConnectionFactory* pConnectionFactory,
									 LinkInputStreamsMgr* pLinkInputStreamsMgr,
									 ILinkDataEndpointNotifications* pNotifications)
{
    XN_VALIDATE_INPUT_PTR(pConnectionFactory);
	XN_VALIDATE_INPUT_PTR(pLinkInputStreamsMgr);
	XN_VALIDATE_INPUT_PTR(pNotifications);

	XnStatus nRetVal = XN_STATUS_OK;
	if (!m_bInitialized)
	{
		m_pConnectionFactory = pConnectionFactory;
		m_nEndpointID = nEndpointID;
		m_pNotifications = pNotifications;
		m_pLinkInputStreamsMgr = pLinkInputStreamsMgr;

        nRetVal = xnOSCreateCriticalSection(&m_hCriticalSection);
        XN_IS_STATUS_OK_LOG_ERROR("Create critical section", nRetVal);
        				
		//We are initialized :)
		m_bInitialized = TRUE;
	}

	return XN_STATUS_OK;
}

XnBool LinkInputDataEndpoint::IsInitialized() const
{
    return m_bInitialized;
}

void LinkInputDataEndpoint::Shutdown()
{
	Disconnect();
	XN_DELETE(m_pConnection);
	m_pConnection = NULL;
    xnOSCloseCriticalSection(&m_hCriticalSection);
	m_bInitialized = FALSE;
}

XnStatus LinkInputDataEndpoint::Connect()
{
	XnStatus nRetVal = XN_STATUS_OK;
    XnChar strDumpName[XN_FILE_MAX_PATH] = "";
    xnl::AutoCSLocker lock(m_hCriticalSection);

	if (!m_bInitialized)
	{
		XN_LOG_ERROR_RETURN(XN_STATUS_NOT_INIT, XN_MASK_LINK, "Not initialized");
	}

	if (m_nConnected == 0)
	{
		if (m_pConnection == NULL)
		{
			//Create input data connection
			nRetVal = m_pConnectionFactory->CreateInputDataConnection(m_nEndpointID, m_pConnection);
			XN_IS_STATUS_OK_LOG_ERROR("Create input data connection", nRetVal);
			xnLogVerbose(XN_MASK_LINK, "Link input data endpoint %u max packet size is %u bytes", m_nEndpointID, m_pConnection->GetMaxPacketSize());
		}
		
		//Tell our async connection object to send incoming data to this object
		nRetVal = m_pConnection->SetDataDestination(this);
		XN_IS_STATUS_OK_LOG_ERROR("Set input data connection data destination", nRetVal);
        //Open dump (if needed)
        nRetVal = xnLinkGetEPDumpName(m_nEndpointID, strDumpName, sizeof(strDumpName));
        XN_IS_STATUS_OK_LOG_ERROR("Get EP Dump name", nRetVal);
        m_pDumpFile = xnDumpFileOpen(strDumpName, "%s.raw", strDumpName);
		//Connect
		nRetVal = m_pConnection->Connect();
		XN_IS_STATUS_OK_LOG_ERROR("Connect input data connection", nRetVal);
	}
    m_nConnected++;

	return XN_STATUS_OK;
}

void LinkInputDataEndpoint::Disconnect()
{
    xnl::AutoCSLocker lock(m_hCriticalSection);
	
    if (m_nConnected == 1)
	{
        //Reference count reaches zero so we REALLY disconnect
		xnDumpFileClose(m_pDumpFile);
		m_pConnection->Disconnect();
		m_pConnection->SetDataDestination(NULL);
	}
    
    if (m_nConnected > 0)
    {
        m_nConnected--;
    }
}

XnBool LinkInputDataEndpoint::IsConnected() const
{
    xnl::AutoCSLocker lock(m_hCriticalSection);
	return (m_nConnected > 0);
}

XnStatus LinkInputDataEndpoint::IncomingData(const void* pData, XnUInt32 nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	xnDumpFileWriteBuffer(m_pDumpFile, pData, nSize);
	nRetVal = m_pLinkInputStreamsMgr->HandleData(pData, nSize);
//	XN_IS_STATUS_OK_LOG_ERROR("Handle data in streams mgr", nRetVal);
	if (nRetVal != XN_STATUS_OK)
	{
		return nRetVal;
	}

	return XN_STATUS_OK;
}


void LinkInputDataEndpoint::HandleDisconnection()
{
    //TODO: Maybe we need to call disconnect here so the thread will stop
	m_nConnected = 0;
	m_pNotifications->HandleLinkDataEndpointDisconnection(m_nEndpointID);
}

XnUInt16 LinkInputDataEndpoint::GetMaxPacketSize() const
{
	return m_pConnection->GetMaxPacketSize();
}

}
