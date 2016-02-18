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

#include "arm_neon.h"
#include <XnOS.h>
#include <stdio.h>

typedef int16x8_t XN_INT128;

typedef XnInt32 XN_INT32;
typedef XnInt16 XN_INT16;
typedef XnUInt64 XN_UINT64;
typedef XnUInt16 XN_UINT16;

static __inline__ __attribute__ ((__always_inline__)) 
int16x8_t XnPacksSigned16(int16x8_t a, int16x8_t b) // _mm_packs_epi16
{
		return vcombine_s8(vqmovn_s16(a), vqmovn_s16(b));
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnSetZero128()  //_mm_sub_epi16
{
        return vdupq_n_s16(0);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnOr128(int16x8_t a, int16x8_t b) //_mm_or_si128
{
	return vorrq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnShiftLeft128(int16x8_t a, const int32_t imm2) //XnShiftLeft128
{
	return vreinterpretq_s16_s8(vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_s16(a), 16 - imm2));
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnShiftRight128(int16x8_t a, const int32_t imm2) //XnShiftLeft128
{
	return vreinterpretq_s16_s8(vextq_s8(vreinterpretq_s8_s16(a), vdupq_n_s8(0), imm2));
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnShiftRight16(int16x8_t a, const XN_INT32 count) //_mm_srai_epi16
{
	return vshrq_n_u16(a, count);
}
static __inline __attribute__ ((__always_inline__))
int16x8_t XnShiftRight16Sign(int16x8_t a, const XN_INT32 imm2)
{
	return vshrq_n_s16(a, imm2);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnAnd128(int16x8_t a, int16x8_t b) // _mm_and_si128
{
	return vandq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnAndNot128(int16x8_t a, int16x8_t b) // _mm_andnot_si128
{
	return vbicq_s16(b, a);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnHAdd16(int16x8_t a, int16x8_t b) // _mm_and_si128
{
	int16x8x2_t tmp = vuzpq_s16(a, b);
	return vaddq_s16(tmp.val[0], tmp.val[1]);
}

static __inline __attribute__ ((__always_inline__))
int8x16_t XnAdd16(int16x8_t a, int16x8_t b) // _mm_add_epi16
{
	return vaddq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnAdd16AndSaturates(int16x8_t a, int16x8_t b) // _mm_adds_epi16
{
	return vqaddq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnAddUnsigned16AndSaturates(int16x8_t a, int16x8_t b) //_mm_adds_epi16
{
    return vqaddq_u16(a, b);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnSub16(int16x8_t a, int16x8_t b)  //_mm_sub_epi16
{
	return vsubq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint8x16_t XnSubSigned16(uint8x16_t a, uint8x16_t b)  //_mm_sub_epi16
{
	return vqsubq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnSubUnSigned16(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vqsubq_u16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnMult16(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vmulq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnMultUnSigned16(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vmulq_u16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnMin16(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vminq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnMax16(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vmaxq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnSetOne16(XN_INT16 a) //_mm_set1_epi16
{
	return vdupq_n_s16(a);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnCompareEqual(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vceqq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnCompareLessThan(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vcltq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnCompareGreaterThan(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vcgtq_s16(a, b);
}

static __inline __attribute__ ((__always_inline__))
int16x8_t XnSet16(XN_INT16 a7, XN_INT16 a6,
				  XN_INT16 a5, XN_INT16 a4,
				  XN_INT16 a3, XN_INT16 a2,
				  XN_INT16 a1, XN_INT16 a0) // _mm_set_epi16
{
	// The array may be optimized away by compiler for const input.
	const XN_INT16	temp[8] = {a0, a1, a2, a3, a4, a5, a6, a7};

	return vld1q_s16(temp);
}

static __inline __attribute__ ((__always_inline__))
uint16x8_t XnAverageUnsigned16(uint16x8_t a, uint16x8_t b)  //_mm_sub_epi16
{
	return vrhaddq_s16(a, b);
}

#endif
