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
#ifndef _XN_LIST_H_
#define _XN_LIST_H_

#include "XnMemory.h"

namespace xnl
{
template <class T>
struct LinkedListNode
{
	LinkedListNode() : pPrev(NULL), pNext(NULL) {}
	LinkedListNode(const T& value) : pPrev(NULL), pNext(NULL), value(value) {}

	struct LinkedListNode<T>* pPrev;
	struct LinkedListNode<T>* pNext;
	T value;
}; // LinkedListNode

template <class T>
class LinkedNodeDefaultAllocator
{
public:
	typedef LinkedListNode<T> LinkedNode;

	static LinkedNode* Allocate(const T& value)
	{
		return XN_NEW(LinkedNode, value);
	}
	static void Deallocate(LinkedNode* pNode)
	{
		XN_DELETE(pNode);
	}
}; // LinkedNodeDefaultAllocator

template <class T, class TAlloc = LinkedNodeDefaultAllocator<T> >
class List
{
public:
	typedef LinkedListNode<T> LinkedNode;
	typedef T Value;
	typedef TAlloc TAllocator;

	class ConstIterator
	{
	public:
		inline ConstIterator() : m_pCurrent(NULL) {}
		inline ConstIterator(LinkedNode* pNode) : m_pCurrent(pNode) {}
		inline ConstIterator(const ConstIterator& other) : m_pCurrent(other.m_pCurrent) {}

		inline ConstIterator& operator++()
		{
			m_pCurrent = m_pCurrent->pNext;
			return *this;
		}
		inline ConstIterator operator++(int)
		{
			ConstIterator retVal(*this);
			++*this;
			return retVal;
		}
		inline ConstIterator& operator--()
		{
			m_pCurrent = m_pCurrent->pPrev;
			return *this;
		}
		inline ConstIterator operator--(int)
		{
			ConstIterator retVal(*this);
			--*this;
			return retVal;
		}
		inline bool operator==(const ConstIterator& other) const
		{
			return m_pCurrent == other.m_pCurrent;
		}
		inline bool operator!=(const ConstIterator& other) const
		{
			return m_pCurrent != other.m_pCurrent;
		}
		inline const T& operator*() const
		{
			return m_pCurrent->value;
		}
		inline const T* operator->() const
		{
			return &m_pCurrent->value;
		}
	protected:
		friend class List;
		LinkedNode* m_pCurrent;
	}; // ConstIterator
	class Iterator : public ConstIterator
	{
	public:
		inline Iterator() : ConstIterator() {}
		inline Iterator(LinkedNode* pNode) : ConstIterator(pNode) {}
		inline Iterator(const Iterator& other) : ConstIterator(other) {}

		inline Iterator& operator++()
		{
			++(*(ConstIterator*)this);
			return *this;
		}
		inline Iterator operator++(int)
		{
			Iterator retVal(*this);
			++*this;
			return retVal;
		}
		inline Iterator& operator--()
		{
			--(*(ConstIterator*)this);
			return *this;
		}
		inline Iterator operator--(int)
		{
			Iterator retVal(*this);
			--*this;
			return retVal;
		}
		inline T& operator*() const
		{
			return this->m_pCurrent->value;
		}
		inline T* operator->() const
		{
			return &this->m_pCurrent->value;
		}
	}; // Iterator
public:
	List()
	{
		Init();
	}
	List(const List& other)
	{
		Init();
		*this = other;
	}
	virtual ~List()
	{
		Clear();
	}
	List& operator=(const List& other)
	{
		Clear();

		XnStatus retVal = XN_STATUS_OK;
		XN_REFERENCE_VARIABLE(retVal);
		for (ConstIterator it = other.Begin(); it != other.End(); ++it)
		{
			retVal = AddLast(*it);
			XN_ASSERT(retVal == XN_STATUS_OK);
		}
		return *this;
	}
	Iterator Begin()
	{
		return Iterator(m_anchor.pNext);
	}
	ConstIterator Begin() const
	{
		return ConstIterator(const_cast<LinkedNode*>(m_anchor.pNext));
	}
	Iterator End()
	{
		return Iterator(&m_anchor);
	}
	ConstIterator End() const
	{
		return ConstIterator(const_cast<LinkedNode*>(&m_anchor));
	}
	Iterator ReverseBegin()
	{
		return Iterator(m_anchor.pPrev);
	}
	ConstIterator ReverseBegin() const
	{
		return ConstIterator(const_cast<LinkedNode*>(m_anchor.pPrev));
	}
	Iterator ReverseEnd()
	{
		return Iterator(&m_anchor);
	}
	ConstIterator ReverseEnd() const
	{
		return ConstIterator(const_cast<LinkedNode*>(&m_anchor));
	}
	XnStatus AddAfter(ConstIterator where, const T& value)
	{
		if (where == End())
		{
			return XN_STATUS_ILLEGAL_POSITION;
		}
		return InsertAfter(where.m_pCurrent, value);
	}
	XnStatus AddBefore(ConstIterator where, const T& value)
	{
		if (where == End())
		{
			return XN_STATUS_ILLEGAL_POSITION;
		}
		return InsertAfter(where.m_pCurrent->pPrev, value);
	}
	XnStatus AddFirst(const T& value)
	{
		return InsertAfter(&m_anchor, value);
	}
	XnStatus AddLast(const T& value)
	{
		return InsertAfter(ReverseBegin().m_pCurrent, value);
	}
	ConstIterator Find(const T& value) const
	{
		ConstIterator iter = Begin();
		for (; iter != End(); ++iter)
		{
			if (*iter == value)
				break;
		}
		return iter;
	}
	Iterator Find(const T& value)
	{
		ConstIterator iter = const_cast<const List<T>*>(this)->Find(value);
		return Iterator(iter.m_pCurrent);
	}
	XnStatus Remove(ConstIterator where)
	{
		if (where == End())
		{
			return XN_STATUS_ILLEGAL_POSITION;
		}
		LinkedListNode<T>* pToRemove = where.m_pCurrent;
		pToRemove->pPrev->pNext = pToRemove->pNext;
		pToRemove->pNext->pPrev = pToRemove->pPrev;

		--m_nSize;

		TAlloc::Deallocate(pToRemove);

		return XN_STATUS_OK;
	}
	XnStatus Remove(const T& value)
	{
		ConstIterator it = Find(value);
		if (it != End())
		{
			return Remove(it);
		}
		else
		{
			return XN_STATUS_NO_MATCH;
		}
	}
	XnStatus Clear()
	{
		while (!IsEmpty())
		{
			Remove(Begin());
		}
		return XN_STATUS_OK;
	}
	bool IsEmpty() const
	{
		return (m_nSize==0);
	}
	unsigned int Size() const
	{
		return m_nSize;
	}
	void CopyTo(T* pArray)
	{
		XN_ASSERT(pArray != NULL);
		unsigned int i = 0;
		for (ConstIterator iter = Begin(); iter != End(); ++iter, ++i)
		{
			pArray[i] = *iter;
		}
	}
protected:
	XnStatus InsertAfter(LinkedNode* pAfter, const T& val)
	{
		LinkedNode* pNewNode = TAlloc::Allocate(val);
		if (pNewNode == NULL)
		{
			XN_ASSERT(FALSE);
			return XN_STATUS_ALLOC_FAILED;
		}
		pNewNode->pPrev = pAfter;
		pNewNode->pNext = pAfter->pNext;

		pAfter->pNext->pPrev = pNewNode;
		pAfter->pNext = pNewNode;

		++m_nSize;
		return XN_STATUS_OK;
	}

	LinkedNode m_anchor;
	XnUInt32 m_nSize;
private:
	void Init()
	{
		m_anchor.pNext = &m_anchor;
		m_anchor.pPrev = &m_anchor;
		m_nSize = 0;
	}
}; // List

} // xnl

#endif // _XN_LIST_H_
