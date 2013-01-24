/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
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
#ifndef __XN_BUFFER_H__
#define __XN_BUFFER_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOS.h>
#include <XnCore.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
/**
* Holds a buffer of data, and provides some common methods for it.
*/
class XnBuffer
{
public:
	XnBuffer() : m_pData(NULL), m_nSize(0), m_nMaxSize(0), m_bAllocated(FALSE) {}
	~XnBuffer() { Free(); }

	/*
	* Allocates a buffer (aligned to default).
	*
	* @param	size		[in]	The number of bytes to allocate.
	*/
	XnStatus Allocate(XnUInt32 nAllocSize);

	/**
	 * Sets an external buffer (instead of allocating)
	 *
	 * @param	pBuffer		[in]	the buffer to use.
	 * @param	nSize		[in]	Buffer size.
	 */
	void SetExternalBuffer(XnUChar* pBuffer, XnUInt32 nSize);

	/*
	* Writes data to the buffer.
	*
	* @param	data		[in]	a pointer to the data to copy from.
	* @param	size		[in]	The number of bytes to copy.
	*/
	inline void UnsafeWrite(const XnUChar* pData, XnUInt32 nDataSize)
	{
		// Some of my callers copy data from within my buffer after Reset()ing.
		// So safely "slide" the to-be-copied chunk back to the buffer's beginning.
		xnOSMemMove(m_pData + m_nSize, pData, nDataSize);
		m_nSize += nDataSize;
	}

	/*
	* Writes data to the buffer, checking for free space
	*
	* @param	data		[in]	a pointer to the data to copy from.
	* @param	size		[in]	The number of bytes to copy.
	*/
	XnStatus Write(const XnUChar* pData, XnUInt32 nDataSize);

	/*
	* Copies buffer data to another location. 
	*
	* @param	dest	[in]	Location to write to.
	*/
	inline void UnsafeCopy(void* pDest)
	{
		xnOSMemCopy(pDest, m_pData, m_nSize);
	}

	/*
	* Empties the buffer.
	*/
	inline void Reset()
	{
		m_nSize = 0;
	}

	inline XnUChar* GetData()
	{
		return m_pData;
	}

	inline XnUInt32 GetSize()
	{
		return m_nSize;
	}

	inline XnUInt32 GetMaxSize()
	{
		return m_nMaxSize;
	}

	/*
	* Frees an allocated buffer.
	*/
	inline void Free()
	{
		if (m_bAllocated)
		{
			XN_ALIGNED_FREE_AND_NULL(m_pData);
			m_bAllocated = FALSE;
		}
	}

	/*
	* Gets the free space in the buffer.
	*/
	inline XnUInt32 GetFreeSpaceInBuffer()
	{
		return XN_MAX(0, (XnInt32)(m_nMaxSize - m_nSize));
	}

	/*
	* Gets the free space in the buffer.
	*/
	inline XnUChar* GetUnsafeWritePointer()
	{
		return m_pData + m_nSize;
	}

	/*
	* Updates the size of the buffer
	*/
	inline void UnsafeUpdateSize(XnUInt32 nWrittenBytes)
	{
		m_nSize += nWrittenBytes;
	}

	inline void UnsafeSetSize(XnUInt32 nSize)
	{
		m_nSize = nSize;
	}

	/*
	* Update the user cookie for this buffer.
	*/
	inline void SetCookie(void* pCookie)
	{
		m_pCookie = pCookie;
	}

	/*
	* Retrieve the user cookie for this buffer.
	*/
	inline void* GetCookie()
	{
		return m_pCookie;
	}

private:
	XnUChar* m_pData;
	XnUInt32 m_nSize;
	XnUInt32 m_nMaxSize;
	XnBool m_bAllocated;
	void* m_pCookie;
};

/*
* Allocates a buffer (aligned to default), and validates that allocation succeeded.
*
* @param	buf			[in]	The buffer to allocate.
* @param	size		[in]	The number of bytes to allocate.
*/
#define XN_VALIDATE_BUFFER_ALLOCATE(buf, size)				\
	{														\
		XnStatus rc = buf.Allocate(size);					\
		XN_IS_STATUS_OK(rc);								\
	}

#endif //__XN_BUFFER_H__
