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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
// NOTE: you must define XN_OS_IMPL before including XnOS.h, otherwise, when mem profiling this file will not compile.
#define XN_OS_IMPL
#undef XN_CROSS_PLATFORM
#include <XnOS.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XN_CORE_API void* XnOSMalloc(const XN_SIZE_T nAllocSize)
{
	return malloc(nAllocSize);
}

XN_CORE_API void* XnOSMallocAligned(const XN_SIZE_T nAllocSize, const XN_SIZE_T nAlignment)
{
	return memalign(nAlignment, nAllocSize);
}

XN_CORE_API void* XnOSCalloc(const XN_SIZE_T nAllocNum, const XN_SIZE_T nAllocSize)
{
	return calloc(nAllocNum, nAllocSize);
}

XN_CORE_API void* XnOSCallocAligned(const XN_SIZE_T nAllocNum, const XN_SIZE_T nAllocSize, const XN_SIZE_T nAlignment)
{
	void* pMemBlock = NULL;
	XN_UINT32 nBlockSize = nAllocNum*nAllocSize;

	// Allocate
	pMemBlock = XnOSMallocAligned(nBlockSize, nAlignment);
	if (pMemBlock != NULL)
	{
		memset(pMemBlock, 0, nBlockSize);
	}

	return pMemBlock;
}

XN_CORE_API void* XnOSRealloc(void* pMemory, const XN_SIZE_T nAllocSize)
{
	return realloc(pMemory, nAllocSize);
}

XN_CORE_API void* XnOSReallocAligned(void* pMemory, const XN_SIZE_T nAllocSize, const XN_SIZE_T nAlignment)
{
	return reallocalign(pMemory, nAllocSize, nAlignment);
}

XN_CORE_API void* XnOSRecalloc(void* pMemory, const XN_SIZE_T nAllocNum, const XN_SIZE_T nAllocSize)
{
	XnOSRealloc(pMemory, nAllocNum*nAllocSize);
	if (pMemory != NULL)
	{
		memset(pMemory, 0, nAllocNum*nAllocSize);
	}

	return pMemory;
}

XN_CORE_API void XnOSFree(const void* pMemBlock)
{
	free((void*)pMemBlock);
}

XN_CORE_API void  XnOSFreeAligned(const void* pMemBlock)
{
	free((void*)pMemBlock);
}

XN_CORE_API void XnOSMemCopy(void* pDest, const void* pSource, XN_UINT32 nCount)
{
	memcpy(pDest, pSource, nCount);
}

XN_CORE_API void XnOSMemSet(void* pDest, XN_UINT8 nValue, XN_UINT32 nCount)
{
	memset(pDest, nValue, nCount);
}

XN_CORE_API XN_UINT64  XnOSEndianSwapUINT64(XN_UINT64 nValue)
{
	return ((nValue >> 56) ) | ((nValue >> 40) & 0x000000000000ff00ULL) |
			((nValue >> 24) & 0x0000000000ff0000ULL) | ((nValue >> 8 ) & 0x00000000ff000000ULL) |
			((nValue << 8 ) & 0x000000ff00000000ULL) | ((nValue << 24) & 0x0000ff0000000000ULL) |
			((nValue << 40) & 0x00ff000000000000ULL) | ((nValue << 56) );
}

XN_CORE_API XN_UINT32  XnOSEndianSwapUINT32(XN_UINT32 nValue)
{
	return  (nValue>>24) | 
			((nValue<<8) & 0x00FF0000) |
			((nValue>>8) & 0x0000FF00) |
			(nValue<<24);
}

XN_CORE_API XN_UINT16 XnOSEndianSwapUINT16(XN_UINT16 nValue)
{
	return ((nValue>>8) | (nValue<<8));
}

XN_CORE_API XN_FLOAT XnOSEndianSwapFLOAT(XN_FLOAT fValue)
{
	XN_UINT32* pnValue = (XN_UINT32*)&fValue;
	XN_UINT32 nValue = XnOSEndianSwapUINT32(*pnValue);
	XN_FLOAT* pfValue = (XN_FLOAT*)&nValue;
	return *pfValue;
}