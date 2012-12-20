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
#ifndef _XN_PRIORITY_QUEUE_H_
#define _XN_PRIORITY_QUEUE_H_

#include "XnQueue.h"

namespace xnl
{

template <class T, int Max, class TAlloc = LinkedNodeDefaultAllocator<T> >
class PriorityQueue
{
public:
	PriorityQueue()
	{}
	PriorityQueue(const PriorityQueue<T, Max, TAlloc>& other)
	{
		*this = other;
	}
	~PriorityQueue()
	{}

	PriorityQueue& operator=(const PriorityQueue<T, Max, TAlloc>& other)
	{
		for (int i = 0; i < Max; ++i)
		{
			m_queues[i] = other.m_queues[i];
		}
		return *this;
	}

	XnStatus Push(const T& value, int priority)
	{
		return m_queues[priority].Push(value);
	}
	XnStatus Pop(T& value)
	{
		for (int i = 0; i < Max; ++i)
		{
			if  (!m_queues[i].IsEmpty())
			{
				return m_queues[i].Pop(value);
			}
		}
		return XN_STATUS_IS_EMPTY;
	}

	const T& Top() const
	{
		for (int i = 0; i < Max; ++i)
		{
			if  (!m_queues[i].IsEmpty())
			{
				return m_queues[i].Top();
			}
		}
	}
	T& Top()
	{
		for (int i = 0; i < Max; ++i)
		{
			if  (!m_queues[i].IsEmpty())
			{
				return m_queues[i].Top();
			}
		}
	}

	bool IsEmpty()
	{
		for (int i = 0; i < Max; ++i)
		{
			if (!m_queues[i].IsEmpty())
				return false;
		}
		return true;
	}
private:
	Queue<T, TAlloc> m_queues[Max];
};

} // xnl


#endif // _XN_PRIORITY_QUEUE_H_
