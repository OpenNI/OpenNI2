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
#ifndef __XN_SENSOR_FIRMWARE_H__
#define __XN_SENSOR_FIRMWARE_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnFirmwareInfo.h"
#include "XnFirmwareCommands.h"
#include "XnSensorFirmwareParams.h"
#include "XnFirmwareStreams.h"
#include "XnSensorFixedParams.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnSensorFirmware
{
public:
	XnSensorFirmware(XnDevicePrivateData* pDevicePrivateData);
	XnStatus Init(XnBool bReset, XnBool bLeanInit);
	void Free();

	inline XnFirmwareInfo* GetInfo() { return m_pInfo; }
	inline const XnFirmwareInfo* GetInfo() const { return m_pInfo; }
	inline XnFirmwareCommands* GetCommands() { return &m_Commands; }
	inline XnSensorFirmwareParams* GetParams() { return &m_Params; }
	inline XnFirmwareStreams* GetStreams() { return &m_Streams; }
	inline XnSensorFixedParams* GetFixedParams() { return &m_FixedParams; }

private:
	XnFirmwareInfo* m_pInfo;
	XnFirmwareCommands m_Commands;
	XnSensorFirmwareParams m_Params;
	XnFirmwareStreams m_Streams;
	XnSensorFixedParams m_FixedParams;
	XnDevicePrivateData* m_pDevicePrivateData;
};

#endif //__XN_SENSOR_FIRMWARE_H__