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
#ifndef XN16ZEMBTABLESCODEC_H
#define XN16ZEMBTABLESCODEC_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnJpeg.h>
#include "XnStreamCompression.h"
#include "XnCodecBase.h"
#include "XnCodecIDs.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class Xn16zEmbTablesCodec : public XnCodecBase
{
public:
	Xn16zEmbTablesCodec(XnUInt16 nMaxValue) : m_nMaxValue(nMaxValue) {}

	virtual XnCodecID GetCodecID() const { return XN_CODEC_16Z_EMB_TABLES; }
	virtual XnCompressionFormats GetCompressionFormat() const { return XN_COMPRESSION_16Z_EMB_TABLE; }

	virtual XnFloat GetWorseCompressionRatio() const { return XN_STREAM_COMPRESSION_DEPTH16Z_WORSE_RATIO; }
	virtual XnUInt32 GetOverheadSize() const { return m_nMaxValue * sizeof(XnUInt16); }

protected:
	virtual XnStatus CompressImpl(const XnUChar* pData, XnUInt32 nDataSize, XnUChar* pCompressedData, XnUInt32* pnCompressedDataSize)
	{
		return XnStreamCompressDepth16ZWithEmbTable((XnUInt16*)pData, nDataSize, pCompressedData, pnCompressedDataSize, m_nMaxValue);
	}

	virtual XnStatus DecompressImpl(const XnUChar* pCompressedData, XnUInt32 nCompressedDataSize, XnUChar* pData, XnUInt32* pnDataSize) 
	{
		return XnStreamUncompressDepth16ZWithEmbTable(pCompressedData, nCompressedDataSize, (XnUInt16*)pData, pnDataSize);
	}

private:
	XnUInt16 m_nMaxValue;
};

#endif // XN16ZEMBTABLESCODEC_H
