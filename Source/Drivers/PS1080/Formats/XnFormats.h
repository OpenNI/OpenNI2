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
#ifndef _XN_FORMATS_H_
#define _XN_FORMATS_H_

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <XnStatus.h>
#include <XnFormatsStatus.h>
#include <XnStreamParams.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_MASK_FORMATS "XnFormats"

//---------------------------------------------------------------------------
// Exported Functions
//---------------------------------------------------------------------------

/**
* This function receives a buffer of pixel data of a known format, and mirrors it.
*
* @param	nOutputFormat	[in]	The format of the pixel data.
* @param	pBuffer			[in]	A pointer to the buffer.
* @param	nBufferSize		[in]	The size of the buffer, in bytes.
* @param	nXRes			[in]	X-resolution (line size in pixels) of the buffer.
*/
XnStatus XnFormatsMirrorPixelData(OniPixelFormat nOutputFormat, XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt32 nXRes);

#endif //_XN_FORMATS_H_
