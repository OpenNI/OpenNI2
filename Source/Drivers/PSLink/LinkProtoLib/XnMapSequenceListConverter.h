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
// XnMapSequenceListConverter.h

#ifndef XNMAPSEQUENCELISTCONVERTER_H
#define XNMAPSEQUENCELISTCONVERTER_H


namespace xn
{

template <typename PixelType>
class XnMapSequenceListConverter
{
private:
	typedef XnUInt16 ValueType;

	struct ValueHeader
	{
		ValueType	nValue;
		XnUInt16	nSequences;
	};

	struct Sequence
	{
		XnUInt16 nOffset;
		XnUInt16 nSequenceCount;
	};

	XnStatus FillMapTopDown(PixelType* pMap, XnUInt32 nHeight, XnUInt32 nWidth, XnUInt32& nX, XnUInt32& nY, XnUInt32 nPixels, PixelType val)
	{
		XnUInt32 nCurrIndex = nWidth * nY + nX;

		for (XnUInt32 pxl = 0; pxl < nPixels; pxl++ )
		{
			// Check indexes
			if (nX >= nWidth || nY >= nHeight)
			{
				return XN_STATUS_BAD_PARAM;
			}
			pMap[nCurrIndex] = val;

			nY++;
			if (nY == nHeight)
			{
				// Move to next column start
				nY = 0;				
				nX++;
				nCurrIndex = nX;
			}
			else
			{
				// Move next row, same column
				nCurrIndex += nWidth;
			}
		}

		return XN_STATUS_OK;
	}

public:

	// We processing the map up down first, because people are more likely to stand next to each other than on top of each other. 
	XnStatus MapToSequenceList(const PixelType* pMap, XnUInt32 nHeight, XnUInt32 nWidth, XnUChar* pSeqBuffer, XnUInt32& nSeqSize)
	{
		const PixelType NA_PIXEL		= (PixelType)-1;
		const PixelType	MAX_PIXEL_SIZE	= (1 << sizeof(ValueType) * 8) - 1;

		// Current sequence related data
		XnUInt16		nOffsetCount		= 0;
		PixelType		sequncePixel		= NA_PIXEL;	

		// Value related data
		ValueHeader*	pCurrValueHeader	= NULL;
		Sequence*		pCurrSeq			= NULL;

		// Output related data
		XnUChar*	pCurrOutput = pSeqBuffer;
		XnUInt		nCurrOutputSize	= 0;

		const PixelType*	pColumn = pMap;
		for (XnUInt32 x = 0; x < nWidth; x++)
		{
			const PixelType* pCurr = pColumn;		

			for (XnUInt32 y = 0; y < nHeight; y++)
			{
				// Make sure no pixel uses NA_PIXEL value and that the pixel will fit ValueHeader.nValue
				if (*pCurr == NA_PIXEL || *pCurr > MAX_PIXEL_SIZE)
				{
					return XN_STATUS_BAD_TYPE;
				}

				// See if the pixel is part of an offset
				if (*pCurr == 0)
				{
					nOffsetCount++;
					sequncePixel = NA_PIXEL;
					pCurrSeq = NULL;
				}
				// See if the pixel is in a sequence
				else if (*pCurr == sequncePixel)
				{
					XN_ASSERT(pCurrSeq);
					pCurrSeq->nSequenceCount++;					
				}
				// New sequnce start
				else
				{
					// Check if we need to add ValueHeader
					if (!pCurrValueHeader || pCurrValueHeader->nValue != *pCurr)
					{
						// New value in front of us, add value header
						nCurrOutputSize += sizeof(ValueHeader);
						if (nCurrOutputSize > nSeqSize) 
							return XN_STATUS_INVALID_BUFFER_SIZE;
						
						pCurrValueHeader = (ValueHeader*)pCurrOutput;
						pCurrOutput		+= sizeof(ValueHeader);

						pCurrValueHeader->nValue		= (ValueType)*pCurr;
						pCurrValueHeader->nSequences	= 1;
					}
					else
					{
						pCurrValueHeader->nSequences++;
					}
					
					
					// Store new sequence pixel
					sequncePixel = *pCurr;

					// Add sequence data
					nCurrOutputSize += sizeof(Sequence);
					if (nCurrOutputSize > nSeqSize) 
						return XN_STATUS_INVALID_BUFFER_SIZE;
				
					pCurrSeq					= (Sequence*)pCurrOutput;
					pCurrSeq->nOffset			= nOffsetCount;
					pCurrSeq->nSequenceCount	= 1;
					nOffsetCount				= 0;

					pCurrOutput += sizeof(Sequence);
				}

				// Move to next row
				pCurr += nWidth;
			}
			pColumn++;
		}		

		nSeqSize = nCurrOutputSize;
		return XN_STATUS_OK;
	}

	XnStatus SequenceListToMap(XnUChar* pSeqBuffer, XnUInt32 nSeqSize, PixelType* pMap, XnUInt32 nHeight, XnUInt32 nWidth)
	{
		XnUInt32 nCurrProcessed = 0;
		XnUInt32 nX = 0;
		XnUInt32 nY = 0;

		XnUChar* pCurr = pSeqBuffer;

		// Default all pixels
		xnOSMemSet(pMap, 0, sizeof(PixelType) * nHeight * nWidth);

		while (nCurrProcessed != nSeqSize)
		{
			nCurrProcessed += sizeof(ValueHeader);
			if (nCurrProcessed > nSeqSize) 
				return XN_STATUS_INVALID_BUFFER_SIZE;

			// Get the value header
			ValueHeader* pCurrValue = (ValueHeader*)pCurr;
			pCurr += sizeof(ValueHeader);

			// Loop on value sequences
			for (XnUInt16 nSeq = 0; nSeq < pCurrValue->nSequences; nSeq++ )
			{
				nCurrProcessed += sizeof(Sequence);
				if (nCurrProcessed > nSeqSize) 
					return XN_STATUS_INVALID_BUFFER_SIZE;
				Sequence* pSeq = (Sequence*)pCurr;
				pCurr += sizeof(Sequence);
				
				// Add offset to x,y
				XnUInt nNaiveY = nY +  pSeq->nOffset;
				nX += nNaiveY / nHeight;
				if (nX	>= nWidth) 
					return XN_STATUS_BAD_PARAM;
				nY = nNaiveY % nHeight;
				
				// Fill value
				XnStatus rc = FillMapTopDown(pMap, nHeight, nWidth, nX, nY, pSeq->nSequenceCount, (PixelType)pCurrValue->nValue);
				XN_IS_STATUS_OK(rc);


			}
		}
		
		return XN_STATUS_OK;
	}
};


}

#endif // XNMAPSEQUENCELISTCONVERTER_H
