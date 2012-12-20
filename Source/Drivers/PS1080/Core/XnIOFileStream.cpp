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
#include <XnIOFileStream.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnIOFileStream::XnIOFileStream(const XnChar* pcsFileName, XnUInt32 nFlags) :
	m_pcsFileName(pcsFileName), m_nFlags(nFlags)
{
}

XnStatus XnIOFileStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = xnOSOpenFile(m_pcsFileName, m_nFlags, &m_hFile);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnIOFileStream::Free()
{
	return xnOSCloseFile(&m_hFile);
}

XnStatus XnIOFileStream::WriteData(const XnUChar *pData, XnUInt32 nDataSize)
{
	return xnOSWriteFile(m_hFile, pData, nDataSize);
}

XnStatus XnIOFileStream::ReadData(XnUChar *pData, XnUInt32 nDataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnUInt32 nReadSize = nDataSize;
	nRetVal = xnOSReadFile(m_hFile, pData, &nReadSize);
	XN_IS_STATUS_OK(nRetVal);

	if (nReadSize != nDataSize)
	{
		return XN_STATUS_OS_FILE_READ_FAILED;
	}

	return (XN_STATUS_OK);
}

XnStatus XnIOFileStream::Tell(XnUInt64* pnOffset)
{
	return xnOSTellFile64(m_hFile, pnOffset);
}

XnStatus XnIOFileStream::Seek(XnUInt64 nOffset)
{
	return xnOSSeekFile64(m_hFile, XN_OS_SEEK_SET, nOffset);
}
