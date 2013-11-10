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
#ifndef XNSTREAMFORMATS_H
#define XNSTREAMFORMATS_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <OniCTypes.h>

//---------------------------------------------------------------------------
// Formats
//---------------------------------------------------------------------------

typedef enum
{
	/** Data is stored uncompressed. */
	XN_COMPRESSION_NONE = 0,
	/** Data is compressed using PS lossless 16-bit depth compression. */
	XN_COMPRESSION_16Z = 1,
	/** Data is compressed using PS lossless 16-bit depth compression with embedded tables. */
	XN_COMPRESSION_16Z_EMB_TABLE = 2,
	/** Data is compressed using PS lossless 8-bit image compression (for grayscale). */
	XN_COMPRESSION_COLOR_8Z = 3,
	/** Data is compressed using JPEG. */
	XN_COMPRESSION_JPEG = 4,
	/** Data is packed in 10-bit values. */
	XN_COMPRESSION_10BIT_PACKED = 5,
} XnCompressionFormats;

#endif // XNSTREAMFORMATS_H
