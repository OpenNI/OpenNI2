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
#include "XnHash.h"

namespace xnl
{

template <class T>
class Pool
{
public:
	Pool(bool protect = false) : m_criticalSection(NULL)
	{
		if (protect)
		{
			XnStatus rc = xnOSCreateCriticalSection(&m_criticalSection);
			if (rc != XN_STATUS_OK)
			{
				m_criticalSection = NULL;
			}
		}
	}
	virtual ~Pool()
	{
		Clear();

		// Remove the used buffers (when pool is destroyed, there is no way to to 
		// reclaim the memory, so they have to be destroyed).
		Lock();
		while (m_used.Begin() != m_used.End())
		{
			T* data = (*m_used.Begin()).Key();
			m_used.Remove(data);
			Destroy(data);
		}
		Unlock();
	}

	virtual bool Initialize(int count)
	{
		Clear();
		Lock();

		for (int i = 0; i < count; ++i)
		{
			m_available.AddLast(Create());
		}

		Unlock();
		return true;
	}

	T* Acquire()
	{
		Lock();

		if (m_available.Size() == 0)
		{
			Unlock();
			return NULL;
		}

		T* data = *m_available.Begin();
		m_available.Remove(data);
		m_used[data] = 1;

		Unlock();

		return data;
	}

	virtual XnStatus Release(T* data)
	{
		return DecRef(data);
	}

	void AddRef(T* data)
	{
		Lock();

		if (m_used.Find(data) != m_used.End())
			m_used[data]++;

		Unlock();
	}

	XnStatus DecRef(T* data)
	{
		Lock();

		if (m_used.Find(data) == m_used.End())
		{
			// Check if already released.
			if (m_available.Find(data) != m_available.End())
			{
				Unlock();
				return XN_STATUS_OK;
			}
			Unlock();
			return XN_STATUS_NO_MATCH;
		}

		m_used[data]--;
		if (m_used[data] == 0)
		{
			m_used.Remove(data);
			if (Valid(data))
			{
				m_available.AddLast(data);
			}
			else
			{
				Destroy(data);
			}

			Unlock();
			return XN_STATUS_OK;
		}

		Unlock();
		return XN_STAUTS_NOT_LAST_REFERENCE;
	}

	virtual void Clear()
	{
		Lock();
		while (m_available.Begin() != m_available.End())
		{
			T* data = *m_available.Begin();
			m_available.Remove(data);
			Destroy(data);
		}
		Unlock();
	}

	int Count()
	{
		Lock();
		int count = m_available.Size() + m_used.Size();
		Unlock();

		return count;
	}

	void Lock() 
	{
		if (m_criticalSection != NULL)
		{
			xnOSEnterCriticalSection(&m_criticalSection);
		}
	}

	void Unlock() 
	{
		if (m_criticalSection != NULL)
		{
			xnOSLeaveCriticalSection(&m_criticalSection);
		}
	}
protected:
	virtual T* Create() {return XN_NEW(T);}
	virtual void Destroy(T* data) {XN_DELETE(data);}
	virtual bool Valid(T* /*data*/) {return true;}

	xnl::Hash<T*, int> m_used;
	xnl::List<T*> m_available;

	XN_CRITICAL_SECTION_HANDLE m_criticalSection;
};

} // xnl

#endif