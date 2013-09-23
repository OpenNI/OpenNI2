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
#include <XnOS.h>
#include <dlfcn.h>
#include <XnLog.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
#if XN_PLATFORM == XN_PLATFORM_ANDROID_ARM
static XnStatus FindLibrary(const XnChar* strLibName, XnChar* cpLibPath, const XnUInt32 nBufferSize)
{
	XnBool bExists;

	// try application libraries
	XnChar strAppLibDir[XN_FILE_MAX_PATH];
	xnOSGetApplicationLibDir(strAppLibDir, sizeof(strAppLibDir));
	snprintf(cpLibPath, nBufferSize, "%s/%s", strAppLibDir, strLibName);
	xnOSDoesFileExist(cpLibPath, &bExists);
	if (bExists)
	{
		return XN_STATUS_OK;
	}

	// otherwise, try current dir
	snprintf(cpLibPath, nBufferSize, "./%s", strLibName);
	xnOSDoesFileExist(cpLibPath, &bExists);
	if (bExists)
	{
		return XN_STATUS_OK;
	}

	// otherwise, try system dir
	snprintf(cpLibPath, nBufferSize, "/system/lib/%s", strLibName);
	xnOSDoesFileExist(cpLibPath, &bExists);
	if (bExists)
	{
		return XN_STATUS_OK;
	}

	return XN_STATUS_OS_FILE_NOT_FOUND;
}
#endif

XN_C_API XnStatus xnOSLoadLibrary(const XnChar* cpFileName, XN_LIB_HANDLE* pLibHandle)
{
	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(cpFileName);
	XN_VALIDATE_OUTPUT_PTR(pLibHandle);

	XnChar strLibPath[XN_FILE_MAX_PATH];
	
#if XN_PLATFORM == XN_PLATFORM_ANDROID_ARM
	XnStatus nRetVal = FindLibrary(cpFileName, strLibPath, sizeof(strLibPath));
	if (nRetVal != XN_STATUS_OK)
	{
		return nRetVal;
	}
#else
	// Resolve the file name to the absolute path. This is necessary because
	// we need to get the absolute path of this library by dladdr() later.
	// Note dladdr() seems to return the path specified to dlopen() "as it is".
	if (NULL == realpath(cpFileName, strLibPath))
	{
		// error
		xnLogWarning(XN_MASK_OS, "Failed to get absolute path for lib: %s\n", cpFileName);
		return XN_STATUS_OS_CANT_LOAD_LIB;
	}
#endif

	// Load the requested shared library via the OS
	*pLibHandle = dlopen(strLibPath, RTLD_NOW);

	// Make sure it succeeded (return value is not NULL). If not return an error....
	if (*pLibHandle == NULL)
	{
		xnLogWarning(XN_MASK_OS, "Failed loading lib: %s\n", dlerror());
		return XN_STATUS_OS_CANT_LOAD_LIB;
	}

	// All is good...
	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnOSFreeLibrary(const XN_LIB_HANDLE LibHandle)
{
	// Make sure the actual shared library handle isn't NULL
	XN_RET_IF_NULL(LibHandle, XN_STATUS_OS_INVALID_LIBRARY);

	// Free the shared library via the OS and make sure it succeeded
	if (dlclose(LibHandle) != 0)
	{
		return (XN_STATUS_OS_CANT_FREE_LIB);
	}

	// All is good...
	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnOSGetProcAddress(const XN_LIB_HANDLE LibHandle, const XnChar* cpProcName, XnFarProc* pProcAddr)
{
	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(cpProcName);
	XN_VALIDATE_OUTPUT_PTR(pProcAddr);

	// Make sure the actual shared library handle isn't NULL
	XN_RET_IF_NULL(LibHandle, XN_STATUS_OS_INVALID_LIBRARY);

	// Get the requested procedure address from the shared library via the OS
	*pProcAddr = (XnFarProc)dlsym(LibHandle, cpProcName);

	// Make sure it succeeded (return value is not NULL). If not return an error....
	XN_VALIDATE_PTR(*pProcAddr, XN_STATUS_OS_PROC_NOT_FOUND);

	// All is good...
	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnOSGetModulePathForProcAddress(void* procAddr, XnChar *strModulePath)
{
	Dl_info info;
	if (!dladdr(procAddr, &info))
	{
		xnLogWarning(XN_MASK_OS, "Failed to get the dl info: %s\n", dlerror());
		return XN_STATUS_ERROR;
	}

	return xnOSStrCopy(strModulePath, info.dli_fname, XN_FILE_MAX_PATH);
}
