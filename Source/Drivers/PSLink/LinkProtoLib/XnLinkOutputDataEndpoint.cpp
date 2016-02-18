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
#include "XnLinkOutputDataEndpoint.h"
#include "IConnectionFactory.h"
#include "IOutputConnection.h"
#include <XnOS.h>
#include <XnLog.h>

#define XN_MASK_LINK "xnLink"

namespace xn
{


LinkOutputDataEndpoint::LinkOutputDataEndpoint()
{
	m_pConnection = NULL;
	m_bInitialized = FALSE;
	m_bConnected = FALSE;
	m_nEndpointID = 0;
}

LinkOutputDataEndpoint::~LinkOutputDataEndpoint()
{
	Shutdown();
}

XnStatus LinkOutputDataEndpoint::Init(XnUInt16 nEndpointID, 
									  IConnectionFactory* pConnectionFactory)
{
	XN_VALIDATE_INPUT_PTR(pConnectionFactory);
	XnStatus nRetVal = XN_STATUS_OK;

	if (!m_bInitialized)
	{
		//Create output data connection
		m_nEndpointID = nEndpointID;

		nRetVal = pConnectionFactory->CreateOutputDataConnection(nEndpointID, m_pConnection);
		XN_IS_STATUS_OK_LOG_ERROR("Create output data connection", nRetVal);

		//We are initialized :)
		m_bInitialized = TRUE;
	}
	
	return XN_STATUS_OK;
}


XnBool LinkOutputDataEndpoint::IsInitialized() const
{
	return m_bInitialized;
}

void LinkOutputDataEndpoint::Shutdown()
{
	Disconnect();
	XN_DELETE(m_pConnection);
	m_pConnection = NULL;
	m_bInitialized = FALSE;
}

XnStatus LinkOutputDataEndpoint::Connect()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (!m_bInitialized)
	{
		XN_LOG_ERROR_RETURN(XN_STATUS_NOT_INIT, XN_MASK_LINK, "Not initialized");
	}

	if (!m_bConnected)
	{
		//Connect
		nRetVal = m_pConnection->Connect();
		XN_IS_STATUS_OK_LOG_ERROR("Connect input data connection", nRetVal);

		//We're connected
		m_bConnected = TRUE;
	}

	return XN_STATUS_OK;
}

void LinkOutputDataEndpoint::Disconnect()
{
	if (m_bConnected)
	{
		m_pConnection->Disconnect();
		m_bConnected = FALSE;
	}
}

XnBool LinkOutputDataEndpoint::IsConnected() const
{
	return m_bConnected;
}


XnUInt16 LinkOutputDataEndpoint::GetMaxPacketSize() const
{
	return m_pConnection->GetMaxPacketSize();
}

XnStatus LinkOutputDataEndpoint::SendData(const void* pData, XnUInt32 nSize)
{
	return m_pConnection->Send(pData, nSize);
}

}