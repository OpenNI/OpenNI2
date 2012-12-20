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
#include <XnLib.h>
#include <XnLog.h>

XN_C_API XnStatus xnOSWaitAndTerminateThread(XN_THREAD_HANDLE* pThreadHandle, XnUInt32 nMilliseconds)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pThreadHandle);

	// first, wait for thread to exit on its own
	nRetVal = xnOSWaitForThreadExit(*pThreadHandle, nMilliseconds);
	if (nRetVal != XN_STATUS_OK)
	{
		// thread did not exit on its own. Kill it.
		xnLogWarning(XN_MASK_OS, "Thread did not shutdown in %u ms. Thread will be killed...", nMilliseconds);
		xnOSTerminateThread(pThreadHandle);
	}
	else
	{
		// thread exited. Just close the handle.
		xnOSCloseThread(pThreadHandle);
	}

	return (XN_STATUS_OK);
}

