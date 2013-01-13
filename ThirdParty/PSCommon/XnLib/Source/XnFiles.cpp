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
#include "XnLib.h"

XN_C_API XnStatus xnOSLoadFile(const XnChar* cpFileName, void* pBuffer, const XnUInt32 nBufferSize)
{
	// Local function variables
	XN_FILE_HANDLE FileHandle;
	XnUInt32 nReadBytes = nBufferSize;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(cpFileName);
	XN_VALIDATE_OUTPUT_PTR(pBuffer);

	if (nBufferSize == 0)
	{
		return XN_STATUS_NULL_OUTPUT_PTR;
	}

	nRetVal = xnOSOpenFile(cpFileName, XN_OS_FILE_READ, &FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSReadFile(FileHandle, pBuffer, &nReadBytes);
	if ((nRetVal != XN_STATUS_OK) || (nReadBytes != nBufferSize))
	{
		xnOSCloseFile(&FileHandle);
		return (XN_STATUS_OS_FILE_READ_FAILED);
	}

	nRetVal = xnOSCloseFile(&FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnOSSaveFile(const XnChar* cpFileName, const void* pBuffer, const XnUInt32 nBufferSize)
{
	// Local function variables
	XN_FILE_HANDLE FileHandle;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(cpFileName);
	XN_VALIDATE_INPUT_PTR(pBuffer);

	nRetVal = xnOSOpenFile(cpFileName, XN_OS_FILE_WRITE | XN_OS_FILE_TRUNCATE, &FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSWriteFile(FileHandle, pBuffer, nBufferSize);
	if (nRetVal != XN_STATUS_OK)
	{
		xnOSCloseFile(&FileHandle);
		return (XN_STATUS_OS_FILE_WRITE_FAILED);
	}

	nRetVal = xnOSCloseFile(&FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnOSAppendFile(const XnChar* cpFileName, const void* pBuffer, const XnUInt32 nBufferSize)
{
	// Local function variables
	XN_FILE_HANDLE FileHandle;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(cpFileName);
	XN_VALIDATE_INPUT_PTR(pBuffer);

	nRetVal = xnOSOpenFile(cpFileName, XN_OS_FILE_WRITE | XN_OS_FILE_APPEND, &FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSWriteFile(FileHandle, pBuffer, nBufferSize);
	if (nRetVal != XN_STATUS_OK)
	{
		xnOSCloseFile(&FileHandle);
		return (XN_STATUS_OS_FILE_WRITE_FAILED);
	}

	nRetVal = xnOSCloseFile(&FileHandle);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}
