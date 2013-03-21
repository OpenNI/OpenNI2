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
#ifndef _XN_YUV_H_
#define _XN_YUV_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDeviceSensor.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define YUV422_U  0
#define YUV422_Y1 1
#define YUV422_V  2
#define YUV422_Y2 3
#define YUV422_BPP 4

#define YUYV_Y1 0
#define YUYV_U  1
#define YUYV_Y2 2
#define YUYV_V  3
#define YUYV_BPP 4

#define YUV420_U   0
#define YUV420_Y1  1
#define YUV420_Y2  2
#define YUV420_V   3
#define YUV420_Y3  4
#define YUV420_Y4  5
#define YUV420_BPP 6

#define YUV_RED   0
#define YUV_GREEN 1
#define YUV_BLUE  2
#define YUV_RGB_BPP 3

/* The size of an input element in the stream. */
#define XN_YUV_TO_RGB_INPUT_ELEMENT_SIZE	8
/* The size of an output element in the stream. */
#define XN_YUV_TO_RGB_OUTPUT_ELEMENT_SIZE	12

//---------------------------------------------------------------------------
// Functions Declaration
//---------------------------------------------------------------------------
void YUV422ToRGB888(const XnUInt8* pYUVImage, XnUInt8* pRGBImage, XnUInt32 nYUVSize, XnUInt32* pnActualRead, XnUInt32* pnRGBSize);
void YUYVToRGB888(const XnUInt8* pYUVImage, XnUInt8* pRGBImage, XnUInt32 nYUVSize, XnUInt32* pnActualRead, XnUInt32* pnRGBSize);
void YUV420ToRGB888(const XnUInt8* pYUVImage, XnUInt8* pRGBImage, XnUInt32 nYUVSize, XnUInt32 nRGBSize);

#endif //_XN_BAYER_H_
