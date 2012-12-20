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
#ifndef __XN_WHOLE_PACKET_PROCESSOR_H__
#define __XN_WHOLE_PACKET_PROCESSOR_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDataProcessor.h"

//---------------------------------------------------------------------------
// XnWholePacketProcessor Class
//---------------------------------------------------------------------------
class XnWholePacketProcessor : public XnDataProcessor
{
public:
	XnWholePacketProcessor(XnDevicePrivateData* pDevicePrivateData, const XnChar* csName, XnUInt32 nMaxPacketSize);
	~XnWholePacketProcessor();

	XnStatus Init();

//---------------------------------------------------------------------------
// Overridden Functions
//---------------------------------------------------------------------------
protected:
	void ProcessPacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize);

//---------------------------------------------------------------------------
// Virtual Functions
//---------------------------------------------------------------------------
protected:
	virtual void ProcessWholePacket(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData) = 0;

private:
	/* The maximum size of the packet. */
	XnUInt32 m_nMaxPacketSize;
	/* A buffer to store whole packet */
	XnBuffer m_WholePacket;
};

#endif //__XN_WHOLE_PACKET_PROCESSOR_H__
