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
#ifndef _XN_COREGLOBALS_H_
#define _XN_COREGLOBALS_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnCore.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
/** The Xiron I/O INI section name. */ 
#define XN_CORE_INI_SECTION "Core"

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
/** Was the Xiron Core subsystem successfully initialized? */ 
extern XnBool g_bXnCoreWasInit;

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------
/** Make sure the core subsystem was initialized. */ 
#define XN_VALIDATE_CORE_INIT()		\
	if (g_bXnCoreWasInit != TRUE)	\
	{									\
		return (XN_STATUS_NOT_INIT);	\
	}

/** Make sure the core subsystem was not initialized already. */ 
#define XN_VALIDATE_CORE_NOT_INIT()		\
	if (g_bXnCoreWasInit == TRUE)		\
	{										\
		return (XN_STATUS_ALREADY_INIT);	\
	}

#endif //_XN_COREGLOBALS_H_