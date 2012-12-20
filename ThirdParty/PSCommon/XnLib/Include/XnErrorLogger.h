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
#ifndef _XN_ERROR_LOGGER_H_
#define _XN_ERROR_LOGGER_H_

#include "XnLib.h"
#include "XnHash.h"
#include "XnOSCpp.h"

namespace xnl
{

class ErrorLogger
{
public:
	~ErrorLogger();

	static ErrorLogger& GetInstance();
	void Clear();

	void Append(const XnChar* cpFormat, ...);

	void AppendV(const XnChar* cpFormat, va_list args);

	const char* GetExtendedError() {return getBuffer()->m_errorBuffer;}
protected:
	static const int ms_bufferSize = 1024;

#if XN_PLATFORM == XN_PLATFORM_WIN32
	struct SingleBuffer
	{
		SingleBuffer() : m_currentEnd(0)
		{
			xnOSMemSet(m_errorBuffer, 0, ms_bufferSize);
		}
		char m_errorBuffer[ms_bufferSize];
		int m_currentEnd;
	};

	xnl::Hash<XN_THREAD_ID, SingleBuffer*> m_buffers;
	xnl::CriticalSection m_bufferLock;
#else
	typedef ErrorLogger SingleBuffer;
	static XN_THREAD_STATIC char m_errorBuffer[ms_bufferSize];
	static XN_THREAD_STATIC int m_currentEnd;
#endif
	SingleBuffer* getBuffer();
private:
	ErrorLogger();

};

}

#endif
