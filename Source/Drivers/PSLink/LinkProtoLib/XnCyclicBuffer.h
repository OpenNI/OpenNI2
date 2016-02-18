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
#ifndef XNCYCLICBUFFER_H
#define XNCYCLICBUFFER_H

#include <XnOS.h>

namespace xn
{

class CyclicBuffer
{
public:
	CyclicBuffer()
	{
		m_pBuffer = NULL;
		m_nBegin = 0;
		m_nEnd = 0;
		m_nBufferSize = 0;
	}
	
	~CyclicBuffer()
	{
		Shutdown();
	}
	
	XnStatus Init(XnUInt32 nMaxSize)
	{
		//Allocate +1 byte for end marker
		m_nBufferSize = nMaxSize + 1;
		m_pBuffer = reinterpret_cast<XnUInt8*>(xnOSMallocAligned(m_nBufferSize, XN_DEFAULT_MEM_ALIGN));
		XN_VALIDATE_ALLOC_PTR(m_pBuffer);
		m_nBegin = 0;
		m_nEnd = 0;
		return XN_STATUS_OK;
	}
	
	void Shutdown()
	{
		XN_ALIGNED_FREE_AND_NULL(m_pBuffer);
	}

	XnBool IsFull() const
	{
		return (((m_nEnd + 1) % m_nBufferSize) == m_nBegin);
	}

	XnStatus Add(const XnUInt8* pSrc, XnUInt32 nSrcSize)
	{
		if (nSrcSize > m_nBufferSize - 1)
		{
			return XN_STATUS_INPUT_BUFFER_OVERFLOW;
		}
		
		//Make more room for new data by moving the beginning of the buffer forward if needed.
		XnUInt32 nMissingSpace = XN_MAX((XnInt32)(nSrcSize - m_nBufferSize + GetSize()), 0);
		m_nBegin = ((m_nBegin + nMissingSpace) % m_nBufferSize);
		//First copy either whole source or as much of it that fits between m_nEnd and buffer end.
		//Then we copy the remaining bytes (if any) to the beginning of the buffer.
		XnUInt32 nSize1 = XN_MIN(nSrcSize, m_nBufferSize - m_nEnd);
		XnUInt32 nSize2 = (nSrcSize - nSize1);
		xnOSMemCopy(m_pBuffer + m_nEnd, pSrc, nSize1);
		xnOSMemCopy(m_pBuffer, pSrc + nSize1, nSize2);
		m_nEnd = (m_nEnd + nSrcSize) % m_nBufferSize;
		return XN_STATUS_OK;		
	}

	//nDestSize is max size on input, actual size on output.
	XnStatus Flush(XnUInt8* pDest, XnUInt32& nDestSize)
	{
		XnInt32 nDiff = (m_nEnd - m_nBegin);
		if (nDiff > 0)
		{
			if (XnUInt32(nDiff) > nDestSize)
			{
				return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
			}

			xnOSMemCopy(pDest, m_pBuffer + m_nBegin, nDiff);
			nDestSize = nDiff;
		}
		else if (nDiff < 0)
		{
			if ((m_nBufferSize + nDiff) > nDestSize)
			{
				return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
			}

			xnOSMemCopy(pDest, m_pBuffer + m_nBegin, m_nBufferSize - m_nBegin);
			xnOSMemCopy(pDest + m_nBufferSize - m_nBegin, m_pBuffer, m_nEnd);
			nDestSize = m_nBufferSize + nDiff;
		}
		else
		{
			nDestSize = 0;
		}
		m_nBegin = 0;
		m_nEnd = 0;

		return XN_STATUS_OK;
	}

	XnUInt32 GetSize() const
	{
		XnInt32 nDiff = (m_nEnd - m_nBegin);
		if (nDiff >= 0)
		{
			return nDiff;
		}
		else
		{
			return m_nBufferSize + nDiff;
		}
	}

private:
	XnUInt8* m_pBuffer;
	XnUInt32 m_nBufferSize;
	XnUInt32 m_nBegin;
	XnUInt32 m_nEnd;
};

}

#endif // XNCYCLICBUFFER_H
