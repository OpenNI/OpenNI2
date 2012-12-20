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
#ifndef _XN_DEVICE_H_
#define _XN_DEVICE_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <XnStatus.h>
#include <XnDDK.h>
#include <XnStreamParams.h>
#include <XnPropertySet.h>
#include <Core/XnBuffer.h>

typedef XnChar XnConnectionString[XN_DEVICE_MAX_STRING_LENGTH];

/** This structure defines the Xiron device configuration (when opening a new device). */ 
typedef struct XnDeviceConfig
{
	/** The connection string (depending on the device this could mean: file name, IP, sensor serial, etc...). */ 
	const XnChar* cpConnectionString;
	/** Optional. A set of initial values to be used. */
	const XnPropertySet* pInitialValues;
} XnDeviceConfig;

typedef struct XnNewStreamDataEventArgs
{
	const XnChar* strStreamName;
	OniFrame* pFrame;
} XnNewStreamDataEventArgs;

typedef void (XN_CALLBACK_TYPE* XnDeviceOnPropertyChangedEventHandler)(const XnChar* ModuleName, XnUInt32 nPropertyId, void* pCookie);
typedef void (XN_CALLBACK_TYPE* XnDeviceOnNewStreamDataEventHandler)(const XnNewStreamDataEventArgs& args, void* pCookie);


#endif //_XN_DEVICE_H_