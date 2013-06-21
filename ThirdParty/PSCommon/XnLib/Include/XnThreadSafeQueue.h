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
#ifndef _XN_THREAD_SAFE_QUEUE_H_
#define _XN_THREAD_SAFE_QUEUE_H_

#include "XnQueue.h"
#include "XnOSCpp.h"

namespace xnl
{
template <class T, class TAlloc = LinkedNodeDefaultAllocator<T> >
class ThreadSafeQueue : protected Queue<T, TAlloc>
{
public:
	typedef Queue<T, TAlloc> Base;

	ThreadSafeQueue() : Base() {}
	ThreadSafeQueue(const Queue& other) : Base()
	{
		*this = other;
	}
	ThreadSafeQueue& operator=(const ThreadSafeQueue& other)
	{
		Base::operator=(other);
		return *this;
	}
	~ThreadSafeQueue() {}

	XnStatus Push(const T& value)
	{
		XnStatus rc = XN_STATUS_OK;
		m_cs.Lock();
		rc = Base::Push(value);
		m_cs.Unlock();
		return rc;
	}
	XnStatus Pop(T& value)
	{
		XnStatus rc = XN_STATUS_OK;
		m_cs.Lock();
		rc = Base::Pop(value);
		m_cs.Unlock();
		return rc;		
	}
	XnUInt32 Size() const
	{
		m_cs.Lock();
		XnUInt32 size = Base::Size();
		m_cs.Unlock();
		return size;
	}

	using Base::IsEmpty;
protected:
	CriticalSection m_cs;
};

} // xnl


#endif // _XN_THREAD_SAFE_QUEUE_H_
