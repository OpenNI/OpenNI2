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
#include "XnUncompressedCodec.h"
#include "Xn16zCodec.h"
#include "Xn16zEmbTablesCodec.h"
#include "Xn8zCodec.h"
#include "XnJpegCodec.h"
#include "XnNiCodec.h"

XnCompressionFormats XnCodec::GetCompressionFormatFromCodecID(XnCodecID /*codecID*/)
{
	/*switch (codecID)
	{*/
	//case XN_CODEC_UNCOMPRESSED:
	//	return XN_COMPRESSION_NONE;
	//case XN_CODEC_16Z:
	//	return XN_COMPRESSION_16Z;
	//case XN_CODEC_16Z_EMB_TABLES:
	//	return XN_COMPRESSION_16Z_EMB_TABLE;
	//case XN_CODEC_8Z:
	//	return XN_COMPRESSION_COLOR_8Z;
	//case XN_CODEC_JPEG:
	//	return XN_COMPRESSION_JPEG;
	//default:
		return (XnCompressionFormats)-1;
	//}
}

XnCodecID XnCodec::GetCodecIDFromCompressionFormat(XnCompressionFormats /*format*/)
{
	//switch (format)
	//{
	//case XN_COMPRESSION_16Z:
	//	return XN_CODEC_16Z;
	//case XN_COMPRESSION_16Z_EMB_TABLE:
	//	return XN_CODEC_16Z_EMB_TABLES;
	//case XN_COMPRESSION_JPEG:
	//	return XN_CODEC_JPEG;
	//case XN_COMPRESSION_NONE:
	//	return XN_CODEC_UNCOMPRESSED;
	//case XN_COMPRESSION_COLOR_8Z:
	//	return XN_CODEC_8Z;
	//default:
		//return XN_CODEC_NULL;
		return (XnCodecID)-1;
	//}
}
