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
#ifndef __XN_GMC_DEBUG_PROCESSOR_H__
#define __XN_GMC_DEBUG_PROCESSOR_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnWholePacketProcessor.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

class XnGMCDebugProcessor : public XnWholePacketProcessor
{
public:
	XnGMCDebugProcessor(XnDevicePrivateData* pDevicePrivateData);
	~XnGMCDebugProcessor();

protected:
	//---------------------------------------------------------------------------
	// Overridden Functions
	//---------------------------------------------------------------------------
	virtual void ProcessWholePacket(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData);

	//---------------------------------------------------------------------------
	// Class Members
	//---------------------------------------------------------------------------
private:
	XnDumpFile* m_DumpTxt;
	XnDumpFile* m_DumpBin;
	XnUInt32 m_nGMCTime;
};

#endif //__XN_GMC_DEBUG_PROCESSOR_H__
