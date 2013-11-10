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
#ifndef XNCORE_H
#define XNCORE_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnLib.h>
#include <XnPlatform.h>
#include <XnStatus.h>
#include <XnPsVersion.h>
#include <Driver/OniDriverAPI.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

/**
* Packs a pointer and a size into an XnGeneralBuffer struct.
XnGeneralBuffer.h-51-*/
inline OniGeneralBuffer XnGeneralBufferPack(void* pData, XnUInt32 nDataSize)
{
	OniGeneralBuffer result;
	result.data = pData;
	result.dataSize = nDataSize;
	return result;
}
#define XN_PACK_GENERAL_BUFFER(x)         XnGeneralBufferPack(&x, sizeof(x))

/**
* Copies one general buffer into another.
*/
XnStatus XnGeneralBufferCopy(OniGeneralBuffer* pDest, const OniGeneralBuffer* pSrc);
XnStatus XnGeneralBufferAlloc(OniGeneralBuffer* pDest, XnUInt32 nSize);
XnStatus XnGeneralBufferRealloc(OniGeneralBuffer* pDest, XnUInt32 nSize);
void XnGeneralBufferFree(OniGeneralBuffer* pDest);

#define XN_VALIDATE_GENERAL_BUFFER_TYPE(gb, t)  \
        if ((gb).dataSize != sizeof(t))                        \
        {                                                                                       \
                return XN_STATUS_INVALID_BUFFER_SIZE;   \
        }


/** represents a value for automatic control for nodes supporting it, as part of the @ref general_int. **/
#define XN_AUTO_CONTROL		XN_MIN_INT32

//---------------------------------------------------------------------------
// Exported Function Declaration
//---------------------------------------------------------------------------
/**
 * This function initializes the core low-level SDK. 
 */
XnStatus XnInit();

/**
* This function initializes the core low-level SDK from an INI file. 
* Please refer to the low-level SDK overview/tutorial section for a complete list of INI entries.
* Note: This function is not very useful on its own. You should use the I/O subsystem initializing instead.
* 
* @param	cpINIFileName		[in]	A path to an INI file.
*/
XnStatus XnInitFromINIFile(const XnChar* cpINIFileName);

/**
* This function shuts down the core low-level SDK. 
* Note: This function is not very useful on its own. You should use the I/O subsystem shutdown instead.
*/
XnStatus XnShutdown();

/**
 * Returns the Xiron version as an integer calculated from this formula:
 * (Xiron major version * 1000 + Xiron minor version)
 *
 * @return An integer representation of the Xiron version.
 */
XnUInt32	XnGetVersion(void);

/**
 * Returns the Xiron version as a string in the following format:
 * "Major.Minor-Platform (MMM DD YYYY HH:MM:SS)"
 * For example: "1.0-Win32 (Sep 19 2006 11:22:33)"
 *
 * @return A string representation of the Xiron version.
 */
const XnChar*	XnGetVersionString(void);

#endif // XNCORE_H
