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
#ifndef __XNBITSET_H__
#define __XNBITSET_H__

#include <XnArray.h>

namespace xnl
{

class BitSet
{
public:
	BitSet() : m_nSize(0) {}

	/** Reserves space in this bitset for the specified number of bits. 
	    This saves you re-allocations and data copies if you know the size in advance. **/
	XnStatus Reserve(XnUInt32 nBits)
	{
		// note: dividing by 8 to get to bytes count
		return m_array.Reserve((nBits / 8) + 1);
	}

	/** Sets the size of the bitset to the specified number of bits and sets them all to 0. **/
	XnStatus SetSize(XnUInt32 nBits)
	{
		// note: dividing by 8 to get to bytes count
		return m_array.SetSize((nBits / 8) + 1, 0);
	}

	/** Sets the bit at nIndex to bValue. **/
	XnStatus Set(XnUInt32 nIndex, XnBool bValue)
	{
		XnUInt32 nArrayIndex = (nIndex / 8);
		XnUInt32 nBitIndex = (nIndex % 8);
		XnUInt8 nMask = (1 << nBitIndex);
		XnUInt8 nOldVal = nArrayIndex < m_array.GetSize() ? m_array[nArrayIndex] : 0;
		XnUInt8 nNewVal = bValue ? (nOldVal | nMask) : (nOldVal & (~nMask));
		XnStatus nRetVal = m_array.Set(nArrayIndex, nNewVal, 0);
		XN_IS_STATUS_OK(nRetVal);
		m_nSize = XN_MAX(m_nSize, nIndex + 1);
		return XN_STATUS_OK;
	}
	
	/** @returns the value of the bit specified by nIndex. **/
	XnBool IsSet(XnUInt32 nIndex) const
	{
		XnUInt32 nArrayIndex = (nIndex / 8);
		XnUInt32 nBitIndex = (nIndex % 8);
		if (nArrayIndex >= m_array.GetSize())
		{
			return FALSE;
		}
		return (m_array[nArrayIndex] & (1 << nBitIndex)) ? TRUE : FALSE;
	}

	/** Copies raw data from a buffer of bytes to this bitset. **/
	XnStatus SetData(const XnUInt8* pData, XnUInt32 nSizeInBytes)
	{
		XnStatus nRetVal = XN_STATUS_OK;
		
		nRetVal = m_array.SetData(pData, nSizeInBytes);
		XN_IS_STATUS_OK(nRetVal);

		m_nSize = (nSizeInBytes * 8);

		return XN_STATUS_OK;
	}

	/** @returns The raw data of this bitset as a buffer of bytes. **/
	const XnUInt8* GetData() const
	{
		return m_array.GetData();
	}

	/** @returns The raw data of this bitset as a buffer of bytes. Allows modification of underlying data. **/
	XnUInt8* GetData()
	{
		return m_array.GetData();
	}

	/** @returns size in bytes of this bitset. **/
	XnUInt32 GetDataSizeInBytes() const
	{
		return m_array.GetSize();
	}

	/** @returns size in bits of this bitset. **/
	XnUInt32 GetSize() const
	{
		return m_nSize;
	}

	/** Clears data in this bitset and sets size to 0. **/
	void Clear()
	{
		m_array.Clear();
		m_nSize = 0;
	}

	/** @returns TRUE if this bitset is empty, FALSE otherwise. **/
	XnBool IsEmpty() const
	{
		return m_array.IsEmpty();
	}
	
private:
	xnl::Array<XnUInt8> m_array;
	XnUInt32 m_nSize;
};

}
#endif // __XNBITSET_H__