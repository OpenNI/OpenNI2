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
#undef XN_CROSS_PLATFORM
#include <XnOS.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XN_CORE_API XnStatus XnOSCreateMutex(XN_MUTEX_HANDLE* pMutexHandle)
{
	return XnOSCreateNamedMutex(pMutexHandle, NULL);
}

XN_CORE_API XnStatus XnOSCreateNamedMutex(XN_MUTEX_HANDLE* pMutexHandle, const XN_CHAR* cpMutexName)
{
	XN_VALIDATE_INPUT_PTR(pMutexHandle);

	XN_VALIDATE_ALLOC(*pMutexHandle, sys_lwmutex_t);

	sys_lwmutex_attribute_t attr;
	sys_lwmutex_attribute_initialize(attr);
	if (cpMutexName != NULL)
	{
		sys_lwmutex_attribute_name_set(attr.name, cpMutexName);
	}


	int nErr = sys_lwmutex_create(*pMutexHandle, &attr);

	if (nErr != CELL_OK)
	{
		return XN_STATUS_OS_MUTEX_CREATION_FAILED;
	}
	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSCloseMutex(XN_MUTEX_HANDLE* pMutexHandle)
{
	XN_VALIDATE_INPUT_PTR(pMutexHandle);

	XN_RET_IF_NULL(*pMutexHandle, XN_STATUS_OS_INVALID_MUTEX);

	int nErr = sys_lwmutex_destroy(*pMutexHandle);

	XN_FREE_AND_NULL(*pMutexHandle);

	if (nErr != CELL_OK)
	{
		return XN_STATUS_OS_MUTEX_CLOSE_FAILED;
	}
	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSLockMutex(const XN_MUTEX_HANDLE MutexHandle, XN_UINT32 nMilliseconds)
{
	XN_VALIDATE_INPUT_PTR(MutexHandle);

	XN_RET_IF_NULL(MutexHandle, XN_STATUS_OS_INVALID_MUTEX);

	int nErr = sys_lwmutex_lock(MutexHandle, nMilliseconds*1000);

	if (nErr != CELL_OK)
	{
		return XN_STATUS_OS_MUTEX_LOCK_FAILED;
	}
	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSUnLockMutex(const XN_MUTEX_HANDLE MutexHandle)
{
	XN_VALIDATE_INPUT_PTR(MutexHandle);
	XN_RET_IF_NULL(MutexHandle, XN_STATUS_OS_INVALID_MUTEX);
	
	int nErr = sys_lwmutex_unlock(MutexHandle);

	if (nErr != CELL_OK)
	{
		return XN_STATUS_OS_MUTEX_UNLOCK_FAILED;
	}
	return XN_STATUS_OK;
}
