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
#include "XnWavelengthCorrectionDebugProcessor.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnWavelengthCorrectionDebugProcessor::XnWavelengthCorrectionDebugProcessor(XnDevicePrivateData* pDevicePrivateData) :
	XnWholePacketProcessor(pDevicePrivateData, "WavelengthCorrectionDebug", sizeof(XnWavelengthCorrectionDebugPacket)),
	m_DumpTxt(NULL)
{
}

XnWavelengthCorrectionDebugProcessor::~XnWavelengthCorrectionDebugProcessor()
{
	xnDumpFileClose(m_DumpTxt);
}

void XnWavelengthCorrectionDebugProcessor::ProcessWholePacket(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData)
{
	m_DumpTxt = xnDumpFileOpenEx("WavelengthCorrectionDebug", TRUE, TRUE, "WavelengthCorrection.csv");
	xnDumpFileWriteString(m_DumpTxt, "HostTimestamp,PacketID,BLast,BCurrent,IsHop,CurrentSlidingWindow,CurrentHopsCount,IsTecCalibrated,WaitPeriod,IsWavelengthUnstable,BestHopsCount,BestSetPoint,BestStep,IsTotallyUnstable,ConfiguredTecSetPoint,CurrentStep\n");

	XnWavelengthCorrectionDebugPacket* pPacket = (XnWavelengthCorrectionDebugPacket*)pData;

	XnUInt64 nTimestamp = 0;
	xnOSGetHighResTimeStamp(&nTimestamp);

	xnDumpFileWriteString(m_DumpTxt, "%llu,%hu,%f,%f,%hu,%x,%hu,%hu,%u,%hu,%hu,%u,%d,%hu,%u,%d\n",
		nTimestamp, pHeader->nPacketID, 
		pPacket->fBLast, pPacket->fBCurrent, pPacket->nIsHop, pPacket->nCurrentSlidingWindow,
		pPacket->nCurrentHopsCount, pPacket->nIsTecCalibrated, pPacket->nWaitPeriod,
		pPacket->nIsWavelengthUnstable, pPacket->BestConf.nBestHopsCount, pPacket->BestConf.nBestSetPoint,
		pPacket->BestConf.nBestStep, pPacket->nIsTotallyUnstable, pPacket->nConfiguredTecSetPoint,
		pPacket->nCurrentStep);
}

