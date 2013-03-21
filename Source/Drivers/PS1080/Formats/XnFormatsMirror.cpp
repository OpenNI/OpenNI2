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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <XnCore.h>
#include "XnFormats.h"
#include <XnOS.h>
#include <XnLog.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_MIRROR_MAX_LINE_SIZE	1920*3

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnStatus XnMirrorOneBytePixels(XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt32 nLineSize)
{
	// Local function variables
	XnUInt8* pSrc = pBuffer;
	XnUInt8 pLineBuffer[XN_MIRROR_MAX_LINE_SIZE];
	XnUInt8* pSrcEnd = pSrc + nBufferSize;
	XnUInt8* pDest = NULL;
	XnUInt8* pDestVal = &pLineBuffer[0] + nLineSize - 1;
	XnUInt8* pDestEnd = &pLineBuffer[0] - 1;

	if (nLineSize > XN_MIRROR_MAX_LINE_SIZE)
	{
		return (XN_STATUS_INTERNAL_BUFFER_TOO_SMALL);
	}

	while (pSrc < pSrcEnd)
	{
		xnOSMemCopy(pLineBuffer, pSrc, nLineSize);

		pDest = pDestVal;
		while (pDest != pDestEnd)
		{
			*pSrc = *pDest;

			pSrc++;
			pDest--;
		}
	}

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnMirrorTwoBytePixels(XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt32 nLineSize)
{
	// Local function variables
	XnUInt16* pSrc = (XnUInt16*)pBuffer;
	XnUInt16 pLineBuffer[XN_MIRROR_MAX_LINE_SIZE];
	XnUInt16* pSrcEnd = pSrc + nBufferSize / sizeof(XnUInt16);
	XnUInt16* pDest = NULL;
	XnUInt16* pDestVal = &pLineBuffer[0] + nLineSize - 1;
	XnUInt16* pDestEnd = &pLineBuffer[0] - 1;
	XnUInt16 nMemCpyLineSize = (XnUInt16)(nLineSize * sizeof(XnUInt16));
	XnUInt16 nValue;

	if (nLineSize > XN_MIRROR_MAX_LINE_SIZE)
	{
		return (XN_STATUS_INTERNAL_BUFFER_TOO_SMALL);
	}

	while (pSrc < pSrcEnd)
	{
		xnOSMemCopy(pLineBuffer, pSrc, nMemCpyLineSize);

		pDest = pDestVal;
		while (pDest != pDestEnd)
		{
			nValue = pDest[0];
			pSrc[0] = nValue;

			pDest--;
			pSrc++;
		}
	}

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnMirrorThreeBytePixels(XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt32 nLineSize)
{
	// Local function variables
	XnUInt8* pSrc = pBuffer;
	XnUInt8 pLineBuffer[XN_MIRROR_MAX_LINE_SIZE];
	XnUInt8* pSrcEnd = pSrc + nBufferSize;
	XnUInt8* pDest = NULL;
	XnUInt8* pDestVal = &pLineBuffer[0] + nLineSize * 3 - 1;
	XnUInt8* pDestEnd = &pLineBuffer[0] - 1;
	XnUInt16 nMemCpyLineSize = (XnUInt16)(nLineSize * 3);

	if (nMemCpyLineSize > XN_MIRROR_MAX_LINE_SIZE)
	{
		return (XN_STATUS_INTERNAL_BUFFER_TOO_SMALL);
	}

	while (pSrc < pSrcEnd)
	{
		xnOSMemCopy(pLineBuffer, pSrc, nMemCpyLineSize);

		pDest = pDestVal;
		while (pDest != pDestEnd)
		{
			*pSrc = *(pDest-2);
			*(pSrc+1) = *(pDest-1);
			*(pSrc+2) = *pDest;

			pSrc+=3;
			pDest-=3;
		}
	}

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnMirrorYUV422Pixels(XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt32 nLineSize)
{
	// Local function variables
	XnUInt8* pSrc = pBuffer;
	XnUInt8 pLineBuffer[XN_MIRROR_MAX_LINE_SIZE];
	XnUInt8* pSrcEnd = (XnUInt8*)pSrc + nBufferSize;
	XnUInt8* pDest = NULL;
	XnUInt8* pDestVal = &pLineBuffer[(nLineSize/2-1)*sizeof(XnUInt32)]; // last element
	XnUInt8* pDestEnd = &pLineBuffer[0]; // first element
	XnUInt32 nMemCpyLineSize = nLineSize/2*sizeof(XnUInt32);

	if (nMemCpyLineSize > XN_MIRROR_MAX_LINE_SIZE)
	{
		return (XN_STATUS_INTERNAL_BUFFER_TOO_SMALL);
	}

	while (pSrc < pSrcEnd)
	{
		xnOSMemCopy(pLineBuffer, pSrc, nMemCpyLineSize);
		pDest = pDestVal;

		while (pDest >= pDestEnd)
		{
			pSrc[0] = pDest[0]; // u
			pSrc[1] = pDest[3]; // y1 <-> y2
			pSrc[2] = pDest[2]; // v
			pSrc[3] = pDest[1]; // y2 <-> y1

			pSrc += 4;
			pDest -= 4;
		}
	}

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnMirrorYUYVPixels(XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt32 nLineSize)
{
	// Local function variables
	XnUInt8* pSrc = pBuffer;
	XnUInt8 pLineBuffer[XN_MIRROR_MAX_LINE_SIZE];
	XnUInt8* pSrcEnd = (XnUInt8*)pSrc + nBufferSize;
	XnUInt8* pDest = NULL;
	XnUInt8* pDestVal = &pLineBuffer[(nLineSize/2-1)*sizeof(XnUInt32)]; // last element
	XnUInt8* pDestEnd = &pLineBuffer[0]; // first element
	XnUInt32 nMemCpyLineSize = nLineSize/2*sizeof(XnUInt32);

	if (nMemCpyLineSize > XN_MIRROR_MAX_LINE_SIZE)
	{
		return (XN_STATUS_INTERNAL_BUFFER_TOO_SMALL);
	}

	while (pSrc < pSrcEnd)
	{
		xnOSMemCopy(pLineBuffer, pSrc, nMemCpyLineSize);
		pDest = pDestVal;

		while (pDest >= pDestEnd)
		{
			pSrc[0] = pDest[2]; // y1 <-> y2
			pSrc[1] = pDest[3]; // u
			pSrc[2] = pDest[0]; // y2 <-> y1
			pSrc[3] = pDest[1]; // v

			pSrc += 4;
			pDest -= 4;
		}
	}

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnFormatsMirrorPixelData(OniPixelFormat nOutputFormat, XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt32 nXRes)
{
	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(pBuffer);

	switch (nOutputFormat)
	{
	case ONI_PIXEL_FORMAT_SHIFT_9_2:
	case ONI_PIXEL_FORMAT_DEPTH_1_MM:
	case ONI_PIXEL_FORMAT_DEPTH_100_UM:
	case ONI_PIXEL_FORMAT_GRAY16:
		return XnMirrorTwoBytePixels(pBuffer, nBufferSize, nXRes);
	case ONI_PIXEL_FORMAT_GRAY8:
		return XnMirrorOneBytePixels(pBuffer, nBufferSize, nXRes);
	case ONI_PIXEL_FORMAT_YUV422:
		return XnMirrorYUV422Pixels(pBuffer, nBufferSize, nXRes);
	case ONI_PIXEL_FORMAT_YUYV:
		return XnMirrorYUYVPixels(pBuffer, nBufferSize, nXRes);
	case ONI_PIXEL_FORMAT_RGB888:
		return XnMirrorThreeBytePixels(pBuffer, nBufferSize, nXRes);
	default:
		xnLogError(XN_MASK_FORMATS, "Mirror was not implemented for output format %d", nOutputFormat);
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}
}
