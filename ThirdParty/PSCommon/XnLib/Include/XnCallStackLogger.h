/*****************************************************************************
*                                                                            *
*  PrimeSense PSCommon Library                                               *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PSCommon.                                            *
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
#ifndef _XN_CALL_STACK_LOGGER_H_
#define _XN_CALL_STACK_LOGGER_H_
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOS.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
namespace xnl
{

#define CALL_STACK_MAX_LENGTH		100
#define CALL_STACK_MESSAGE_LENGTH	100

// Class for logging call stacks.
// Must be declared with following parameters:
//   NUM_ELEMENTS - maximum number of elements that can be logged (must be added first).
//   HISTORY_LENGTH - number of call stack histories to be stored (cyclic buffer).
//   CALL_STACK_DEPTH - maximum depth of call stack (max number of functions in stack).
template <int NUM_ELEMENTS, int HISTORY_LENGTH, int CALL_STACK_DEPTH>
class CallStackLogger
{
public:
	// Single function in stack.
	typedef XnChar XnStackFunction[CALL_STACK_MAX_LENGTH];

	typedef struct
	{
		// Thread identifier.
		XN_THREAD_ID tid;

		// User message.
		XnChar msg[CALL_STACK_MESSAGE_LENGTH];

		// Timestamp.
		XnUInt64 timestamp;

		// Call stack.
		XnStackFunction stack[CALL_STACK_DEPTH];

	} CallStack;

	typedef struct  
	{
		// Pointer to element for which the call stack was created.
		void* pElementPtr;

		// Call stack history (cyclic).
		CallStack history[HISTORY_LENGTH];

		// Index in the history to next empty slot.
		int historyIndex;

	} Element;

public:

	CallStackLogger()
	{
		memset(&m_elements, 0, sizeof(m_elements));
	}

	// Add an element to the list of elements.
	bool Add(void* pElement)
	{
		for (int i = 0; i < NUM_ELEMENTS; ++i)
		{
			if (m_elements[i].pElementPtr == NULL)
			{
				m_elements[i].pElementPtr = pElement;
				return true;
			}
			else if (m_elements[i].pElementPtr == pElement)
			{
				return true;
			}
		}

		return false;
	}

	void Remove(void* pElement)
	{
		for (int i = 0; i < NUM_ELEMENTS; ++i)
		{
			if (m_elements[i].pElementPtr == pElement)
			{
				m_elements[i].pElementPtr = NULL;
				return;
			}
		}
	}

	// Log the call stack. May fail if no more space for element.
	void Log(void* pElement, const char* format, ...)
	{
		// Verify valid element.
		if (pElement == NULL)
		{
			return;
		}

		// Find the element in the list.
		for (int i = 0; i < NUM_ELEMENTS; ++i)
		{
			if (m_elements[i].pElementPtr == pElement)
			{
				CallStack* pCallStack = &m_elements[i].history[m_elements[i].historyIndex];

				// Prepare the pointers to stack funcs.
				XnChar* pstrStackFuncs[CALL_STACK_DEPTH];
				for (int j = 0; j < CALL_STACK_DEPTH; ++j)
				{
					pstrStackFuncs[j] = pCallStack->stack[j];
				}

				// Get the call stack.
				int nFuncs = CALL_STACK_DEPTH;
				xnOSGetCurrentCallStack(1/*self*/, pstrStackFuncs, sizeof(XnStackFunction), &nFuncs);

				// Clear unused stack entries.
				for (int k = nFuncs; k < CALL_STACK_DEPTH; ++k)
				{
					pCallStack->stack[k][0] = '\0';
				}

				// Get the thread ID.
				xnOSGetCurrentThreadID(&pCallStack->tid);

				// Build message string.
				va_list args;
				va_start(args, format);
				vsnprintf(pCallStack->msg, sizeof(pCallStack->msg)-1, format, args);
				va_end(args);

				// Get the timestamp.
				xnOSGetHighResTimeStamp(&pCallStack->timestamp);

				// Increment history
				m_elements[i].historyIndex = (m_elements[i].historyIndex+1) % HISTORY_LENGTH;

				// Mark the next element in history as blank (to make finding wrap-around easier).
				CallStack* pNextCallStack = &m_elements[i].history[m_elements[i].historyIndex];
				pNextCallStack->tid = 0;
				pNextCallStack->msg[0] = '\0';
				pNextCallStack->timestamp = 0;

				break;
			}
		}
	}

public:
	Element m_elements[NUM_ELEMENTS];
};

} // xnl

#endif // _XN_CALL_STACK_LOGGER_H_