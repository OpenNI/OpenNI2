/*****************************************************************************
*                                                                            *
*  PrimeSense PSCommon Library                                               *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PSCommon.                                            *
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
#include <unistd.h>
#undef XN_CROSS_PLATFORM
#include <XnOS.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XN_CORE_API XnStatus XnOSGetFileList(const XN_CHAR* cpSearchPattern, const XN_CHAR* cpPrefixPath, XN_CHAR cpFileList[][XN_FILE_MAX_PATH], const XN_UINT32 nMaxFiles, XN_UINT32* pnFoundFiles)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSOpenFile(const XN_CHAR* cpFileName, const XN_UINT32 nFlags, XN_FILE_HANDLE* pFile)
{
	XN_VALIDATE_INPUT_PTR(pFile);

	// Deal with other flags

	// Read mode?
	if (nFlags & XN_OS_FILE_READ)
	{
		*pFile = fopen(cpFileName, "rb");
		if (*pFile == NULL)
			return XN_STATUS_OS_FILE_OPEN_FAILED;
		return XN_STATUS_OK;
	}

	// Write mode?
	if (nFlags & XN_OS_FILE_WRITE == 0)
	{
		// No read and no write? then do what?
		return XN_STATUS_OS_FILE_OPEN_FAILED;
	}

	XN_BOOL bAppend = false;
	// Write mode!
	if (nFlags & XN_OS_FILE_APPEND)
		bAppend = true;

	*pFile = fopen(cpFileName, bAppend ? "ab" : "wb");
	if (*pFile == NULL)
		return XN_STATUS_OS_FILE_OPEN_FAILED;

	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSCloseFile(XN_FILE_HANDLE* pFile)
{
	XN_VALIDATE_INPUT_PTR(pFile);
	XN_RET_IF_NULL(*pFile, XN_STATUS_OS_INVALID_FILE);

	fclose(*pFile);
	*pFile = NULL;

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSReadFile(const XN_FILE_HANDLE File, void* pBuffer, XN_UINT32* pnBufferSize)
{
	XN_VALIDATE_INPUT_PTR(pBuffer);
	XN_VALIDATE_INPUT_PTR(pnBufferSize);

	XN_RET_IF_NULL(File, XN_STATUS_OS_INVALID_FILE);

	*pnBufferSize = fread(pBuffer, 1, *pnBufferSize, File);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSWriteFile(const XN_FILE_HANDLE File, const void* pBuffer, const XN_UINT32 nBufferSize)
{
	XN_VALIDATE_OUTPUT_PTR(pBuffer);
	XN_RET_IF_NULL(File, XN_STATUS_OS_INVALID_FILE);

	fwrite(pBuffer, 1, nBufferSize, File);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSSeekFile(const XN_FILE_HANDLE File, const XnOSSeekType SeekType, const XN_UINT32 nOffset)
{
	XN_RET_IF_NULL(File, XN_STATUS_OS_INVALID_FILE);

	XN_UINT32 nSeekType;
	switch (SeekType)
	{
	case XN_OS_SEEK_CUR:
		nSeekType = SEEK_CUR; break;
	case XN_OS_SEEK_SET:
		nSeekType = SEEK_SET; break;
	case XN_OS_SEEK_END:
		nSeekType = SEEK_END; break;
	default:
		return XN_STATUS_OS_INVALID_SEEK_TYPE;
	}

	fseek(File, nOffset, nSeekType);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSTellFile(const XN_FILE_HANDLE File, XN_UINT32* nFilePos)
{
	XN_VALIDATE_OUTPUT_PTR(nFilePos);
	XN_RET_IF_NULL(File, XN_STATUS_OS_INVALID_FILE);

	*nFilePos = ftell(File);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSFlushFile(const XN_FILE_HANDLE File)
{
	XN_RET_IF_NULL(File, XN_STATUS_OS_INVALID_FILE);

	fflush(File);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSFileExists(const XN_CHAR* cpFileName, XN_BOOL* bResult)
{
	XN_VALIDATE_INPUT_PTR(cpFileName);
	XN_VALIDATE_OUTPUT_PTR(bResult);

	FILE* pTemp = fopen(cpFileName, "rb");

	*bResult = pTemp != NULL;

	if (pTemp != NULL)
		fclose(pTemp);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSGetFileSize(const XN_CHAR* cpFileName, XN_UINT32* pnFileSize)
{
	// Local function variables
	XN_FILE_HANDLE FileHandle;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(cpFileName);
	XN_VALIDATE_OUTPUT_PTR(pnFileSize);

	nRetVal = XnOSOpenFile(cpFileName, XN_OS_FILE_READ, &FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	XnOSSeekFile(FileHandle, XN_OS_SEEK_END, 0);
	XnOSTellFile(FileHandle, pnFileSize);

	nRetVal = XnOSCloseFile(&FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSCreateDirectory(const XN_CHAR* cpDirName)
{
	// Local function variables
	XN_INT32 nRetVal = 0;

	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(cpDirName);

	nRetVal = mkdir(cpDirName, S_IRWXU|S_IRWXG|S_IRWXO);;
	if (nRetVal != 0)
	{
		return (XN_STATUS_OS_FAILED_TO_CREATE_DIR);
	}

	// All is good...
	return (XN_STATUS_OK);
}
