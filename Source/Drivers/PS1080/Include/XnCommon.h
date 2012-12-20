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
#ifndef __XN_COMMON_H__
#define __XN_COMMON_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnStatus.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_VENDOR_PRIMESENSE "PrimeSense"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
typedef enum XnPrimeSenseErrorModules
{
	XN_ERROR_GROUP_SECURITY = 0,
	XN_ERROR_GROUP_FORMATS = 1000,
	XN_ERROR_GROUP_DDK = 2000,
	XN_ERROR_GROUP_DEVICE = 3000,
	XN_ERROR_GROUP_IO = 4000,
	XN_ERROR_GROUP_EE_CORE = 5000,
	XN_ERROR_GROUP_EE_FRAMEWORK = 6000,
	XN_ERROR_GROUP_EE_NITE = 7000,
} XnPrimeSenseErrorModules;

#define XN_PS_STATUS_MESSAGE_MAP_START(module)								\
	XN_STATUS_MESSAGE_MAP_START_FROM(XN_ERROR_GROUP_PRIMESENSE, module)

#define XN_PS_STATUS_MESSAGE_MAP_END(module)								\
	XN_STATUS_MESSAGE_MAP_END_FROM(XN_ERROR_GROUP_PRIMESENSE, module)

#endif // __XN_COMMON_H__
