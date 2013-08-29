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
#ifndef _XN_EVENT_H_
#define _XN_EVENT_H_

#include "XnOSCpp.h"
#include "XnList.h"

namespace xnl
{

template <typename FuncPtr>
struct CallbackT
{
	CallbackT(FuncPtr func, void* pCookie) : pFunc(func), pCookie(pCookie) {}

	FuncPtr pFunc;
	void* pCookie;
};

template <typename FuncPtr>
class EventInterface
{
public:
	typedef FuncPtr HandlerPtr;
	typedef CallbackT<FuncPtr> Callback;
	typedef EventInterface Interface;

	~EventInterface()
	{
		Clear();
		xnOSCloseCriticalSection(&m_hLock);
		xnOSCloseCriticalSection(&m_hModLock);
	}
	XnStatus Register(FuncPtr pFunc, void* pCookie, XnCallbackHandle& handle)
	{
		XnStatus retVal = XN_STATUS_OK;

		XN_VALIDATE_INPUT_PTR(pFunc);

		Callback* pCallback = NULL;
		pCallback = XN_NEW(Callback, pFunc, pCookie);

		{
			AutoCSLocker locker(m_hModLock);
			retVal = m_toAdd.AddLast(pCallback);
		}

		if (retVal != XN_STATUS_OK)
		{
			XN_DELETE(pCallback);
			return retVal;
		}

		handle = (XnCallbackHandle)pCallback;
		return XN_STATUS_OK;
	}
	XnStatus Unregister(XnCallbackHandle handle)
	{
		XnStatus retVal = XN_STATUS_OK;

		Callback* pCallback = (Callback*)handle;

		{
			AutoCSLocker locker(m_hModLock);
			if (!RemoveCallback(m_toAdd, pCallback))
			{
				retVal = m_toRemove.AddLast(pCallback);
			}
		}
		return retVal;
	}
protected:
	typedef List<Callback*> CallbackPtrList;

	EventInterface()
	{
		Init();
	}
	EventInterface(const EventInterface& other)
	{
		Init();
		*this = other;
	}
	EventInterface& operator=(const EventInterface& other)
	{
		Clear();
		AutoCSLocker otherLocker(other.m_hLock);
		AutoCSLocker otherModLocker(other.m_hModLock);
		AutoCSLocker locker(m_hLock);
		AutoCSLocker modLocker(m_hModLock);

		m_callbacks = other.m_callbacks;
		m_toAdd = other.m_toAdd;
		m_toRemove = other.m_toRemove;

		ApplyListChanges();

		return *this;
	}
	XnStatus Clear()
	{
		AutoCSLocker locker(m_hLock);
		AutoCSLocker modLocker(m_hModLock);

		ApplyListChanges();

		for (typename CallbackPtrList::ConstIterator it = m_callbacks.Begin(); it != m_callbacks.End(); ++it)
		{
			Callback* pCallback = *it;
			XN_DELETE(pCallback);
		}

		m_callbacks.Clear();
		m_toRemove.Clear();
		m_toAdd.Clear();

		return XN_STATUS_OK;
	}

	XnStatus ApplyListChanges()
	{
		AutoCSLocker locker(m_hLock);
		AutoCSLocker modLocker(m_hModLock);

		for (typename CallbackPtrList::ConstIterator it = m_toAdd.Begin(); it != m_toAdd.End(); ++it)
		{
			m_callbacks.AddLast(*it);
		}
		m_toAdd.Clear();

		for (typename CallbackPtrList::ConstIterator it = m_toRemove.Begin(); it != m_toRemove.End(); ++it)
		{
			Callback* pCallback = *it;
			RemoveCallback(m_callbacks, pCallback);
		}
		m_toRemove.Clear();

		return XN_STATUS_OK;
	}

	bool RemoveCallback(CallbackPtrList& list, Callback* pCallback)
	{
		typename CallbackPtrList::Iterator it = list.Find(pCallback);
		if (it != list.End())
		{
			list.Remove(it);
			XN_DELETE(pCallback);
			return true;
		}
		return false;
	}
	XN_CRITICAL_SECTION_HANDLE m_hLock;
	CallbackPtrList m_callbacks;
	CallbackPtrList m_toAdd;
	CallbackPtrList m_toRemove;
private:
	XN_CRITICAL_SECTION_HANDLE m_hModLock;
	void Init()
	{
		m_hLock = NULL;
		XnStatus retVal = xnOSCreateCriticalSection(&m_hLock);
		if (retVal != XN_STATUS_OK)
		{
			//XN_ASSERT(false);
		}

		m_hModLock = NULL;
		retVal = xnOSCreateCriticalSection(&m_hModLock);
		if (retVal != XN_STATUS_OK)
		{
			//XN_ASSERT(false);
		}
	}
};

struct HandlerFuncNoArgs
{
	typedef void (XN_CALLBACK_TYPE* FuncPtr)(void* pCookie);
};

template <class Arg1>
struct HandlerFunc1Arg
{
	typedef void (XN_CALLBACK_TYPE* FuncPtr)(Arg1 arg1, void* pCookie);
};

template <class Arg1, class Arg2>
struct HandlerFunc2Args
{
	typedef void (XN_CALLBACK_TYPE* FuncPtr)(Arg1 arg1, Arg2 arg2, void* pCookie);
};

template <class Arg1, class Arg2, class Arg3>
struct HandlerFunc3Args
{
	typedef void (XN_CALLBACK_TYPE* FuncPtr)(Arg1 arg1, Arg2 arg2, Arg3 arg3, void* pCookie);
};

template <class Arg1, class Arg2, class Arg3, class Arg4>
struct HandlerFunc4Args
{
	typedef void (XN_CALLBACK_TYPE* FuncPtr)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, void* pCookie);
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
struct HandlerFunc5Args
{
	typedef void (XN_CALLBACK_TYPE* FuncPtr)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, void* pCookie);
};

template<class FuncPtr>
class EventBase : public EventInterface<FuncPtr>
{
public:
	using EventInterface<FuncPtr>::Clear;
};

class EventNoArgs : public EventBase<HandlerFuncNoArgs::FuncPtr>
{
public:
	XnStatus Raise()
	{
		AutoCSLocker locker(this->m_hLock);
		ApplyListChanges();

		for (CallbackPtrList::ConstIterator it = m_callbacks.Begin(); it != m_callbacks.End(); ++it)
		{
			Callback* pCallback = *it;
			pCallback->pFunc(pCallback->pCookie);
		}

		ApplyListChanges();
		return XN_STATUS_OK;
	}
};

template <class Arg1>
class Event1Arg : public EventBase<typename HandlerFunc1Arg<Arg1>::FuncPtr>
{
	typedef EventBase<typename HandlerFunc1Arg<Arg1>::FuncPtr> Base;
public:
	XnStatus Raise(Arg1 arg)
	{
		AutoCSLocker locker(this->m_hLock);
		this->ApplyListChanges();

		for (typename Base::CallbackPtrList::ConstIterator it = this->m_callbacks.Begin(); it != this->m_callbacks.End(); ++it)
		{
			typename Base::Callback* pCallback = *it;
			pCallback->pFunc(arg, pCallback->pCookie);
		}

		this->ApplyListChanges();
		return XN_STATUS_OK;
	}
};

template <class EventArgs>
class Event : public Event1Arg<const EventArgs&>
{};

template <class Arg1, class Arg2>
class Event2Args : public EventBase<typename HandlerFunc2Args<Arg1, Arg2>::FuncPtr>
{
	typedef EventBase<typename HandlerFunc2Args<Arg1, Arg2>::FuncPtr> Base;
public:
	XnStatus Raise(Arg1 arg1, Arg2 arg2)
	{
		AutoCSLocker locker(this->m_hLock);
		this->ApplyListChanges();

		for (typename Base::CallbackPtrList::ConstIterator it = this->m_callbacks.Begin(); it != this->m_callbacks.End(); ++it)
		{
			typename Base::Callback* pCallback = *it;
			pCallback->pFunc(arg1, arg2, pCallback->pCookie);
		}

		this->ApplyListChanges();
		return XN_STATUS_OK;
	}
};

template <class Arg1, class Arg2, class Arg3>
class Event3Args : public EventBase<typename HandlerFunc3Args<Arg1, Arg2, Arg3>::FuncPtr>
{
	typedef EventBase<typename HandlerFunc3Args<Arg1, Arg2, Arg3>::FuncPtr> Base;
public:
	XnStatus Raise(Arg1 arg1, Arg2 arg2, Arg3 arg3)
	{
		AutoCSLocker locker(this->m_hLock);
		this->ApplyListChanges();

		for (typename Base::CallbackPtrList::ConstIterator it = this->m_callbacks.Begin(); it != this->m_callbacks.End(); ++it)
		{
			typename Base::Callback* pCallback = *it;
			pCallback->pFunc(arg1, arg2, arg3, pCallback->pCookie);
		}

		this->ApplyListChanges();
		return XN_STATUS_OK;
	}
};

template <class Arg1, class Arg2, class Arg3, class Arg4>
class Event4Args : public EventBase<typename HandlerFunc4Args<Arg1, Arg2, Arg3, Arg4>::FuncPtr>
{
	typedef EventBase<typename HandlerFunc4Args<Arg1, Arg2, Arg3, Arg4>::FuncPtr> Base;
public:
	XnStatus Raise(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
	{
		AutoCSLocker locker(this->m_hLock);
		this->ApplyListChanges();

		for (typename Base::CallbackPtrList::ConstIterator it = this->m_callbacks.Begin(); it != this->m_callbacks.End(); ++it)
		{
			typename Base::Callback* pCallback = *it;
			pCallback->pFunc(arg1, arg2, arg3, arg4, pCallback->pCookie);
		}

		this->ApplyListChanges();
		return XN_STATUS_OK;
	}
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
class Event5Args : public EventBase<typename HandlerFunc5Args<Arg1, Arg2, Arg3, Arg4, Arg5>::FuncPtr>
{
	typedef EventBase<typename HandlerFunc5Args<Arg1, Arg2, Arg3, Arg4, Arg5>::FuncPtr> Base;
public:
	XnStatus Raise(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
	{
		AutoCSLocker locker(this->m_hLock);
		this->ApplyListChanges();

		for (typename Base::CallbackPtrList::ConstIterator it = this->m_callbacks.Begin(); it != this->m_callbacks.End(); ++it)
		{
			typename Base::Callback* pCallback = *it;
			pCallback->pFunc(arg1, arg2, arg3, arg4, arg5, pCallback->pCookie);
		}

		this->ApplyListChanges();
		return XN_STATUS_OK;
	}
};

} // xnl

#endif // _XN_EVENT_H_