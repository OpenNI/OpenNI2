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

#ifndef _XN_SIMD_H_
#define _XN_SIMD_H_

#if !defined(XN_NEON) && !defined(XN_SSE) && !defined(_ARC)
// try to detect SSE by platform and compile flags
	#if defined(__GNUC__) 
		#if (defined(__i386__) || defined(__x86_64__)) \
    		 && (defined(__SSE2__) || defined(__SSE3__) || defined(__SSSE3__)) 
			#define XN_SSE
		#endif
	#else
	// windows, default to sse
		#define XN_SSE
	#endif
#endif

// Define XN_INT128
#ifdef XN_NEON
	#include "XnSIMD-Neon.h"
#elif defined(XN_SSE)
	#include "XnSIMD-SSE.h"
#else
	#include "XnSIMD-None.h"
#endif

#if 0
// Use this for debugging
inline const char *ToHexString(const XN_INT128 &val)
{
    static char buffer[33];
    unsigned char *px = (unsigned char *)&val;
    for(int i=0; i<16; i++) sprintf(&buffer[2*i],"%02x",(int)px[i]);
    buffer[32] = '\0';
    return buffer;
}
#endif

#endif // _XN_SIMD_H_
