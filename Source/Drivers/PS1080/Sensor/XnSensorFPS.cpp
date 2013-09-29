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
#include "XnSensorFPS.h"
#include "XnDeviceSensor.h"
#include <XnLog.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
/* The number of frames to average FPS over */
#define XN_SENSOR_FPS_FRAME_COUNT	180

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnSensorFPS::XnSensorFPS() :
	m_nLastPrint(0),
	m_FramesDump(NULL)
{
	m_FramesDump = xnDumpFileOpen(XN_MASK_SENSOR_FPS, "FramesTimes.csv");
	xnDumpFileWriteString(m_FramesDump, "TS,Type,FrameID,FrameTS\n");
}

XnSensorFPS::~XnSensorFPS()
{
	xnDumpFileClose(m_FramesDump);
}

void XnSensorFPS::Mark(XnFPSData* /*pFPS*/, const XnChar* csName, XnUInt32 nFrameID, XnUInt64 nTS)
{
	if (!xnLogIsEnabled(XN_MASK_SENSOR_FPS, XN_LOG_VERBOSE))
		return;

	XnUInt64 nNow;
	xnOSGetHighResTimeStamp(&nNow);

	xnDumpFileWriteString(m_FramesDump, "%llu,%s,%u,%llu\n", nNow, csName, nFrameID, nTS);
}
