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
#include <XnCore/XnCoreGlobals.h>

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
XnOSTimer g_XnOSHighResGlobalTimer;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XN_CORE_API XnStatus XnOSGetEpochTime(XN_UINT32* nEpochTime)
{
	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_OUTPUT_PTR(nEpochTime);

	*nEpochTime = sys_time_get_system_time() / 1000000;

	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSGetTimeStamp(XN_UINT64* nTimeStamp)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;

	// Get the high resolution global timer value
	nRetVal = XnOSGetHighResTimeStamp(nTimeStamp);
	XN_IS_STATUS_OK(nRetVal);

	// Convert from microseconds to milliseconds
	*nTimeStamp /= 1000;

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSGetHighResTimeStamp(XN_UINT64* nTimeStamp)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;

	// Check if the core subsystem is initialized
	XN_VALIDATE_CORE_INIT();	

	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_OUTPUT_PTR(nTimeStamp);

	// Get the high resolution global timer value
	nRetVal = XnOSQueryTimer(g_XnOSHighResGlobalTimer, nTimeStamp);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSSleep(XN_UINT32 nMilliseconds)
{
	sys_timer_usleep(nMilliseconds*1000);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnOSStartTimer(XnOSTimer* pTimer)
{
	XN_VALIDATE_INPUT_PTR(pTimer);

	pTimer->nStartTime = sys_time_get_system_time();
	pTimer->bRunning = true;
	pTimer->nInMicros = 1000;

	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSStartHighResTimer(XnOSTimer* pTimer)
{
	XnStatus rc = XnOSStartTimer(pTimer);
	XN_IS_STATUS_OK(rc);

	pTimer->nInMicros = 1;

	return XN_STATUS_OK;

}

XN_CORE_API XnStatus XnOSQueryTimer(XnOSTimer Timer, XN_UINT64* pnTimeSinceStart)
{
	XN_VALIDATE_OUTPUT_PTR(pnTimeSinceStart);

	if (!Timer.bRunning)
		return XN_STATUS_OS_TIMER_QUERY_FAILED;

	*pnTimeSinceStart = sys_time_get_system_time() - Timer.nStartTime;
	*pnTimeSinceStart /= Timer.nInMicros;

	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSStopTimer(XnOSTimer* pTimer)
{
	XN_VALIDATE_INPUT_PTR(pTimer);

	pTimer->bRunning = false;

	return XN_STATUS_OK;
}
