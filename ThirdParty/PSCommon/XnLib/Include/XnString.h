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
#ifndef _XN_STRING_H_
#define _XN_STRING_H_

#include <XnPlatform.h>
#include "XnMemory.h"

namespace xnl
{

class String
{
	typedef XnChar* Iterator;
	typedef const XnChar* ConstIterator;
	typedef XnChar& Reference;
	typedef const XnChar& ConstReference;
public:
	String() :
	  m_str(NULL),
		m_capacity(0)
	  {}
	String(const XnChar* str) :
	  m_str(NULL), m_capacity(0)
	{
		Initialize(str, (XnUInt32)(strlen(str)+1));
	}
	String(const XnChar* str, XnUInt32 size) :
		m_str(NULL), m_capacity(0)
	{
		Initialize(str, size);
	}
	String(const String& other) : m_str(NULL), m_capacity(0)
	{
		*this = other;
	}
	~String()
	{
		Destroy();
	}

	String& operator=(const String& other)
	{
		Destroy();
		Initialize(other.m_str, other.m_capacity);

		return *this;
	}
	bool operator==(const String& other) const
	{
		if (Size() != other.Size())
			return false;

		for (XnUInt32 i = 0; i < Size(); ++i)
		{
			if (m_str[i] != other[i])
			{
				return false;
			}
		}
		return true;
	}
	bool operator!=(const String& other) const
	{
		return !operator==(other);
	}

	const XnChar* Data() const {return m_str;}

	String operator+(const String& other);
	String& operator+=(const String& other);

	XnUInt32 Size() const {return (XnUInt32)(strlen(m_str));}
	XnUInt32 Capacity() const {return m_capacity;}

	Iterator Begin() {return m_str;}
	ConstIterator Begin() const {return m_str;}
	Iterator End() {return m_str+m_capacity;}
	ConstIterator End() const {return m_str+m_capacity;}

	Reference operator[](int i)
	{
		Resize(i+1);
		return m_str[i];
	}
	ConstReference operator[](int i) const
	{
		return m_str[i];
	}
	void Resize(int newSize)
	{
		if (m_capacity >= (XnUInt32)newSize)
		{
			return;
		}

		XnChar* newData = XN_NEW_ARR(XnChar, newSize);
		if (m_str != NULL)
		{
			strncpy(newData, m_str, m_capacity);
			XN_DELETE_ARR(m_str);
		}
		m_str = newData;
		m_capacity = newSize;
	}

	ConstIterator Find(XnChar c, XnUInt16 which = 1) const
	{
		XnUInt16 found = 0;
		for (ConstIterator iter = Begin(); iter != End(); ++iter)
		{
			if (*iter == c)
			{
				++found;
				if (found == which)
					return iter;
			}
		}
		return End();
	}
private:
	void Initialize(const XnChar* str, XnUInt32 maxCapacity)
	{

		Resize(maxCapacity);
		strncpy(m_str, str, maxCapacity);

	}
	void Destroy()
	{
		if (m_str != NULL)
		{
			XN_DELETE_ARR(m_str);
			m_str = NULL;
		}
		m_capacity = 0;
	}


	XnChar* m_str;
	XnUInt32 m_capacity;
};
} // xnl


#endif // _XN_STRING_H_