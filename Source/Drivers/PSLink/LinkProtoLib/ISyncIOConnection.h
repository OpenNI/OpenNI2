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
#ifndef ISYNCIOCONNECTION_H
#define ISYNCIOCONNECTION_H

#include <XnPlatform.h>
#include <XnStatus.h>

#include "IOutputConnection.h"
#include "ISyncInputConnection.h"

namespace xn
{

class ISyncIOConnection : virtual public ISyncInputConnection, 
                          virtual public IOutputConnection
{
public:
	virtual ~ISyncIOConnection() {}
	
	/**
	 * Receives data from the stream.
	 *
	 * @param	pData		[out]	Buffer to hold received data.
	 * @param	nSize		[in]	Size of data to receive.
	 */
	virtual XnStatus Receive(void* pData, XnUInt32& nSize) = 0;
	
	/**
	 * Sends a buffer on the connection.
	 *
	 * @param	pData		[out]	Buffer that holds data to send.
	 * @param	nDataSize	[in]	Size of data to send.
	 */
	virtual XnStatus Send(const void* pData, XnUInt32 nSize) = 0;
};

}

#endif // ISYNCIOCONNECTION_H
