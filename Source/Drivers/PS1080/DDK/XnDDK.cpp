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
#include <XnDDK.h>
#include <Formats/XnFormats.h>
#include <XnOS.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnResolutions XnDDKGetResolutionFromXY(XnUInt32 nXRes, XnUInt32 nYRes)
{
	if (nXRes == 320  && nYRes == 240)        return XN_RESOLUTION_QVGA;
	if (nXRes == 640  && nYRes == 480)        return XN_RESOLUTION_VGA;
	if (nXRes == 1280 && nYRes == 1024)       return XN_RESOLUTION_SXGA;
	if (nXRes == 1600 && nYRes == 1200)       return XN_RESOLUTION_UXGA;
	if (nXRes == 160  && nYRes == 120)        return XN_RESOLUTION_QQVGA;
	if (nXRes == 176  && nYRes == 144)        return XN_RESOLUTION_QCIF;
	if (nXRes == 423  && nYRes == 240)        return XN_RESOLUTION_240P;
	if (nXRes == 352  && nYRes == 288)        return XN_RESOLUTION_CIF;
	if (nXRes == 640  && nYRes == 360)        return XN_RESOLUTION_WVGA;
	if (nXRes == 864  && nYRes == 480)        return XN_RESOLUTION_480P;
	if (nXRes == 800  && nYRes == 600)        return XN_RESOLUTION_SVGA;
	if (nXRes == 1024 && nYRes == 576)        return XN_RESOLUTION_576P;
	if (nXRes == 960  && nYRes == 720)        return XN_RESOLUTION_DV;
	if (nXRes == 1280 && nYRes == 720)        return XN_RESOLUTION_720P;
	// check if this is one of our special resolutions
	if (nXRes == 800  && nYRes == 448)        return XN_RESOLUTION_800_448;
	if (nXRes == 1280 && nYRes == 960)        return XN_RESOLUTION_1280_960;
										      return XN_RESOLUTION_CUSTOM;
}

XnBool XnDDKGetXYFromResolution(XnResolutions res, XnUInt32* pnXRes, XnUInt32* pnYRes)
{
	switch(res)
	{
		case XN_RESOLUTION_QVGA:      *pnXRes = 320;  *pnYRes = 240;  break;
		case XN_RESOLUTION_VGA:       *pnXRes = 640;  *pnYRes = 480;  break;
		case XN_RESOLUTION_SXGA:      *pnXRes = 1280; *pnYRes = 1024; break;
		case XN_RESOLUTION_UXGA:      *pnXRes = 1600; *pnYRes = 1200; break;
		case XN_RESOLUTION_QQVGA:     *pnXRes = 160;  *pnYRes = 120;  break;
		case XN_RESOLUTION_QCIF:      *pnXRes = 176;  *pnYRes = 144;  break;
		case XN_RESOLUTION_240P:      *pnXRes = 423;  *pnYRes = 240;  break;
		case XN_RESOLUTION_CIF:       *pnXRes = 352;  *pnYRes = 288;  break;
		case XN_RESOLUTION_WVGA:      *pnXRes = 640;  *pnYRes = 360;  break;
		case XN_RESOLUTION_480P:      *pnXRes = 864;  *pnYRes = 480;  break;
		case XN_RESOLUTION_SVGA:      *pnXRes = 800;  *pnYRes = 600;  break;
		case XN_RESOLUTION_576P:      *pnXRes = 1024; *pnYRes = 576;  break;
		case XN_RESOLUTION_DV:        *pnXRes = 960;  *pnYRes = 720;  break;
		case XN_RESOLUTION_720P:      *pnXRes = 1280; *pnYRes = 720;  break;
		// check if this is one of our special resolutions
		case XN_RESOLUTION_800_448:		*pnXRes = 800;	*pnYRes = 448;  break;
		case XN_RESOLUTION_1280_960:	*pnXRes = 1280; *pnYRes = 960;  break;
		case XN_RESOLUTION_CUSTOM:		return FALSE;
	}
	return TRUE;
}

const XnChar* XnDDKGetResolutionName(XnResolutions res)
{
	switch (res)
	{
	case XN_RESOLUTION_QVGA:	return "QVGA";
	case XN_RESOLUTION_VGA:		return "VGA";
	case XN_RESOLUTION_SXGA:	return "SXGA";
	case XN_RESOLUTION_UXGA:	return "UXGA";
	case XN_RESOLUTION_QQVGA:	return "QQVGA";
	case XN_RESOLUTION_QCIF:	return "QCIF";
	case XN_RESOLUTION_240P:	return "240P";
	case XN_RESOLUTION_CIF:		return "CIF";
	case XN_RESOLUTION_WVGA:	return "WVGA";
	case XN_RESOLUTION_480P:	return "480P";	
	case XN_RESOLUTION_SVGA:	return "SVGA";
	case XN_RESOLUTION_576P:	return "576P";
	case XN_RESOLUTION_DV:		return "DV";
	case XN_RESOLUTION_720P:	return "720P";
	// check if this is one of our special resolutions
	case XN_RESOLUTION_CUSTOM:	return "Custom";
	case XN_RESOLUTION_800_448: return "800x448";
	case XN_RESOLUTION_1280_960:return "1280x960";
	default: 
		XN_ASSERT(FALSE);
		return "Custom";
	}
}
