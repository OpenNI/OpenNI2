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
#include <XnUSB.h>

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
XnUInt32 g_nRefCount = 0;

//---------------------------------------------------------------------------
// Function Declaration
//---------------------------------------------------------------------------
XnStatus xnUSBPlatformSpecificInit();
XnStatus xnUSBPlatformSpecificShutdown();

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XN_C_API XnStatus xnUSBInit()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (g_nRefCount == 0)
	{
		nRetVal = xnUSBPlatformSpecificInit();
		XN_IS_STATUS_OK(nRetVal);
	}

	++g_nRefCount;

	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnUSBShutdown()
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_ASSERT(g_nRefCount != 0);

	if (--g_nRefCount == 0)
	{
		nRetVal = xnUSBPlatformSpecificShutdown();
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}
