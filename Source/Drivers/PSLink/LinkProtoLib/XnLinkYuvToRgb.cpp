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
#include <XnPlatform.h>
#include <XnStatusCodes.h>
#if (XN_PLATFORM == XN_PLATFORM_WIN32)
	#include <emmintrin.h>
#endif

#include "XnLinkYuvToRgb.h"

#define YUV422_U  0
#define YUV422_Y1 1
#define YUV422_V  2
#define YUV422_Y2 3

#define RGB888_RED   0
#define RGB888_GREEN 1
#define RGB888_BLUE  2

namespace xn
{

/*
http://en.wikipedia.org/wiki/YUV

From YUV to RGB:
R =     Y + 1.13983 V
G =     Y - 0.39466 U - 0.58060 V
B =     Y + 2.03211 U
*/ 
XnStatus LinkYuvToRgb::Yuv422ToRgb888(const XnUInt8* pSrc, XnSizeT srcSize, XnUInt8* pDst, XnSizeT& dstSize)
{
	if (dstSize < srcSize * RGB_888_BYTES_PER_PIXEL / YUV_422_BYTES_PER_PIXEL)
	{
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

#if (XN_PLATFORM == XN_PLATFORM_WIN32)
	const XnUInt8* pYUVLast = pSrc + srcSize - 8;

	const __m128 minus128 = _mm_set_ps1(-128);
	const __m128 plus113983 = _mm_set_ps1(1.13983F);
	const __m128 minus039466 = _mm_set_ps1(-0.39466F);
	const __m128 minus058060 = _mm_set_ps1(-0.58060F);
	const __m128 plus203211 = _mm_set_ps1(2.03211F);
	const __m128 zero = _mm_set_ps1(0);
	const __m128 plus255 = _mm_set_ps1(255);

	// define YUV floats
	__m128 y;
	__m128 u;
	__m128 v;

	__m128 temp;

	// define RGB floats
	__m128 r;
	__m128 g;
	__m128 b;

	// define RGB integers
	__m128i iR;
	__m128i iG;
	__m128i iB;

	XnUInt32* piR = (XnUInt32*)&iR;
	XnUInt32* piG = (XnUInt32*)&iG;
	XnUInt32* piB = (XnUInt32*)&iB;

	while (pSrc <= pYUVLast)
	{
		// process 4 pixels at once (values should be ordered backwards)
		y = _mm_set_ps(pSrc[YUV422_Y2 + YUV_422_BYTES_PER_PIXEL], pSrc[YUV422_Y1 + YUV_422_BYTES_PER_PIXEL], pSrc[YUV422_Y2], pSrc[YUV422_Y1]);
		u = _mm_set_ps(pSrc[YUV422_U + YUV_422_BYTES_PER_PIXEL],  pSrc[YUV422_U + YUV_422_BYTES_PER_PIXEL],  pSrc[YUV422_U],  pSrc[YUV422_U]);
		v = _mm_set_ps(pSrc[YUV422_V + YUV_422_BYTES_PER_PIXEL],  pSrc[YUV422_V + YUV_422_BYTES_PER_PIXEL],  pSrc[YUV422_V],  pSrc[YUV422_V]);

		u = _mm_add_ps(u, minus128); // u -= 128
		v = _mm_add_ps(v, minus128); // v -= 128

		temp = _mm_mul_ps(plus113983, v);
		r = _mm_add_ps(y, temp);

		temp = _mm_mul_ps(minus039466, u);
		g = _mm_add_ps(y, temp);
		temp = _mm_mul_ps(minus058060, v);
		g = _mm_add_ps(g, temp);

		temp = _mm_mul_ps(plus203211, u);
		b = _mm_add_ps(y, temp);

		// make sure no value is smaller than 0
		r = _mm_max_ps(r, zero);
		g = _mm_max_ps(g, zero);
		b = _mm_max_ps(b, zero);

		// make sure no value is bigger than 255
		r = _mm_min_ps(r, plus255);
		g = _mm_min_ps(g, plus255);
		b = _mm_min_ps(b, plus255);

		// convert floats to int16 (there is no conversion to uint8, just to int8).
		iR = _mm_cvtps_epi32(r);
		iG = _mm_cvtps_epi32(g);
		iB = _mm_cvtps_epi32(b);

		// extract the 4 pixels RGB values.
		// because we made sure values are between 0 and 255, we can just take the lower byte
		// of each INT16
		pDst[0] = (XnUInt8)piR[0];
		pDst[1] = (XnUInt8)piG[0];
		pDst[2] = (XnUInt8)piB[0];

		pDst[3] = (XnUInt8)piR[1];
		pDst[4] = (XnUInt8)piG[1];
		pDst[5] = (XnUInt8)piB[1];

		pDst[6] = (XnUInt8)piR[2];
		pDst[7] = (XnUInt8)piG[2];
		pDst[8] = (XnUInt8)piB[2];

		pDst[9] = (XnUInt8)piR[3];
		pDst[10] = (XnUInt8)piG[3];
		pDst[11] = (XnUInt8)piB[3];

		// advance the streams
		pSrc += 8;
		pDst += 12;
	}
#else
	const XnUInt8* pCurrYUV = pSrc;
	XnUInt8* pCurrRGB = pDst;
	const XnUInt8* pLastYUV = pSrc + srcSize - YUV_422_BYTES_PER_PIXEL;

	while (pCurrYUV <= pLastYUV)
	{
		pCurrRGB[RGB888_RED]   = XnUInt8(pCurrYUV[YUV422_Y1]                                + 1.13983 * pCurrYUV[YUV422_V] + 0.5);
		pCurrRGB[RGB888_GREEN] = XnUInt8(pCurrYUV[YUV422_Y1] - 0.39466 * pCurrYUV[YUV422_U] - 0.58060 * pCurrYUV[YUV422_V] + 0.5);
		pCurrRGB[RGB888_BLUE]  = XnUInt8(pCurrYUV[YUV422_Y1] + 2.03211 * pCurrYUV[YUV422_U]                                + 0.5);

		pCurrRGB += RGB_888_BYTES_PER_PIXEL;

		pCurrRGB[RGB888_RED]   = XnUInt8(pCurrYUV[YUV422_Y2]                                + 1.13983 * pCurrYUV[YUV422_V] + 0.5);
		pCurrRGB[RGB888_GREEN] = XnUInt8(pCurrYUV[YUV422_Y2] - 0.39466 * pCurrYUV[YUV422_U] - 0.58060 * pCurrYUV[YUV422_V] + 0.5);
		pCurrRGB[RGB888_BLUE]  = XnUInt8(pCurrYUV[YUV422_Y2] + 2.03211 * pCurrYUV[YUV422_U]                                + 0.5);

		pCurrRGB += RGB_888_BYTES_PER_PIXEL;
		pCurrYUV += YUV_422_BYTES_PER_PIXEL;
	}
#endif

	dstSize = srcSize * RGB_888_BYTES_PER_PIXEL / YUV_422_BYTES_PER_PIXEL;

	return XN_STATUS_OK;
}

}
