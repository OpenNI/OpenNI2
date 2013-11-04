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
#include <XnOS.h>

#include "XnLink24zYuv422Parser.h"
#include "XnLinkYuvToRgb.h"

namespace xn
{

Link24zYuv422Parser::Link24zYuv422Parser(XnUInt32 xRes, XnUInt32 yRes, XnBool transformToRGB) : 
	m_dataFromPrevPacket(NULL),
	m_dataFromPrevPacketBytes(0),
	m_lineWidthBytes(xRes * LinkYuvToRgb::YUV_422_BYTES_PER_PIXEL), // 4 bytes for every 2 pixels
	m_rgbFrameSize(xRes * yRes * LinkYuvToRgb::RGB_888_BYTES_PER_PIXEL),
	m_expectedFrameSize(xRes * yRes * LinkYuvToRgb::YUV_422_BYTES_PER_PIXEL),
	m_transformToRGB(transformToRGB),
	m_tempYuvImage(NULL),
	m_tempYuvImageBytes(0)
{
}

Link24zYuv422Parser::~Link24zYuv422Parser()
{
	xnOSFree(m_dataFromPrevPacket);
	xnOSFree(m_tempYuvImage);
}

XnStatus Link24zYuv422Parser::Init()
{
	m_dataFromPrevPacket = (XnUInt8*)xnOSMallocAligned(m_lineWidthBytes, XN_DEFAULT_MEM_ALIGN);
	XN_VALIDATE_ALLOC_PTR(m_dataFromPrevPacket);

	if (m_transformToRGB)
	{
		m_tempYuvImage = (XnUInt8*)xnOSMallocAligned(m_rgbFrameSize, XN_DEFAULT_MEM_ALIGN);
		XN_VALIDATE_ALLOC_PTR(m_tempYuvImage);
	}

	return XN_STATUS_OK;
}

XnStatus Link24zYuv422Parser::ParsePacketImpl(XnLinkFragmentation fragmentation, const XnUInt8* pSrc, const XnUInt8* pSrcEnd, XnUInt8*& pDst, const XnUInt8* pDstEnd)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if ((fragmentation | XN_LINK_FRAG_BEGIN) != 0)
	{
		m_dataFromPrevPacketBytes = 0;
		m_tempYuvImageBytes = 0;
	}

	const XnUInt8* pInput = pSrc;
	XnSizeT inputSize = pSrcEnd - pSrc;

	// if there's data left from previous packet, append new data to it
	if (m_dataFromPrevPacketBytes > 0)
	{
		if (m_dataFromPrevPacketBytes + inputSize > m_expectedFrameSize)
		{
			XN_ASSERT(FALSE);
			m_dataFromPrevPacketBytes = 0;
			return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
		}

		xnOSMemCopy(m_dataFromPrevPacket, pSrc, inputSize);
		pInput = m_dataFromPrevPacket;
		inputSize = m_dataFromPrevPacketBytes + inputSize;
	}

	XnUInt8* pOutput = pDst;
	XnSizeT outputSize = pDstEnd - pDst;

	if (m_transformToRGB)
	{
		pOutput = m_tempYuvImage + m_tempYuvImageBytes;
		outputSize = m_rgbFrameSize - m_tempYuvImageBytes;
	}
	
	XnSizeT actualRead;
	nRetVal = Uncompress24z(pInput, inputSize, pOutput, &outputSize, m_lineWidthBytes, &actualRead, (fragmentation | XN_LINK_FRAG_END) == XN_LINK_FRAG_END);
	XN_IS_STATUS_OK(nRetVal);

	pDst += outputSize;

	// if we have bytes left, keep them for next packet
	if (actualRead < inputSize)
	{
		m_dataFromPrevPacketBytes = inputSize - actualRead;
		xnOSMemMove(m_dataFromPrevPacket, pInput + actualRead, m_dataFromPrevPacketBytes);
	}

	if ((fragmentation | XN_LINK_FRAG_END) != 0)
	{
		outputSize = pDstEnd - pDst;
		LinkYuvToRgb::Yuv422ToRgb888(m_tempYuvImage, m_tempYuvImageBytes, pDst, outputSize);
		pDst += outputSize;
	}
	
	return (XN_STATUS_OK);
}

XnStatus Link24zYuv422Parser::Uncompress24z(const XnUInt8* pInput, XnSizeT nInputSize,
	XnUInt8* pOutput, XnSizeT* pnOutputSize, XnUInt32 nLineSize,
	XnSizeT* pnActualRead, XnBool bLastPart)
{
	// Input is made of 4-bit elements.
	const XnUInt8* pInputOrig = pInput;
	const XnUInt8* pInputEnd = pInput + nInputSize;
	XnUInt8* pOrigOutput = pOutput;
	XnUInt8* pOutputEnd = pOutput + (*pnOutputSize);
	XnUInt8 nLastFullValue[4] = {0};	

	// NOTE: we use variables of type uint32 instead of uint8 as an optimization (better CPU usage)
	XnUInt32 nTempValue = 0;	
	XnUInt32 cInput = 0;
	XnBool bReadByte = TRUE;

	const XnUInt8* pInputLastPossibleStop = pInputOrig;
	XnUInt8* pOutputLastPossibleStop = pOrigOutput;

	*pnActualRead = 0;
	*pnOutputSize = 0;

	XnUInt32 nChannel = 0;
	XnUInt32 nCurLineSize = 0;

	while (pInput < pInputEnd)
	{
		cInput = *pInput;

		if (bReadByte)
		{
			bReadByte = FALSE;

			if (cInput < 0xd0) // 0x0 to 0xc are diffs
			{
				// take high_element only
				// diffs are between -6 and 6 (0x0 to 0xc)
				nLastFullValue[nChannel] += XnInt8((cInput >> 4) - 6);
			}
			else if (cInput < 0xe0) // 0xd is dummy
			{
				// Do nothing
				continue;
			}
			else // 0xe is not used, so this must be 0xf - full
			{
				// take two more elements
				nTempValue = (cInput & 0x0f) << 4;

				if (++pInput == pInputEnd)
					break;

				nTempValue += (*pInput >> 4);
				nLastFullValue[nChannel] = (XnUInt8)nTempValue;
			}
		}
		else
		{
			// take low-element
			cInput &= 0x0f;
			bReadByte = TRUE;
			pInput++;

			if (cInput < 0xd) // 0x0 to 0xc are diffs
			{
				// diffs are between -6 and 6 (0x0 to 0xc)
				nLastFullValue[nChannel] += (XnInt8)(cInput - 6);
			}
			else if (cInput < 0xe) // 0xd is dummy
			{
				// Do nothing
				continue;
			}
			else // 0xe is not in use, so this must be 0xf - full
			{
				if (pInput == pInputEnd)
					break;

				// take two more elements
				nLastFullValue[nChannel] = *pInput;
				pInput++;
			}
		}

		// write output
		if (pOutput > pOutputEnd)
		{
			return (XN_STATUS_OUTPUT_BUFFER_OVERFLOW);
		}

		*pOutput = nLastFullValue[nChannel];
		pOutput++;

		nChannel++;
		switch (nChannel)
		{
		case 2:
			nLastFullValue[3] = nLastFullValue[1];
			break;
		case 4:
			nLastFullValue[1] = nLastFullValue[3];
			nChannel = 0;
			break;
		}

		nCurLineSize++;
		if (nCurLineSize == nLineSize)
		{
			pInputLastPossibleStop = pInput;
			pOutputLastPossibleStop = pOutput;

			nLastFullValue[0] = nLastFullValue[1] = nLastFullValue[2] = nLastFullValue[3] = 0;
			nCurLineSize = 0;
		}
	}

	if (bLastPart == TRUE)
	{
		*pnOutputSize = (pOutput - pOrigOutput);
		*pnActualRead += (pInput - pInputOrig);
	}
	else if ((pOutputLastPossibleStop != pOrigOutput) && (pInputLastPossibleStop != pInputOrig))
	{
		*pnOutputSize = (pOutputLastPossibleStop - pOrigOutput);
		*pnActualRead +=(pInputLastPossibleStop - pInputOrig);
	}

	// All is good...
	return (XN_STATUS_OK);
}

}