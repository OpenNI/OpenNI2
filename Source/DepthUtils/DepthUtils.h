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
#ifndef DEPTHUTILS_H
#define DEPTHUTILS_H

#include "OniCTypes.h"

#pragma pack (push, 1)

typedef struct
{
	unsigned short nStartLines;
	unsigned short nEndLines;
	unsigned short nCroppingLines;
} PadInfo;
typedef struct
{
	int nRGS_DX_CENTER;
	int nRGS_AX;
	int nRGS_BX;
	int nRGS_CX;
	int nRGS_DX;
	int nRGS_DX_START;
	int nRGS_AY;
	int nRGS_BY;
	int nRGS_CY;
	int nRGS_DY;
	int nRGS_DY_START;
	int nRGS_DX_BETA_START;
	int nRGS_DY_BETA_START;
	int nRGS_ROLLOUT_BLANK;
	int nRGS_ROLLOUT_SIZE;
	int nRGS_DX_BETA_INC;
	int nRGS_DY_BETA_INC;
	int nRGS_DXDX_START;
	int nRGS_DXDY_START;
	int nRGS_DYDX_START;
	int nRGS_DYDY_START;
	int nRGS_DXDXDX_START;
	int nRGS_DYDXDX_START;
	int nRGS_DXDXDY_START;
	int nRGS_DYDXDY_START;
	int nBACK_COMP1;
	int nRGS_DYDYDX_START;
	int nBACK_COMP2;
	int nRGS_DYDYDY_START;
} RegistrationInfo;

typedef struct
{
	int magic;
	int version;
	char deviceName[80];
	char serial[80];
	struct  // 1080
	{
		PadInfo padInfo_QQVGA;	
		PadInfo padInfo_QVGA;
		PadInfo padInfo_VGA;
		RegistrationInfo registrationInfo_QQVGA;
		RegistrationInfo registrationInfo_QVGA;
		RegistrationInfo registrationInfo_VGA;

		double zpps;
		int zpd;
		double dcrcdist;

		int rgbRegXRes;
		int rgbRegYRes;
		int cmosVGAOutputXRes;
		int sensorWinOffsetX;
		int sensorWinOffsetY;
		int rgbRegXValScale;
		int s2dPelConst;
		double s2dConstOffset;

	} params1080;
} DepthUtilsSensorCalibrationInfo;

#pragma pack (pop)

static const int ONI_DEPTH_UTILS_CALIBRATION_INFO_MAGIC = 0x023a;

struct _DepthUtils;
typedef _DepthUtils* DepthUtilsHandle;

extern "C"
{
	int DepthUtilsInitialize(DepthUtilsSensorCalibrationInfo* pCalibrationInfo, DepthUtilsHandle* handle);
	void DepthUtilsShutdown(DepthUtilsHandle* handle);

	int DepthUtilsTranslatePixel(DepthUtilsHandle handle, unsigned int x, unsigned int y, unsigned short z, unsigned int* pX, unsigned int* pY);
	int DepthUtilsTranslateDepthMap(DepthUtilsHandle handle, unsigned short* depthMap);

	int DepthUtilsSetDepthConfiguration(DepthUtilsHandle handle, int xres, int yres, OniPixelFormat format, int isMirrored);
	int DepthUtilsSetColorResolution(DepthUtilsHandle handle, int xres, int yres);
}

#endif // DEPTHUTILS_H
