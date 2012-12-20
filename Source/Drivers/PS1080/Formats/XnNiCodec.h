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
#ifndef __XN_XN_CODEC_H__
#define __XN_XN_CODEC_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnCodec.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
//class XN_FORMATS_CPP_API XnNiCodec : public XnCodec
//{
//public:
//	XnNiCodec(xn::Codec& codec) : m_codec(codec) {}
//	virtual ~XnNiCodec() {}
//
//	virtual XnCompressionFormats GetCompressionFormat() const { return XnCodec::GetCompressionFormatFromCodecID(m_codec.GetCodecID()); }
//
//	virtual XnStatus Compress(const XnUChar* pData, XnUInt32 nDataSize, XnUChar* pCompressedData, XnUInt32* pnCompressedDataSize)
//	{
//		return m_codec.EncodeData(pData, nDataSize, pCompressedData, *pnCompressedDataSize, pnCompressedDataSize);
//	}
//
//	virtual XnStatus Decompress(const XnUChar* pCompressedData, XnUInt32 nCompressedDataSize, XnUChar* pData, XnUInt32* pnDataSize)
//	{
//		return m_codec.DecodeData(pCompressedData, nCompressedDataSize, pData, *pnDataSize, pnDataSize);
//	}
//
//private:
//	xn::Codec m_codec;
//};
//

#endif // __XN_XN_CODEC_H__