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
#ifndef CAPTURE_H
#define CAPTURE_H

// --------------------------------
// Includes
// --------------------------------
#include "Device.h"

// --------------------------------
// Global Variables
// --------------------------------
extern DeviceParameter g_DepthCapturing;
extern DeviceParameter g_ColorCapturing;
extern DeviceParameter g_IRCapturing;

// --------------------------------
// Function Declarations
// --------------------------------
void captureInit();
void captureBrowse(int);
void captureStart(int nDelay);
void captureRestart(int);
void captureStop(int);
bool isCapturing();

void captureSetDepthFormat(int format);
void captureSetColorFormat(int format);
void captureSetIRFormat(int format);
const char* captureGetDepthFormatName();
const char* captureGetColorFormatName();
const char* captureGetIRFormatName();

void captureRun();
void captureSingleFrame(int);

void getCaptureMessage(char* pMessage);

#endif // CAPTURE_H
