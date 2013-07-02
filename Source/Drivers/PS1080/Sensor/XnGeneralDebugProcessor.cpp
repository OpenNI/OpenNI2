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

