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
#ifndef XNSOCKETCONNECTIONFACTORY_H
#define XNSOCKETCONNECTIONFACTORY_H

#include "IConnectionFactory.h"
#include "XnSyncSocketConnection.h"
#include "XnSyncServerSocketConnection.h"

#include <XnArray.h>

namespace xn
{

class SocketConnectionFactory: public IConnectionFactory
{
public:
	enum Type {TYPE_CLIENT, TYPE_SERVER};
	SocketConnectionFactory(Type type);
	virtual ~SocketConnectionFactory();
	virtual XnStatus Init(const XnChar* strConnString);
	virtual void Shutdown();
	virtual XnBool IsInitialized() const;
	virtual XnUInt16 GetNumInputDataConnections() const;
	virtual XnUInt16 GetNumOutputDataConnections() const;

	/** The pointer returned by GetControlConnection() belongs to the connection factory and 
	    must not be deleted by caller. **/
	virtual XnStatus GetControlConnection(ISyncIOConnection*& pConnection);	

	// nID is used for server, as the server has several output connections.
	virtual XnStatus CreateOutputDataConnection(XnUInt16 nID, IOutputConnection*& pConnection);		

	// nID is used for client, as the client has several output connections.
	virtual XnStatus CreateInputDataConnection(XnUInt16 nID, IAsyncInputConnection*& pConnection);	

	static XnStatus AddEnumerationTarget(const XnChar* strConnString);
	
	static XnStatus EnumerateConnStrings(XnUInt16 nProductID, XnConnectionString*& astrConnStrings, XnUInt32& nCount);
	static void FreeConnStringsList(XnConnectionString* astrConnStrings);

	static const XnUInt16 DATA_OUT_MAX_PACKET_SIZE;
	static const XnUInt16 DATA_IN_MAX_PACKET_SIZE;

private:
	static const XnUInt16 CONTROL_MAX_PACKET_SIZE;

	static XnStatus ParseConnectionString(const XnChar* strConnString, XnChar* strIP, XnUInt32 nIPBufSize, XnUInt16& nPort);
	static XnStatus EncodeConnectionString(XnChar* strConnString, XnUInt32 nBufferSize, const XnChar* strIP, XnUInt16 nPort);

	// patch: we need to define a struct to keep in XnArray (XnArray cannot work directly with XnConnectionString)
	typedef struct ConnectionStringStruct
	{
		XnConnectionString m_strConn;
	} ConnectionStringStruct;

	static const XnUInt16 NUM_SERVER_TO_CLIENT_DATA_CONNECTIONS;
	static xnl::Array<SyncSocketConnection> s_controlConnections;
	static XnStatus GetControlConnectionImpl(const XnChar* strIP, XnUInt16 nPort, SyncSocketConnection*& pControlConnection);
	static XnStatus TryAndAddEnumerationTarget(xnl::Array<ConnectionStringStruct>& result, const XnChar* strConnString);
	static XnStatus AddConfigFileTarget(xnl::Array<ConnectionStringStruct>& result, XnUInt16 nProductID);
	
	// The client implementation returns ISyncIOConnection object that could not be deleted (return from s_controlConnections), 
	// I want the behavior to be consistent for both client and server.
	SyncServerSocketListener m_serverListener;

	Type m_type;
	XnChar m_strIP[XN_FILE_MAX_PATH];
	XnUInt16 m_nControlPort;
	XnUInt16 m_nDataOutPort;
	XnUInt16 m_nDataInBasePort;
	XnBool m_bInitialized;

	static xnl::Array<ConnectionStringStruct> s_enumerationTargets;
};

}

#endif // XNSOCKETCONNECTIONFACTORY_H

