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
#ifndef __XN_8Z_CODEC_H__
#define __XN_8Z_CODEC_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnCodecBase.h"
#include <Formats/XnStreamCompression.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class Xn8zCodec : public XnCodecBase
{
public:
	virtual XnCompressionFormats GetCompressionFormat() const { return XN_COMPRESSION_COLOR_8Z; }
	virtual XnFloat GetWorseCompressionRatio() const { return XN_STREAM_COMPRESSION_IMAGE8Z_WORSE_RATIO; }
	virtual XnUInt32 GetOverheadSize() const { return 0; }

protected:
	virtual XnStatus CompressImpl(const XnUChar* pData, XnUInt32 nDataSize, XnUChar* pCompressedData, XnUInt32* pnCompressedDataSize)
	{
		return XnStreamCompressImage8Z(pData, nDataSize, pCompressedData, pnCompressedDataSize);
	}

	virtual XnStatus DecompressImpl(const XnUChar* pCompressedData, XnUInt32 nCompressedDataSize, XnUChar* pData, XnUInt32* pnDataSize)
	{
		return XnStreamUncompressImage8Z(pCompressedData, nCompressedDataSize, pData, pnDataSize);
	}
};

#endif //__XN_8Z_CODEC_H__