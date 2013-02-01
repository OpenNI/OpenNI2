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
#ifndef _XN_SMART_POINTER_H_
#define _XN_SMART_POINTER_H_

#include <stdio.h>

namespace xnl
{
	
template <class T>
class SmartPointer
{
public:
	SmartPointer(T* ptr = 0, bool takeCharge = true) :
	  m_pCounterPtr(0)
	  {
		  if (ptr != 0)
		  {
			  m_pCounterPtr = XN_NEW(CountedPointer, ptr, takeCharge);
		  }
	  }
	  SmartPointer(const SmartPointer<T>& other)
	  {
		  acquire(other.m_pCounterPtr);
	  }
	  SmartPointer& operator=(const SmartPointer& other)
	  {
		  if (m_pCounterPtr != other.m_pCounterPtr)
		  {
			  release();
			  acquire(other.m_pCounterPtr);
		  }
		  return *this;
	  }

	  virtual ~SmartPointer()
	  {
		  release();
	  }

	  bool operator==(const SmartPointer& other) const
	  {
		  return (m_pCounterPtr == other.m_pCounterPtr);
	  }
	  bool operator!=(const SmartPointer& other) const
	  {
		  return (m_pCounterPtr != other.m_pCounterPtr);
	  }

	  T& operator*() const
	  {
		  return *m_pCounterPtr->ptr;
	  }
	  T* operator->() const
	  {
		  return m_pCounterPtr->ptr;
	  }
	  T* get() const
	  {
		  return m_pCounterPtr!=NULL?m_pCounterPtr->ptr:NULL;
	  }
private:
	class CountedPointer
	{
	public:
		CountedPointer(T* ptr, bool takeCharge) : 
		  ptr(ptr), refcount(1), takeCharge(takeCharge)
		  {}
		  T* ptr;
		  int refcount;
		  bool takeCharge;
	};

	void acquire(CountedPointer* pOther)
	{
		m_pCounterPtr = pOther;
		if (m_pCounterPtr != 0)
		{
			++m_pCounterPtr->refcount;
		}
	}
	void release()
	{
		printf("--> Release\n");
		if (m_pCounterPtr != 0)
		{
			--m_pCounterPtr->refcount;
			if (m_pCounterPtr->refcount == 0)
			{
				printf("Deleting\n");
				if (m_pCounterPtr->takeCharge)
				{
					printf("Deleting internal\n");
					XN_DELETE(m_pCounterPtr->ptr);
				}
				XN_DELETE(m_pCounterPtr);
			}
			m_pCounterPtr = 0;
		}
		printf("<-- Release\n");
	}

	CountedPointer* m_pCounterPtr;
};

} // xnl

#endif // _XN_SMART_POINTER_H_
