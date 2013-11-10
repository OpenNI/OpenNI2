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
#ifndef KEYBOARD_H
#define KEYBOARD_H

// --------------------------------
// Defines
// --------------------------------
#define KEYBOARD_GROUP_PRESETS "Presets"
#define KEYBOARD_GROUP_DISPLAY "Display"
#define KEYBOARD_GROUP_GENERAL "General"
#define KEYBOARD_GROUP_DEVICE "Device"
#define KEYBOARD_GROUP_CAPTURE "Capture"
#define KEYBOARD_GROUP_PLAYER "Player"

// --------------------------------
// Types
// --------------------------------
typedef void (*ActionFunc)(int);
typedef void (*KeyboardInputEnded)(bool ok, const char* userInput);

// --------------------------------
// Function Declarations
// --------------------------------
void startKeyboardMap();
void startKeyboardGroup(const char* csName);
void registerKey(unsigned char key, const char* Description, ActionFunc func, int arg);
void registerSpecialKey(char key, const char* Description, ActionFunc func, int arg);
void endKeyboardGroup();
void endKeyboardMap();
char getRegisteredKey(ActionFunc func, int arg);
int  getRegisteredSpecialKey(ActionFunc func, int arg);
void handleKey(unsigned char key);
void handleSpecialKey(int k);

void getGroupItems(const char* csGroupName, int *pSpecialKeys, unsigned char* pKeys, const char** pDescs, int* pSpecialCount, int* pCount);

void startKeyboardInputMode(const char* message, bool numbersOnly, KeyboardInputEnded callback);
bool isInKeyboardInputMode();
const char* getCurrentKeyboardInputMessage();


#endif // KEYBOARD_H
