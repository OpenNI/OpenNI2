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
#ifndef __SSE_NONE_H__
#define __SSE_NONE_H__

#include <XnPlatform.h>

#ifndef INLINE
#define INLINE inline
#endif

#include <limits.h>

#define SSE_M128_NUM_OF_BYTES 8

typedef union
{
	XnInt16 m_data[SSE_M128_NUM_OF_BYTES];
	XnUInt16 m_udata[SSE_M128_NUM_OF_BYTES];
	XnInt8 m_i8[16];
} XN_INT128;


/* Adds the 8 signed or unsigned 16-bit integers in a to the
 * 8 signed or unsigned 16-bit integers in b.
 * r0 := a0 + b0
 * r1 := a1 + b1
 * ...
 * r7 := a7 + b7 */
INLINE XN_INT128 XnAdd16(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i)
		result.m_data[i] = a.m_data[i] + b.m_data[i];

	return result;
}

/* Adds the pares of the 16 bit numbers in a,b
 * r0 := a0 + a1
 * r1 := a2 + a3
 * ...
 * r7 := b6+ b7 */
INLINE XN_INT128 XnHAdd16(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	result.m_data[0] = a.m_data[0] + a.m_data[1];
	result.m_data[1] = a.m_data[2] + a.m_data[3];
	result.m_data[2] = a.m_data[4] + a.m_data[5];
	result.m_data[3] = a.m_data[6] + a.m_data[7];
	result.m_data[4] = b.m_data[0] + b.m_data[1];
	result.m_data[5] = b.m_data[2] + b.m_data[3];
	result.m_data[6] = b.m_data[4] + b.m_data[5];
	result.m_data[7] = b.m_data[6] + b.m_data[7];

	return result;
}

/*
 * Adds the 8 unsigned 16-bit integers in a to the 8 unsigned 16-bit integers in b and saturates.
 * r0 := UnsignedSaturate(a0 + b0)
 * r1 := UnsignedSaturate(a1 + b1)
 * ...
 * r15 := UnsignedSaturate(a7 + b7)
 */
INLINE XN_INT128 XnAddUnsigned16AndSaturates(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i)
	{
		XnUInt32 res = (XnUInt32)a.m_data[i] + (XnUInt32)b.m_data[i];

		if (res > 0xFFFF)
			result.m_udata[i] = 0xFFFF;
		else
			result.m_udata[i] = (XnUInt16)res;

	}

	return result;
}

/* Subtracts the 8 signed or unsigned 16-bit integers of b
 * from the 8 signed or unsigned 16-bit integers of a.
 * r0 := a0 - b0
 * r1 := a1 - b1
 * ...
 * r7 := a7 - b7 */
INLINE XN_INT128 XnSub16(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i)
		result.m_data[i] = a.m_data[i] - b.m_data[i];

	return result;
}

/* Sets the 128-bit value to zero.
 * r := 0x0 */
INLINE XN_INT128 XnSetZero128 ()
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i)
		result.m_data[i] = 0;

	return result;

}

/* Shifts the 128-bit value in a right by imm bytes while shifting
 * in zeros. imm must be an immediate.
 * r := srl(a, imm*8)
 *
 * NOTE: in SSE, elements are ordered 7,6,5,4,3,2,1,0. So, right shift is actually
 * left shift... */
INLINE XN_INT128 XnShiftRight128 (XN_INT128 a, XnInt32 imm)
{
	XN_INT128 result = XnSetZero128();

	XnUChar* pA = (XnUChar*)&a.m_data;
	XnUChar* pResult = (XnUChar*)&result.m_data;

	for (int i = 0; (i+imm) < SSE_M128_NUM_OF_BYTES*sizeof(XnInt16); ++i)
		pResult[i] = pA[i+imm];

	return result;
}

/*Shifts the 8 signed 16-bit integers in a right by count bits while shifting in the sign bit.
r0 := a0 >> count
r1 := a1 >> count
...
r7 := a7 >> count
*/
INLINE XN_INT128 XnShiftRight16Sign (XN_INT128 ar, XnInt32 count)
{
	XN_INT128 ret;
	
	XnInt16* r = (XnInt16*)&ret.m_data;
	XnInt16* a = (XnInt16*)&ar.m_data;

	r[0] = a[0] >> count; 	r[1] = a[1] >> count;
	r[2] = a[2] >> count; 	r[3] = a[3] >> count;
	r[4] = a[4] >> count; 	r[5] = a[5] >> count;
	r[6] = a[6] >> count; 	r[7] = a[7] >> count;
	
	return ret;
}

/* Shifts the 128-bit value in a left by imm bytes
 * while shifting in zeros. imm must be an immediate.
 * r := a << (imm * 8)
 *
 * NOTE: in SSE, elements are ordered 7,6,5,4,3,2,1,0. So, left shift is actually
 * right shift... */
INLINE XN_INT128 XnShiftLeft128 (XN_INT128 a, XnInt32 imm)
{
	XN_INT128 result = XnSetZero128();

	XnUChar* pA = (XnUChar*)&a.m_data;
	XnUChar* pResult = (XnUChar*)&result.m_data;

	for (int i = imm; i < SSE_M128_NUM_OF_BYTES*sizeof(XnInt16); ++i)
		pResult[i] = pA[i-imm];

	return result;
}

/* Computes the pairwise maxima of the 8 signed 16-bit integers
 * from a and the 8 signed 16-bit integers from b.
 * r0 := max(a0, b0)
 * r1 := max(a1, b1)
 * ...
 * r7 := max(a7, b7) */
INLINE XN_INT128 XnMax16 (XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
		result.m_data[i] = XN_MAX(a.m_data[i], b.m_data[i]);

	return result;
}

/* Computes the bitwise AND of the 128-bit value in a and
 * the 128-bit value in b.
 * r := a & b */
INLINE XN_INT128 XnAnd128 (XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
		result.m_data[i] = (a.m_data[i] & b.m_data[i]);

	return result;
}

/* Computes the bitwise AND of the 128-bit value in b
 * and the bitwise NOT of the 128-bit value in a.
 * r := (~a) & b */
INLINE XN_INT128 XnAndNot128 (XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
		result.m_data[i] = ((~a.m_data[i]) & b.m_data[i]);

	return result;
}

/* Sets the 8 signed 16-bit integer values to w
 * r0 := w
 * r1 := w
 * ...
 * r7 := w */
INLINE XN_INT128 XnSetOne16 (XnInt16 w)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
		result.m_data[i] = w;

	return result;
}

/* Sets the 8 signed 16-bit integer values
 * r0 := w0
 * r1 := w1
 * ...
 * r7 := w7*/
INLINE XN_INT128 XnSet16 (XnInt16 w7, XnInt16 w6, XnInt16 w5, XnInt16 w4, XnInt16 w3, XnInt16 w2, XnInt16 w1, XnInt16 w0)
{
	XN_INT128 result;

	result.m_data[0] = w0;
	result.m_data[1] = w1;
	result.m_data[2] = w2;
	result.m_data[3] = w3;
	result.m_data[4] = w4;
	result.m_data[5] = w5;
	result.m_data[6] = w6;
	result.m_data[7] = w7;

	return result;
}

/* Computes the bitwise OR of the 128-bit value in a and
 * the 128-bit value in b.
 * r := a | b */
INLINE XN_INT128 XnOr128 (XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
		result.m_data[i] = (a.m_data[i] | b.m_data[i]);;

	return result;
}

/* Computes the pairwise minima of the 8 signed 16-bit integers
 *  from a and the 8 signed 16-bit integers from b.
 * r0 := min(a0, b0)
 * r1 := min(a1, b1)
 * ...
 * r7 := min(a7, b7) */
INLINE XN_INT128 XnMin16 (XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
		result.m_data[i] = XN_MIN(a.m_data[i],b.m_data[i]);

	return result;
}

/* Adds the 8 signed 16-bit integers in a to the 8 signed 16-bit
 * integers in b and saturates.
 * 0 := SignedSaturate(a0 + b0)
 * r1 := SignedSaturate(a1 + b1)
 * ...
 * r7 := SignedSaturate(a7 + b7) */
INLINE XN_INT128 XnAdd16AndSaturates (XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
	{
		int res = (int)a.m_data[i] + (int)b.m_data[i];

		if (res > 0x7FFF)
			result.m_data[i] = 0x7FFF;
		else if (res < (XnInt16)0x8000)
			result.m_data[i] = (XnInt16)0x8000;
		else
			result.m_data[i] = res;
	}

	return result;
}

/* Compares the 8 signed 16-bit integers in a and the 8 signed 16-bit integers in b for greater than
 * r0 := (a0 > b0) ? 0xffff : 0x0
 * r1 := (a1 > b1) ? 0xffff : 0x0
 * ...
 * r7 := (a7 > b7) ? 0xffff : 0x0 */
INLINE XN_INT128  XnCompareGreaterThan(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
	{
		if( a.m_data[i] > b.m_data[i] )
			result.m_data[i] = (XnInt16)0xFFFF;
		else
			result.m_data[i] = 0;
	}

	return result;
}

/* Compares the 8 signed 16-bit integers in a and the 8 signed 16-bit integers in b for less than.
 * r0 := (a0 < b0) ? 0xffff : 0x0
 * r1 := (a1 < b1) ? 0xffff : 0x0
 * ...
 * r7 := (a7 < b7) ? 0xffff : 0x0 */
INLINE XN_INT128  XnCompareLessThan(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
	{
		if( a.m_data[i] < b.m_data[i] )
			result.m_data[i] = (XnInt16)0xFFFF;
		else
			result.m_data[i] = 0;
	}

	return result;
}

/* Compares the 8 signed or unsigned 16-bit integers in a and the 8 signed or unsigned 16-bit integers in b for equality.
 * r0 := (a0 == b0) ? 0xffff : 0x0
 * r1 := (a1 == b1) ? 0xffff : 0x0
 * ...
 * r7 := (a7 == b7) ? 0xffff : 0x0 */
INLINE XN_INT128  XnCompareEqual(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i=0; i<SSE_M128_NUM_OF_BYTES; i++)
	{
		if( a.m_data[i] == b.m_data[i] )
			result.m_data[i] = (XnInt16)0xFFFF;
		else
			result.m_data[i] = 0;
	}

	return result;
}

/* Computes the average of the 8 unsigned 16-bit integers in a and the 8 unsigned 16-bit integers in b and rounds.
 * r0 := (a0 + b0) / 2
 * r1 := (a1 + b1) / 2
 *  ...
 * r7 := (a7 + b7) / 2 */
INLINE XN_INT128  XnAverageUnsigned16(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i)
		result.m_data[i] = ((XnUInt16)a.m_data[i] + (XnUInt16)b.m_data[i] + 1)/2;

	return result;
}

/* Subtracts the 8 unsigned 16-bit integers of b from the 8 unsigned 16-bit integers of a and saturates.
 * r0 := UnsignedSaturate(a0 - b0)
 * r1 := UnsignedSaturate(a1 - b1)
 * ...
 * r7 := UnsignedSaturate(a7 - b7) */
INLINE XN_INT128 XnSubSigned16(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i)
		result.m_data[i] = (XnUInt16)a.m_data[i] - (XnUInt16)b.m_data[i];

	return result;
}

/* Shifts the 8 unsigned 16-bit integers in a right by count bits while shifting in zeroes. 
 * r0 := srl(a0, count) 
 * r1 := srl(a1, count) 
 * ... 
 * r7 := srl(a7, count) */ 
INLINE XN_INT128  XnShiftRight16(XN_INT128 a, XnInt32 count) 
{ 
	XN_INT128 result; 

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i ) 
		result.m_udata[i] = a.m_udata[i] >> count; 

	return result; 
} 


/* Multiplies the 8 signed or unsigned 16-bit integers from a by the 8 signed or unsigned 16-bit integers from b.
 * r0 := (a0 * b0)[15:0]
 * r1 := (a1 * b1)[15:0]
 * ...
 * r7 := (a7 * b7)[15:0] */
INLINE XN_INT128  XnMult16(XN_INT128 a, XN_INT128 b)
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i )
		result.m_data[i] = (XnInt16)((a.m_data[i] * b.m_data[i]) & 0xFFFF);

	return result;
}

/* Packs the 16 signed 16-bit integers from a and b into 8-bit integers and saturates.
 * r0 := SignedSaturate(a0)
 * r1 := SignedSaturate(a1)
 * ...
 * r7 := SignedSaturate(a7)
 * r8 := SignedSaturate(b0)
 * r9 := SignedSaturate(b1)
 * ...
 * r15 := SignedSaturate(b7)
 */
INLINE XN_INT128 XnPacksSigned16(XN_INT128 a, XN_INT128 b) // _mm_packs_epi16
{
	XN_INT128 result;

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i )
	{
		if (a.m_data[i] > 0x7F)
			result.m_i8[i] = 0x7F;
		else if (a.m_data[i] < (XnInt8)0x80)
			result.m_i8[i] = (XnInt8)0x80;
		else
			result.m_i8[i] = (XnInt8)a.m_data[i];
	}

	for (int i = 0; i < SSE_M128_NUM_OF_BYTES; ++i )
	{
		if (b.m_data[i] > 0x7F)
			result.m_i8[i + 8] = 0x7F;
		else if (b.m_data[i] < (XnInt8)0x80)
			result.m_i8[i + 8] = (XnInt8)0x80;
		else
			result.m_i8[i + 8] = (XnInt8)b.m_data[i];
	}

	return result;
}

#endif
