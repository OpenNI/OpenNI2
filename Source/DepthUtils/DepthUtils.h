#ifndef _DEPTH_UTILS_H_
#define _DEPTH_UTILS_H_

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

#endif // _DEPTH_UTILS_H_