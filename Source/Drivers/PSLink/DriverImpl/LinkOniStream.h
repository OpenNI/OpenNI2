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
#ifndef LINKONISTREAM_H
#define LINKONISTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <Driver/OniDriverAPI.h>
#include <XnLib.h>
#include "PrimeClient.h"
#include <XnLinkFrameInputStream.h>


//using namespace oni::driver;
//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnDeviceStream;
class LinkOniDevice;

class LinkOniStream :
	public oni::driver::StreamBase
{
public:
	LinkOniStream(const char* configFile, const char* configSection, xn::PrimeClient* pSensor, OniSensorType sensorType, LinkOniDevice* pDevice);
	~LinkOniStream();

	virtual XnStatus Init();

	virtual void setServices(oni::driver::StreamServices* pStreamServices);

	virtual int getRequiredFrameSize() { return m_pInputStream->GetRequiredFrameSize(); }

	OniStatus start();
	void stop();

	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
	virtual OniBool isPropertySupported(int propertyId);

	LinkOniDevice* GetDevice() { return m_pDevice; }
	xn::LinkFrameInputStream* GetDeviceStream() { return m_pInputStream; }

protected:
	XnStatus setIntPropertyFromINI(const char* key, int propertyId);

	const char* m_configFile;
	const char* m_configSection;
	OniSensorType m_sensorType;
	xn::PrimeClient* m_pSensor;
	LinkOniDevice* m_pDevice;

	XnUInt16 m_streamId;
	// TODO: should have used generic stream instead, but openni2 doesn't support non-frame streams yet.
	xn::LinkFrameInputStream* m_pInputStream;
	XnCallbackHandle m_hNewDataCallback;

private:
	void destroy();
	XnBool m_started;

	static void XN_CALLBACK_TYPE OnNewStreamDataEventHandler(const xn::NewFrameEventArgs& args, void* pCookie);
};

#endif // LINKONISTREAM_H
