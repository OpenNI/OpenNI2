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
#include "XnIONetworkStream.h"
#include <XnLog.h>

#define XN_MASK_IO_NET_STREAM "IoNetStream"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnIONetworkStream::XnIONetworkStream(XN_SOCKET_HANDLE hSocket) :
	m_nReadTimeout(XN_WAIT_INFINITE),
	m_hSocket(hSocket),
	m_bIsConnected(TRUE)
{
}

XnStatus XnIONetworkStream::Init()
{
	return (XN_STATUS_OK);
}

XnStatus XnIONetworkStream::Free()
{
	m_bIsConnected = FALSE;
	//We don't close the socket here because we don't own it - whoever opened it should be the one to close it.
	return XN_STATUS_OK;
}

XnStatus XnIONetworkStream::WriteData(const XnUChar *pData, XnUInt32 nDataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = xnOSSendNetworkBuffer(m_hSocket, (const XnChar*)pData, nDataSize);
	if (nRetVal != XN_STATUS_OK)
	{
		m_bIsConnected = FALSE;
		return (nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnIONetworkStream::ReadData(XnUChar *pData, XnUInt32 nDataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnUInt32 nTotalRead = 0;

	// read until we get all the data we want
	while (nTotalRead < nDataSize)
	{
		XnUInt32 nReadSize = nDataSize - nTotalRead;
		nRetVal = xnOSReceiveNetworkBuffer(m_hSocket, (XnChar*)(pData + nTotalRead), &nReadSize, m_nReadTimeout);
		if (nRetVal != XN_STATUS_OK)
		{
			if (nRetVal == XN_STATUS_OS_NETWORK_CONNECTION_CLOSED)
			{
				xnLogVerbose(XN_MASK_IO_NET_STREAM, "Network connection was closed gracefully");
				m_bIsConnected = FALSE;
			}
			else if (nRetVal != XN_STATUS_OS_NETWORK_TIMEOUT)
			{
				xnLogError(XN_MASK_IO_NET_STREAM, "Got an error while reading network buffer: %s", xnGetStatusString(nRetVal));
				m_bIsConnected = FALSE;
			}
			return (nRetVal);
		}

		nTotalRead += nReadSize;
	}

	return (XN_STATUS_OK);
}


void XnIONetworkStream::SetReadTimeout(XnUInt32 nMicrosecondsReadTimeout)
{
	m_nReadTimeout = nMicrosecondsReadTimeout;
}
