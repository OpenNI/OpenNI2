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
#ifndef LINKPROTOLIBVERSION_H
#define LINKPROTOLIBVERSION_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define LINK_PROTO_LIB_MAJOR_VERSION 0
#define LINK_PROTO_LIB_MINOR_VERSION 25

#define LINK_PROTO_LIB_BRIEF_VERSION_STRING \
	XN_STRINGIFY(LINK_PROTO_LIB_MAJOR_VERSION) "." \
	XN_STRINGIFY(LINK_PROTO_LIB_MINOR_VERSION)

#define LINK_PROTO_LIB_VERSION (LINK_PROTO_LIB_MAJOR_VERSION * 100 + LINK_PROTO_LIB_MINOR_VERSION)

#define LINK_PROTO_LIB_VERSION_STRING \
	LINK_PROTO_LIB_BRIEF_VERSION_STRING  "-" \
	XN_PLATFORM_STRING " (" XN_TIMESTAMP ")"

#endif // LINKPROTOLIBVERSION_H
