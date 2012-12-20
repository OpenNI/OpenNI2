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
#ifndef _XN_PROPERTY_H_
#define _XN_PROPERTY_H_

#include "XnEvent.h"

namespace xnl
{

template <class T>
class Property
{
public:
#ifdef PROPERTY_CB_OLD_NEW
	typedef void (XN_CALLBACK_TYPE*PropertyCallback)(const T& oldValue, const T& newValue, void* pCookie);
#elif defined(PROPERTY_CB_NEW)
	typedef void (XN_CALLBACK_TYPE*PropertyCallback)(const T& newValue, void* pCookie);
#else
	typedef void (XN_CALLBACK_TYPE*PropertyCallback)(void* pCookie);
#endif

	Property(const T& defaultValue) : m_value(defaultValue)
	{}

	const T& Get() const
	{
		return m_value;
	}
	void Set(const T& value)
	{
#ifdef PROPERTY_CB_OLD_NEW
		const T& oldValue = m_value;
		m_value = value;
		m_valueChanged.Raise(oldValue, m_value);
#elif defined(PROPERTY_CB_NEW)
		m_value = value;
		m_valueChanged.Raise(m_value);
#else
		m_value;
		m_valueChanged.Raise();
#endif
	}

	XnStatus RegisterToChange(PropertyCallback callback, void* pCookie, XnCallbackHandle& handle)
	{
		return m_valueChanged.Register(callback, pCookie, handle);
	}
	void UnregisterFromChange(XnCallbackHandle handle)
	{
		m_valueChanged.Unregister(handle);
	}

private:
	// No copies, no empty
	Property();
	Property(const Property& other);
	Property& operator=(const Property& other);

	T m_value;

#ifdef PROPERTY_CB_OLD_NEW
	Event2Arg<T, T> m_valueChanged; // const T& , const T& ??
#elif defined(PROPERTY_CB_NEW)
	Event1Arg<T> m_valueChanged;	// const T& ??
#else
	EventNoArgs m_valueChanged;
#endif
};

} // xnl

#endif //_XN_PROPERTY_H_
