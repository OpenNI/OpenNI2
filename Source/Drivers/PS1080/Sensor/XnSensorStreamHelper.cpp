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
#include "XnSensorStreamHelper.h"
#include "XnStreamProcessor.h"
#include <XnLog.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnSensorStreamHelper::XnSensorStreamHelper(XnSensorObjects* pObjects) : 
	m_pSensorStream(NULL),
	m_pStream(NULL),
	m_pObjects(pObjects)
{
}

XnSensorStreamHelper::~XnSensorStreamHelper()
{
	Free();
}

XnStatus XnSensorStreamHelper::Init(IXnSensorStream* pSensorStream, XnDeviceStream* pStream)
{
	XnStatus nRetVal = XN_STATUS_OK;

	m_pSensorStream = pSensorStream;
	m_pStream = pStream;
	
	nRetVal = m_pSensorStream->MapPropertiesToFirmware();
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::Free()
{
	if (m_pStream != NULL)
	{
		GetFirmware()->GetStreams()->ReleaseStream(m_pStream->GetType(), m_pStream);
	}

	m_FirmwareProperties.Clear();

	return XN_STATUS_OK;
}

XnStatus XnSensorStreamHelper::Configure()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnResolutions nRes;
	XnUInt32 nFPS;
	m_pSensorStream->GetFirmwareStreamConfig(&nRes, &nFPS);

	// claim the stream
	nRetVal = GetFirmware()->GetStreams()->ClaimStream(m_pStream->GetType(), nRes, nFPS, m_pStream);
	XN_IS_STATUS_OK(nRetVal);

	// configure the stream
	nRetVal = m_pSensorStream->ConfigureStreamImpl();
	if (nRetVal != XN_STATUS_OK)
	{
		GetFirmware()->GetStreams()->ReleaseStream(m_pStream->GetType(), m_pStream);
		return (nRetVal);
	}

	// create data processor
	XnDataProcessor* pProcessor;
	nRetVal = m_pSensorStream->CreateDataProcessor(&pProcessor);
	if (nRetVal != XN_STATUS_OK)
	{
		GetFirmware()->GetStreams()->ReleaseStream(m_pStream->GetType(), m_pStream);
		return (nRetVal);
	}

	// and register it
	nRetVal = GetFirmware()->GetStreams()->ReplaceStreamProcessor(m_pStream->GetType(), m_pStream, pProcessor);
	if (nRetVal != XN_STATUS_OK)
	{
		GetFirmware()->GetStreams()->ReleaseStream(m_pStream->GetType(), m_pStream);
		return (nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::FinalOpen()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_pSensorStream->OpenStreamImpl();
	if (nRetVal != XN_STATUS_OK)
	{
		GetFirmware()->GetStreams()->ReleaseStream(m_pStream->GetType(), m_pStream);
		return (nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::Open()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// configure the stream
	nRetVal = Configure();
	XN_IS_STATUS_OK(nRetVal);

	// Update frequency (it might change on specific stream configuration)
	XnFrequencyInformation FrequencyInformation;
	nRetVal = XnHostProtocolAlgorithmParams(m_pObjects->pDevicePrivateData, XN_HOST_PROTOCOL_ALGORITHM_FREQUENCY, &FrequencyInformation, sizeof(XnFrequencyInformation), (XnResolutions)0, 0);
	XN_IS_STATUS_OK(nRetVal);

	m_pObjects->pDevicePrivateData->fDeviceFrequency = XN_PREPARE_VAR_FLOAT_IN_BUFFER(FrequencyInformation.fDeviceFrequency);

	// and now turn it on
	nRetVal = FinalOpen();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::Close()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (GetFirmware()->GetStreams()->IsClaimed(m_pStream->GetType(), m_pStream))
	{
		nRetVal = m_pSensorStream->CloseStreamImpl();
		XN_IS_STATUS_OK(nRetVal);

		GetFirmware()->GetStreams()->ReleaseStream(m_pStream->GetType(), m_pStream);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::RegisterDataProcessorProperty(XnActualIntProperty& Property)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// mark it so
	XnSensorStreamHelperCookie* pCookie;
	nRetVal = m_FirmwareProperties.Get(&Property, pCookie);
	XN_IS_STATUS_OK(nRetVal);

	pCookie->bProcessorProp = TRUE;

	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::MapFirmwareProperty(XnActualIntProperty& Property, XnActualIntProperty& FirmwareProperty, XnBool bAllowChangeWhileOpen, XnSensorStreamHelper::ConvertCallback pStreamToFirmwareFunc /* = 0 */)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// init data
	XnSensorStreamHelperCookie cookie(&Property, &FirmwareProperty, bAllowChangeWhileOpen, pStreamToFirmwareFunc);

	// add it to the list
	nRetVal = m_FirmwareProperties.Set(&Property, cookie);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::ConfigureFirmware(XnActualIntProperty& Property)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnSensorStreamHelperCookie* pPropData = NULL;
	nRetVal = m_FirmwareProperties.Get(&Property, pPropData);
	XN_IS_STATUS_OK(nRetVal);

	XnUInt64 nFirmwareValue = Property.GetValue();

	if (pPropData->pStreamToFirmwareFunc != NULL)
	{
		nRetVal = pPropData->pStreamToFirmwareFunc(Property.GetValue(), &nFirmwareValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	nRetVal = pPropData->pFirmwareProp->SetValue(nFirmwareValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::BeforeSettingFirmwareParam(XnActualIntProperty& Property, XnUInt16 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnSensorStreamHelperCookie* pPropData = NULL;
	nRetVal = m_FirmwareProperties.Get(&Property, pPropData);
	XN_IS_STATUS_OK(nRetVal);

	pPropData->CurrentTransaction.bShouldOpen = FALSE;
	pPropData->CurrentTransaction.bChooseProcessor = FALSE;

	// if stream is closed, we can just update the prop.
	if (m_pStream->IsOpen())
	{
		// check if we need to close the stream first
		if (pPropData->bAllowWhileOpen)
		{
			// before actual changing it, check if this is a processor property
			if (pPropData->bProcessorProp)
			{
				// lock processor
				nRetVal = GetFirmware()->GetStreams()->LockStreamProcessor(m_pStream->GetType(), m_pStream);
				XN_IS_STATUS_OK(nRetVal);
				pPropData->CurrentTransaction.bChooseProcessor = TRUE;
			}

			// OK. change the value
			XnUInt64 nFirmwareValue = nValue;

			if (pPropData->pStreamToFirmwareFunc != NULL)
			{
				nRetVal = pPropData->pStreamToFirmwareFunc(nValue, &nFirmwareValue);
				XN_IS_STATUS_OK(nRetVal);
			}

			// set the param in firmware
			nRetVal = pPropData->pFirmwareProp->SetValue(nFirmwareValue);
			XN_IS_STATUS_OK(nRetVal);

			// no need to do anything after property will be set
			pPropData->CurrentTransaction.bShouldOpen = FALSE;
		}
		else
		{
			// we can't change the firmware param. We should first close the stream
			nRetVal = m_pStream->Close();
			XN_IS_STATUS_OK(nRetVal);

			// after property will be set, we need to reopen the stream
			pPropData->CurrentTransaction.bShouldOpen = TRUE;
		}
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::AfterSettingFirmwareParam(XnActualIntProperty& Property)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnSensorStreamHelperCookie* pPropData = NULL;
	nRetVal = m_FirmwareProperties.Get(&Property, pPropData);
	XN_IS_STATUS_OK(nRetVal);
	
	if (pPropData->CurrentTransaction.bShouldOpen)
	{
		nRetVal = m_pStream->Open();
		XN_IS_STATUS_OK(nRetVal);
	}
	else if (pPropData->CurrentTransaction.bChooseProcessor)
	{
		XnDataProcessor* pProcessor = NULL;
		nRetVal = m_pSensorStream->CreateDataProcessor(&pProcessor);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = GetFirmware()->GetStreams()->ReplaceStreamProcessor(m_pStream->GetType(), m_pStream, pProcessor);
		XN_IS_STATUS_OK(nRetVal);

		// and unlock
		nRetVal = GetFirmware()->GetStreams()->UnlockStreamProcessor(m_pStream->GetType(), m_pStream);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::SimpleSetFirmwareParam(XnActualIntProperty& Property, XnUInt16 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = BeforeSettingFirmwareParam(Property, nValue);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = Property.UnsafeUpdateValue(nValue);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = AfterSettingFirmwareParam(Property);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::BeforeSettingDataProcessorProperty()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (m_pStream->IsOpen())
	{
		nRetVal = GetFirmware()->GetStreams()->LockStreamProcessor(m_pStream->GetType(), m_pStream);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::AfterSettingDataProcessorProperty()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_pStream->IsOpen())
	{
		XnDataProcessor* pProcessor = NULL;
		nRetVal = m_pSensorStream->CreateDataProcessor(&pProcessor);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = GetFirmware()->GetStreams()->ReplaceStreamProcessor(m_pStream->GetType(), m_pStream, pProcessor);
		XN_IS_STATUS_OK(nRetVal);

		// and unlock
		nRetVal = GetFirmware()->GetStreams()->UnlockStreamProcessor(m_pStream->GetType(), m_pStream);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::UpdateFromFirmware(XnActualIntProperty& Property)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnSensorStreamHelperCookie* pPropData = NULL;
	nRetVal = m_FirmwareProperties.Get(&Property, pPropData);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pPropData->pStreamProp->UnsafeUpdateValue(pPropData->pFirmwareProp->GetValue());
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorStreamHelper::BatchConfig(const XnActualPropertiesHash& props)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnBool bShouldClose = FALSE;

	if (m_pStream->IsOpen())
	{
		// check if one of the properties requires to close the stream
		for (FirmareProperties::Iterator it = m_FirmwareProperties.Begin(); it != m_FirmwareProperties.End(); ++it)
		{
			if (!it->Value().bAllowWhileOpen)
			{
				XnProperty* pProp;
				if (XN_STATUS_OK == props.Get(it->Value().pStreamProp->GetId(), pProp))
				{
					bShouldClose = TRUE;
					break;
				}
			}
		}
	}

	if (bShouldClose)
	{
		xnLogVerbose(XN_MASK_DEVICE_SENSOR, "closing stream before batch config...");
		nRetVal = m_pStream->Close();
		XN_IS_STATUS_OK(nRetVal);
	}

	nRetVal = m_pStream->XnDeviceStream::BatchConfig(props);
	XN_IS_STATUS_OK(nRetVal);

	if (bShouldClose)
	{
		xnLogVerbose(XN_MASK_DEVICE_SENSOR, "re-opening stream after batch config...");
		nRetVal = m_pStream->Open();
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnFirmwareCroppingMode XnSensorStreamHelper::GetFirmwareCroppingMode(XnCroppingMode nValue, XnBool bEnabled)
{
	if (!bEnabled)
		return XN_FIRMWARE_CROPPING_MODE_DISABLED;

	switch (nValue)
	{
	case XN_CROPPING_MODE_NORMAL:
		return XN_FIRMWARE_CROPPING_MODE_NORMAL;
	case XN_CROPPING_MODE_INCREASED_FPS:
		return GetFirmware()->GetInfo()->bIncreasedFpsCropSupported ? XN_FIRMWARE_CROPPING_MODE_INCREASED_FPS : XN_FIRMWARE_CROPPING_MODE_NORMAL;
	case XN_CROPPING_MODE_SOFTWARE_ONLY:
		return XN_FIRMWARE_CROPPING_MODE_DISABLED;
	default:
		return XN_FIRMWARE_CROPPING_MODE_NORMAL;
	}
}
