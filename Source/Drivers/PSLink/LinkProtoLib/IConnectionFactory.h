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
#ifndef ICONNECTIONFACTORY_H
#define ICONNECTIONFACTORY_H

#include <XnStatus.h>
#include <XnPlatform.h>

namespace xn
{

class ISyncIOConnection;
class IOutputConnection;
class IAsyncInputConnection;

class IConnectionFactory
{
public:
	virtual ~IConnectionFactory() {}
	virtual XnStatus Init(const XnChar* strConnString) = 0;
	virtual void Shutdown() = 0;
	virtual XnBool IsInitialized() const = 0;
	virtual XnUInt16 GetNumInputDataConnections() const = 0;
	virtual XnUInt16 GetNumOutputDataConnections() const = 0;

	/** The pointer returned by GetControlConnection() belongs to the connection factory and 
	    must not be deleted by caller. **/
	virtual XnStatus GetControlConnection(ISyncIOConnection*& pConnection) = 0;

	virtual XnStatus CreateOutputDataConnection(XnUInt16 nID, IOutputConnection*& pConnection) = 0;
	virtual XnStatus CreateInputDataConnection(XnUInt16 nID, IAsyncInputConnection*& pConnection) = 0;
};

}

#endif // ICONNECTIONFACTORY_H
