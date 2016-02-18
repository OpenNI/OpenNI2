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
#include "XnGeneralDebugProcessor.h"
#include <XnDump.h>

XnGeneralDebugProcessor::XnGeneralDebugProcessor(XnDevicePrivateData* pDevicePrivateData) : XnDataProcessor(pDevicePrivateData, "GeneralDebug"), m_pDump(NULL)
{

}

XnGeneralDebugProcessor::~XnGeneralDebugProcessor()
{
	xnDumpFileClose(m_pDump);
}

void XnGeneralDebugProcessor::ProcessPacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize)
{
	if (nDataOffset == 0)
	{
		// start of data. The first uint16 is the number of fields in the header, and then we have the data itself
		const XnUInt16* pFields = (const XnUInt16*)pData;
		XnUInt16 nFields = *pFields;
		++pFields;

		XnChar strFileName[XN_FILE_MAX_PATH] = "";
		XnUInt32 nCharsWritten = 0;
		XnUInt32 nLength = 0;
		xnOSStrFormat(strFileName, XN_FILE_MAX_PATH, &nCharsWritten, "FirmwareDebug.");
		nLength += nCharsWritten;

		for (XnUInt16 i = 0; i < nFields; ++i)
		{
			xnOSStrFormat(strFileName + nLength, XN_FILE_MAX_PATH - nLength, &nCharsWritten, "%02d.", *pFields);
			++pFields;
			nLength += nCharsWritten;
		}

		xnOSStrFormat(strFileName + nLength, XN_FILE_MAX_PATH - nLength, &nCharsWritten, ".raw");

		xnDumpFileClose(m_pDump);
		m_pDump = xnDumpFileOpenEx("FirmwareDebug", TRUE, TRUE, strFileName);

		const XnUChar* pDataStart = (const XnUChar*)pFields;
		nDataSize -= XnUInt32(pDataStart - pData);
		pData = pDataStart;
	}

	xnDumpFileWriteBuffer(m_pDump, pData, nDataSize);

	if (nDataOffset + nDataSize == pHeader->nBufSize)
	{
		// end of data
		xnDumpFileClose(m_pDump);
		m_pDump = NULL;
	}
}

