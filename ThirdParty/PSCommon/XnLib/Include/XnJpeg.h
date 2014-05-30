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
#ifndef _XN_JPEG_H_
#define _XN_JPEG_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <XnOS.h>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
#define XN_STREAM_COMPRESSION_DEPTH16Z_WORSE_RATIO 1.333F
#define XN_STREAM_COMPRESSION_IMAGE8Z_WORSE_RATIO 1.333F
#define XN_STREAM_COMPRESSION_IMAGEJ_WORSE_RATIO 1.2F
#define XN_STREAM_COMPRESSION_CONF4_WORSE_RATIO 0.51F
#define XN_STREAM_COMPRESSION_JPEG_DEFAULT_QUALITY 90

//---------------------------------------------------------------------------
// Foreword decelerations
//---------------------------------------------------------------------------
struct XnStreamCompJPEGContext;
typedef XnStreamCompJPEGContext XnStreamCompJPEGContext;

struct XnStreamUncompJPEGContext;
typedef XnStreamUncompJPEGContext XnStreamUncompJPEGContext;

//---------------------------------------------------------------------------
// Functions Declaration
//---------------------------------------------------------------------------
XnStatus XnStreamInitCompressImageJ(  XnStreamCompJPEGContext** ppStreamCompJPEGContext);
XnStatus XnStreamCompressImage8J(     XnStreamCompJPEGContext** ppStreamCompJPEGContext, const XnUInt8* pInput, XnUInt8* pOutput, XnUInt32* pnOutputSize, const XnUInt32 nXRes, const XnUInt32 nYRes, const XnUInt32 nQuality);
XnStatus XnStreamCompressImage24J(    XnStreamCompJPEGContext** ppStreamCompJPEGContext, const XnUInt8* pInput, XnUInt8* pOutput, XnUInt32* pnOutputSize, const XnUInt32 nXRes, const XnUInt32 nYRes, const XnUInt32 nQuality);
XnStatus XnStreamFreeCompressImageJ(  XnStreamCompJPEGContext** ppStreamCompJPEGContext);

XnStatus XnStreamInitUncompressImageJ(XnStreamUncompJPEGContext** ppStreamUncompJPEGContext);
XnStatus XnStreamUncompressImageJ(    XnStreamUncompJPEGContext** ppStreamUncompJPEGContext, const XnUInt8* pInput, const XnUInt32 nInputSize, XnUInt8* pOutput, XnUInt32* pnOutputSize);
XnStatus XnStreamFreeUncompressImageJ(XnStreamUncompJPEGContext** ppStreamUncompJPEGContext);

#endif // _XN_JPEG_H_