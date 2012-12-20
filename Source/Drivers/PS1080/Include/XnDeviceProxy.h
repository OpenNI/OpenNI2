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
#ifndef __XN_DEVICE_PROXY_H__
#define __XN_DEVICE_PROXY_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>

//---------------------------------------------------------------------------
// Exported Functions
//---------------------------------------------------------------------------

/*****************************/
/* XnDevice interface		 */
/*****************************/
#define XN_DEVICE_PROXY_PROTO_APPEND(prefix, name) prefix ## name
#define XN_DEVICE_PROXY_PROTO(name) XN_DEVICE_PROXY_PROTO_APPEND(XnDeviceProxy, name)
#define XN_DEVICE_INTERFACE_FUNCTION(name, sig)	XN_DDK_API XnStatus XN_DEVICE_PROXY_PROTO(name)sig;
#include <XnDeviceProto.inl>
#undef XN_DEVICE_INTERFACE_FUNCTION

/****************************/
/* Specific Proxy Functions */
/****************************/

/**
* Gets a list of supported devices, meaning, devices loaded by device manager.
*
* @param	aDeviceDefinitions	[in]		An array of XnDeviceDefinition to be filled with information.
* @param	pnCount				[in/out]	In: the size of the array. Out: the number of elements filled in the array.
*/
XN_DDK_API XnStatus XnDeviceProxyGetDeviceList(XnDeviceDefinition* aDeviceDefinitions, XnUInt32* pnCount);

/**
* Enumerates a specific device, by device name. 
*
* @param	csDeviceName			[in]		The name of the device to enumerate.
* @param	aConnectionStrings		[in]		An array to be filled with connection strings.
* @param	pnCount					[in/out]	In: the size of the array. Out: the number of elements filled in the array.
*/
XN_DDK_API XnStatus XnDeviceProxyEnumerateDeviceByName(const XnChar* csDeviceName, XnConnectionString* aConnectionStrings, XnUInt32* pnCount);

/**
* Creates a device by name.
*
* @param	csDeviceName			[in]		The name of the device to create. The special value "Auto" will create any available device.
* @param	pDeviceHandle			[out]		The opened device handle. If the function fails, NULL is returned.
* @param	pDeviceConfig			[in]		The requested device configuration mode. Contains the mode (read/write) and the target connection string.
*/
XN_DDK_API XnStatus XnDeviceProxyCreateDeviceByName(const XnChar* csDeviceName, XnDeviceHandle* pDeviceHandle, const XnDeviceConfig* pDeviceConfig);

/**
* Creates a device by definitions in INI file.
*
* @param	strIniFileName			[in]		INI file to use for initialization.
* @param	strSectionName			[in]		section name in INI file that describes the device.
* @param	pDeviceHandle			[out]		The opened device handle. If the function fails, NULL is returned.
* @param	pInitialValues			[in]		Optional. A set of initial values to be used.
*/
XN_DDK_API XnStatus XnDeviceProxyCreateDeviceByINIFile(const XnChar* strIniFileName, const XnChar* strSectionName, XnDeviceHandle* pDeviceHandle, const XnPropertySet* pInitialValues);

/**
* Destroys a stream output object that was previously created using CreateStreamOutput.
*
* @param	csDeviceName	[in]	The name of the device that created this object.
* @param	ppStreamOutput	[in]	The stream output object to destroy.
*/
XN_DDK_API XnStatus XnDeviceProxyDestroyStreamOutputByName(const XnChar* csDeviceName, XnStreamData** ppStreamOutput);

/**
* Gets the name of an opened device.
*
* @param	DeviceHandle	[in]		The requested device handle.
* @param	csDeviceName	[in/out]	A string to be filled with its name. The buffer must be at least XN_DEVICE_MAX_STRING_LENGTH long.
*/
XN_DDK_API XnStatus XnDeviceProxyGetDeviceName(XnDeviceHandle DeviceHandle, XnChar* csDeviceName);

#endif //__XN_DEVICE_PROXY_H__