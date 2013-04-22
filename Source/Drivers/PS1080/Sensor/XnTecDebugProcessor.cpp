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
#include "XnTecDebugProcessor.h"
#include "XnSensor.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_SENSOR_TEC_DEBUG_MAX_PACKET_SIZE		256

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnTecDebugProcessor::XnTecDebugProcessor(XnDevicePrivateData* pDevicePrivateData) :
	XnWholePacketProcessor(pDevicePrivateData, "TecDebug", XN_SENSOR_TEC_DEBUG_MAX_PACKET_SIZE),
	m_Dump(NULL)
{
}

XnTecDebugProcessor::~XnTecDebugProcessor()
{
	xnDumpFileClose(m_Dump);
}

void XnTecDebugProcessor::ProcessWholePacket(const XnSensorProtocolResponseHeader* /*pHeader*/, const XnUChar* pData)
{
	if (m_Dump == NULL)
	{
		m_Dump = xnDumpFileOpenEx("TecDebug", TRUE, TRUE, "TecDebug.csv");
	}

	xnDumpFileWriteString(m_Dump, "%S\n", (XnChar*)pData);

	if (m_pDevicePrivateData->pSensor->IsTecDebugPring())
	{
		printf("%S\n", (XnWChar*)pData);
	}
}

