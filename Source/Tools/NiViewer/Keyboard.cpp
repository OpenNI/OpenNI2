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
// Includes
// --------------------------------
#include "Keyboard.h"
#include <string.h>
#include <XnOS.h>

// --------------------------------
// Types
// --------------------------------
typedef struct XnKeyboardAction
{
	char key;
	const char* csDescription;
	ActionFunc pCallbackFunc;
	int nCallbackArg;
} XnKeyboardAction;

typedef struct XnKeyboardGroup
{
	const char* csName;
	int nFirst;
	int nLast;
	int nSpecialFirst;
	int nSpecialLast;
} XnKeyboardGroup;

// --------------------------------
// Global Variables
// --------------------------------
XnKeyboardAction g_KeyboardMap[500];
XnKeyboardAction g_KeyboardSpecialMap[100];
XnKeyboardGroup g_Groups[20];
int g_nRegisteredKeys = 0;
int g_nRegisteredSpecialKeys = 0;
int g_nRegisteredGroups = 0;

static bool g_bUserInput = false;
static bool g_bUserInputNumbersOnly = false;
static char g_strUserInput[1024];
static int g_userInputStartPos = 0;
static KeyboardInputEnded g_userInputCallback;

// --------------------------------
// Code
// --------------------------------
void startKeyboardMap()
{
	g_nRegisteredKeys = 0;
}

void startKeyboardGroup(const char* csName)
{
	g_Groups[g_nRegisteredGroups].csName = csName;
	g_Groups[g_nRegisteredGroups].nFirst = g_nRegisteredKeys;
	g_Groups[g_nRegisteredGroups].nSpecialFirst = g_nRegisteredSpecialKeys;
}

void registerKey(unsigned char key, const char* Description, ActionFunc func, int arg)
{
	XnKeyboardAction* pKey = &g_KeyboardMap[g_nRegisteredKeys++];
	pKey->key = key;
	pKey->csDescription = Description;
	pKey->pCallbackFunc = func;
	pKey->nCallbackArg = arg;
}

void registerSpecialKey(char key, const char* Description, ActionFunc func, int arg)
{
	XnKeyboardAction* pKey = &g_KeyboardSpecialMap[g_nRegisteredSpecialKeys++];
	pKey->key = key;
	pKey->csDescription = Description;
	pKey->pCallbackFunc = func;
	pKey->nCallbackArg = arg;
}

void endKeyboardGroup()
{
	g_Groups[g_nRegisteredGroups].nLast = g_nRegisteredKeys;
	g_Groups[g_nRegisteredGroups].nSpecialLast = g_nRegisteredSpecialKeys;
	g_nRegisteredGroups++;
}

void endKeyboardMap()
{

}

char getRegisteredKey(ActionFunc func, int arg)
{
	for (int i = 0; i < g_nRegisteredKeys; ++i)
	{
		if (g_KeyboardMap[i].pCallbackFunc == func && g_KeyboardMap[i].nCallbackArg == arg)
			return g_KeyboardMap[i].key;
	}

	return 0;
}

int getRegisteredSpecialKey(ActionFunc func, int arg)
{
	for (int i = 0; i < g_nRegisteredKeys; ++i)
	{
		if (g_KeyboardSpecialMap[i].pCallbackFunc == func && g_KeyboardSpecialMap[i].nCallbackArg == arg)
			return g_KeyboardSpecialMap[i].key;
	}

	return 0;
}

void handleKey(unsigned char k)
{
	if (g_bUserInput)
	{
		if (k == 13 || k == 27) // ENTER or ESC
		{
			if (g_userInputCallback != NULL)
			{
				g_userInputCallback(k == 13, &g_strUserInput[g_userInputStartPos]);
			}

			g_bUserInput = false;
		}

		if (g_bUserInputNumbersOnly && (k < '0' || k > '9'))
		{
			// ignore
			return;
		}

		int len = (int)strlen(g_strUserInput);
		g_strUserInput[len] = k;
		g_strUserInput[++len] = '\0';

		return;
	}

	for (int i = 0; i < g_nRegisteredKeys; ++i)
	{
		if (k == g_KeyboardMap[i].key)
		{
			g_KeyboardMap[i].pCallbackFunc(g_KeyboardMap[i].nCallbackArg);
			return;
		}
	}
}

void handleSpecialKey(int k)
{
	for (int i = 0; i < g_nRegisteredSpecialKeys; ++i)
	{
		if (k == g_KeyboardSpecialMap[i].key)
		{
			g_KeyboardSpecialMap[i].pCallbackFunc(g_KeyboardSpecialMap[i].nCallbackArg);
			return;
		}
	}
}

int findGroup(const char* csGroupName)
{
	for (int i = 0; i < g_nRegisteredGroups; ++i)
	{
		if (strcmp(g_Groups[i].csName, csGroupName) == 0)
			return i;
	}

	return -1;
}

void getGroupItems(const char* csGroupName, int *pSpecialKeys, unsigned char* pKeys, const char** pDescs, int* pSpecialCount, int* pCount)
{
	// find group
	int nGroup = findGroup(csGroupName);

	int nCount = 0;
	for (int nEntry = g_Groups[nGroup].nSpecialFirst; nEntry < g_Groups[nGroup].nSpecialLast; nEntry++, nCount++)
	{
		pSpecialKeys[nCount] = g_KeyboardSpecialMap[nEntry].key;
		pDescs[nCount]       = g_KeyboardSpecialMap[nEntry].csDescription;
	}
	*pSpecialCount = nCount;

	nCount = 0;
	for (int nEntry = g_Groups[nGroup].nFirst; nEntry < g_Groups[nGroup].nLast; nEntry++, nCount++)
	{
		pKeys[nCount] = g_KeyboardMap[nEntry].key;
		pDescs[nCount + *pSpecialCount] = g_KeyboardMap[nEntry].csDescription;
	}

	*pCount = nCount;
}

void startKeyboardInputMode(const char* message, bool numbersOnly, KeyboardInputEnded callback)
{
	g_bUserInput = true;
	g_bUserInputNumbersOnly = numbersOnly;
	xnOSStrCopy(g_strUserInput, message, sizeof(g_strUserInput));
	g_userInputStartPos = (int)strlen(message);
	g_userInputCallback = callback;
}

const char* getCurrentKeyboardInputMessage()
{
	return g_strUserInput;
}

bool isInKeyboardInputMode()
{
	return g_bUserInput;
}
