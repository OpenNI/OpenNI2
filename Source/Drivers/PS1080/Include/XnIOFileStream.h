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
#ifndef XNIOFILESTREAM_H
#define XNIOFILESTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnIOStream.h>
#include <XnOS.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnIOFileStream : public XnIOStream
{
public:
	XnIOFileStream(const XnChar* pcsFileName, XnUInt32 nFlags);
	~XnIOFileStream() { Free(); }

	virtual XnStatus WriteData(const XnUChar* pData, XnUInt32 nDataSize);
	virtual XnStatus ReadData(XnUChar* pData, XnUInt32 nDataSize);
	virtual XnStatus Init();
	virtual XnStatus Free();

	XnStatus Tell(XnUInt64* pnOffset);
	XnStatus Seek(XnUInt64 nOffset);

private:
	const XnChar* m_pcsFileName;
	XnUInt32 m_nFlags;
	XN_FILE_HANDLE m_hFile;
};

#endif // XNIOFILESTREAM_H
