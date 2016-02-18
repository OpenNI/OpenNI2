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
#ifndef XNUNCOMPRESSEDCODEC_H
#define XNUNCOMPRESSEDCODEC_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnCodecBase.h"
#include "XnCodecIDs.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnUncompressedCodec : public XnCodecBase
{
public:
	XnUncompressedCodec() {}
	virtual ~XnUncompressedCodec() {}

	virtual XnCodecID GetCodecID() const { return XN_CODEC_UNCOMPRESSED; }

	virtual XnCompressionFormats GetCompressionFormat() const { return XN_COMPRESSION_NONE; }
	virtual XnFloat GetWorseCompressionRatio() const { return 1.0; }
	virtual XnUInt32 GetOverheadSize() const { return 0; }

protected:
	virtual XnStatus CompressImpl(const XnUChar* pData, XnUInt32 nDataSize, XnUChar* pCompressedData, XnUInt32* pnCompressedDataSize)
	{
		if (nDataSize > *pnCompressedDataSize)
		{
			return (XN_STATUS_OUTPUT_BUFFER_OVERFLOW);
		}

		xnOSMemCopy(pCompressedData, pData, nDataSize);
		*pnCompressedDataSize = nDataSize;
		return (XN_STATUS_OK);
	}

	virtual XnStatus DecompressImpl(const XnUChar* pCompressedData, XnUInt32 nCompressedDataSize, XnUChar* pData, XnUInt32* pnDataSize)
	{
		if (nCompressedDataSize > *pnDataSize)
		{
			return (XN_STATUS_OUTPUT_BUFFER_OVERFLOW);
		}

		xnOSMemCopy(pData, pCompressedData, nCompressedDataSize);
		*pnDataSize = nCompressedDataSize;
		return (XN_STATUS_OK);
	}
};

#endif // XNUNCOMPRESSEDCODEC_H
