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
#include "OniDriverHandler.h"
#include "XnLib.h"
#include <XnLog.h>

#define OniGetProcAddress(function)																\
{																								\
	rc = xnOSGetProcAddress(m_libHandle, XN_STRINGIFY(function), (XnFarProc*)&funcs.function);	\
	if (rc != ONI_STATUS_OK)																	\
	{																							\
		xnLogWarning("DriverHandler", "LibraryHandler: Couldn't find function %s in %s. Stopping", XN_STRINGIFY(function), library);	\
		errorLogger.Append("LibraryHandler: Couldn't find function %s in %s. Stopping", XN_STRINGIFY(function), library);		\
		return;																					\
	}																							\
}

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

DriverHandler::DriverHandler(const char* library, xnl::ErrorLogger& errorLogger)
{
	m_valid = false;

	xnOSMemSet(&funcs, 0, sizeof(funcs));

	XnStatus rc = xnOSLoadLibrary(library, &m_libHandle);
	if (rc != XN_STATUS_OK)
	{
		errorLogger.Append("LibraryHandler: Couldn't load library %s", library);
		return;
	}

	OniGetProcAddress(oniDriverCreate);
	OniGetProcAddress(oniDriverDestroy);
	OniGetProcAddress(oniDriverInitialize);
	OniGetProcAddress(oniDriverTryDevice);

	OniGetProcAddress(oniDriverDeviceOpen);
	OniGetProcAddress(oniDriverDeviceClose);
	OniGetProcAddress(oniDriverDeviceGetSensorInfoList);

	OniGetProcAddress(oniDriverDeviceCreateStream);
	OniGetProcAddress(oniDriverDeviceDestroyStream);
	OniGetProcAddress(oniDriverDeviceSetProperty);
	OniGetProcAddress(oniDriverDeviceGetProperty);
	OniGetProcAddress(oniDriverDeviceIsPropertySupported);
	OniGetProcAddress(oniDriverDeviceSetPropertyChangedCallback);
	OniGetProcAddress(oniDriverDeviceNotifyAllProperties);
	OniGetProcAddress(oniDriverDeviceInvoke);
	OniGetProcAddress(oniDriverDeviceIsCommandSupported);
	OniGetProcAddress(oniDriverDeviceIsImageRegistrationModeSupported);
	OniGetProcAddress(oniDriverDeviceTryManualTrigger);

	OniGetProcAddress(oniDriverStreamSetServices);
	OniGetProcAddress(oniDriverStreamSetProperty);
	OniGetProcAddress(oniDriverStreamGetProperty);
	OniGetProcAddress(oniDriverStreamIsPropertySupported);
	OniGetProcAddress(oniDriverStreamSetPropertyChangedCallback);
	OniGetProcAddress(oniDriverStreamNotifyAllProperties);
	OniGetProcAddress(oniDriverStreamInvoke);
	OniGetProcAddress(oniDriverStreamIsCommandSupported);
	OniGetProcAddress(oniDriverStreamStart);
	OniGetProcAddress(oniDriverStreamStop);
	OniGetProcAddress(oniDriverStreamGetRequiredFrameSize);
	OniGetProcAddress(oniDriverStreamSetNewFrameCallback);
	OniGetProcAddress(oniDriverStreamConvertDepthToColorCoordinates);

	OniGetProcAddress(oniDriverEnableFrameSync);
	OniGetProcAddress(oniDriverDisableFrameSync);

	m_valid = true;
}

DriverHandler::~DriverHandler()
{
	if (m_valid)
	{
		xnOSFreeLibrary(m_libHandle);
		m_libHandle = NULL;
	}
	m_valid = false;
}

ONI_NAMESPACE_IMPLEMENTATION_END
