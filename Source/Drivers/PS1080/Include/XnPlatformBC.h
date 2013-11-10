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
#ifndef XNPLATFORMBC_H
#define XNPLATFORMBC_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
#define _XN_DEPRECATED_TYPE(newType, oldType)	\
	XN_API_DEPRECATED("Please use " XN_STRINGIFY(newType) " instead.") typedef newType oldType;

_XN_DEPRECATED_TYPE(XnChar, XN_CHAR)
_XN_DEPRECATED_TYPE(XnUChar, XN_UCHAR)
_XN_DEPRECATED_TYPE(XnInt8, XN_INT8)
_XN_DEPRECATED_TYPE(XnUInt8, XN_UINT8)
_XN_DEPRECATED_TYPE(XnInt16, XN_INT16)
_XN_DEPRECATED_TYPE(XnUInt16, XN_UINT16)
_XN_DEPRECATED_TYPE(XnInt32, XN_INT32)
_XN_DEPRECATED_TYPE(XnUInt32, XN_UINT32)
_XN_DEPRECATED_TYPE(XnInt64, XN_INT64)
_XN_DEPRECATED_TYPE(XnUInt64, XN_UINT64)
_XN_DEPRECATED_TYPE(XnFloat, XN_FLOAT)
_XN_DEPRECATED_TYPE(XnDouble, XN_DOUBLE)
_XN_DEPRECATED_TYPE(XnBool, XN_BOOL)
_XN_DEPRECATED_TYPE(XnWChar, XN_WCHAR)
_XN_DEPRECATED_TYPE(XnInt, XN_LONG)
_XN_DEPRECATED_TYPE(XnUInt, XN_ULONG)

XN_API_DEPRECATED("Please use OniDepthPixel instead") typedef XnUInt16 XN_DEPTH_TYPE; 
XN_API_DEPRECATED("Please use XnRGB24Pixel instead") typedef XnUChar XN_IMAGE_TYPE; 

#endif // XNPLATFORMBC_H
