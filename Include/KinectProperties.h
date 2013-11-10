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
#ifndef KINECTPROPERTIES_H
#define KINECTPROPERTIES_H

#include <OniCTypes.h>

/* 
 * private properties of Microsoft Kinect devices.
 *
 * @remarks 
 * properties structure is 0x045eXXYY (045e = Microsoft's USB vendor ID)
 * where XX is range and YY is code.
 * range values:
 * 00 - common stream properties
 * 10 - depth stream properties
 * 20 - color stream properties
 * E0 - device commands
 * F0 - device properties
 */
enum
{
	KINECT_PROPERTY_BASE = 0x045e0000,

	/*******************************************************************/
	/* Common stream properties (00-)                                  */
	/*******************************************************************/

	/*******************************************************************/
	/* Depth stream properties (10-)                                   */
	/*******************************************************************/

	/** OniBool, set and get.
	 * Maps to Near Mode in Kinect SDK.
	 * Also maps to XN_STREAM_PROPERTY_CLOSE_RANGE in PS1080.h.
	 */
	KINECT_DEPTH_PROPERTY_CLOSE_RANGE = KINECT_PROPERTY_BASE + 0x1001,

	/*******************************************************************/
	/* Color stream properties (20-)                                   */
	/*******************************************************************/

	/*******************************************************************/
	/* Device commands (E0-)                                           */
	/*******************************************************************/

	/*******************************************************************/
	/* Device properties (F0-)                                         */
	/*******************************************************************/

	/* 3D sensing properties (F0-) */

	/** OniBool, set and get.
	 * Maps to !NuiGetForceInfraredEmitterOff in Kinect SDK.
	 * Also maps to XN_MODULE_PROPERTY_EMITTER_STATE.
	 */
	KINECT_DEVICE_PROPERTY_EMITTER_STATE = KINECT_PROPERTY_BASE + 0xF001,

	/* Non- 3D sensing bonus properties (F8) */

	/** long, set and get.
	 * Maps to NuiCameraElevationGetAngle and NuiCameraElevationSetAngle in Kinect SDK.
	 */
	KINECT_DEVICE_PROPERTY_CAMERA_ELEVATION = KINECT_PROPERTY_BASE + 0xF801,

	/** KinectVector3f, get only.
	 * Maps to NuiAccelerometerGetCurrentReading.
	 */
	KINECT_DEVICE_PROPERTY_ACCELEROMETER = KINECT_PROPERTY_BASE + 0xF802,

	/** String, get only.
	 * Maps to NuiAudioArrayId.
	 * Useful to find the mic array on the sensor when developing audio-enabled applications.
	 */
	KINECT_DEVICE_PROPERTY_AUDIO_ARRAY_ID = KINECT_PROPERTY_BASE + 0xF803,

};

typedef struct
{
	float x;
	float y;
	float z;
} KinectVector3f;

#endif // KINECTPROPERTIES_H
