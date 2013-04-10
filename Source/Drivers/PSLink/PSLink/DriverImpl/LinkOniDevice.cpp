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
#include "LinkOniDevice.h"
#include "LinkOniDriver.h"

#include "LinkDeviceEnumeration.h"
#include <PS1200Device.h>
#include <XnPsVersion.h>

//---------------------------------------------------------------------------
// LinkOniDevice class
//---------------------------------------------------------------------------
#define XN_MASK_LINK_DEVICE "LinkDevice"

LinkOniDevice::LinkOniDevice(const XnChar* uri, oni::driver::DriverServices& driverServices, LinkOniDriver* pDriver) :
	m_pSensor(NULL), m_didReset(false), m_driverServices(driverServices), m_pDriver(pDriver)
{
	xnOSMemCopy(&m_info, LinkDeviceEnumeration::GetDeviceInfo(uri), sizeof(m_info));
}

LinkOniDevice::~LinkOniDevice()
{
	// free the allocated arrays
	for(int i=0; i < m_numSensors; ++i)
	{
		XN_DELETE_ARR(m_sensors[i].pSupportedVideoModes);
	}
	
	Destroy();
}

XnStatus LinkOniDevice::readSupportedModesFromStream(XnStreamInfo &info, xnl::Array<XnStreamVideoMode> &aSupportedModes)
{
	XnUInt16 streamId;
	XnStatus nRetVal = m_pSensor->CreateInputStream(info.m_nStreamType, info.m_strCreationInfo, streamId);
	XN_IS_STATUS_OK(nRetVal);

	// TODO: make sure this cast doesn't make us problems
	xn::LinkFrameInputStream *pInputStream = (xn::LinkFrameInputStream *)m_pSensor->GetInputStream(streamId);
	XN_VALIDATE_OUTPUT_PTR(pInputStream);

	aSupportedModes.CopyFrom(pInputStream->GetSupportedVideoModes());

	m_pSensor->DestroyInputStream(streamId);
	return XN_STATUS_OK;
}

XnStatus LinkOniDevice::FillSupportedVideoModes()
{
	int                           nSupportedModes = 0;
	xnl::Array<XnStreamVideoMode> aSupportedModes;
	
	xnl::Array<XnStreamInfo> aEnumerated;

	int s = -1;
	int writeIndex;

	// Depth
	m_pSensor->EnumerateStreams((XnStreamType)XN_LINK_STREAM_TYPE_SHIFTS, aEnumerated);
	for (int c = 0; c < (int)aEnumerated.GetSize(); ++c)
	{
		XnStatus nRetVal = readSupportedModesFromStream(aEnumerated[c], aSupportedModes);
		XN_IS_STATUS_OK(nRetVal);

		++s;
		m_sensors[s].sensorType             = ONI_SENSOR_DEPTH;
		m_sensors[s].pSupportedVideoModes   = XN_NEW_ARR(OniVideoMode, aSupportedModes.GetSize());
		XN_VALIDATE_ALLOC_PTR(m_sensors[s].pSupportedVideoModes);
		nSupportedModes = aSupportedModes.GetSize();

		writeIndex = 0;
		for(int i=0; i < nSupportedModes; ++i)
		{
			m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
			m_sensors[s].pSupportedVideoModes[writeIndex].fps         = aSupportedModes[i].m_nFPS;
			m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX = aSupportedModes[i].m_nXRes;
			m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY = aSupportedModes[i].m_nYRes;

			bool foundMatch = false;
			for (int j = 0; j < writeIndex; ++j)
			{
				if (m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat == m_sensors[s].pSupportedVideoModes[j].pixelFormat &&
					m_sensors[s].pSupportedVideoModes[writeIndex].fps         == m_sensors[s].pSupportedVideoModes[j].fps         &&
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
		m_sensors[s].numSupportedVideoModes = writeIndex;
		m_numSensors = s+1;
	}
	aEnumerated.Clear();

	// IR
	m_pSensor->EnumerateStreams((XnStreamType)XN_LINK_STREAM_TYPE_IR, aEnumerated);
	for (int c = 0; c < (int)aEnumerated.GetSize(); ++c)
	{
		XnStatus nRetVal = readSupportedModesFromStream(aEnumerated[c], aSupportedModes);
		XN_IS_STATUS_OK(nRetVal);

		++s;
		m_sensors[s].sensorType             = ONI_SENSOR_IR;
		m_sensors[s].pSupportedVideoModes   = XN_NEW_ARR(OniVideoMode, aSupportedModes.GetSize());
		XN_VALIDATE_ALLOC_PTR(m_sensors[s].pSupportedVideoModes);
		nSupportedModes = aSupportedModes.GetSize();
	
		writeIndex = 0;
		for(int i=0; i < nSupportedModes; ++i)
		{
			m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
			m_sensors[s].pSupportedVideoModes[writeIndex].fps         = aSupportedModes[i].m_nFPS;
			m_sensors[s].pSupportedVideoModes[writeIndex].resolutionX = aSupportedModes[i].m_nXRes;
			m_sensors[s].pSupportedVideoModes[writeIndex].resolutionY = aSupportedModes[i].m_nYRes;
			
			bool foundMatch = false;
			for (int j = 0; j < writeIndex; ++j)
			{
				if (m_sensors[s].pSupportedVideoModes[writeIndex].pixelFormat == m_sensors[s].pSupportedVideoModes[j].pixelFormat &&
					m_sensors[s].pSupportedVideoModes[writeIndex].fps         == m_sensors[s].pSupportedVideoModes[j].fps         &&
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
		m_sensors[s].numSupportedVideoModes = writeIndex;
		m_numSensors = s+1;
	}
	aEnumerated.Clear();

/*	// Color

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
			LinkOniColorStream::GetAllowedOniOutputFormatForInputFormat((XnIOImageFormats)pSupportedModes[j].nFormat, aOniFormats, &nOniFormats);
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
*/
	return XN_STATUS_OK;
}

XnStatus LinkOniDevice::Init()
{
	xn::PrimeClient *pPrimeClient = new xn::PS1200Device();
	XN_VALIDATE_ALLOC_PTR(pPrimeClient);

	XnStatus retVal = pPrimeClient->Init(m_info.uri, XN_TRANSPORT_TYPE_USB);
	if (retVal != XN_STATUS_OK)
    {
        xnLogError(XN_MASK_LINK_DEVICE, "Failed to initialize prime client: %s", xnGetStatusString(retVal));
        XN_ASSERT(FALSE);
        XN_DELETE(pPrimeClient);
        return retVal;
    }

    retVal = pPrimeClient->Connect();
    if (retVal != XN_STATUS_OK)
    {
        xnLogError(XN_MASK_LINK_DEVICE, "Failed to connect to device: %s", xnGetStatusString(retVal));
        XN_ASSERT(FALSE);
        XN_DELETE(pPrimeClient);
        return retVal;
    }
    
    if (!m_didReset)
    {
        retVal = pPrimeClient->SoftReset();
        if (retVal != XN_STATUS_OK)
        {
            xnLogError(XN_MASK_LINK_DEVICE, "Failed to reset device: %s", xnGetStatusString(retVal));
            XN_ASSERT(FALSE);
            XN_DELETE(pPrimeClient);
            return retVal;
        }

        m_didReset = TRUE;
    }
/* TODO can we actually create more than one?
	retVal = m_createdDevices.Set(strCreationInfo, pPrimeClient);
	if (retVal != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_EXPORTED_PRIME_CLIENT, "Can't add device to created list: %s", xnGetStatusString(retVal));
		XN_ASSERT(FALSE);
		XN_DELETE(pPrimeClient);
		return retVal;
	}
	*/
	m_pSensor = pPrimeClient;

	return FillSupportedVideoModes();
}

void LinkOniDevice::Destroy()
{
	if (m_pSensor == NULL)
	{
		return;
	}

	// TODO can we actually create more than one?
	//m_createdDevices.Remove(pPrimeClient->GetConnectionString());

	m_pSensor->Disconnect();
	m_pSensor->Shutdown();
	XN_DELETE(m_pSensor);
	m_pSensor = NULL;
}

OniStatus LinkOniDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
	*numSensors = m_numSensors;
	*pSensors   = m_sensors;

	return ONI_STATUS_OK;
}

oni::driver::StreamBase* LinkOniDevice::createStream(OniSensorType sensorType)
{
	LinkOniStream* pStream;

	if (sensorType == ONI_SENSOR_DEPTH)
	{
		pStream = XN_NEW(LinkOniDepthStream, m_pSensor, this);
	}
	else if (sensorType == ONI_SENSOR_IR)
	{
		pStream = XN_NEW(LinkOniIRStream, m_pSensor, this);
	}
	//else if (sensorType == ONI_SENSOR_COLOR)
	//{
	//	pStream = XN_NEW(LinkOniColorStream, &m_sensor, this);
	//}
	else
	{
		m_driverServices.errorLoggerAppend("LinkOniDevice: Can't create a stream of type %d", sensorType);
		return NULL;
	}

	XnStatus nRetVal = pStream->Init();
	if (nRetVal != XN_STATUS_OK)
	{
		m_driverServices.errorLoggerAppend("LinkOniDevice: Can't initialize stream of type %d: %s", sensorType, xnGetStatusString(nRetVal));
		XN_DELETE(pStream);
		return NULL;
	}

	return pStream;
}

void LinkOniDevice::destroyStream(oni::driver::StreamBase* pStream)
{
	XN_DELETE(pStream);
}

OniStatus LinkOniDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	switch (propertyId)
	{
	case ONI_DEVICE_PROPERTY_FIRMWARE_VERSION:
		{
			EXACT_PROP_SIZE_DO(*pDataSize, OniVersion)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}
			OniVersion* version = (OniVersion*)data;
			XnDetailedVersion xnVersion = m_pSensor->GetFWVersion();
			
			version->major =       xnVersion.m_nMajor;
			version->minor =       xnVersion.m_nMinor;
			version->maintenance = xnVersion.m_nMaintenance;
			version->build =       xnVersion.m_nBuild;
			break;
		}

	case ONI_DEVICE_PROPERTY_HARDWARE_VERSION:
		ENSURE_PROP_SIZE_DO(*pDataSize, short)
		{
			m_driverServices.errorLoggerAppend("Unexpected size: %d != %d or %d or %d\n", *pDataSize, sizeof(short), sizeof(int), sizeof(uint64_t));
			XN_ASSERT(FALSE);
			return ONI_STATUS_BAD_PARAMETER;
		}
		ASSIGN_PROP_VALUE_INT(data, *pDataSize, m_pSensor->GetHWVersion());			
		break;
		
	case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:
		{
			const XnChar *serialNumber = m_pSensor->GetSerialNumber();
			
			if (xnOSStrCopy((XnChar*)data, serialNumber, *pDataSize) != XN_STATUS_OK)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, xnOSStrLen(serialNumber));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}
			break;
		}
	case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
		{
			EXACT_PROP_SIZE_DO(*pDataSize, OniVersion)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}
			OniVersion* version = (OniVersion*)data;
			version->major		 = XN_PS_MAJOR_VERSION;
			version->minor		 = XN_PS_MINOR_VERSION;
			version->maintenance = XN_PS_MAINTENANCE_VERSION;
			version->build		 = XN_PS_BUILD_VERSION;
			break;
		}
/*	case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
		{
			m_pSensor
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
		break;*/

	// Internal Link Properties
	case LINK_PROP_FW_VERSION:
		EXACT_PROP_SIZE_DO(*pDataSize, XnDetailedVersion)
		{
			m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(XnDetailedVersion));
			XN_ASSERT(FALSE);
			return ONI_STATUS_BAD_PARAMETER;
		}
		*(XnDetailedVersion*)data = m_pSensor->GetFWVersion();
		break;

	case LINK_PROP_VERSIONS_INFO_COUNT:
		{
			ENSURE_PROP_SIZE(*pDataSize, int);
			xnl::Array<XnComponentVersion> versions;
			nRetVal = m_pSensor->GetComponentsVersions(versions);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);

			ASSIGN_PROP_VALUE_INT(data, *pDataSize, versions.GetSize());
			break;
		}

		//case :
		//TODO: Implement Get emitter active
		//break;

	case LINK_PROP_VERSIONS_INFO:
		{
			xnl::Array<XnComponentVersion> components;
			nRetVal = m_pSensor->GetComponentsVersions(components);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);

			XnUInt32 nExpectedSize = components.GetSize() * sizeof(XnComponentVersion);
			if (*pDataSize != (int)nExpectedSize)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, nExpectedSize);
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			xnOSMemCopy(data, components.GetData(), nExpectedSize);
			break;
		}

	// PS1200Device only
	case PS_PROPERTY_USB_INTERFACE:
		{
			ENSURE_PROP_SIZE(*pDataSize, XnUInt8);
			ASSIGN_PROP_VALUE_INT(data, *pDataSize, 0);
			XnUInt8 nInterface = 0;
			nRetVal = ((xn::PS1200Device*)m_pSensor)->GetUsbAltInterface(nInterface);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			if (nInterface == 0)
			{
				*(XnUInt8*)data = PS_USB_INTERFACE_ISO_ENDPOINTS;
			}
			else if (nInterface == 1)
			{
				*(XnUInt8*)data = PS_USB_INTERFACE_BULK_ENDPOINTS;
			}
			else
			{
				XN_ASSERT(FALSE);
				return ONI_STATUS_ERROR;
			}
		}
		break;

	default:
		return ONI_STATUS_BAD_PARAMETER;
	}

	return ONI_STATUS_OK;
}

OniStatus LinkOniDevice::setProperty(int propertyId, const void* data, int dataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	switch (propertyId)
	{
/*	case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
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
		break;*/


	// Internal Link Properties
	// int props
	case LINK_PROP_EMITTER_ACTIVE:
		nRetVal = m_pSensor->SetEmitterActive(*(XnBool*)data);
		XN_IS_STATUS_OK_LOG_ERROR_RET("Set emitter active", nRetVal, ONI_STATUS_ERROR);
		break;

	case LINK_PROP_FW_LOG:
		if (*(int*)data != 0)
		{
			nRetVal = m_pSensor->StartFWLog();
			XN_IS_STATUS_OK_LOG_ERROR_RET("Start FW Log", nRetVal, ONI_STATUS_ERROR);
		}
		else
		{
			nRetVal = m_pSensor->StopFWLog();
			XN_IS_STATUS_OK_LOG_ERROR_RET("Stop FW Log", nRetVal, ONI_STATUS_ERROR);
		}
		break;

		// general props
	case LINK_PROP_FORMAT_ZONE:
		{
			EXACT_PROP_SIZE_DO(dataSize, XnPropFormatZone)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(XnPropFormatZone));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			const XnPropFormatZone* pPropFormatZone = reinterpret_cast<const XnPropFormatZone*>(data);
			nRetVal = m_pSensor->FormatZone(pPropFormatZone->m_nZone);
			XN_IS_STATUS_OK_LOG_ERROR_RET("Format Zone", nRetVal, ONI_STATUS_ERROR);
			break;
		}

	case LINK_PROP_UPLOAD_FILE:
		{
			EXACT_PROP_SIZE_DO(dataSize, XnPropUploadFile)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(XnPropUploadFile));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			const XnPropUploadFile* pPropUploadFile = reinterpret_cast<const XnPropUploadFile*>(data);
			nRetVal = m_pSensor->UploadFileOnControlEP(static_cast<XnChar*>(pPropUploadFile->m_nFileName),  
				static_cast<XnBool>(pPropUploadFile->m_nbOverrideFactorySettings));
			XN_IS_STATUS_OK_LOG_ERROR_RET("Upload File", nRetVal, ONI_STATUS_ERROR);
			break;
		}

	case LINK_PROP_DOWNLOAD_FILE:
		{
			EXACT_PROP_SIZE_DO(dataSize, XnPropUploadFile)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(XnPropUploadFile));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			const XnPropDownloadFile* pPropDownloadFile = reinterpret_cast<const XnPropDownloadFile*>(data);
			nRetVal = m_pSensor->DownloadFile(static_cast<XnUInt>(pPropDownloadFile->m_nZone),  
				static_cast<XnChar*>(pPropDownloadFile->m_nStrFirmwareFileName),
				static_cast<XnChar*>(pPropDownloadFile->m_nStrTargetFile));
			XN_IS_STATUS_OK_LOG_ERROR_RET("Download File", nRetVal, ONI_STATUS_ERROR);
			break;
		}

		// string props
	case LINK_PROP_PRESET_FILE:
		nRetVal = m_pSensor->RunPresetFile((XnChar *)data);
		XN_IS_STATUS_OK_LOG_ERROR_RET("RunPresetFile", nRetVal, ONI_STATUS_ERROR);
		break;
	
	// PS1200Device only
	case PS_PROPERTY_USB_INTERFACE:
		{
			ENSURE_PROP_SIZE(dataSize, XnUInt8);
			XnUsbInterfaceType type = (XnUsbInterfaceType)*(XnUInt8*)data;
			if (type == PS_USB_INTERFACE_ISO_ENDPOINTS)
			{
				nRetVal = ((xn::PS1200Device*)m_pSensor)->SetUsbAltInterface(0);
				XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			}
			else if (type == PS_USB_INTERFACE_BULK_ENDPOINTS)
			{
				nRetVal = ((xn::PS1200Device*)m_pSensor)->SetUsbAltInterface(1);
				XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			}
			else if (type != PS_USB_INTERFACE_DONT_CARE)
			{
				return ONI_STATUS_BAD_PARAMETER;
			}
		}
		break;

	default:
			return ONI_STATUS_BAD_PARAMETER;
	}

	return ONI_STATUS_OK;
}

OniBool LinkOniDevice::isPropertySupported(int propertyId)
{
	switch (propertyId)
	{
	case ONI_DEVICE_PROPERTY_FIRMWARE_VERSION:
	case ONI_DEVICE_PROPERTY_HARDWARE_VERSION:
	case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:
	case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
	//case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:

	// Internal Link Properties
	case LINK_PROP_FW_VERSION:
	case LINK_PROP_VERSIONS_INFO_COUNT:
	case LINK_PROP_VERSIONS_INFO:
	case LINK_PROP_EMITTER_ACTIVE:
	case LINK_PROP_FW_LOG:
	case LINK_PROP_FORMAT_ZONE:
	case LINK_PROP_UPLOAD_FILE:
	case LINK_PROP_DOWNLOAD_FILE:
	case LINK_PROP_PRESET_FILE:
	// PS1200Device only
	case PS_PROPERTY_USB_INTERFACE:
		return true;
	default:
		return false;
	}
}

void LinkOniDevice::notifyAllProperties()
{
	XnDetailedVersion version;
	int size = sizeof(version);
	getProperty(LINK_PROP_FW_VERSION, &version, &size);
	raisePropertyChanged(LINK_PROP_FW_VERSION, &version, size);
	
	XnUInt8 altusb;
	size = sizeof(altusb);
	getProperty(PS_PROPERTY_USB_INTERFACE, &altusb, &size);
	raisePropertyChanged(PS_PROPERTY_USB_INTERFACE, &altusb, size);
}

OniStatus LinkOniDevice::invoke(int commandId, void* data, int dataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	switch (commandId)
	{
	case PS_COMMAND_AHB_READ:
		{
			EXACT_PROP_SIZE_DO(dataSize, XnCommandAHB)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(XnCommandAHB));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			XnCommandAHB* pPropReadAHB = reinterpret_cast<XnCommandAHB*>(data);
			nRetVal = m_pSensor->ReadAHB(pPropReadAHB->address, 
				static_cast<XnUInt8>(pPropReadAHB->offsetInBits), 
				static_cast<XnUInt8>(pPropReadAHB->widthInBits), 
				pPropReadAHB->value);
			XN_IS_STATUS_OK_LOG_ERROR_RET("Read AHB", nRetVal, ONI_STATUS_ERROR);
			break;
		}

	case PS_COMMAND_AHB_WRITE:
		{
			EXACT_PROP_SIZE_DO(dataSize, XnCommandAHB)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(XnCommandAHB));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			const XnCommandAHB* pPropWriteAHB = reinterpret_cast<const XnCommandAHB*>(data);
			nRetVal = m_pSensor->WriteAHB(pPropWriteAHB->address, 
				pPropWriteAHB->value, 
				static_cast<XnUInt8>(pPropWriteAHB->offsetInBits), 
				static_cast<XnUInt8>(pPropWriteAHB->widthInBits));
			XN_IS_STATUS_OK_LOG_ERROR_RET("Write AHB", nRetVal, ONI_STATUS_ERROR);
			break;
		}

	case PS_COMMAND_I2C_READ:
		{
			EXACT_PROP_SIZE_DO(dataSize, XnCommandI2C)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(XnCommandI2C));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			XnCommandI2C* pPropReadI2C = reinterpret_cast<XnCommandI2C*>(data);
			nRetVal = m_pSensor->ReadI2C(
				static_cast<XnUInt8>(pPropReadI2C->deviceID),  
				static_cast<XnUInt8>(pPropReadI2C->addressSize), 
				pPropReadI2C->address, 
				static_cast<XnUInt8>(pPropReadI2C->valueSize),
				pPropReadI2C->value);
			XN_IS_STATUS_OK_LOG_ERROR_RET("Read I2C", nRetVal, ONI_STATUS_ERROR);
			break;
		}

	case PS_COMMAND_I2C_WRITE:
		{
			EXACT_PROP_SIZE_DO(dataSize, XnCommandI2C)
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", dataSize, sizeof(XnCommandI2C));
				XN_ASSERT(FALSE);
				return ONI_STATUS_BAD_PARAMETER;
			}

			const XnCommandI2C* pPropWriteI2C = reinterpret_cast<const XnCommandI2C*>(data);
			nRetVal = m_pSensor->WriteI2C(static_cast<XnUInt8>(pPropWriteI2C->deviceID), 
				static_cast<XnUInt8>(pPropWriteI2C->addressSize), 
				pPropWriteI2C->address, 
				static_cast<XnUInt8>(pPropWriteI2C->valueSize),
				pPropWriteI2C->value,
				pPropWriteI2C->mask);
			XN_IS_STATUS_OK_LOG_ERROR_RET("Write I2C", nRetVal, ONI_STATUS_ERROR);
			break;
		}

	case PS_COMMAND_SOFT_RESET:
		nRetVal = m_pSensor->SoftReset();
		XN_IS_STATUS_OK_LOG_ERROR_RET("Soft reset", nRetVal, ONI_STATUS_ERROR);
		break;

	case PS_COMMAND_POWER_RESET:
		nRetVal = m_pSensor->HardReset();
		XN_IS_STATUS_OK_LOG_ERROR_RET("Power reset", nRetVal, ONI_STATUS_ERROR);
		break;

	default:
		return DeviceBase::invoke(commandId, data, dataSize);
	}

	return ONI_STATUS_OK;
}

OniBool LinkOniDevice::isCommandSupported(int commandId)
{
	switch (commandId)
	{
	case PS_COMMAND_AHB_READ:
	case PS_COMMAND_AHB_WRITE:
	case PS_COMMAND_I2C_READ:
	case PS_COMMAND_I2C_WRITE:
	case PS_COMMAND_SOFT_RESET:
	case PS_COMMAND_POWER_RESET:
		return true;
	default:
		return DeviceBase::isCommandSupported(commandId);
	}
}

/*OniStatus LinkOniDevice::EnableFrameSync(LinkOniStream** pStreams, int streamCount)
{

	// Translate the LinkOniStream to XnDeviceStream.
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


void LinkOniDevice::DisableFrameSync()
{
	XnStatus rc = m_sensor.SetFrameSyncStreamGroup(NULL, 0);
	if (rc != XN_STATUS_OK)
	{
		m_driverServices.errorLoggerAppend("Error setting frame-sync group (rc=%d)\n", rc);
	}
}

OniBool LinkOniDevice::isImageRegistrationModeSupported(OniImageRegistrationMode mode)
{
	return (mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR || mode == ONI_IMAGE_REGISTRATION_OFF);
}
*/
