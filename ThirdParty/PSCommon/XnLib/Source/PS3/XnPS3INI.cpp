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
XN_CORE_API XnStatus XnOSReadStringFromINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, XN_CHAR* cpDest, const XN_UINT32 nDestLength)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSReadFloatFromINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, XN_FLOAT* fDest)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSReadDoubleFromINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, XN_DOUBLE* fDest)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSReadIntFromINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, XN_UINT32* nDest)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSWriteStringToINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, const XN_CHAR* cpSrc)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSWriteFloatToINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, const XN_FLOAT fSrc)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSWriteDoubleToINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, const XN_DOUBLE fSrc)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnOSWriteIntToINI(const XN_CHAR* cpINIFile, const XN_CHAR* cpSection, const XN_CHAR* cpKey, const XN_UINT32 nSrc)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}
