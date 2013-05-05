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
#include "XnPacked11DepthProcessor.h"
#include <XnProfiling.h>
#ifdef XN_NEON
#include <arm_neon.h>
#endif

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
/* The size of an input element in the stream. */
#define XN_INPUT_ELEMENT_SIZE 11
/* The size of an output element in the stream. */
#define XN_OUTPUT_ELEMENT_SIZE 16

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

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnPacked11DepthProcessor::XnPacked11DepthProcessor(XnSensorDepthStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnDepthProcessor(pStream, pHelper, pBufferManager)
{
}

XnStatus XnPacked11DepthProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnDepthProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_BUFFER_ALLOCATE(m_ContinuousBuffer, XN_INPUT_ELEMENT_SIZE);

	return (XN_STATUS_OK);
}

XnPacked11DepthProcessor::~XnPacked11DepthProcessor()
{
}

XnStatus XnPacked11DepthProcessor::Unpack11to16(const XnUInt8* pcInput, const XnUInt32 nInputSize, XnUInt32* pnActualRead)
{
	const XnUInt8* pOrigInput = pcInput;

	XnUInt32 nElements = nInputSize / XN_INPUT_ELEMENT_SIZE; // floored
	XnUInt32 nNeededOutput = nElements * XN_OUTPUT_ELEMENT_SIZE;

	*pnActualRead = 0;
	XnBuffer* pWriteBuffer = GetWriteBuffer();

	// Check there is enough room for the depth pixels
	if (!CheckWriteBufferForOverflow(nNeededOutput))
	{
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	XnUInt16* pnOutput = (XnUInt16*)pWriteBuffer->GetUnsafeWritePointer();

	XnUInt16 a0,a1,a2,a3,a4,a5,a6,a7;
#ifdef XN_NEON
	XnUInt16 depth[8];
	uint16x8_t Q0;
#endif

	// Convert the 11bit packed data into 16bit shorts
	for (XnUInt32 nElem = 0; nElem < nElements; ++nElem)
	{
		// input:	0,  1,  2,3,  4,  5,  6,7,  8,  9,10
		//			-,---,---,-,---,---,---,-,---,---,-
		// bits:	8,3,5,6,2,8,1,7,4,4,7,1,8,2,6,5,3,8
		//			---,---,-----,---,---,-----,---,---
		// output:	  0,  1,    2,  3,  4,    5,  6,  7

		a0 = (XN_TAKE_BITS(pcInput[0],8,0) << 3) | XN_TAKE_BITS(pcInput[1],3,5);
		a1 = (XN_TAKE_BITS(pcInput[1],5,0) << 6) | XN_TAKE_BITS(pcInput[2],6,2);
		a2 = (XN_TAKE_BITS(pcInput[2],2,0) << 9) | (XN_TAKE_BITS(pcInput[3],8,0) << 1) | XN_TAKE_BITS(pcInput[4],1,7);
		a3 = (XN_TAKE_BITS(pcInput[4],7,0) << 4) | XN_TAKE_BITS(pcInput[5],4,4);
		a4 = (XN_TAKE_BITS(pcInput[5],4,0) << 7) | XN_TAKE_BITS(pcInput[6],7,1);
		a5 = (XN_TAKE_BITS(pcInput[6],1,0) << 10) | (XN_TAKE_BITS(pcInput[7],8,0) << 2) | XN_TAKE_BITS(pcInput[8],2,6);
		a6 = (XN_TAKE_BITS(pcInput[8],6,0) << 5) | XN_TAKE_BITS(pcInput[9],5,3);
		a7 = (XN_TAKE_BITS(pcInput[9],3,0) << 8) | XN_TAKE_BITS(pcInput[10],8,0);


#ifdef XN_NEON
		depth[0] = GetOutput(a0);
		depth[1] = GetOutput(a1);
		depth[2] = GetOutput(a2);
		depth[3] = GetOutput(a3);
		depth[4] = GetOutput(a4);
		depth[5] = GetOutput(a5);
		depth[6] = GetOutput(a6);
		depth[7] = GetOutput(a7);

		// Load
		Q0 = vld1q_u16(depth);
		// Store
		vst1q_u16(pnOutput, Q0);
#else
		pnOutput[0] = GetOutput(a0);
		pnOutput[1] = GetOutput(a1);
		pnOutput[2] = GetOutput(a2);
		pnOutput[3] = GetOutput(a3);
		pnOutput[4] = GetOutput(a4);
		pnOutput[5] = GetOutput(a5);
		pnOutput[6] = GetOutput(a6);
		pnOutput[7] = GetOutput(a7);
#endif

		pcInput += XN_INPUT_ELEMENT_SIZE;
		pnOutput += 8;
	}

	*pnActualRead = (XnUInt32)(pcInput - pOrigInput);
	pWriteBuffer->UnsafeUpdateSize(nNeededOutput);

	return XN_STATUS_OK;
}

void XnPacked11DepthProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData, XnUInt32 /*nDataOffset*/, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnPacked11DepthProcessor::ProcessFramePacketChunk")

	XnStatus nRetVal = XN_STATUS_OK;

	// check if we have data from previous packet
	if (m_ContinuousBuffer.GetSize() != 0)
	{
		// fill in to a whole element
		XnUInt32 nReadBytes = XN_MIN(nDataSize, XN_INPUT_ELEMENT_SIZE - m_ContinuousBuffer.GetSize());
		m_ContinuousBuffer.UnsafeWrite(pData, nReadBytes);
		pData += nReadBytes;
		nDataSize -= nReadBytes;

		if (m_ContinuousBuffer.GetSize() == XN_INPUT_ELEMENT_SIZE)
		{
			// process it
			XnUInt32 nActualRead = 0;
			Unpack11to16(m_ContinuousBuffer.GetData(), XN_INPUT_ELEMENT_SIZE, &nActualRead);
			m_ContinuousBuffer.Reset();
		}
	}

	// find out the number of input elements we have
	XnUInt32 nActualRead = 0;
	nRetVal = Unpack11to16(pData, nDataSize, &nActualRead);
	if (nRetVal == XN_STATUS_OK)
	{
		pData += nActualRead;
		nDataSize -= nActualRead;

		// if we have any bytes left, store them for next packet.
		if (nDataSize > 0)
		{
			// no need to check for overflow. there can not be a case in which more than XN_INPUT_ELEMENT_SIZE
			// are left.
			m_ContinuousBuffer.UnsafeWrite(pData, nDataSize);
		}
	}

	XN_PROFILING_END_SECTION
}

void XnPacked11DepthProcessor::OnStartOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnDepthProcessor::OnStartOfFrame(pHeader);
	m_ContinuousBuffer.Reset();
}

void XnPacked11DepthProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnDepthProcessor::OnEndOfFrame(pHeader);
	m_ContinuousBuffer.Reset();
}
