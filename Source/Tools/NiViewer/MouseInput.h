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
// --------------------------------
// Types
// --------------------------------
#ifndef MOUSEINPUT_H
#define MOUSEINPUT_H

typedef struct
{
	int X;
	int Y;
} IntPair;

typedef struct
{
	double X;
	double Y;
} DoublePair;

typedef struct 
{
	double fBottom;
	double fLeft;
	double fTop;
	double fRight;
} DoubleRect;

typedef struct  
{
	int uBottom;
	int uLeft;
	int uTop;
	int uRight;
} IntRect;

typedef enum SelectionState
{
	SELECTION_NONE,
	SELECTION_ACTIVE,
	SELECTION_DONE
} SelectionState;

typedef void (*SelectionRectangleChangedPtr)(SelectionState state, IntRect selection);
typedef void (*CursorMovedPtr)(IntPair location);

// --------------------------------
// Functions
// --------------------------------
void mouseInputMotion(int x, int y);
void mouseInputButton(int button, int state, int x, int y);
void mouseInputRegisterForSelectionRectangle(SelectionRectangleChangedPtr pFunc);
void mouseInputRegisterForCursorMovement(CursorMovedPtr pFunc);



#endif // MOUSEINPUT_H
