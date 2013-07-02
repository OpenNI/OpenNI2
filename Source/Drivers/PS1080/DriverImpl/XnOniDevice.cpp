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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnOniDevice.h"
#include "XnOniStream.h"
#include "XnOniDriver.h"
#include "../Sensor/XnDeviceEnumeration.h"
#include "../DDK/XnPropertySetInternal.h"

//---------------------------------------------------------------------------
// XnOniDevice class
//---------------------------------------------------------------------------
XnOniDevice::XnOniDevice(const XnChar* uri, oni::driver::DriverServices& driverServices, XnOniDriver* pDriver) : m_driverServices(driverServices), m_pDriver(pDriver)
{
	xnOSMemCopy(&m_info, XnDeviceEnumeration::GetDeviceInfo(uri), sizeof(m_info));
}

XnOniDevice::~XnOniDevice()
{
	// free the allocated arrays
	for(int i=0; i < m_numSensors; ++i)
	{
		XN_DELETE_ARR(m_sensors[i].pSupportedVideoModes);
	}
	m_sensor.Destroy();
}

XnStatus XnOniDevice::FillSupportedVideoModes()
{
	XnUInt32 nSupportedModes      = 0;
	XnCmosPreset* pSupportedModes = NULL;
	
	int s = 0;

	// Depth
	nSupportedModes = m_sensor.GetDevicePrivateData()->FWInfo.depthModes.GetSize();
	pSupportedModes = m_sensor.GetDevicePrivateData()->FWInfo.depthModes.GetData();

	m_sensors[s].sensorType             = ONI_SENSOR_DEPTH;
	m_sensors[s].pSupportedVideoModes   = XN_NEW_ARR(OniVideoMode, nSupportedModes);
	XN_VALIDATE_ALLOC_PTR(m_sensors[s].pSupportedVideoModes);
	
	OniPixelFormat depthFormats[] = { ONI_PIXEL_FORMAT_DEPTH_1_MM, ONI_PIXEL_FORMAT_DEPTH_100_UM };
	XnSizeT depthFormatsCount = sizeof(depthFormats) / sizeof(depthFormats[0]);

	int writeIndex = 0;
	for(XnUInt32 i = 0; i < nSupportedModes; ++i)
	{
		for (XnSizeT formatIndex = 0; formatIndex < depthFormatsCount; ++formatIndex)
		{
			m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat = depthFormats[formatIndex];
			m_sensors[s].pSupportedVideoModes[writeIndex].fps = pSupportedModes[i].nFPS;
			XnBool bOK = XnDDKGetXYFromResolution(
				(XnResolutions)pSupportedModes[i].nResolution,
				(XnUInt32*)&m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX,
				(XnUInt32*)&m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY
				);
			XN_ASSERT(bOK);
			XN_REFERENCE_VARIABLE(bOK);

			bool foundMatch = false;
			for (int j = 0; j < writeIndex; ++j)
			{
				if (m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat == m_sensors[s].pSupportedVideoModes[j].pixelFormat &&
					m_sensors[s].pSupportedVideoModes[writeIndex].fps == m_sensors[s].pSupportedVideoModes[j].fps &&
					m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX == m_sensors[s].pSupportedVideoModes[j].resolutionX &&
					m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY == m_sensors[s].pSupportedVideoModes[j].resolutionY)
				{
					// Already know this configuration
					foundMatch = true;
					break;
				}
			}
			if (!foundMatch)
			{
				++writeIndex;
			}
		}
	}
	m_sensors[s].numSupportedVideoModes = writeIndex;

	// Image

	// first, make sure that our sensor actually supports Image
	XnUInt64 nImageSupported = FALSE;
	XnStatus nRetVal = m_sensor.GetProperty(XN_MASK_DEVICE, XN_MODULE_PROPERTY_IMAGE_SUPPORTED, &nImageSupported);
	XN_IS_STATUS_OK(nRetVal);
	if (nImageSupported)
	{
		++s;
		nSupportedModes = m_sensor.GetDevicePrivateData()->FWInfo.imageModes.GetSize();
		pSupportedModes = m_sensor.GetDevicePrivateData()->FWInfo.imageModes.GetData();

		m_sensors[s].sensorType             = ONI_SENSOR_COLOR;
		m_sensors[s].numSupportedVideoModes = 0; // to be changed later..
		m_sensors[s].pSupportedVideoModes   = XN_NEW_ARR(OniVideoMode, nSupportedModes * 10);
		XN_VALIDATE_ALLOC_PTR(m_sensors[s].pSupportedVideoModes);
		
		writeIndex = 0;
		for(XnUInt32 j=0; j < nSupportedModes; ++j)
		{
			// make an OniVideoMode for each OniFormat supported by the input format
			OniPixelFormat aOniFormats[10];
			int       nOniFormats = 0;
			XnOniColorStream::GetAllowedOniOutputFormatForInputFormat((XnIOImageFormats)pSupportedModes[j].nFormat, aOniFormats, &nOniFormats);
			for(int curOni=0; curOni<nOniFormats; ++curOni)
			{
				m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat = aOniFormats[curOni];
			
				m_sensors[s].pSupportedVideoModes[writeIndex].fps = pSupportedModes[j].nFPS;
				XnBool bOK = XnDDKGetXYFromResolution(
					(XnResolutions)pSupportedModes[j].nResolution,
					(XnUInt32*)&m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX,
					(XnUInt32*)&m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY
					);
				XN_ASSERT(bOK);
				XN_REFERENCE_VARIABLE(bOK);

				bool foundMatch = false;
				for (int i = 0; i < writeIndex; ++i)
				{
					if (m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat == m_sensors[s].pSupportedVideoModes[i].pixelFormat &&
						m_sensors[s].pSupportedVideoModes[writeIndex].fps == m_sensors[s].pSupportedVideoModes[i].fps &&
						m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX == m_sensors[s].pSupportedVideoModes[i].resolutionX &&
						m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY == m_sensors[s].pSupportedVideoModes[i].resolutionY)
					{
						// Already know this configuration
						foundMatch = true;
						break;
					}
				}
				if (!foundMatch)
				{
					++writeIndex;
				}
			}
		}
		m_sensors[s].numSupportedVideoModes = writeIndex;
	}

	// IR
	++s;
	nSupportedModes = m_sensor.GetDevicePrivateData()->FWInfo.irModes.GetSize();
	pSupportedModes = m_sensor.GetDevicePrivateData()->FWInfo.irModes.GetData();

	m_sensors[s].sensorType             = ONI_SENSOR_IR;
	m_sensors[s].pSupportedVideoModes   = XN_NEW_ARR(OniVideoMode, nSupportedModes*2);
	XN_VALIDATE_ALLOC_PTR(m_sensors[s].pSupportedVideoModes);
	
	OniPixelFormat irFormats[] = {ONI_PIXEL_FORMAT_GRAY16, ONI_PIXEL_FORMAT_RGB888};
	writeIndex = 0;
	for(XnUInt32 i=0; i < nSupportedModes; ++i)
	{
		for (int fmt = 0; fmt <= 1; ++fmt)
		{
			m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat = irFormats[fmt];
			m_sensors[s].pSupportedVideoModes[writeIndex].fps = pSupportedModes[i].nFPS;
			XnBool bOK = XnDDKGetXYFromResolution(
				(XnResolutions)pSupportedModes[i].nResolution,
				(XnUInt32*)&m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX,
				(XnUInt32*)&m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY
				);
			XN_ASSERT(bOK);
			XN_REFERENCE_VARIABLE(bOK);

			bool foundMatch = false;
			for (int j = 0; j < writeIndex; ++j)
			{
				if (m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat == m_sensors[s].pSupportedVideoModes[j].pixelFormat &&
					m_sensors[s].pSupportedVideoModes[writeIndex].fps == m_sensors[s].pSupportedVideoModes[j].fps &&
					m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX == m_sensors[s].pSupportedVideoModes[j].resolutionX &&
					m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY == m_sensors[s].pSupportedVideoModes[j].resolutionY)
				{
					// Already know this configuration
					foundMatch = true;
					break;
				}
			}
			if (!foundMatch)
			{
				++writeIndex;
			}
		}
	}
	m_sensors[s].numSupportedVideoModes = writeIndex;
	m_numSensors = s+1;

	return XN_STATUS_OK;
}

XnStatus XnOniDevice::Init(const char* mode)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_PROPERTY_SET_CREATE_ON_STACK(initialValues);

	if (mode != NULL)
	{
		nRetVal = XnPropertySetAddModule(&initialValues, XN_MODULE_NAME_DEVICE);
		XN_IS_STATUS_OK(nRetVal);

		for (int i = 0; mode[i] != '\0'; ++i)
		{
			switch (mode[i])
			{
			case 'L':
				nRetVal = XnPropertySetAddIntProperty(&initialValues, XN_MODULE_NAME_DEVICE, XN_MODULE_PROPERTY_LEAN_INIT, TRUE);
				XN_IS_STATUS_OK(nRetVal);
				break;
			case 'R':
				nRetVal = XnPropertySetAddIntProperty(&initialValues, XN_MODULE_NAME_DEVICE, XN_MODULE_PROPERTY_RESET_SENSOR_ON_STARTUP, FALSE);
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		}
	}

	XnDeviceConfig config;
	config.cpConnectionString = m_info.uri;
	config.pInitialValues = &initialValues;
	XnStatus retVal = m_sensor.Init(&config);
	XN_IS_STATUS_OK(retVal);

	nRetVal = FillSupportedVideoModes();
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

OniStatus XnOniDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
	*numSensors = m_numSensors;
	*pSensors   = m_sensors;

	return ONI_STATUS_OK;
}

oni::driver::StreamBase* XnOniDevice::createStream(OniSensorType sensorType)
{
	XnOniStream* pStream;

	if (sensorType == ONI_SENSOR_DEPTH)
	{
		pStream = XN_NEW(XnOniDepthStream, &m_sensor, this);
	}
	else if (sensorType == ONI_SENSOR_COLOR)
	{
		pStream = XN_NEW(XnOniColorStream, &m_sensor, this);
	}
	else if (sensorType == ONI_SENSOR_IR)
	{
		pStream = XN_NEW(XnOniIRStream, &m_sensor, this);
	}
	else
	{
		m_driverServices.errorLoggerAppend("XnOniDevice: Can't create a stream of type %d", sensorType);
		return NULL;
	}

	XnStatus nRetVal = pStream->Init();
	if (nRetVal != XN_STATUS_OK)
	{
		m_driverServices.errorLoggerAppend("XnOniDevice: Can't initialize stream of type %d: %s", sensorType, xnGetStatusString(nRetVal));
		XN_DELETE(pStream);
		return NULL;
	}

	return pStream;
}

void XnOniDevice::destroyStream(oni::driver::StreamBase* pStream)
{
	XN_DELETE(pStream);
}

OniStatus XnOniDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
	switch (propertyId)
	{
	case ONI_DEVICE_PROPERTY_FIRMWARE_VERSION:
		{
			XnVersions &versions = m_sensor.GetDevicePrivateData()->Version;
			XnUInt32 nCharsWritten = 0;
			XnStatus rc = xnOSStrFormat((XnChar*)data, *pDataSize, &nCharsWritten, "%d.%d.%d", versions.nMajor, versions.nMinor, versions.nBuild);
			if (rc != XN_STATUS_OK)
			{
				m_driverServices.errorLoggerAppend("Couldn't get firmware version: %s\n", xnGetStatusString(rc));
				return ONI_STATUS_BAD_PARAMETER;
			}
			*pDataSize = nCharsWritten+1;

			break;
		}
	case ONI_DEVICE_PROPERTY_HARDWARE_VERSION:
		{
			XnVersions &versions = m_sensor.GetDevicePrivateData()->Version;
			int hwVer = versions.HWVer;
			if (*pDataSize == sizeof(int))
			{
				(*((int*)data)) = hwVer;
			}
			else if (*pDataSize == sizeof(short))
			{
				(*((short*)data)) = (short)hwVer;
			}
			else if (*pDataSize == sizeof(uint64_t))
			{
				(*((uint64_t*)data)) = (uint64_t)hwVer;
			}
			else
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d or %d or %d\n", *pDataSize, sizeof(short), sizeof(int), sizeof(uint64_t));
				return ONI_STATUS_ERROR;
			}
			break;
		}
	case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:
		{
			XnStatus rc = m_sensor.DeviceModule()->GetProperty(XN_MODULE_PROPERTY_SERIAL_NUMBER, data, pDataSize);
			if (rc != XN_STATUS_OK)
			{
				m_driverServices.errorLoggerAppend("Couldn't get serial number: %s\n", xnGetStatusString(rc));
				return ONI_STATUS_BAD_PARAMETER;
			}

			break;
		}
	case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
		{
			if (*pDataSize == sizeof(OniVersion))
			{
				OniVersion* version = (OniVersion*)data;
				version->major		 = XN_PS_MAJOR_VERSION;
				version->minor		 = XN_PS_MINOR_VERSION;
				version->maintenance = XN_PS_MAINTENANCE_VERSION;
				version->build		 = XN_PS_BUILD_VERSION;
			}
			else
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
				return ONI_STATUS_ERROR;
			}
		}
		break;
	case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
		{
			if (*pDataSize == sizeof(OniImageRegistrationMode))
			{
				OniImageRegistrationMode* mode = (OniImageRegistrationMode*)data;

				// Find the depth stream in the sensor.
				XnDeviceStream* pDepth = NULL;
				XnStatus xnrc = m_sensor.GetStream(XN_STREAM_NAME_DEPTH, &pDepth);
				if (xnrc != XN_STATUS_OK)
				{
					return ONI_STATUS_BAD_PARAMETER;
				}

				// Set the mode in the depth stream.
				XnUInt64 val;
				xnrc = pDepth->GetProperty(XN_STREAM_PROPERTY_REGISTRATION, &val);
				if (xnrc != XN_STATUS_OK)
				{
					return ONI_STATUS_ERROR;
				}

				// Update the return value.
				*mode = (val == 1) ? ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR : ONI_IMAGE_REGISTRATION_OFF;
			}
			else
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniImageRegistrationMode));
				return ONI_STATUS_ERROR;
			}
		}
		break;
	default:
		XnStatus nRetVal = m_sensor.DeviceModule()->GetProperty(propertyId, data, pDataSize);
		if (nRetVal != XN_STATUS_OK)
		{
			m_driverServices.errorLoggerAppend("Failed to set property %x: %s", propertyId, xnGetStatusString(nRetVal));
			return ONI_STATUS_BAD_PARAMETER;
		}
	}

	return ONI_STATUS_OK;
}

OniStatus XnOniDevice::setProperty(int propertyId, const void* data, int dataSize)
{
	switch (propertyId)
	{
	case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
		{
			if (dataSize == sizeof(OniImageRegistrationMode))
			{
				OniImageRegistrationMode* mode = (OniImageRegistrationMode*)data;

				// Find the depth stream in the sensor.
				XnDeviceStream* pDepth = NULL;
				XnStatus xnrc = m_sensor.GetStream(XN_STREAM_NAME_DEPTH, &pDepth);
				if (xnrc != XN_STATUS_OK)
				{
					return ONI_STATUS_BAD_PARAMETER;
				}

				// Set the mode in the depth stream.
				XnUInt64 val = (*mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) ? 1 : 0;
				xnrc = pDepth->SetProperty(XN_STREAM_PROPERTY_REGISTRATION, val);
				if (xnrc != XN_STATUS_OK)
				{
					return ONI_STATUS_ERROR;
				}
			}
			else
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(OniImageRegistrationMode));
				return ONI_STATUS_ERROR;
			}
		}
		break;
	default:
		XnStatus nRetVal = m_sensor.DeviceModule()->SetProperty(propertyId, data, dataSize);
		if (nRetVal != XN_STATUS_OK)
		{
			m_driverServices.errorLoggerAppend("Failed to set property %x: %s", propertyId, xnGetStatusString(nRetVal));
			return ONI_STATUS_BAD_PARAMETER;
		}
	}
	return ONI_STATUS_OK;
}
OniBool XnOniDevice::isPropertySupported(int propertyId)
{
	if (propertyId == ONI_DEVICE_PROPERTY_DRIVER_VERSION ||
		propertyId == ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION ||
		propertyId == ONI_DEVICE_PROPERTY_FIRMWARE_VERSION ||
		propertyId == ONI_DEVICE_PROPERTY_HARDWARE_VERSION ||
		propertyId == ONI_DEVICE_PROPERTY_SERIAL_NUMBER)
	{
		return TRUE;
	}
	else
	{
		XnBool propertyExists = FALSE;
		m_sensor.DeviceModule()->DoesPropertyExist(propertyId, &propertyExists);
		return propertyExists;
	}
}

void XnOniDevice::notifyAllProperties()
{
	XnUInt32 nValue = (XnUInt32)m_sensor.GetCurrentUsbInterface();
	int size = sizeof(nValue);
	raisePropertyChanged(XN_MODULE_PROPERTY_USB_INTERFACE, &nValue, sizeof(nValue));

	nValue = m_sensor.GetDeviceMirror();
	raisePropertyChanged(XN_MODULE_PROPERTY_MIRROR, &nValue, sizeof(nValue));

	nValue = m_sensor.GetDeviceMirror();
	raisePropertyChanged(XN_STREAM_PROPERTY_CLOSE_RANGE, &nValue, sizeof(nValue));

	getProperty(XN_MODULE_PROPERTY_RESET_SENSOR_ON_STARTUP, &nValue, &size);
	raisePropertyChanged(XN_MODULE_PROPERTY_RESET_SENSOR_ON_STARTUP, &nValue, sizeof(nValue));

	getProperty(XN_MODULE_PROPERTY_LEAN_INIT, &nValue, &size);
	raisePropertyChanged(XN_MODULE_PROPERTY_LEAN_INIT, &nValue, sizeof(nValue));

	XnChar strValue[XN_DEVICE_MAX_STRING_LENGTH];
	size = sizeof(strValue);
	getProperty(XN_MODULE_PROPERTY_SERIAL_NUMBER, strValue, &size);
	raisePropertyChanged(XN_MODULE_PROPERTY_SERIAL_NUMBER, strValue, size);

	XnVersions versions;
	size = sizeof(versions);
	getProperty(XN_MODULE_PROPERTY_VERSION, &versions, &size);
	raisePropertyChanged(XN_MODULE_PROPERTY_VERSION, &versions, size);
}

OniStatus XnOniDevice::EnableFrameSync(XnOniStream** pStreams, int streamCount)
{
	// Translate the XnOniStream to XnDeviceStream.
	xnl::Array<XnDeviceStream*> streams(streamCount);
	streams.SetSize(streamCount);
	for (int i = 0; i < streamCount; ++i)
	{
		streams[i] = pStreams[i]->GetDeviceStream();
	}

	// Set the frame sync group.
	XnStatus rc = m_sensor.SetFrameSyncStreamGroup(streams.GetData(), streamCount);
	if (rc != XN_STATUS_OK)
	{
		m_driverServices.errorLoggerAppend("Error setting frame-sync group (rc=%d)\n", rc);
		return ONI_STATUS_ERROR;
	}

	return ONI_STATUS_OK;
}

void XnOniDevice::DisableFrameSync()
{
	XnStatus rc = m_sensor.SetFrameSyncStreamGroup(NULL, 0);
	if (rc != XN_STATUS_OK)
	{
		m_driverServices.errorLoggerAppend("Error setting frame-sync group (rc=%d)\n", rc);
	}
}

OniBool XnOniDevice::isImageRegistrationModeSupported(OniImageRegistrationMode mode)
{
	return (mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR || mode == ONI_IMAGE_REGISTRATION_OFF);
}
