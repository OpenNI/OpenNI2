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
#ifndef __XN_JPEG_CODEC_H__
#define __XN_JPEG_CODEC_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnCodecBase.h"
#include <Formats/XnStreamCompression.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnJpegCodec : public XnCodecBase
{
public:
	XnJpegCodec(XnBool bRGB, XnUInt32 nXRes, XnUInt32 nYRes, XnUInt32 nQuality = XN_STREAM_COMPRESSION_JPEG_DEFAULT_QUALITY) :
		m_bRGB(bRGB), m_nXRes(nXRes), m_nYRes(nYRes), m_nQuality(nQuality)
	{}

	~XnJpegCodec()
	{
		XnStreamFreeCompressImageJ(&m_CompJPEGContext);
		XnStreamFreeUncompressImageJ(&m_UncompJPEGContext);
	}

	XnStatus Init()
	{
		XnStatus nRetVal = XN_STATUS_OK;

		nRetVal = XnStreamInitCompressImageJ(&m_CompJPEGContext);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = XnStreamInitUncompressImageJ(&m_UncompJPEGContext);
		if (nRetVal != XN_STATUS_OK)
		{
			XnStreamFreeCompressImageJ(&m_CompJPEGContext);
			return (nRetVal);
		}

		return (XN_STATUS_OK);
	}

	virtual XnCompressionFormats GetCompressionFormat() const { return XN_COMPRESSION_JPEG; }
	virtual XnFloat GetWorseCompressionRatio() const { return XN_STREAM_COMPRESSION_IMAGEJ_WORSE_RATIO; }
	virtual XnUInt32 GetOverheadSize() const { return 0; }

protected:
	XN_DISABLE_COPY_AND_ASSIGN(XnJpegCodec);

	virtual XnStatus CompressImpl(const XnUChar* pData, XnUInt32 /*nDataSize*/, XnUChar* pCompressedData, XnUInt32* pnCompressedDataSize)
	{
		if (m_bRGB)
		{
			return XnStreamCompressImage24J(&m_CompJPEGContext, pData, pCompressedData, pnCompressedDataSize, m_nXRes, m_nYRes, m_nQuality);
		}
		else
		{
			return XnStreamCompressImage8J(&m_CompJPEGContext, pData, pCompressedData, pnCompressedDataSize, m_nXRes, m_nYRes, m_nQuality);
		}
	}

	virtual XnStatus DecompressImpl(const XnUChar* pCompressedData, XnUInt32 nCompressedDataSize, XnUChar* pData, XnUInt32* pnDataSize)
	{
		return XnStreamUncompressImageJ(&m_UncompJPEGContext, pCompressedData, nCompressedDataSize, pData, pnDataSize);
	}

private:
	const XnBool m_bRGB;
	const XnUInt32 m_nXRes;
	const XnUInt32 m_nYRes;
	const XnUInt32 m_nQuality;
	XnStreamCompJPEGContext m_CompJPEGContext;
	XnStreamUncompJPEGContext m_UncompJPEGContext;
};

#endif //__XN_JPEG_CODEC_H__