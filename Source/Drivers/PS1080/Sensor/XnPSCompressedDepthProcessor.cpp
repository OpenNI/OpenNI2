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
#include "XnPSCompressedDepthProcessor.h"
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnPSCompressedDepthProcessor::XnPSCompressedDepthProcessor(XnSensorDepthStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager) :
	XnDepthProcessor(pStream, pHelper, pBufferManager)
{
}

XnStatus XnPSCompressedDepthProcessor::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnDepthProcessor::Init();
	XN_IS_STATUS_OK(nRetVal);

	XN_VALIDATE_BUFFER_ALLOCATE(m_RawData, GetExpectedOutputSize());

	return XN_STATUS_OK;
}

XnPSCompressedDepthProcessor::~XnPSCompressedDepthProcessor()
{
}

#define XN_CHECK_UNC_DEPTH_OUTPUT(x, y, z)			\
	if (x >= y)										\
	{												\
		return (XN_STATUS_OUTPUT_BUFFER_OVERFLOW);	\
	}												\
	if (z >= XN_DEVICE_SENSOR_MAX_SHIFT_VALUE)		\
	{												\
		z = XN_DEVICE_SENSOR_NO_DEPTH_VALUE;		\
	}	

#define XN_DEPTH_OUTPUT(pDepthOutput, pOutputEnd, nValue)					                \
	XN_CHECK_UNC_DEPTH_OUTPUT(pDepthOutput, pOutputEnd, nValue)				                \
	*pDepthOutput = GetOutput(nValue);										                \
	++pDepthOutput;

#define INIT_INPUT(pInput, nInputSize)					\
	const XnUInt8* __pInputOrig = pInput;				\
	const XnUInt8* __pCurrInput = pInput;				\
	const XnUInt8* __pInputEnd = pInput + nInputSize;	\
	XnBool __bShouldReadByte = TRUE;					\
	XnUInt32 __nLastByte = 0;

#define GET_NEXT_INPUT(nInput)								\
	if (__bShouldReadByte)									\
	{														\
		if (__pCurrInput == __pInputEnd)					\
			break;											\
															\
		/* read from input */								\
		__nLastByte = *__pCurrInput;						\
		__bShouldReadByte = FALSE;							\
															\
		/* take high 4-bits */								\
		nInput = __nLastByte >> 4;							\
															\
		__pCurrInput++;										\
	}														\
	else													\
	{														\
		/* byte already read. take its low 4-bits */		\
		nInput = __nLastByte & 0x0F;						\
		__bShouldReadByte = TRUE;							\
	}

/** True if input is in a steady state (not in the middle of a byte) */
#define CAN_INPUT_STOP_HERE __bShouldReadByte

/** Gets a pointer to n elements before current input */
#define GET_PREV_INPUT(n) __pCurrInput - n/2;

#define GET_INPUT_READ_BYTES (__pCurrInput - __pInputOrig);

XnStatus XnPSCompressedDepthProcessor::UncompressDepthPS(const XnUInt8* pInput, const XnUInt32 nInputSize,
								   XnUInt16* pDepthOutput, XnUInt32* pnOutputSize,
								   XnUInt32* pnActualRead, XnBool bLastPart)
{
	// Input is made of 4-bit elements.
	INIT_INPUT(pInput, nInputSize);

	XnUInt16* pOutputEnd = pDepthOutput + (*pnOutputSize / sizeof(OniDepthPixel));
	XnUInt16 nLastValue = 0;

	const XnUInt8* pInputOrig = pInput;
	XnUInt16* pOutputOrig = pDepthOutput;

	const XnUInt8* pInputLastPossibleStop = pInputOrig;
	XnUInt16* pOutputLastPossibleStop = pOutputOrig;

	// NOTE: we use variables of type uint32 instead of uint8 as an optimization (better CPU usage)
	XnUInt32 nInput;
	XnUInt32 nLargeValue;
	XnBool bCanStop;

	for (;;)
	{
		bCanStop = CAN_INPUT_STOP_HERE;
		GET_NEXT_INPUT(nInput);

		switch (nInput)
		{
		case 0xd: // Dummy.
			// Do nothing
			break;
		case 0xe: // RLE
			// read count
			GET_NEXT_INPUT(nInput);

			// should repeat last value (nInput + 1) times
			nInput++;
			while (nInput != 0)
			{
				XN_DEPTH_OUTPUT(pDepthOutput, pOutputEnd, nLastValue);
				--nInput;
			}
			break;

		case 0xf: // Full (or large)
			// read next element
			GET_NEXT_INPUT(nInput);

			// First bit tells us if it's a large diff (turned on) or a full value (turned off)
			if (nInput & 0x8) // large diff (7-bit)
			{
				// turn off high bit, and shift left
				nLargeValue = (nInput - 0x8) << 4;

				// read low 4-bits
				GET_NEXT_INPUT(nInput);

				nLargeValue |= nInput;
				// diff values are from -64 to 63 (0x00 to 0x7f)
				nLastValue += ((XnInt16)nLargeValue - 64);
			}
			else // Full value (15-bit)
			{
				if (bCanStop)
				{
					// We can stop here. First input is a full value
					pInputLastPossibleStop = GET_PREV_INPUT(2);
					pOutputLastPossibleStop = pDepthOutput;
				}

				nLargeValue = (nInput << 12);

				// read 3 more elements
				GET_NEXT_INPUT(nInput);
				nLargeValue |= nInput << 8;

				GET_NEXT_INPUT(nInput);
				nLargeValue |= nInput << 4;

				GET_NEXT_INPUT(nInput);
				nLastValue = (XnUInt16)(nLargeValue | nInput);
			}

			XN_DEPTH_OUTPUT(pDepthOutput, pOutputEnd, nLastValue);

			break;
		default: // all rest (smaller than 0xd) are diffs
			// diff values are from -6 to 6 (0x0 to 0xc)
			nLastValue += ((XnInt16)nInput - 6);
			XN_DEPTH_OUTPUT(pDepthOutput, pOutputEnd, nLastValue);
		}
	}

	if (bLastPart == TRUE)
	{
		*pnOutputSize = (XnUInt32)(pDepthOutput - pOutputOrig) * sizeof(XnUInt16);
		*pnActualRead = (XnUInt32)GET_INPUT_READ_BYTES;
	}
	else
	{
		*pnOutputSize = (XnUInt32)(pOutputLastPossibleStop - pOutputOrig) * sizeof(XnUInt16);
		*pnActualRead = (XnUInt32)(pInputLastPossibleStop - pInputOrig) * sizeof(XnUInt8);
	}

	// All is good...
	return (XN_STATUS_OK);
}

void XnPSCompressedDepthProcessor::ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize)
{
	XN_PROFILING_START_SECTION("XnPSCompressedDepthProcessor::ProcessFramePacketChunk")

	XnBuffer* pWriteBuffer = GetWriteBuffer();

	const XnUChar* pBuf = NULL;
	XnUInt32 nBufSize = 0;

	// check if we have bytes stored from previous calls
	if (m_RawData.GetSize() > 0)
	{
		// we have no choice. We need to append current buffer to previous bytes
		if (m_RawData.GetFreeSpaceInBuffer() < nDataSize)
		{
			xnLogWarning(XN_MASK_SENSOR_PROTOCOL_DEPTH, "Bad overflow depth! %d", m_RawData.GetSize());
			FrameIsCorrupted();
		}
		else
		{
			m_RawData.UnsafeWrite(pData, nDataSize);
		}

		pBuf = m_RawData.GetData();
		nBufSize = m_RawData.GetSize();
	}
	else
	{
		// we can process the data directly
		pBuf = pData;
		nBufSize = nDataSize;
	}

	XnUInt32 nOutputSize = pWriteBuffer->GetFreeSpaceInBuffer();
	XnUInt32 nWrittenOutput = nOutputSize;
	XnUInt32 nActualRead = 0;
	XnBool bLastPart = pHeader->nType == XN_SENSOR_PROTOCOL_RESPONSE_DEPTH_END && (nDataOffset + nDataSize) == pHeader->nBufSize;
	XnStatus nRetVal = UncompressDepthPS(pBuf, nBufSize, (XnUInt16*)pWriteBuffer->GetUnsafeWritePointer(), 
			&nWrittenOutput, &nActualRead, bLastPart);

	if (nRetVal != XN_STATUS_OK)
	{
		FrameIsCorrupted();

		static XnUInt64 nLastPrinted = 0;

		XnUInt64 nCurrTime;
		xnOSGetTimeStamp(&nCurrTime);

		if (nOutputSize != 0 || (nCurrTime - nLastPrinted) > 1000) 
		{
			xnLogWarning(XN_MASK_SENSOR_PROTOCOL_DEPTH, "Uncompress depth failed: %s. Input Size: %u, Output Space: %u, Last Part: %d.", xnGetStatusString(nRetVal), nBufSize, nOutputSize, bLastPart);

			xnOSGetTimeStamp(&nLastPrinted);
		}
	}

	pWriteBuffer->UnsafeUpdateSize(nWrittenOutput);

	nBufSize -= nActualRead;
	m_RawData.Reset();

	// if we have any bytes left, keep them for next time
	if (nBufSize > 0)
	{
		pBuf += nActualRead;
		m_RawData.UnsafeWrite(pBuf, nBufSize);
	}

	XN_PROFILING_END_SECTION
}

void XnPSCompressedDepthProcessor::OnStartOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnDepthProcessor::OnStartOfFrame(pHeader);
	m_RawData.Reset();
}

void XnPSCompressedDepthProcessor::OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader)
{
	XnDepthProcessor::OnEndOfFrame(pHeader);
	m_RawData.Reset();
}

