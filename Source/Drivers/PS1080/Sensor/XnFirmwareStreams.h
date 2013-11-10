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
#ifndef XNFIRMWARESTREAMS_H
#define XNFIRMWARESTREAMS_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDataProcessorHolder.h"
#include <XnStringsHash.h>
#include <DDK/XnDeviceStream.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_STREAM_NAME_GMC "GMC"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnFirmwareStreams
{
public:
	XnFirmwareStreams(XnDevicePrivateData* pDevicePrivateData);
	~XnFirmwareStreams();

	XnStatus Init();

	XnStatus ClaimStream(const XnChar* strType, XnResolutions nRes, XnUInt32 nFPS, XnDeviceStream* pOwner);
	XnStatus ReleaseStream(const XnChar* strType, XnDeviceStream* pOwner);
	XnStatus LockStreamProcessor(const XnChar* strType, XnDeviceStream* pOwner);
	XnStatus UnlockStreamProcessor(const XnChar* strType, XnDeviceStream* pOwner);
	XnStatus ReplaceStreamProcessor(const XnChar* strType, XnDeviceStream* pOwner, XnDataProcessor* pProcessor);
	XnBool IsClaimed(const XnChar* strType, XnDeviceStream* pStream);

	void ProcessPacketChunk(XnSensorProtocolResponseHeader* pHeader, XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize);

private:
	XnStatus CheckClaimStream(const XnChar* strType, XnResolutions nRes, XnUInt32 nFPS, XnDeviceStream* pOwner);

	XnDevicePrivateData* m_pDevicePrivateData;

	class XnFirmwareStreamData
	{
	public:
		XnDataProcessorHolder* pProcessorHolder;
		const XnChar* strType;
		XnResolutions nRes;
		XnUInt32 nFPS;
		XnDeviceStream* pOwner;
	};

	typedef XnStringsHashT<XnFirmwareStreamData> XnFirmwareStreamsHash;

	XnFirmwareStreamsHash m_FirmwareStreams;

	XnDataProcessorHolder m_DepthProcessor;
	XnDataProcessorHolder m_ImageProcessor;
	XnDataProcessorHolder m_AudioProcessor;
	XnDataProcessorHolder m_GMCDebugProcessor;
	XnDataProcessorHolder m_WavelengthCorrectionDebugProcessor;
	XnDataProcessorHolder m_TecDebugProcessor;
	XnDataProcessorHolder m_NesaDebugProcessor;
	XnDataProcessorHolder m_GeneralDebugProcessor1;
	XnDataProcessorHolder m_GeneralDebugProcessor2;
};

#endif // XNFIRMWARESTREAMS_H
