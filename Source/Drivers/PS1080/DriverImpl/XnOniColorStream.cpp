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
#include "XnOniColorStream.h"
#include "../Sensor/XnSensorImageStream.h"

//---------------------------------------------------------------------------
// Static Data
//---------------------------------------------------------------------------
static const XnUInt32 INVALID_INPUT_FORMAT = 9999;
// the order in the allowed input formats is the preferred one
static XnIOImageFormats g_anAllowedRGBFormats[]   = { XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_IO_IMAGE_FORMAT_YUV422, XN_IO_IMAGE_FORMAT_BAYER, XN_IO_IMAGE_FORMAT_UNCOMPRESSED_BAYER, XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUYV };
static XnIOImageFormats g_anAllowedYUVFormats[]   = { XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_IO_IMAGE_FORMAT_YUV422 };
static XnIOImageFormats g_anAllowedYUYVFormats[]   = { XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUYV };
static XnIOImageFormats g_anAllowedJPEGFormats[]  = { XN_IO_IMAGE_FORMAT_JPEG };
static XnIOImageFormats g_anAllowedGray8Formats[] = { XN_IO_IMAGE_FORMAT_BAYER, XN_IO_IMAGE_FORMAT_UNCOMPRESSED_BAYER };

void XnOniColorStream::GetAllowedOniOutputFormatForInputFormat(XnIOImageFormats inputFormat, OniPixelFormat *aOniFormats, int *nOniFormats)
{
	*nOniFormats = 0;
	for(XnUInt32 i=0; i<(sizeof(g_anAllowedRGBFormats)/sizeof(XnIOImageFormats)); ++i)
	{
		if(g_anAllowedRGBFormats[i] == inputFormat)
		{
			aOniFormats[*nOniFormats] = ONI_PIXEL_FORMAT_RGB888;
			++(*nOniFormats);
			break;
		}
	}
	for(XnUInt32 i=0; i<(sizeof(g_anAllowedYUVFormats)/sizeof(XnIOImageFormats)); ++i)
	{
		if(g_anAllowedYUVFormats[i] == inputFormat)
		{
			aOniFormats[*nOniFormats] = ONI_PIXEL_FORMAT_YUV422;
			++(*nOniFormats);
			break;
		}
	}
	for(XnUInt32 i=0; i<(sizeof(g_anAllowedYUYVFormats)/sizeof(XnIOImageFormats)); ++i)
	{
		if(g_anAllowedYUYVFormats[i] == inputFormat)
		{
			aOniFormats[*nOniFormats] = ONI_PIXEL_FORMAT_YUYV;
			++(*nOniFormats);
			break;
		}
	}
	for(XnUInt32 i=0; i<(sizeof(g_anAllowedJPEGFormats)/sizeof(XnIOImageFormats)); ++i)
	{
		if(g_anAllowedJPEGFormats[i] == inputFormat)
		{
			aOniFormats[*nOniFormats] = ONI_PIXEL_FORMAT_JPEG;
			++(*nOniFormats);
			break;
		}
	}
	for(XnUInt32 i=0; i<(sizeof(g_anAllowedGray8Formats)/sizeof(XnIOImageFormats)); ++i)
	{
		if(g_anAllowedGray8Formats[i] == inputFormat)
		{
			aOniFormats[*nOniFormats] = ONI_PIXEL_FORMAT_GRAY8;
			++(*nOniFormats);
			break;
		}
	}
}

XnBool XnOniColorStream::IsSupportedInputFormat(XnIOImageFormats inputFormat, OniPixelFormat oniFormat)
{
	return IsPreferredInputFormat(inputFormat, (XnIOImageFormats)XN_MAX_UINT32, oniFormat);
}

XnBool XnOniColorStream::IsPreferredInputFormat(XnIOImageFormats inputFormat, XnIOImageFormats thanFormat, OniPixelFormat oniFormat)
{
	XnIOImageFormats *aAllowedFormats;
	int               nAllowedFormats;

	switch(oniFormat)
	{
	case ONI_PIXEL_FORMAT_RGB888:
		aAllowedFormats = g_anAllowedRGBFormats;
		nAllowedFormats = sizeof(g_anAllowedRGBFormats)/sizeof(g_anAllowedRGBFormats[0]);
		break;
	case ONI_PIXEL_FORMAT_YUV422:
		aAllowedFormats = g_anAllowedYUVFormats;
		nAllowedFormats = sizeof(g_anAllowedYUVFormats)/sizeof(g_anAllowedYUVFormats[0]);
		break;
	case ONI_PIXEL_FORMAT_YUYV:
		aAllowedFormats = g_anAllowedYUYVFormats;
		nAllowedFormats = sizeof(g_anAllowedYUYVFormats)/sizeof(g_anAllowedYUYVFormats[0]);
		break;
	case ONI_PIXEL_FORMAT_JPEG:
		aAllowedFormats = g_anAllowedJPEGFormats;
		nAllowedFormats = sizeof(g_anAllowedJPEGFormats)/sizeof(g_anAllowedJPEGFormats[0]);
		break;
	case ONI_PIXEL_FORMAT_GRAY8:
		aAllowedFormats = g_anAllowedGray8Formats;
		nAllowedFormats = sizeof(g_anAllowedGray8Formats)/sizeof(g_anAllowedGray8Formats[0]);
		break;
	default:
		return FALSE;
	}

	for(int i=0; i<nAllowedFormats; ++i)
	{
		// the order in the allowed input formats is the preferred one
		if(aAllowedFormats[i] == thanFormat) {
			return FALSE;
		}
		if(aAllowedFormats[i] == inputFormat) {
			return TRUE;
		}
	}
	
	// none of the formats is supported :|
	return FALSE;
}

//---------------------------------------------------------------------------
// XnSensorImageGenerator class
//---------------------------------------------------------------------------

XnOniColorStream::XnOniColorStream(XnSensor* pSensor, XnOniDevice* pDevice) : 
	XnOniMapStream(pSensor, XN_STREAM_TYPE_IMAGE, ONI_SENSOR_COLOR, pDevice)
{
}

