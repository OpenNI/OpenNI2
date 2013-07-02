/*****************************************************************************
*                                                                            *
*  PrimeSense Sensor 5.x Alpha                                               *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PrimeSense Sensor                                    *
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
#include "XnNesaDebugProcessor.h"
#include "XnSensor.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_SENSOR_TEC_DEBUG_MAX_PACKET_SIZE		256

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnNesaDebugProcessor::XnNesaDebugProcessor(XnDevicePrivateData* pDevicePrivateData) :
	XnWholePacketProcessor(pDevicePrivateData, "NesaDebug", XN_SENSOR_TEC_DEBUG_MAX_PACKET_SIZE),
	m_Dump(NULL)
{
}

XnNesaDebugProcessor::~XnNesaDebugProcessor()
{
	xnDumpFileClose(m_Dump);
}

void XnNesaDebugProcessor::ProcessWholePacket(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData)
{
	if (m_Dump == NULL)
	{
		m_Dump = xnDumpFileOpenEx("NesaDebug", TRUE, TRUE, "NesaDebug.txt");
	}

	xnDumpFileWriteString(m_Dump, "%S\n", (XnChar*)pData);
	printf("%S\n", (XnWChar*)pData);
}
