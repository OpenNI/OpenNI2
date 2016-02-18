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
#ifndef XNSHIFTTODEPTHSTREAMHELPER_H
#define XNSHIFTTODEPTHSTREAMHELPER_H

#include <DDK/XnDeviceModule.h>
#include <DDK/XnShiftToDepth.h>

class XnShiftToDepthStreamHelper
{
public:
	XnShiftToDepthStreamHelper();
	virtual ~XnShiftToDepthStreamHelper();

	XnStatus Init(XnDeviceModule* pModule);
	XnStatus Free();

	inline OniDepthPixel* GetShiftToDepthTable() const { return m_ShiftToDepthTables.pShiftToDepthTable; }
	inline XnUInt16* GetDepthToShiftTable() const { return m_ShiftToDepthTables.pDepthToShiftTable; }

protected:
	inline XnActualGeneralProperty& ShiftToDepthTableProperty() { return m_ShiftToDepthTable; }
	inline XnActualGeneralProperty& DepthToShiftTableProperty() { return m_DepthToShiftTable; }

private:
	XnStatus RaiseChangeEvents();
	XnStatus InitShiftToDepth();
	XnStatus OnShiftToDepthPropertyValueChanged();
	XnStatus OnDeviceS2DTablesSizeChanged();
	XnStatus GetShiftToDepthConfig(XnShiftToDepthConfig& Config);
	XnStatus GetShiftToDepthTableImpl(const OniGeneralBuffer& gbValue) const;
	XnStatus GetDepthToShiftTableImpl(const OniGeneralBuffer& gbValue) const;

	// callbacks
	static XnStatus XN_CALLBACK_TYPE GetShiftToDepthTableCallback(const XnActualGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetDepthToShiftTableCallback(const XnActualGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ShiftToDepthPropertyValueChangedCallback(const XnProperty* pSender, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE DeviceS2DTablesSizeChangedCallback(const XnProperty* pSender, void* pCookie);

	XnActualGeneralProperty m_ShiftToDepthTable;
	XnActualGeneralProperty m_DepthToShiftTable;
	XnShiftToDepthTables m_ShiftToDepthTables;
	XnDeviceModule* m_pModule;
	XnBool m_bPropertiesAdded;
};

#endif // XNSHIFTTODEPTHSTREAMHELPER_H
