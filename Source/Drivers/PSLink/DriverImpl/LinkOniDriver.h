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
#ifndef LINKONIDRIVER_H
#define LINKONIDRIVER_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <Driver/OniDriverAPI.h>
#include <XnLib.h>
#include <XnStringsHash.h>
#include "LinkOniDevice.h"
#include <XnLogWriterBase.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class LinkOniDriver :
	public oni::driver::DriverBase
{
public:
	LinkOniDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices), m_writer(pDriverServices), m_connectedEventHandle(NULL), m_disconnectedEventHandle(NULL)
	{
		m_configFilePath[0] = '\0';
	}

	virtual OniStatus initialize(oni::driver::DeviceConnectedCallback deviceConnectedCallback, oni::driver::DeviceDisconnectedCallback deviceDisconnectedCallback, oni::driver::DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie);
	virtual void shutdown();

	virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* mode);
	virtual void deviceClose(oni::driver::DeviceBase* pDevice);

	/*
	virtual void* enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount);
	virtual void disableFrameSync(void* frameSyncGroup);
	*/

	void ClearDevice(const char* uri);

protected:
	static void XN_CALLBACK_TYPE OnDevicePropertyChanged(const XnChar* ModuleName, XnUInt32 nPropertyId, void* pCookie);
	static void XN_CALLBACK_TYPE OnDeviceConnected(const OniDeviceInfo& deviceInfo, void* pCookie);
	static void XN_CALLBACK_TYPE OnDeviceDisconnected(const OniDeviceInfo& deviceInfo, void* pCookie);

	//uri -> LinkOniDevice map
	xnl::StringsHash<LinkOniDevice*> m_devices;

private:
	class LinkOpenNILogWriter : public XnLogWriterBase
	{
	public:
		LinkOpenNILogWriter(OniDriverServices* pDriverServices);
		virtual void WriteEntry(const XnLogEntry* pEntry);
		virtual void WriteUnformatted(const XnChar* strMessage);

	private:
		OniDriverServices* m_pDriverServices;
	};

	typedef struct 
	{
		LinkOniDevice* pDevice;
	} FrameSyncGroup;

	void resolveConfigFilePath();

	LinkOpenNILogWriter m_writer;
	XnCallbackHandle m_connectedEventHandle;
	XnCallbackHandle m_disconnectedEventHandle;
	char m_configFilePath[XN_FILE_MAX_PATH];
};

#endif // LINKONIDRIVER_H
