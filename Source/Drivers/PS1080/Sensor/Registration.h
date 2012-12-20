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
#ifndef _XN_REGISTRATION_H_
#define _XN_REGISTRATION_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDeviceSensor.h"

#define XN_REG_PARAB_COEFF 4
#define XN_REG_X_SCALE	16

class XnSensorDepthStream; // Forward Declaration

class XnRegistration
{
public:
	XnRegistration();
	~XnRegistration() { Free(); }

	XnStatus Init(XnDevicePrivateData* pDevicePrivateData, XnSensorDepthStream* pDepthStream, XnUInt16* pDepthToShiftTable);
	XnStatus Free();
	void Apply(OniDepthPixel* pDM);
	XnStatus TranslateSinglePixel(XnUInt32 x, XnUInt32 y, OniDepthPixel z, XnUInt32& imageX, XnUInt32& imageY);

	inline XnBool IsInitialized() { return m_bInitialized; }

private:
	void BuildDepthToShiftTable(XnUInt16* pDepth2Shift, XnSensorDepthStream* m_pStream);
	XnStatus BuildRegTable();
	XnStatus BuildRegTable1000();
	XnStatus BuildRegTable1080();
	void Apply1000(OniDepthPixel* pInput, OniDepthPixel* pOutput);
	void Apply1080(OniDepthPixel* pInput, OniDepthPixel* pOutput);
	XnStatus TranslateSinglePixel1080(XnUInt32 x, XnUInt32 y, OniDepthPixel z, XnUInt32& imageX, XnUInt32& imageY);

	XnBool m_bInitialized;

	XnDevicePrivateData* m_pDevicePrivateData;
	XnSensorDepthStream* m_pDepthStream;
	XnUInt16* m_pDepthToShiftTable;
	XnBool m_bD2SAlloc;
	XnUInt16* m_pRegistrationTable;
	XnRegistrationPaddingInformation m_padInfo;
	OniDepthPixel* m_pTempBuffer;
	XnDouble m_dShiftFactor;
	XnBool m_b1000;
};

#endif //_XN_RGBREG_H_
