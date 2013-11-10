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
#ifndef LINKONIMAPSTREAM_H
#define LINKONIMAPSTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LinkOniStream.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class LinkOniMapStream :
	public LinkOniStream
{
public:
	LinkOniMapStream(const char* configFile, const char* configSection, xn::PrimeClient* pSensor, OniSensorType sensorType, LinkOniDevice* pDevice);
	virtual ~LinkOniMapStream();

	virtual XnStatus Init();

	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
	virtual OniBool isPropertySupported(int propertyId);
	virtual void notifyAllProperties();

	XnStatus GetVideoMode(OniVideoMode* pVideoMode);
	XnStatus SetVideoMode(OniVideoMode* pVideoMode);

	XnStatus GetMirror(OniBool* pEnabled);
	XnStatus SetMirror(OniBool enabled);

	XnStatus GetCropping(OniCropping &cropping);
	XnStatus SetCropping(const OniCropping &cropping);

protected:
	struct SupportedVideoMode
	{
		OniVideoMode OutputMode;
		XnFwPixelFormat nInputFormat;
	};

	virtual XnStatus GetDefaultVideoMode(OniVideoMode* pVideoMode);

	int                 m_nSupportedModesCount;
	SupportedVideoMode*	m_aSupportedModes;

private:
	XnStatus FillSupportedVideoModes();
};

#endif // LINKONIMAPSTREAM_H
