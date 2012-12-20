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
#ifndef __XN_CODEC_BASE_H__
#define __XN_CODEC_BASE_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnCodec.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnCodecBase : public XnCodec
{
public:
	static XnCompressionFormats GetCompressionFormatFromCodecID(XnCodecID codecID);
	static XnCodecID GetCodecIDFromCompressionFormat(XnCompressionFormats format);

	XnCodecBase() {}
	virtual ~XnCodecBase() {}

	virtual XnStatus Init() { return XN_STATUS_OK; }

	virtual XnCompressionFormats GetCompressionFormat() const = 0;

	XnStatus Compress(const XnUChar* pData, XnUInt32 nDataSize, XnUChar* pCompressedData, XnUInt32* pnCompressedDataSize)
	{
		XnStatus nRetVal = XN_STATUS_OK;

		XN_VALIDATE_INPUT_PTR(pData);
		XN_VALIDATE_INPUT_PTR(pCompressedData);
		XN_VALIDATE_OUTPUT_PTR(pnCompressedDataSize);

		if ((nDataSize * GetWorseCompressionRatio() + GetOverheadSize()) > *pnCompressedDataSize)
		{
			return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
		}

		nRetVal = CompressImpl(pData, nDataSize, pCompressedData, pnCompressedDataSize);
		XN_IS_STATUS_OK(nRetVal);

		return (XN_STATUS_OK);
	}

	XnStatus Decompress(const XnUChar* pCompressedData, XnUInt32 nCompressedDataSize, XnUChar* pData, XnUInt32* pnDataSize)
	{
		XnStatus nRetVal = XN_STATUS_OK;

		XN_VALIDATE_INPUT_PTR(pCompressedData);
		XN_VALIDATE_INPUT_PTR(pData);
		XN_VALIDATE_OUTPUT_PTR(pnDataSize);

		nRetVal = DecompressImpl(pCompressedData, nCompressedDataSize, pData, pnDataSize);
		XN_IS_STATUS_OK(nRetVal);

		return (XN_STATUS_OK);
	}

	virtual XnUInt32 GetOverheadSize() const = 0;
	virtual XnFloat GetWorseCompressionRatio() const = 0;

protected:
	virtual XnStatus CompressImpl(const XnUChar* pData, XnUInt32 nDataSize, XnUChar* pCompressedData, XnUInt32* pnCompressedDataSize) = 0;
	virtual XnStatus DecompressImpl(const XnUChar* pCompressedData, XnUInt32 nCompressedDataSize, XnUChar* pData, XnUInt32* pnDataSize) = 0;
};

#endif // __XN_CODEC_BASE_H__