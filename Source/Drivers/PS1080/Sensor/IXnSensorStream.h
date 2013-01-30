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
#ifndef __I_XN_SENSOR_STREAM_H__
#define __I_XN_SENSOR_STREAM_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnStatus.h>
#include <XnStreamParams.h>

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------
class XnDataProcessor;

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class IXnSensorStream
{
public:
	virtual ~IXnSensorStream() {}

	virtual void GetFirmwareStreamConfig(XnResolutions* pnRes, XnUInt32* pnFPS) = 0;
	virtual XnStatus ConfigureStreamImpl() = 0;
	virtual XnStatus OpenStreamImpl() = 0;
	virtual XnStatus CloseStreamImpl() = 0;
	virtual XnStatus CreateDataProcessor(XnDataProcessor** ppProcessor) = 0;
	virtual XnStatus MapPropertiesToFirmware() = 0;
};

#endif //__I_XN_SENSOR_STREAM_H__
