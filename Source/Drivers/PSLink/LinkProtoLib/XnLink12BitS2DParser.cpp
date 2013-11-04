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
#include "XnLink12BitS2DParser.h"
#include "XnShiftToDepth.h"
#include "XnLinkProtoUtils.h"
#include <XnLog.h>

#ifdef XN_NEON
#include <arm_neon.h>
#endif

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------
/* Returns a set of <count> bits. For example XN_ON_BITS(4) returns 0xF */
#define XN_ON_BITS(count)				((1 << count)-1)

/* Creates a mask of <count> bits in offset <offset> */
#define XN_CREATE_MASK(count, offset)	(XN_ON_BITS(count) << offset)

/* Takes the <count> bits in offset <offset> from <source>.
*  For example: 
*  If we want 3 bits located in offset 2 from 0xF4:
*  11110100
*     ---
*  we get 101, which is 0x5.
*  and so, XN_TAKE_BITS(0xF4,3,2) == 0x5.
*/
#define XN_TAKE_BITS(source, count, offset)		((source & XN_CREATE_MASK(count, offset)) >> offset)

namespace xn
{

Link12BitS2DParser::Link12BitS2DParser(const XnShiftToDepthTables& shiftToDepthTables) :
	m_pShiftToDepth(shiftToDepthTables.pShiftToDepthTable)
{
}

Link12BitS2DParser::~Link12BitS2DParser()
{
}

XnStatus Link12BitS2DParser::ParsePacketImpl(XnLinkFragmentation fragmentation,
											 const XnUInt8* pSrc, 
											 const XnUInt8* pSrcEnd, 
											 XnUInt8*& pDst, 
											 const XnUInt8* pDstEnd)
{
	XN_ASSERT(m_pShiftToDepth != NULL);
	OniDepthPixel*& pDstPixel = reinterpret_cast<OniDepthPixel*&>(pDst);
	const OniDepthPixel* pDstPixelEnd = reinterpret_cast<const OniDepthPixel*>(pDstEnd);

	if ((fragmentation & XN_LINK_FRAG_BEGIN) != 0)
	{
		//Reset state for new frame
		m_ContinuousBufferSize=0;
	}

	XnUInt32 bytesWritten = ProcessFramePacketChunk(pSrc,pDst,(XnUInt32)(pSrcEnd - pSrc));
	
	pDstPixel += (OniDepthPixel)(bytesWritten/2); //progress pDst by the number of pixels (bytes divided by two)
	if (pDstPixel > pDstPixelEnd) //Do we have enough room for this packet?
	{ 
		XN_ASSERT(FALSE);
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	return XN_STATUS_OK;
}

XnUInt32 Link12BitS2DParser::ProcessFramePacketChunk(const XnUInt8* pData,XnUInt8* pDest, XnUInt32 nDataSize)
{
	
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 totalRead = 0;
	XnUInt32 totalWrite = 0;

	// check if we have data from previous packet
	if (m_ContinuousBufferSize!= 0)
	{
		// fill in to a whole element
		XnUInt32 nReadBytes = XN_MIN(nDataSize, XN_INPUT_ELEMENT_SIZE - m_ContinuousBufferSize);

		xnOSMemCopy(m_ContinuousBuffer + m_ContinuousBufferSize, pData, nReadBytes);
		m_ContinuousBufferSize += nReadBytes;

		pData += nReadBytes;
		nDataSize -= nReadBytes;

		if (m_ContinuousBufferSize == XN_INPUT_ELEMENT_SIZE)
		{
			// process it
			XnUInt32 nActualRead = 0;
			XnUInt32 nActualWritten = 0;
			Unpack12to16(m_ContinuousBuffer,pDest, XN_INPUT_ELEMENT_SIZE, &nActualRead, &nActualWritten);
			pDest += nActualWritten;
			totalRead += nActualRead;
			totalWrite += nActualWritten;
			m_ContinuousBufferSize = 0;
		}
	}

	// find out the number of input elements we have
	XnUInt32 nActualRead = 0;
	XnUInt32 nActualWritten = 0;
	nRetVal = Unpack12to16(pData, pDest, nDataSize, &nActualRead, &nActualWritten);
	totalRead += nActualRead;
	totalWrite += nActualWritten;
	if (nRetVal == XN_STATUS_OK)
	{
		pData += nActualRead;
		nDataSize -= nActualRead;

		// if we have any bytes left, store them for next packet.
		if (nDataSize > 0)
		{
			// no need to check for overflow. there can not be a case in which more than XN_INPUT_ELEMENT_SIZE
			// are left.

			xnOSMemCopy(m_ContinuousBuffer + m_ContinuousBufferSize, pData, nDataSize);
			m_ContinuousBufferSize += nDataSize;
		}
	}
	return totalWrite; //return total written bytes
}

XnStatus Link12BitS2DParser::Unpack12to16(const XnUInt8* pcInput,XnUInt8* pDest, const XnUInt32 nInputSize, XnUInt32* pnActualRead, XnUInt32* pnActualWritten)
{
	const XnUInt8* pOrigInput = (XnUInt8*)pcInput;

	XnUInt32 nElements = nInputSize / XN_INPUT_ELEMENT_SIZE; // floored
	//XnUInt32 nNeededOutput = nElements * XN_OUTPUT_ELEMENT_SIZE;
	
	*pnActualRead = 0;

	XnUInt16 *pnOutput = (XnUInt16*)pDest;
	XnUInt16 shift[16];
#ifdef XN_NEON
	XnUInt16 depth[16];
	uint8x8x3_t inD3;
	uint8x8_t rshft4D, lshft4D;
	uint16x8_t rshft4Q, lshft4Q;
	uint16x8_t depthQ;
	uint16x8x2_t shiftQ2;
#endif

	// Convert the 11bit packed data into 16bit shorts
	for (XnUInt32 nElem = 0; nElem < nElements; ++nElem)
	{
#ifndef XN_NEON
		// input:	0,  1,2,3,  4,5,6,  7,8,9, 10,11,12, 13,14,15, 16,17,18, 19,20,21, 22,23
		//			-,---,-,-,---,-,-,---,-,-,---,--,--,---,--,--,---,--,--,---,--,--,---,--
		// bits:	8,4,4,8,8,4,4,8,8,4,4,8,8,4,4, 8, 8,4,4, 8, 8,4,4, 8, 8,4,4, 8, 8,4,4, 8
		//			---,---,---,---,---,---,---,----,----,----,----,----,----,----,----,----
		// output:	  0,  1,  2,  3,  4,  5,  6,   7,   8,   9,  10,  11,  12,  13,  14,  15

		shift[0] = (XN_TAKE_BITS(pcInput[0],8,0) << 4) | XN_TAKE_BITS(pcInput[1],4,4);
		shift[1] = (XN_TAKE_BITS(pcInput[1],4,0) << 8) | XN_TAKE_BITS(pcInput[2],8,0);
		shift[2] = (XN_TAKE_BITS(pcInput[3],8,0) << 4) | XN_TAKE_BITS(pcInput[4],4,4);
		shift[3] = (XN_TAKE_BITS(pcInput[4],4,0) << 8) | XN_TAKE_BITS(pcInput[5],8,0);
		shift[4] = (XN_TAKE_BITS(pcInput[6],8,0) << 4) | XN_TAKE_BITS(pcInput[7],4,4);
		shift[5] = (XN_TAKE_BITS(pcInput[7],4,0) << 8) | XN_TAKE_BITS(pcInput[8],8,0);
		shift[6] = (XN_TAKE_BITS(pcInput[9],8,0) << 4) | XN_TAKE_BITS(pcInput[10],4,4);
		shift[7] = (XN_TAKE_BITS(pcInput[10],4,0) << 8) | XN_TAKE_BITS(pcInput[11],8,0);
		shift[8] = (XN_TAKE_BITS(pcInput[12],8,0) << 4) | XN_TAKE_BITS(pcInput[13],4,4);
		shift[9] = (XN_TAKE_BITS(pcInput[13],4,0) << 8) | XN_TAKE_BITS(pcInput[14],8,0);
		shift[10] = (XN_TAKE_BITS(pcInput[15],8,0) << 4) | XN_TAKE_BITS(pcInput[16],4,4);
		shift[11] = (XN_TAKE_BITS(pcInput[16],4,0) << 8) | XN_TAKE_BITS(pcInput[17],8,0);
		shift[12] = (XN_TAKE_BITS(pcInput[18],8,0) << 4) | XN_TAKE_BITS(pcInput[19],4,4);
		shift[13] = (XN_TAKE_BITS(pcInput[19],4,0) << 8) | XN_TAKE_BITS(pcInput[20],8,0);
		shift[14] = (XN_TAKE_BITS(pcInput[21],8,0) << 4) | XN_TAKE_BITS(pcInput[22],4,4);
		shift[15] = (XN_TAKE_BITS(pcInput[22],4,0) << 8) | XN_TAKE_BITS(pcInput[23],8,0);

		pnOutput[0] = m_pShiftToDepth[(shift[0])];
		pnOutput[1] = m_pShiftToDepth[(shift[1])];
		pnOutput[2] = m_pShiftToDepth[(shift[2])];
		pnOutput[3] = m_pShiftToDepth[(shift[3])];
		pnOutput[4] = m_pShiftToDepth[(shift[4])];
		pnOutput[5] = m_pShiftToDepth[(shift[5])];
		pnOutput[6] = m_pShiftToDepth[(shift[6])];
		pnOutput[7] = m_pShiftToDepth[(shift[7])];
		pnOutput[8] = m_pShiftToDepth[(shift[8])];
		pnOutput[9] = m_pShiftToDepth[(shift[9])];
		pnOutput[10] = m_pShiftToDepth[(shift[10])];
		pnOutput[11] = m_pShiftToDepth[(shift[11])];
		pnOutput[12] = m_pShiftToDepth[(shift[12])];
		pnOutput[13] = m_pShiftToDepth[(shift[13])];
		pnOutput[14] = m_pShiftToDepth[(shift[14])];
		pnOutput[15] = m_pShiftToDepth[(shift[15])];
#else
		// input:	0,  1,2    (X8)
		//			-,---,-
		// bits:	8,4,4,8    (X8)
		//			---,---
		// output:	  0,  1    (X8)

		// Split 24 bytes into 3 vectors (64 bit each)
		inD3 = vld3_u8(pcInput);

		// rshft4D0 contains 4 MSB of second vector (placed at offset 0)
		rshft4D = vshr_n_u8(inD3.val[1], 4);
		// lshft4D0 contains 4 LSB of second vector (placed at offset 4)
		lshft4D = vshl_n_u8(inD3.val[1], 4);

		// Expand 64 bit vectors to 128 bit (8 values of 16 bits)
		shiftQ2.val[0] = vmovl_u8(inD3.val[0]);
		shiftQ2.val[1] = vmovl_u8(inD3.val[2]);
		rshft4Q = vmovl_u8(rshft4D);
		lshft4Q = vmovl_u8(lshft4D);

		// Even indexed shift = 8 bits from first vector + 4 MSB bits of second vector
		shiftQ2.val[0] = vshlq_n_u16(shiftQ2.val[0], 4);
		shiftQ2.val[0] = vorrq_u16(shiftQ2.val[0], rshft4Q);

		// Odd indexed shift = 4 LSB bits of second vector + 8 bits from third vector
		lshft4Q = vshlq_n_u16(lshft4Q, 4);
		shiftQ2.val[1] = vorrq_u16(shiftQ2.val[1], lshft4Q);

		// Interleave shift values to a single vector
		vst2q_u16(shift, shiftQ2);

		depth[0] = m_pShiftToDepth[(shift[0])];
		depth[1] = m_pShiftToDepth[(shift[1])];

		depth[2] = m_pShiftToDepth[(shift[2])];
		depth[3] = m_pShiftToDepth[(shift[3])];

		depth[4] = m_pShiftToDepth[(shift[4])];
		depth[5] = m_pShiftToDepth[(shift[5])];

		depth[6] = m_pShiftToDepth[(shift[6])];
		depth[7] = m_pShiftToDepth[(shift[7])];

		// Load
		depthQ = vld1q_u16(depth);
		//Store
		vst1q_u16(pnOutput, depthQ);

		depth[8] = m_pShiftToDepth[(shift[8])];
		depth[9] = m_pShiftToDepth[(shift[9])];

		depth[10] = m_pShiftToDepth[(shift[10])];
		depth[11] = m_pShiftToDepth[(shift[11])];

		depth[12] = m_pShiftToDepth[(shift[12])];
		depth[13] = m_pShiftToDepth[(shift[13])];

		depth[14] = m_pShiftToDepth[(shift[14])];
		depth[15] = m_pShiftToDepth[(shift[15])];

		// Load
		depthQ = vld1q_u16(depth + 8);
		// Store
		vst1q_u16(pnOutput + 8, depthQ);
#endif
		pcInput += XN_INPUT_ELEMENT_SIZE;
		pnOutput += 16;
	}
	
	*pnActualRead = (XnUInt32)(pcInput - pOrigInput); // total bytes 
	*pnActualWritten = (XnUInt32)((XnUInt8*)pnOutput - pDest);

	return XN_STATUS_OK;
}

}
