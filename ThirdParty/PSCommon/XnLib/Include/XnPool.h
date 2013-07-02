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
#ifndef _XN_POOL_H_
#define _XN_POOL_H_

#include "XnList.h"
#include <XnOSCpp.h>

namespace xnl
{

template <class T, bool TThreadSafe = true>
class Pool
{
public:
	Pool() : m_pFirstAvailable(NULL)
	{
	}

	~Pool()
	{
	}

	void Lock() { m_lock.Lock(); }
	void Unlock() { m_lock.Unlock(); }

	T* Acquire()
	{
		Lock();

		TInPool* pResult;
		if (m_pFirstAvailable == NULL)
		{
			// we need to allocate new object
			pResult = XN_NEW(TInPool);
			pResult->refCount = 1;
			pResult->pNextAvailable = NULL;
			m_all.AddLast(pResult);
		}
		else
		{
			// take the first available
			pResult = m_pFirstAvailable;
			// remove it from the available list
			m_pFirstAvailable = pResult->pNextAvailable;
			// and initialize it
			pResult->refCount = 1;
			pResult->pNextAvailable = NULL;
		}

		Unlock();

		return pResult;
	}

	void AddRef(T* data)
	{
		TInPool* pObject = (TInPool*)data;
		Lock();
		++pObject->refCount;
		Unlock();
	}

	void Release(T* data)
	{
		TInPool* pObject = (TInPool*)data;
		Lock();
		if (--pObject->refCount == 0)
		{
			// add it to the available list
			pObject->pNextAvailable = m_pFirstAvailable;
			m_pFirstAvailable = pObject;
		}
		Unlock();
	}

	void Clear()
	{
		Lock();
		while (m_all.Begin() != m_all.End())
		{
			TInPool* pObject = *m_all.Begin();
			m_all.Remove(m_all.Begin());
			XN_DELETE(pObject);
		}
		Unlock();
	}

	int Count()
	{
		int count = 0;

		Lock();
		count = m_all.Size();
		Unlock();

		return count;
	}

protected:
	class TInPool : public T
	{
	public:
		int refCount;
		TInPool* pNextAvailable;
	};

	VirtualLock<TThreadSafe> m_lock;
	List<TInPool*> m_all;
	TInPool* m_pFirstAvailable;
};

} // xnl

#endif