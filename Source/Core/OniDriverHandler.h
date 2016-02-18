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
#ifndef ONIDRIVERHANDLER_H
#define ONIDRIVERHANDLER_H

#include "OniCAPI.h"
#include "OniCTypes.h"
#include "XnErrorLogger.h"
#include "XnPlatform.h"
#include "XnErrorLogger.h"
#include "OniCommon.h"
#include "Driver/OniDriverTypes.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

typedef void (ONI_CALLBACK_TYPE* OniDriverDeviceConnected)(const OniDeviceInfo*, void* pCookie);
typedef void (ONI_CALLBACK_TYPE* OniDriverDeviceDisconnected)(const OniDeviceInfo*, void* pCookie);
typedef void (ONI_CALLBACK_TYPE* OniDriverDeviceStateChanged)(const OniDeviceInfo*, OniDeviceState, void* pCookie);

typedef void (ONI_CALLBACK_TYPE* OniDriverNewFrame)(void* streamHandle, OniFrame*, void* pCookie);
typedef void (ONI_CALLBACK_TYPE* OniDriverPropertyChanged)(void* sender, int propertyId, const void* data, int dataSize, void* pCookie);

struct Funcs
{
	// As Driver
	void (ONI_C_DECL* oniDriverCreate)(OniDriverServices* driverServices);
	void (ONI_C_DECL* oniDriverDestroy)();
	OniStatus (ONI_C_DECL* oniDriverInitialize)(OniDriverDeviceConnected connectedCallback, OniDriverDeviceDisconnected disconnectedCallback, 
                                                OniDriverDeviceStateChanged deviceStateChangedCallback, void* pCookie);
	void (ONI_C_DECL* oniDriverRun)();

	OniStatus (ONI_C_DECL* oniDriverTryDevice)(const char* uri);

	// As Device
	void* (ONI_C_DECL* oniDriverDeviceOpen)(const char* uri, const char* mode);
	void (ONI_C_DECL* oniDriverDeviceClose)(void* deviceHandle);

	OniStatus (ONI_C_DECL* oniDriverDeviceGetSensorInfoList)(void* deviceHandle, OniSensorInfo** pSensors, int* numSensors);

	void* (ONI_C_DECL* oniDriverDeviceCreateStream)(void* deviceHandle, OniSensorType sensorType);
	void (ONI_C_DECL* oniDriverDeviceDestroyStream)(void* deviceHandle, void* streamHandle);

	OniStatus (ONI_C_DECL* oniDriverDeviceSetProperty)(void* deviceHandle, int propertyId, const void* data, int dataSize);
	OniStatus (ONI_C_DECL* oniDriverDeviceGetProperty)(void* deviceHandle, int propertyId, void* data, int* pDataSize);
	OniBool (ONI_C_DECL* oniDriverDeviceIsPropertySupported)(void* deviceHandle, int propertyId);
	void (ONI_C_DECL* oniDriverDeviceSetPropertyChangedCallback)(void* deviceHandle, OniDriverPropertyChanged handler, void* pCookie);
	void (ONI_C_DECL* oniDriverDeviceNotifyAllProperties)(void* deviceHandle);
	OniStatus (ONI_C_DECL* oniDriverDeviceInvoke)(void* deviceHandle, int commandId, const void* data, int dataSize);
	OniBool (ONI_C_DECL* oniDriverDeviceIsCommandSupported)(void* deviceHandle, int commandId);
	OniStatus (ONI_C_DECL* oniDriverDeviceTryManualTrigger)(void* deviceHandle);
	OniBool (ONI_C_DECL* oniDriverDeviceIsImageRegistrationModeSupported)(void* deviceHandle, OniImageRegistrationMode mode);

	// As Stream
	void (ONI_C_DECL* oniDriverStreamSetServices)(void* streamHandle, OniStreamServices* pServices);
	OniStatus (ONI_C_DECL* oniDriverStreamSetProperty)(void* streamHandle, int propertyId, const void* data, int dataSize);
	OniStatus (ONI_C_DECL* oniDriverStreamGetProperty)(void* streamHandle, int propertyId, void* data, int* pDataSize);
	OniBool (ONI_C_DECL* oniDriverStreamIsPropertySupported)(void* streamHandle, int propertyId);
	void (ONI_C_DECL* oniDriverStreamSetPropertyChangedCallback)(void* streamHandle, OniDriverPropertyChanged handler, void* pCookie);
	void (ONI_C_DECL* oniDriverStreamNotifyAllProperties)(void* streamHandle);
	OniStatus (ONI_C_DECL* oniDriverStreamInvoke)(void* streamHandle, int commandId, const void* data, int dataSize);
	OniBool (ONI_C_DECL* oniDriverStreamIsCommandSupported)(void* streamHandle, int commandId);

	OniStatus (ONI_C_DECL* oniDriverStreamStart)(void* streamHandle);
	void (ONI_C_DECL* oniDriverStreamStop)(void* streamHandle);

	int (ONI_C_DECL* oniDriverStreamGetRequiredFrameSize)(void* streamHandle);

	void (ONI_C_DECL* oniDriverStreamSetNewFrameCallback)(void* streamHandle, OniDriverNewFrame handler, void* pCookie);
	OniStatus (ONI_C_DECL* oniDriverStreamConvertDepthToColorCoordinates)(void* depthStreamHandle, void* colorStreamHandle, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY);

	void* (ONI_C_DECL* oniDriverEnableFrameSync)(void** pStreamHandles, int streamCount);
	void (ONI_C_DECL* oniDriverDisableFrameSync)(void* frameSyncGroup);
};


class DriverHandler
{
public:
	DriverHandler(const char* library, xnl::ErrorLogger& errorLogger);
	~DriverHandler();

	bool isValid() const {return m_valid;}

	void create(OniDriverServices* driverServices) const 
	{
		(*funcs.oniDriverCreate)(driverServices);
	}
	void destroy() const 
	{
		(*funcs.oniDriverDestroy)();
	}
	OniStatus initialize(OniDriverDeviceConnected connectedCallback, OniDriverDeviceDisconnected disconnectedCallback, 
					OniDriverDeviceStateChanged deviceStateChangedCallback, void* pCookie) const 
	{
		return (*funcs.oniDriverInitialize)(connectedCallback, disconnectedCallback, deviceStateChangedCallback, pCookie);
	}

	OniStatus tryDevice(const char* uri) const 
	{
		return (*funcs.oniDriverTryDevice)(uri);
	}

	void* deviceOpen(const char* uri, const char* mode) const 
	{
		return (*funcs.oniDriverDeviceOpen)(uri, mode);
	}
	void deviceClose(void* deviceHandle) const 
	{
		(*funcs.oniDriverDeviceClose)(deviceHandle);
	}

	OniStatus deviceGetSensorInfoList(void* deviceHandle, OniSensorInfo** pSensors, int* numSensors) const 
	{
		return (*funcs.oniDriverDeviceGetSensorInfoList)(deviceHandle, pSensors, numSensors);
	}

	void* deviceCreateStream(void* deviceHandle, OniSensorType sensorType) const 
	{
		return (*funcs.oniDriverDeviceCreateStream)(deviceHandle, sensorType); // return stream
	}

	void deviceDestroyStream(void* deviceHandle, void* streamHandle) const
	{
		(*funcs.oniDriverDeviceDestroyStream)(deviceHandle, streamHandle);
	}

	OniStatus deviceSetProperty(void* deviceHandle, int propertyId, const void* data, int dataSize) const 
	{
		return (*funcs.oniDriverDeviceSetProperty)(deviceHandle, propertyId, data, dataSize);
	}
	OniStatus deviceGetProperty(void* deviceHandle, int propertyId, void* data, int* pDataSize) const 
	{
		return (*funcs.oniDriverDeviceGetProperty)(deviceHandle, propertyId, data, pDataSize);
	}
	OniBool deviceIsPropertySupported(void* deviceHandle, int propertyId) const 
	{
		return (*funcs.oniDriverDeviceIsPropertySupported)(deviceHandle, propertyId);
	}
	void deviceSetPropertyChangedCallback(void* deviceHandle, OniDriverPropertyChanged handler, void* pCookie) const
	{
		(*funcs.oniDriverDeviceSetPropertyChangedCallback)(deviceHandle, handler, pCookie);
	}
	void deviceNotifyAllProperties(void* deviceHandle) const
	{
		(*funcs.oniDriverDeviceNotifyAllProperties)(deviceHandle);
	}
	OniStatus deviceInvoke(void* deviceHandle, int commandId, const void* data, int dataSize) const
	{
		return (*funcs.oniDriverDeviceInvoke)(deviceHandle, commandId, data, dataSize);
	}
	OniBool deviceIsCommandSupported(void* deviceHandle, int commandId) const 
	{
		return (*funcs.oniDriverDeviceIsCommandSupported)(deviceHandle, commandId);
	}
	OniBool deviceIsImageRegistrationModeSupported(void* deviceHandle, OniImageRegistrationMode mode) const 
	{
		return (*funcs.oniDriverDeviceIsImageRegistrationModeSupported)(deviceHandle, mode);
	}
	OniStatus tryManualTrigger(void* deviceHandle) const
	{
		return (*funcs.oniDriverDeviceTryManualTrigger)(deviceHandle);
	}
	////////////////////////////

	void streamSetServices(void* streamHandle, OniStreamServices* pServices) const
	{
		(*funcs.oniDriverStreamSetServices)(streamHandle, pServices);
	}
	OniStatus streamSetProperty(void* streamHandle, int propertyId, const void* data, int dataSize) const
	{
		return (*funcs.oniDriverStreamSetProperty)(streamHandle, propertyId, data, dataSize);
	}
	OniStatus streamGetProperty(void* streamHandle, int propertyId, void* data, int* pDataSize) const
	{
		return (*funcs.oniDriverStreamGetProperty)(streamHandle, propertyId, data, pDataSize);
	}
	OniBool streamIsPropertySupported(void* streamHandle, int propertyId) const
	{
		return (*funcs.oniDriverStreamIsPropertySupported)(streamHandle, propertyId);
	}
	void streamSetPropertyChangedCallback(void* streamHandle, OniDriverPropertyChanged handler, void* pCookie) const
	{
		(*funcs.oniDriverStreamSetPropertyChangedCallback)(streamHandle, handler, pCookie);
	}
	void streamNotifyAllProperties(void* streamHandle) const
	{
		(*funcs.oniDriverStreamNotifyAllProperties)(streamHandle);
	}
	OniStatus streamInvoke(void* streamHandle, int commandId, const void* data, int dataSize) const
	{
		return (*funcs.oniDriverStreamInvoke)(streamHandle, commandId, data, dataSize);
	}
	OniBool streamIsCommandSupported(void* streamHandle, int commandId) const
	{
		return (*funcs.oniDriverStreamIsCommandSupported)(streamHandle, commandId);
	}

	OniStatus streamStart(void* streamHandle) const
	{
		return (*funcs.oniDriverStreamStart)(streamHandle);
	}
	void streamStop(void* streamHandle) const 
	{
		(*funcs.oniDriverStreamStop)(streamHandle);
	}

	int streamGetRequiredFrameSize(void* streamHandle) const
	{
		return (*funcs.oniDriverStreamGetRequiredFrameSize)(streamHandle);
	}

	void streamSetNewFrameCallback(void* streamHandle, OniDriverNewFrame handler, void* pCookie) const 
	{
		(*funcs.oniDriverStreamSetNewFrameCallback)(streamHandle, handler, pCookie);
	}

	OniStatus convertDepthPointToColor(void* depthStreamHandle, void* colorStreamHandle, int depthX, int depthY, OniDepthPixel DepthZ, int* pColorX, int* pColorY) const
	{
		return (*funcs.oniDriverStreamConvertDepthToColorCoordinates)(depthStreamHandle, colorStreamHandle, depthX, depthY, DepthZ, pColorX, pColorY);
	}

	void* enableFrameSync(void** streamHandles, int streamCount) const
	{
		return (*funcs.oniDriverEnableFrameSync)(streamHandles, streamCount);
	}
	void disableFrameSync(void* frameSyncGroup) const
	{
		return (*funcs.oniDriverDisableFrameSync)(frameSyncGroup);
	}

private:
	Funcs funcs;

	XN_LIB_HANDLE m_libHandle;
	bool m_valid;
};



ONI_NAMESPACE_IMPLEMENTATION_END


#endif // ONIDRIVERHANDLER_H
