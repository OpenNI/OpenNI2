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
#include "XnCodecFactory.h"
#include "XnIntProperty.h"
#include <Formats/XnUncompressedCodec.h>
#include <Formats/Xn16zCodec.h>
#include <Formats/Xn16zEmbTablesCodec.h>
#include <Formats/Xn8zCodec.h>
#include <Formats/XnJpegCodec.h>
#include <XnLog.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnStatus XnCodecFactory::Create(XnCompressionFormats nFormat, XnDeviceModule* pStream, const XnChar* /*StreamName*/, XnCodec** ppCodec)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnCodec* pCodec = NULL;

	switch (nFormat)
	{
	case XN_COMPRESSION_NONE:
		{
			XN_VALIDATE_NEW_AND_INIT(pCodec, XnUncompressedCodec);
		}
		break;
	case XN_COMPRESSION_16Z:
		{
			XN_VALIDATE_NEW_AND_INIT(pCodec, Xn16zCodec);
		}
		break;
	case XN_COMPRESSION_16Z_EMB_TABLE:
		{
			// first we need to find max depth
			XnUInt64 nMaxDepth;

			nRetVal = pStream->GetProperty(XN_STREAM_PROPERTY_MAX_DEPTH, &nMaxDepth);
			XN_IS_STATUS_OK(nRetVal);

			XN_VALIDATE_NEW_AND_INIT(pCodec, Xn16zEmbTablesCodec, (OniDepthPixel)nMaxDepth);
		}
		break;
	case XN_COMPRESSION_COLOR_8Z:
		{
			XN_VALIDATE_NEW_AND_INIT(pCodec, Xn8zCodec);
		}
		break;
	case XN_COMPRESSION_JPEG:
		{
			// check what is the output format
			XnUInt64 nOutputFormat;
			nRetVal = pStream->GetProperty(XN_STREAM_PROPERTY_OUTPUT_FORMAT, &nOutputFormat);
			XN_IS_STATUS_OK(nRetVal);

			XnBool bRGB = FALSE;

			switch (nOutputFormat)
			{
			case ONI_PIXEL_FORMAT_GRAY8:
				bRGB = FALSE;
				break;
			case ONI_PIXEL_FORMAT_RGB888:
				bRGB = TRUE;
				break;
			default:
				XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DDK, "Codec factory currently supports JPEG codec only for streams of type Gray8 or RGB24!");
			}

			// take X and Y res
			XnUInt64 nXRes, nYRes;
			nRetVal = pStream->GetProperty(XN_STREAM_PROPERTY_X_RES, &nXRes);
			XN_IS_STATUS_OK(nRetVal);

			nRetVal = pStream->GetProperty(XN_STREAM_PROPERTY_Y_RES, &nYRes);
			XN_IS_STATUS_OK(nRetVal);

			XN_VALIDATE_NEW_AND_INIT(pCodec, XnJpegCodec, bRGB, (XnUInt32)nXRes, (XnUInt32)nYRes);
		}
		break;
	default:
		XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DDK, "Codec factory does not support compression type %d", nFormat);
	}

	*ppCodec = pCodec;
	return (XN_STATUS_OK);
}

void XnCodecFactory::Destroy(XnCodec* pCodec)
{
	XN_DELETE(pCodec);
}

