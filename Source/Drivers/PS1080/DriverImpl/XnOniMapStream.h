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
#ifndef __XN_ONI_MAP_STREAM_H__
#define __XN_ONI_MAP_STREAM_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnOniStream.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnOniMapStream :
	public XnOniStream
{
public:
	XnOniMapStream(XnSensor* pSensor, const XnChar* strName, OniSensorType sensorType, XnOniDevice* pDevice);
	virtual ~XnOniMapStream();

	virtual XnStatus Init();

	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual XnStatus SetPropertyImpl(int propertyId, const void* data, int dataSize);
	virtual OniBool isPropertySupported(int propertyId);
	virtual void notifyAllProperties();

	XnStatus GetVideoMode(OniVideoMode* pVideoMode);
	XnStatus SetVideoMode(OniVideoMode* pVideoMode);

	XnStatus GetMirror(OniBool* pEnabled);
	XnStatus SetMirror(OniBool* pEnabled);

	XnStatus GetCropping(OniCropping &cropping);
	XnStatus SetCropping(const OniCropping &cropping);

protected:
	struct SupportedVideoMode
	{
		OniVideoMode OutputMode;
		XnUInt32 nInputFormat;
	};

	XnUInt32		m_nSupportedModesCount;
	SupportedVideoMode*	m_aSupportedModes;

private:
	XnStatus FillSupportedVideoModes();
};

#endif // __XN_ONI_MAP_STREAM_H__
