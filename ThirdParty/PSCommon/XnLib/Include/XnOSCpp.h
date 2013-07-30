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
#ifndef _XN_OS_CPP_H_
#define _XN_OS_CPP_H_

#include "XnLib.h"

namespace xnl
{

class NoLock
{
public:
	void Lock() {}
	void Unlock() {}
};

class CriticalSection
{
public:
	CriticalSection()
	{
		xnOSCreateCriticalSection(&m_cs);
	}
	~CriticalSection()
	{
		Unlock();
		xnOSCloseCriticalSection(&m_cs);
	}

	void Lock()
	{
		xnOSEnterCriticalSection(&m_cs);
	}
	void Unlock()
	{
		xnOSLeaveCriticalSection(&m_cs);
	}
private:
	CriticalSection(const CriticalSection& other);
	CriticalSection& operator=(const CriticalSection& other);

	friend class AutoCSLocker;

	XN_CRITICAL_SECTION_HANDLE m_cs;
};

class Mutex
{
public:
	Mutex()
	{
		xnOSCreateMutex(&m_mutex);
	}
	Mutex(const char* name)
	{
		xnOSCreateNamedMutex(&m_mutex, name);
	}
	~Mutex()
	{
		Unlock();
		xnOSCloseMutex(&m_mutex);
	}
	XnStatus Lock(XnUInt32 milliseconds = XN_WAIT_INFINITE)
	{
		return xnOSLockMutex(m_mutex, milliseconds);
	}
	void Unlock()
	{
		xnOSUnLockMutex(m_mutex);
	}
private:
	Mutex(const Mutex& other);
	Mutex& operator=(const Mutex& other);

	friend class AutoMutexLocker;
	XN_MUTEX_HANDLE m_mutex;
};

template<bool TThreadSafe>
class VirtualLock {};

template<>
class VirtualLock<true> : public CriticalSection {};

template<>
class VirtualLock<false> : public NoLock {};

class AutoMutexLocker
{
public:
	inline AutoMutexLocker(XN_MUTEX_HANDLE hMutex, XnUInt32 nMilliseconds) : m_hMutex(hMutex)
	{
		Lock(nMilliseconds);
	}
	AutoMutexLocker(const Mutex& mutex, XnUInt32 nMilliseconds) : m_hMutex(mutex.m_mutex)
	{
		Lock(nMilliseconds);
	}
	XnStatus GetStatus() const
	{
		return m_nStatus;
	}

	inline ~AutoMutexLocker()
	{
		Unlock();
	}
private:
	void Lock(XnUInt32 milliseconds)
	{
		m_nStatus = xnOSLockMutex(m_hMutex, milliseconds);
	}
	void Unlock()
	{
		if (m_nStatus == XN_STATUS_OK)
		{
			//Only unlock if we managed to lock in the first place
			xnOSUnLockMutex(m_hMutex);
		}
	}
	XN_MUTEX_HANDLE m_hMutex;
	XnStatus m_nStatus;
};

class AutoCSLocker
{
public:
	inline AutoCSLocker(const AutoCSLocker& other) : m_cs(other.m_cs), m_locked(FALSE)
	{
		Lock();
	}
	inline AutoCSLocker& operator=(const AutoCSLocker& other)
	{
		Unlock();
		m_cs = other.m_cs;
		Lock();
		return *this;
	}
	inline AutoCSLocker(XN_CRITICAL_SECTION_HANDLE handle) : m_cs(handle), m_locked(FALSE)
	{
		Lock();
	}
	inline AutoCSLocker(const CriticalSection& cs) : m_cs(cs.m_cs), m_locked(FALSE)
	{
		Lock();
	}
	inline ~AutoCSLocker()
	{
		Unlock();
	}

	inline void Lock()
	{
		if (!m_locked)
		{
			xnOSEnterCriticalSection(&m_cs);
			m_locked = TRUE;
		}
	}
	inline void Unlock()
	{
		if (m_locked)
		{
			xnOSLeaveCriticalSection(&m_cs);
			m_locked = FALSE;
		}
	}
private:
	XN_CRITICAL_SECTION_HANDLE m_cs;
	bool m_locked;
};

class Time
{
public:
	static XnUInt64 Now()
	{
		XnUInt64 now;
		xnOSGetTimeStamp(&now);
		return now;
	}
};

class Timer
{
public:
};

class OSEvent
{
public:
	OSEvent() : m_hEvent(NULL) {}

	~OSEvent()
	{
		Close();
	}

	operator XN_EVENT_HANDLE() const
	{
		return m_hEvent;
	}

	XnStatus Create(XnBool bManualReset)
	{
		return xnOSCreateEvent(&m_hEvent, bManualReset);
	}

	XnStatus Create(const XnChar* strName, XnBool bManualReset, XnBool bAllowOtherUsers = FALSE)
	{
		return xnOSCreateNamedEventEx(&m_hEvent, strName, bManualReset, bAllowOtherUsers);
	}

	XnStatus Open(const XnChar* strName, XnBool bEnableOtherUsers = FALSE)
	{
		return xnOSOpenNamedEventEx(&m_hEvent, strName, bEnableOtherUsers);
	}

	XnStatus Close()
	{
		return (m_hEvent != NULL) ? xnOSCloseEvent(&m_hEvent) : XN_STATUS_OK;
	}

	XnStatus Set()
	{
		return xnOSSetEvent(m_hEvent);
	}

	XnStatus Reset()
	{
		return xnOSResetEvent(m_hEvent);
	}

	XnStatus Wait(XnUInt32 nMilliseconds)
	{
		return xnOSWaitEvent(m_hEvent, nMilliseconds);
	}

private:
	XN_EVENT_HANDLE m_hEvent;
};

} // xnl

#endif // _XN_OS_CPP_H_