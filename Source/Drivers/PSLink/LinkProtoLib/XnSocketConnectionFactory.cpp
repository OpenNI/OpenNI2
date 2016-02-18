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
#include "XnSocketConnectionFactory.h"
#include "XnSyncSocketConnection.h"
#include "XnSocketInConnection.h"
#include "XnServerSocketInConnection.h"
#include "XnClientSocketInConnection.h"
#include <XnMacros.h>
#include <XnStatus.h>
#include <XnLog.h>

#define XN_MASK_SOCKETS "xnSockets"

namespace xn
{

#define PRIME_CLIENT_INSTALL_PATH_ENV "PRIME_CLIENT_INSTALL_PATH"
#define PRIME_CLIENT_CONFIG_FILE "PrimeClient.ini"



const XnUInt16 SocketConnectionFactory::CONTROL_MAX_PACKET_SIZE = 65535;
const XnUInt16 SocketConnectionFactory::NUM_SERVER_TO_CLIENT_DATA_CONNECTIONS = 1;
const XnUInt16 SocketConnectionFactory::DATA_OUT_MAX_PACKET_SIZE = 65535;
const XnUInt16 SocketConnectionFactory::DATA_IN_MAX_PACKET_SIZE = 65535;
xnl::Array<SocketConnectionFactory::ConnectionStringStruct> SocketConnectionFactory::s_enumerationTargets;

xnl::Array<SyncSocketConnection> SocketConnectionFactory::s_controlConnections;

SocketConnectionFactory::SocketConnectionFactory(Type type)
{
	xnOSMemSet(m_strIP, 0, sizeof(m_strIP));
	m_nControlPort = 0;
	m_nDataOutPort = 0;
	m_nDataInBasePort = 0;
	m_bInitialized = FALSE;
	m_type = type;
}

SocketConnectionFactory::~SocketConnectionFactory()
{
	Shutdown();
}

XnStatus SocketConnectionFactory::Init(const XnChar* strConnString)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = xnOSInitNetwork();
	XN_IS_STATUS_OK_LOG_ERROR("Init network", nRetVal);
	nRetVal = ParseConnectionString(strConnString, m_strIP, sizeof(m_strIP), m_nControlPort);
	XN_IS_STATUS_OK_LOG_ERROR("Parse connection string", nRetVal);

	/* The ports are always arranged in the following order:
	  - Control Port
	  - Client to server data port
	  - Server to client data port number 1
	  - Server to client data port number 2
	     .
	     .
		 .
	  - Server to client data port number NUM_SERVER_TO_CLIENT_DATA_CONNECTIONS
	*/
	if (m_type == TYPE_SERVER)
	{
		m_nDataInBasePort = (m_nControlPort + 1);
		m_nDataOutPort = (m_nDataInBasePort + 1);

		// Initialize listener
		nRetVal = m_serverListener.Init(m_strIP, 
										m_nControlPort, 
										m_nControlPort + 1,
										m_nControlPort + 2,
										NUM_SERVER_TO_CLIENT_DATA_CONNECTIONS,
										CONTROL_MAX_PACKET_SIZE,
										DATA_OUT_MAX_PACKET_SIZE,
										DATA_IN_MAX_PACKET_SIZE);

		if (nRetVal != XN_STATUS_OK)
		{
			return nRetVal;
		}
	}
	else
	{
		m_nDataOutPort = (m_nControlPort + 1);

		//0x81 is first input port in USB - we want first input port in sockets to be 20002
		m_nDataInBasePort = m_nDataOutPort + 1;
	}

	m_bInitialized = TRUE;
	return XN_STATUS_OK;
}

void SocketConnectionFactory::Shutdown()
{
	m_bInitialized = FALSE;

	if (m_type == TYPE_SERVER)
	{
		m_serverListener.Shutdown();
	}
}

XnBool SocketConnectionFactory::IsInitialized() const
{
	return m_bInitialized;
}

XnUInt16 SocketConnectionFactory::GetNumInputDataConnections() const
{
	if (m_type == TYPE_SERVER) 
	{
		return 1;
	}
	else
	{
		return NUM_SERVER_TO_CLIENT_DATA_CONNECTIONS;
	}
}

XnUInt16 SocketConnectionFactory::GetNumOutputDataConnections() const
{
	if (m_type == TYPE_SERVER) 
	{
		return NUM_SERVER_TO_CLIENT_DATA_CONNECTIONS;
	}
	else
	{
		return 1;
	}
}


XnStatus SocketConnectionFactory::GetControlConnection(ISyncIOConnection*& pConn)
{
	SyncSocketConnection* pSyncSocketConn = NULL;
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_type == TYPE_CLIENT)
	{
		nRetVal = GetControlConnectionImpl(m_strIP, m_nControlPort, pSyncSocketConn);
		XN_IS_STATUS_OK_LOG_ERROR("Get client control connection", nRetVal);
	}
	else
	{
		return m_serverListener.GetControlConnection(pConn);
	}
	
	pConn = pSyncSocketConn;
	return XN_STATUS_OK;
}

XnStatus SocketConnectionFactory::CreateOutputDataConnection(XnUInt16 nID, IOutputConnection*& pConn)
{
	if (!m_bInitialized)
	{
		return XN_STATUS_NOT_INIT;
	}
	SyncSocketConnection* pSyncSocketConnection = NULL;
	if (m_type == TYPE_SERVER)
	{
		// We do not create a new connection, we accept a connection from the listener. 
		return m_serverListener.CreateOutputDataConnection(nID, pConn);
	}
	else
	{
		//TODO: Change SyncSocketConnection to ClientSyncSocketConnection
		pSyncSocketConnection = XN_NEW(SyncSocketConnection);
	}

	XN_VALIDATE_ALLOC_PTR(pSyncSocketConnection);
	XnStatus nRetVal = pSyncSocketConnection->Init(m_strIP, m_nDataOutPort, DATA_OUT_MAX_PACKET_SIZE);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SOCKETS, "Initialize output data socket for ip '%s', port %u: %s", 
			m_strIP, m_nDataOutPort, xnGetStatusString(nRetVal));
		XN_ASSERT(FALSE);
		XN_DELETE(pSyncSocketConnection);
		return nRetVal;
	}

	pConn = pSyncSocketConnection;
	return XN_STATUS_OK;
}

XnStatus SocketConnectionFactory::CreateInputDataConnection(XnUInt16 nID, IAsyncInputConnection*& pConn)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (!m_bInitialized)
	{
		return XN_STATUS_NOT_INIT;
	}
	SocketInConnection* pInSocketConn = NULL;
	if (m_type == TYPE_SERVER)
	{
		// We do not create a new connection, we accept a connection from the listener. 
		// Only one input data connection in server, so nID is not used
		return m_serverListener.CreateInputDataConnection(pConn);
	}
	else
	{
		pInSocketConn = XN_NEW(ClientSocketInConnection);
	}
	XN_VALIDATE_ALLOC_PTR(pInSocketConn);

	nRetVal = pInSocketConn->Init(m_strIP, m_nDataInBasePort + nID, DATA_IN_MAX_PACKET_SIZE);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SOCKETS, "Initialize input data socket for ip '%s', port %u: %s", 
			m_strIP, m_nDataInBasePort + nID, xnGetStatusString(nRetVal));
		XN_ASSERT(FALSE);
		XN_DELETE(pInSocketConn);
		return nRetVal;
	}

	pConn = pInSocketConn;
	return XN_STATUS_OK;
}

XnStatus SocketConnectionFactory::AddEnumerationTarget(const XnChar* strConnString)
{
	XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = s_enumerationTargets.SetSize(s_enumerationTargets.GetSize()+1);
	XN_IS_STATUS_OK_LOG_ERROR("Add to enumeration targets", nRetVal);
	nRetVal = xnOSStrCopy(s_enumerationTargets[s_enumerationTargets.GetSize()-1].m_strConn, 
		strConnString, sizeof(XnConnectionString));
	XN_IS_STATUS_OK_LOG_ERROR("Copy connection string", nRetVal);
	return XN_STATUS_OK;
}

XnStatus SocketConnectionFactory::TryAndAddEnumerationTarget(xnl::Array<ConnectionStringStruct>& result, const XnChar* strConnString)
{
	XnStatus nRetVal = XN_STATUS_OK;
	SyncSocketConnection* pConnection = NULL;
	XnChar strIP[XN_FILE_MAX_PATH];
	XnUInt16 nPort = 0;
	nRetVal = ParseConnectionString(strConnString, strIP, sizeof(strIP), nPort);
	XN_IS_STATUS_OK_LOG_ERROR("Parse connection string", nRetVal);

	nRetVal = GetControlConnectionImpl(strIP, nPort, pConnection);
	XN_IS_STATUS_OK_LOG_ERROR("Get control connection", nRetVal);

	nRetVal = pConnection->Connect();
	if (nRetVal == XN_STATUS_OK)
	{
		ConnectionStringStruct connString;
		EncodeConnectionString(connString.m_strConn, sizeof(connString.m_strConn), strIP, nPort);
		
		// make sure we have space
		nRetVal = result.AddLast(connString);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		xnLogInfo(XN_MASK_SOCKETS, "Couldn't connect to %s:%u - '%s'", strIP, nPort, xnGetStatusString(nRetVal));
	}

	return (XN_STATUS_OK);
}

XnStatus SocketConnectionFactory::AddConfigFileTarget(xnl::Array<ConnectionStringStruct>& result, XnUInt16 nProductID)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnChar strConfigFile[XN_FILE_MAX_PATH];
	nRetVal = xnOSGetEnvironmentVariable(PRIME_CLIENT_INSTALL_PATH_ENV, strConfigFile, XN_FILE_MAX_PATH);
	if (nRetVal == XN_STATUS_OK)
	{
		nRetVal = xnOSStrAppend(strConfigFile, "/Config/", XN_FILE_MAX_PATH);
		XN_IS_STATUS_OK(nRetVal);
	}
	else if (nRetVal == XN_STATUS_OS_ENV_VAR_NOT_FOUND)
	{
		nRetVal = xnOSStrCopy(strConfigFile, "./", sizeof(strConfigFile));
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		return (nRetVal);
	}

	nRetVal = xnOSStrAppend(strConfigFile, PRIME_CLIENT_CONFIG_FILE, XN_FILE_MAX_PATH);
	XN_IS_STATUS_OK(nRetVal);

	XnBool bDoesConfigExist = FALSE;
	nRetVal = xnOSDoesFileExist(strConfigFile, &bDoesConfigExist);
	XN_IS_STATUS_OK(nRetVal);

	if (!bDoesConfigExist)
	{
		// no file
		return (XN_STATUS_OK);
	}

	// file exist. read IP address and port from it
	XnChar strProductID[80];
	sprintf(strProductID, "%04X", nProductID);

	XnChar strIP[XN_FILE_MAX_PATH];
	nRetVal = xnOSReadStringFromINI(strConfigFile, strProductID, "IPAddress", strIP, sizeof(strIP));
	if (nRetVal != XN_STATUS_OK)
	{
		// not found. nothing to connect to
		return (XN_STATUS_OK);
	}

	XnInt32 nPort = 0;
	nRetVal = xnOSReadIntFromINI(strConfigFile, strProductID, "Port", &nPort);
	if (nRetVal != XN_STATUS_OK)
	{
		// not found. nothing to connect to
		return (XN_STATUS_OK);
	}

	XnConnectionString strConnString;
	nRetVal = EncodeConnectionString(strConnString, sizeof(strConnString), strIP, XnUInt16(nPort));
	XN_IS_STATUS_OK_LOG_ERROR("Encode connection string", nRetVal);
	nRetVal = TryAndAddEnumerationTarget(result, strConnString);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus SocketConnectionFactory::EnumerateConnStrings(XnUInt16 nProductID, 
													          XnConnectionString*& astrConnStrings, 
													          XnUInt32& nCount)
{
	XnStatus nRetVal = XN_STATUS_OK;

	astrConnStrings = NULL;
	nCount = 0;

	nRetVal = xnOSInitNetwork();
	XN_IS_STATUS_OK_LOG_ERROR("Init network", nRetVal);
	
	// Enumeration targets can be received from 2 sources:
	// 1. via the AddEnumerationTarget() method.
	// 2. from config file.
	xnl::Array<ConnectionStringStruct> result;

	for (XnUInt32 i = 0; i < s_enumerationTargets.GetSize(); ++i)
	{
		nRetVal = TryAndAddEnumerationTarget(result, s_enumerationTargets[i].m_strConn);
		XN_IS_STATUS_OK(nRetVal);
	}

	// add from config file
	nRetVal = AddConfigFileTarget(result, nProductID);
	XN_IS_STATUS_OK(nRetVal);

	astrConnStrings = (XnConnectionString*)xnOSCalloc(result.GetSize(), sizeof(astrConnStrings[0]));
	XN_VALIDATE_ALLOC_PTR(astrConnStrings);

	for (XnUInt32 i = 0; i < result.GetSize(); ++i)
	{
		nRetVal = xnOSStrCopy(astrConnStrings[i], result[i].m_strConn, sizeof(astrConnStrings[i]));
		XN_IS_STATUS_OK(nRetVal);
	}

	nCount = result.GetSize();

	return XN_STATUS_OK;
}

void SocketConnectionFactory::FreeConnStringsList(XnConnectionString* astrConnStrings)
{
	xnOSFree(astrConnStrings);
}

XnStatus SocketConnectionFactory::GetControlConnectionImpl(const XnChar* strIP, 
														   XnUInt16 nPort, 
														   SyncSocketConnection*& pControlConnection)
{
	XnStatus nRetVal = XN_STATUS_OK;
	pControlConnection = NULL;
	
	//Try and find existing connection
	for (XnUInt32 i = 0; i < s_controlConnections.GetSize(); i++)
	{
		if ((xnOSStrCmp(s_controlConnections[i].GetIP(), strIP) == 0) && 
			(s_controlConnections[i].GetPort() == nPort))
		{
			pControlConnection = &s_controlConnections[i];
			break;
		}
	}

	if (pControlConnection == NULL)
	{
		//Control connection is not in array - add it
		nRetVal = s_controlConnections.SetSize(s_controlConnections.GetSize() + 1);
		XN_IS_STATUS_OK_LOG_ERROR("Add to control connections array", nRetVal);
		pControlConnection = &s_controlConnections[s_controlConnections.GetSize() - 1];
	}

	if (!pControlConnection->IsInitialized())
	{
		//Control connection it not initialized - initialize it
		/*TODO: Change XN_CONTROL_PREDEFINED_MAX_PACKET_SIZE to variable once we have service discovery that 
		  tells us the control endpoint packet size in LinkControlEndpoint. */
		nRetVal = pControlConnection->Init(strIP, nPort, CONTROL_MAX_PACKET_SIZE);
		if (nRetVal != XN_STATUS_OK)
		{
			xnLogError(XN_MASK_SOCKETS, "Failed to initialize control socket for ip '%s', port %u: %s", 
				strIP, nPort, xnGetStatusString(nRetVal));
			XN_ASSERT(FALSE);
			pControlConnection = NULL;
			return nRetVal;
		}
	}

	return XN_STATUS_OK;
}

XnStatus SocketConnectionFactory::ParseConnectionString(const XnChar* strConnString, 
														XnChar* strIP, 
														XnUInt32 nIPBufSize, 
														XnUInt16& nPort)
{	
	XnStatus nRetVal = XN_STATUS_OK;
	const XnChar* pColon = NULL;
	const XnChar* strPort = NULL;
	XnUInt32 nTempPort = 0;

	pColon = strchr(strConnString, ':');
	if (pColon == NULL)
	{
		xnLogError(XN_MASK_SOCKETS, "Invalid connection string - missing ':'.");
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}

	//Take IP address from beginning of connection string
	nRetVal = xnOSStrNCopy(strIP, strConnString, XnUInt32(pColon - strConnString), nIPBufSize);
	XN_IS_STATUS_OK_LOG_ERROR("Copy IP address", nRetVal);
	strIP[pColon - strConnString] = '\0';
	//Take port number from chars after colon
	strPort = (pColon + 1);
	nTempPort = atoi(strPort);
	if ((nTempPort == 0) || (nTempPort > XN_MAX_UINT16))
	{
		xnLogError(XN_MASK_SOCKETS, "Invalid connection string - bad port number %u", nTempPort);
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}
	nPort = XnUInt16(nTempPort);
	return XN_STATUS_OK;
}

XnStatus SocketConnectionFactory::EncodeConnectionString(XnChar* strConnString, XnUInt32 nBufferSize, const XnChar* strIP, XnUInt16 nPort)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 nCharsWritten = 0;
	nRetVal = xnOSStrFormat(strConnString, nBufferSize, &nCharsWritten, "%s:%u", strIP, nPort);
	XN_IS_STATUS_OK_LOG_ERROR("Format connection string", nRetVal);
	return XN_STATUS_OK;
}

}
