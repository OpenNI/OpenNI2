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
#ifndef ONIINTERNAL_H
#define ONIINTERNAL_H

#include "OniPlatform.h"

ONI_C_API void oniWriteLogEntry(const char* mask, int severity, const char* message);

// ONI_C_API OniStatus oniSetLogOutputFolder(const char* strOutputFolder);
// 
// ONI_C_API OniStatus oniGetLogFileName(char* strFileName, const int& nBufferSize);
// 
// ONI_C_API OniStatus oniSetLogMinSeverity(const char* strMask, const int& nMinSeverity);
// 
// ONI_C_API OniStatus oniSetLogConsoleOutput(const OniBool bConsoleOutput);
// 
// ONI_C_API OniStatus oniSetLogFileOutput(const OniBool bFileOutput);
// #if ONI_PLATFORM == ONI_PLATFORM_ANDROID_ARM
// ONI_C_API OniStatus oniSetLogAndroidOutput(OniBool bAndroidOutput);
//#endif
#endif // ONIINTERNAL_H
