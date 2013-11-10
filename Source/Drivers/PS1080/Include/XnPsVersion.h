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
#ifndef XNPSVERSION_H
#define XNPSVERSION_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
/** Xiron major version. */ 
#define XN_PS_MAJOR_VERSION 5
/** Xiron minor version. */ 
#define XN_PS_MINOR_VERSION 2
/** Xiron maintenance version. */ 
#define XN_PS_MAINTENANCE_VERSION 0
/** Xiron build version. */ 
#define XN_PS_BUILD_VERSION 0

/** Xiron version (in brief string format): "Major.Minor.Maintenance (Build)" */ 
#define XN_PS_BRIEF_VERSION_STRING \
	XN_STRINGIFY(XN_PS_MAJOR_VERSION) "." \
	XN_STRINGIFY(XN_PS_MINOR_VERSION) "." \
	XN_STRINGIFY(XN_PS_MAINTENANCE_VERSION) \
	" (Build " XN_STRINGIFY(XN_PS_BUILD_VERSION) ")"

/** Xiron version (in numeric format): (Xiron major version * 100000000 + Xiron minor version * 1000000 + Xiron maintenance version * 10000 + Xiron build version). */
#define XN_PS_VERSION (XN_PS_MAJOR_VERSION*100000000 + XN_PS_MINOR_VERSION*1000000 + XN_PS_MAINTENANCE_VERSION*10000 + XN_PS_BUILD_VERSION)

/** Xiron version (in string format): "Major.Minor.Maintenance.Build-Platform (MMM DD YYYY HH:MM:SS)". */ 
#define XN_PS_VERSION_STRING \
		XN_PS_BRIEF_VERSION_STRING  "-" \
		XN_PLATFORM_STRING " (" XN_TIMESTAMP ")"

#endif // XNPSVERSION_H

