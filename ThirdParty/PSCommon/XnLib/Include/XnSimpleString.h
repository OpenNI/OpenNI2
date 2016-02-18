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
#ifndef _XN_SIMPLE_STRING_H_
#define _XN_SIMPLE_STRING_H_

#include "XnOSStrings.h"

namespace xnl
{

template<int LEN>
class SimpleString
{
public:
	SimpleString() 
	{
		m_data[0] = '\0';
	}

	~SimpleString()
	{}

	SimpleString(const char* str) 
	{
		xnOSStrCopy(m_data, str, sizeof(m_data));
	}

	SimpleString(const SimpleString& other)
	{
		xnOSStrCopy(m_data, other.m_data, sizeof(m_data));
	}

	SimpleString& operator=(const SimpleString& other)
	{
		xnOSStrCopy(m_data, other.m_data, sizeof(m_data));
		return *this;
	}

	bool operator==(const SimpleString& other) const
	{
		char* pThis = m_data;
		char* pOther = other.m_data;

		for (;;)
		{
			if (*pThis != *pOther)
				return false;

			if (*pThis == '\0')
				return true;

			++pThis;
			++pOther;
		}
	}

	bool operator!=(const SimpleString& other) const
	{
		return !operator==(other);
	}

	char& operator[](int i)
	{
		return m_data[i];
	}
	const char& operator[](int i) const
	{
		return m_data[i];
	}

	XnUInt32 length() { return xnOSStrLen(m_data); }

	char* getData() { return m_data; }
	const char* getData() const { return m_data; }

private:
	char m_data[LEN];
};

typedef SimpleString<XN_FILE_MAX_PATH> FileName;

}

#endif //_XN_SIMPLE_STRING_H_