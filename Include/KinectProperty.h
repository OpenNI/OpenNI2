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
#ifndef _KINECT_PROPERTY_H_
#define _KINECT_PROPERTY_H_

#include <OniCTypes.h>

/* 
 * private properties of Microsoft Kinect devices.
 *
 * @remarks 
 * properties structure is 0x0360XXYY where XX is range and YY is code.
 * range values:
 * F0 - device properties
 * E0 - device commands
 * 00 - common stream properties
 * 10 - depth stream properties
 * 20 - color stream properties
 */
enum
{
	/*******************************************************************/
	/* Device properties                                               */
	/*******************************************************************/

	/** long, set and get */
	KINECT_DEVICE_PROPERTY_CAMERA_ELEVATION = 0x0360E001, // NuiCameraElevationGetAngle and NuiCameraElevationSetAngle

	/** KVector4, get */
	KINECT_DEVICE_PROPERTY_ACCELEROMETER = 0x0360E02, // NuiAccelerometerGetCurrentReading

	/*******************************************************************/
	/* Common stream properties                                        */
	/*******************************************************************/


	/*******************************************************************/
	/* Depth stream properties                                         */
	/*******************************************************************/

	/** OniBool, set and get */
	KINECT_DEPTH_PROPERTY_NEAR_MODE = 0x03601001,	// Near mode

	/*******************************************************************/
	/* Color stream properties                                         */
	/*******************************************************************/
};

typedef struct _KVector4
{
	float x;
	float y;
	float z;
	float w;
} 	KVector4;

#endif //#define _KINECT_PROPERTY_H_

