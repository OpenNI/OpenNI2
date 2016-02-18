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
#include "XnLinkLogParser.h"
#include "XnLinkProtoUtils.h"
#include <XnLog.h>

namespace xn
{

LinkLogParser::LinkLogParser() : m_copyDataToOutput(false)
{
}

LinkLogParser::~LinkLogParser()
{
	//Close any open log files
	for(xnl::Hash<XnUInt8, XnDumpFile*>::Iterator iter = m_activeLogs.Begin(); iter!=m_activeLogs.End(); ++iter)
		xnDumpFileClose(iter->Value());
	m_activeLogs.Clear();

}

XnStatus LinkLogParser::ParsePacketImpl(XnLinkFragmentation /*fragmentation*/,
	const XnUInt8* pSrc, 
	const XnUInt8* pSrcEnd, 
	XnUInt8*& pDst, 
	const XnUInt8* pDstEnd)
{
	//Copy data to output buffer if needed (The log dumps data anyway, so most time we wont need it and save the memcopy
	//Otherwise, we do not advance pDst, so Data size remains 0
	if(m_copyDataToOutput)
	{
		XnSizeT nPacketDataSize = pSrcEnd - pSrc;
		if (pDst + nPacketDataSize > pDstEnd)
		{
			XN_ASSERT(FALSE);
			return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
		}

		xnOSMemCopy(pDst, pSrc, nPacketDataSize);
		pDst += nPacketDataSize;
	}

	XnStatus nRetVal = XN_STATUS_OK;
	XnLinkLogParam* logHeader = (XnLinkLogParam*)pSrc;

	//Parsed data
	XnUInt8 fileID;
	XnLinkLogCommand command;
	XnChar logFileName[XN_LINK_MAX_LOG_FILE_NAME_LENGTH];

	fileID = logHeader->m_ID;
	command = (XnLinkLogCommand)logHeader->command;

	XnUInt16 actualDataSize = XN_PREPARE_VAR16_IN_BUFFER(logHeader->size); //We may have padding at the end

	pSrc += sizeof(XnLinkLogParam);
	actualDataSize -= sizeof(XnLinkLogParam);

	//Parse file name
	if(command == XN_LINK_LOG_COMMAND_OPEN || command == XN_LINK_LOG_COMMAND_OPEN_APPEND)
	{
		//Copy file name to our XnChar array
		XnUInt8* inputFileName = ((XnLinkLogFileParam*)pSrc)->logFileName;
		int i = 0;
		for(; i < XN_LINK_MAX_LOG_FILE_NAME_LENGTH && *inputFileName != '\0'; ++i, ++inputFileName)
			logFileName[i] = (XnChar)*inputFileName;
		logFileName[i] = '\0';

		pSrc += sizeof(XnLinkLogFileParam);
		actualDataSize -= sizeof(XnLinkLogFileParam);
	}

	//Write the data to the matching file
	switch (command)
	{
	case XN_LINK_LOG_COMMAND_OPEN_APPEND:
		nRetVal = XN_STATUS_NOT_IMPLEMENTED;
		if (nRetVal != XN_STATUS_OK)                                                                                                                           
		{                                                                                                                                                                          
			xnLoggerError(XN_LOGGER_RETVAL_CHECKS, "Failed to Append log file \'%s\': %s", logFileName, xnGetStatusString(nRetVal));       
			XN_ASSERT(FALSE);                                                                                                                                      
			return (nRetVal);                                                                                                                                      
		}
		break;
	case XN_LINK_LOG_COMMAND_OPEN:
		xnLogVerbose("", "Received open command for file %s id %d\n", logFileName, fileID);
		nRetVal = OpenLogFile(fileID, logFileName);
		if (nRetVal != XN_STATUS_OK)                                                                                                                           
		{
			xnLoggerError(XN_LOGGER_RETVAL_CHECKS, "Failed to Open log file \'%s\': %s", logFileName, xnGetStatusString(nRetVal));       
			XN_ASSERT(FALSE);                                                                                                                                      
			return (nRetVal);                                                                                                                                      
		}
		break;
	case XN_LINK_LOG_COMMAND_CLOSE:
		xnLogVerbose("", "Received close command for file id %d\n", fileID);
		nRetVal = CloseLogFile(fileID);
		if (nRetVal != XN_STATUS_OK)                                                                                                                           
		{                                                                                                                                                                          
			xnLoggerError(XN_LOGGER_RETVAL_CHECKS, "Failed to Close log file #%d: %s", fileID, xnGetStatusString(nRetVal));      
			XN_ASSERT(FALSE);                                                                                                                                      
			return (nRetVal);                                                                                                                                      
		}
		break;
	case XN_LINK_LOG_COMMAND_WRITE:
		nRetVal = WriteToLogFile(fileID, pSrc, actualDataSize);
		if (nRetVal != XN_STATUS_OK)                                                                                                                           
		{                                                                                                                                                                          
			xnLoggerError(XN_LOGGER_RETVAL_CHECKS, "Failed to Write log file #%d: %s", fileID, xnGetStatusString(nRetVal));      
			XN_ASSERT(FALSE);                                                                                                                                      
			return (nRetVal);                                                                                                                                      
		}
		break;
	default:
		xnLogWarning(XN_MASK_LINK, "Invalid command: %d", (int)command);
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	return XN_STATUS_OK;
}



XnStatus LinkLogParser::OpenLogFile( XnUInt8 fileID, const XnChar* fileName )
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnDumpFile* pTargetFile;

	//We should not have a file with this ID
	if(m_activeLogs.Find(fileID) != m_activeLogs.End())
	{
		xnLogWarning(XN_MASK_LINK, "Attempting to open existing log file. ID: %d, name: %s", (int)fileID, fileName);
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}

	//Prefix current timestamp to file name (timestamp is 25 chars)
	XnChar filenameWithTimestamp[XN_LINK_MAX_LOG_FILE_NAME_LENGTH + 25 + 1];
	time_t currtime;
	time(&currtime);
	strftime(filenameWithTimestamp, sizeof(filenameWithTimestamp)-1, "%Y_%m_%d__%H_%M_%S.", localtime(&currtime)); 
	xnOSStrAppend(filenameWithTimestamp, fileName, sizeof(filenameWithTimestamp)-1);

	//Open file and add to collection
	pTargetFile = xnDumpFileOpenEx("", true, false, filenameWithTimestamp);
	if(pTargetFile == NULL)
		nRetVal = XN_STATUS_ERROR;

	if(nRetVal == XN_STATUS_OK)
		m_activeLogs[fileID] = pTargetFile;

	return nRetVal;
}

XnStatus LinkLogParser::CloseLogFile( XnUInt8 fileID )
{
	XnStatus nRetVal = XN_STATUS_OK;

	//We should have a file with this ID
	xnl::Hash<XnUInt8, XnDumpFile*>::Iterator fileRecord = m_activeLogs.Find(fileID);

	if(fileRecord == m_activeLogs.End())
	{
		xnLogWarning(XN_MASK_LINK, "Attempting to close non existing log file. ID: %d", (int)fileID);
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}

	XnDumpFile* pTargetFile = fileRecord->Value();

	//Close file and remove from collection
	xnDumpFileClose(pTargetFile);
	m_activeLogs.Remove(fileRecord);

	return nRetVal;

}

XnStatus LinkLogParser::WriteToLogFile( XnUInt8 fileID, const void* pData, XnUInt32 dataLength )
{
	//We should have a file with this ID
	xnl::Hash<XnUInt8, XnDumpFile*>::Iterator fileRecord = m_activeLogs.Find(fileID);

	if(fileRecord == m_activeLogs.End())
	{
		xnLogWarning(XN_MASK_LINK, "Attempting to write to non existing log file. ID: %d", (int)fileID);
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}

	xnDumpFileWriteBuffer(fileRecord->Value(), pData, dataLength);
	return XN_STATUS_OK;
}

void LinkLogParser::GenerateOutputBuffer(bool toCreate)
{
	m_copyDataToOutput = toCreate;
}


}
