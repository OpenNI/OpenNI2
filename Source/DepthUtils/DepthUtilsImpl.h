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
#ifndef DEPTHUTILSIMPL_H
#define DEPTHUTILSIMPL_H

#include <XnLib.h>
#include "DepthUtils.h"

#define MAX_Z 65535

class DepthUtilsImpl
{
public:
	DepthUtilsImpl();
	~DepthUtilsImpl();

	XnStatus Initialize(DepthUtilsSensorCalibrationInfo* pBlob);
	XnStatus Free();

	XnStatus Apply(unsigned short* pOutput);

	XnStatus SetDepthConfiguration(int xres, int yres, OniPixelFormat format, bool isMirrored);

	XnStatus SetColorResolution(int xres, int yres);

	XnStatus TranslateSinglePixel(XnUInt32 x, XnUInt32 y, unsigned short z, XnUInt32& imageX, XnUInt32& imageY);
private:
	void BuildDepthToShiftTable(XnUInt16* pRGBRegDepthToShiftTable, int xres);
	XnStatus BuildRegistrationTable(XnUInt16* pRegTable, RegistrationInfo* pRegInfo, XnUInt16** pDepthToShiftTable, int xres, int yres);

	void CreateDXDYTables (XnDouble* RegXTable, XnDouble* RegYTable,
		XnUInt32 resX, XnUInt32 resY,
		XnInt64 AX6, XnInt64 BX6, XnInt64 CX2, XnInt64 DX2,
		XnUInt32 deltaBetaX,
		XnInt64 AY6, XnInt64 BY6, XnInt64 CY2, XnInt64 DY2,
		XnUInt32 deltaBetaY,
		XnInt64 dX0, XnInt64 dY0,
		XnInt64 dXdX0, XnInt64 dXdY0, XnInt64 dYdX0, XnInt64 dYdY0,
		XnInt64 dXdXdX0, XnInt64 dYdXdX0, XnInt64 dYdXdY0, XnInt64 dXdXdY0,
		XnInt64 dYdYdX0, XnInt64 dYdYdY0,
		XnUInt32 startingBetaX, XnUInt32 startingBetaY);
	void CreateDXDYTablesInternal(XnDouble* RegXTable, XnDouble* RegYTable,
		XnInt32 resX, XnInt32 resY,
		XnInt64 AX6, XnInt64 BX6, XnInt64 CX2, XnInt64 DX2,
		XnInt32 deltaBetaX,
		XnInt64 AY6, XnInt64 BY6, XnInt64 CY2, XnInt64 DY2,
		XnInt32 deltaBetaY,
		XnInt64 dX0, XnInt64 dY0,
		XnInt64 dXdX0, XnInt64 dXdY0, XnInt64 dYdX0, XnInt64 dYdY0,
		XnInt64 dXdXdX0, XnInt64 dYdXdX0, XnInt64 dYdXdY0, XnInt64 dXdXdY0,
		XnInt64 dYdYdX0, XnInt64 dYdYdY0,
		XnInt32 betaX, XnInt32 betaY);

private:
	DepthUtilsSensorCalibrationInfo m_blob;
	XnUInt16* m_pDepthToShiftTable_QQVGA;
	XnUInt16* m_pDepthToShiftTable_QVGA;
	XnUInt16* m_pDepthToShiftTable_VGA;
	XnUInt16* m_pRegistrationTable_QQVGA;
	XnUInt16* m_pRegistrationTable_QVGA;
	XnUInt16* m_pRegistrationTable_VGA;

	PadInfo* m_pPadInfo;
	RegistrationInfo* m_pRegistrationInfo;
	XnUInt16* m_pRegTable;
	XnUInt16* m_pDepth2ShiftTable;

	bool m_bD2SAlloc;
	bool m_bInitialized;
	bool m_isMirrored;
	struct
	{
		int x, y;
	} m_depthResolution, m_colorResolution;

};


#endif // DEPTHUTILSIMPL_H
