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
#ifndef _XN_SIMD_SSE_H_
#define _XN_SIMD_SSE_H_

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include <XnOS.h>

typedef __m128i XN_INT128;

typedef XnInt32 XN_INT32;
typedef XnInt16 XN_INT16;


#define XnShiftRight128(a, imm) _mm_srli_si128(a, imm)
#define XnShiftLeft128(a, imm) _mm_slli_si128(a, imm)
#define XnShiftRight16(a, imm) _mm_srli_epi16(a, imm)
#define XnShiftLeft16(a, imm) _mm_slli_epi16(a, imm)

#define XnShiftRight16Sign(a, imm) _mm_srai_epi16(a, imm)

#ifdef WIN32
#define XN_FORCE_INLINE inline
#else
#define XN_FORCE_INLINE __inline __attribute__ ((__always_inline__))
#endif

static XN_FORCE_INLINE
XN_INT128 XnAnd128(XN_INT128 a, XN_INT128 b) // _mm_and_si128
{
	return _mm_and_si128(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnAndNot128(XN_INT128 a, XN_INT128 b) // _mm_andnot_si128
{
	return _mm_andnot_si128(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnAdd16(XN_INT128 a, XN_INT128 b) // _mm_add_epi16
{
	return _mm_add_epi16(a, b);
}
static XN_FORCE_INLINE
XN_INT128 XnHAdd16(XN_INT128 a, XN_INT128 b) // _mm_hadd_epi16 - SSSE3!
{
	return _mm_hadd_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnAdd16AndSaturates(XN_INT128 a, XN_INT128 b) // _mm_adds_epi16
{
	return _mm_adds_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnAddUnsigned16AndSaturates(XN_INT128 a, XN_INT128 b) // _mm_adds_epu16 
{
	return _mm_adds_epu16 (a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnSub16(XN_INT128 a, XN_INT128 b)  //_mm_sub_epi16
{
	return _mm_sub_epi16(a, b);
}
static XN_FORCE_INLINE
XN_INT128 XnSubSigned16(XN_INT128 a, XN_INT128 b) // _mm_subs_epu16
{
	return _mm_subs_epu16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnMult16(XN_INT128 a, XN_INT128 b) //_mm_mullo_epi16 
{
	return _mm_mullo_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnMin16(XN_INT128 a, XN_INT128 b) // _mm_min_epi16
{
	return _mm_min_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnMax16(XN_INT128 a, XN_INT128 b) // _mm_max_epi16
{
	return _mm_max_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnSetZero128() // _mm_setzero_si128
{
	return _mm_setzero_si128();
}

static XN_FORCE_INLINE
XN_INT128 XnSetOne16(XN_INT16 a) //_mm_set1_epi16
{
	return _mm_set1_epi16(a);
}

static XN_FORCE_INLINE
XN_INT128 XnSet16(XN_INT16 a7, XN_INT16 a6,    
							   XN_INT16 a5, XN_INT16 a4,   
							   XN_INT16 a3, XN_INT16 a2,   
							   XN_INT16 a1, XN_INT16 a0) // _mm_set_epi16
{
	return _mm_set_epi16(a7, a6, a5, a4, a3, a2, a1, a0);
}

static XN_FORCE_INLINE
XN_INT128 XnCompareGreaterThan(XN_INT128 a, XN_INT128 b) //_mm_cmpgt_epi16
{
	return _mm_cmpgt_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnCompareLessThan(XN_INT128 a, XN_INT128 b) //_mm_cmplt_epi16
{
	return _mm_cmplt_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnCompareEqual(XN_INT128 a, XN_INT128 b) // _mm_cmpeq_epi16
{
	return _mm_cmpeq_epi16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnAverageUnsigned16(XN_INT128 a, XN_INT128 b) // _mm_avg_epu16
{
	return _mm_avg_epu16(a, b);
}

static XN_FORCE_INLINE
XN_INT128 XnOr128(XN_INT128 a, XN_INT128 b) // _mm_or_si128
{
	return _mm_or_si128(a,b);
}

static XN_FORCE_INLINE
XN_INT128 XnPacksSigned16(XN_INT128 a, XN_INT128 b) // _mm_packs_epi16
{
	return _mm_packs_epi16(a,b);
}

#endif
