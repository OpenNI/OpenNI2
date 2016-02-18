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
#ifndef DRAW_H
#define DRAW_H

// --------------------------------
// Includes
// --------------------------------
#define _CRT_SECURE_NO_DEPRECATE 1
#include <GL/gl.h>

// --------------------------------
// Defines
// --------------------------------
#define PRESET_COUNT 13

// --------------------------------
// Types
// --------------------------------
typedef enum
{
	OVERLAY,
	SIDE_BY_SIDE
} ScreenArrangementType;

typedef enum 
{
	DEPTH_OFF,
	LINEAR_HISTOGRAM,
	PSYCHEDELIC,
	PSYCHEDELIC_SHADES,
	RAINBOW,
	CYCLIC_RAINBOW,
	CYCLIC_RAINBOW_HISTOGRAM,
	STANDARD_DEVIATION,
	NUM_OF_DEPTH_DRAW_TYPES,
} DepthDrawColoringType;

extern const char* g_DepthDrawColoring[NUM_OF_DEPTH_DRAW_TYPES];

typedef enum
{
	COLOR_OFF,
	COLOR_NORMAL,
	DEPTH_MASKED_COLOR,
	NUM_OF_COLOR_DRAW_TYPES,
} ColorDrawColoringType;

extern const char* g_ColorDrawColoring[NUM_OF_COLOR_DRAW_TYPES];

typedef struct  
{
	DepthDrawColoringType Coloring;
	float fTransparency;
} DepthDrawConfig;

typedef struct
{
	ColorDrawColoringType Coloring;
} ImageDrawConfig;

typedef struct
{
	DepthDrawConfig Depth;
	ImageDrawConfig Color;
	ScreenArrangementType ScreenArrangement;
} StreamsDrawConfig;

// --------------------------------
// Drawing functions
// --------------------------------
void drawInit();
void displayMessage(const char* csFormat, ...);
void displayError(const char* csFormat, ...);
void setPreset(int preset);
const char* getPresetName(int preset);
void setScreenLayout(int layout);
void toggleFullScreen(int);
void togglePointerMode(int);
void toggleHelpScreen(int);
void drawFrame();
void windowReshaped(int width, int height);
void setDepthDrawing(int nColoring);
void setColorDrawing(int nColoring);
void setErrorState(const char* strFormat, ...);
void resetIRHistogram(int /*dummy*/);

#endif // DRAW_H
