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
#include "XnSensorFirmware.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnSensorFirmware::XnSensorFirmware(XnDevicePrivateData* pDevicePrivateData) :
	m_pInfo(&pDevicePrivateData->FWInfo),
	m_Commands(pDevicePrivateData),
	m_Params(m_pInfo, &m_Commands),
	m_Streams(pDevicePrivateData),
	m_FixedParams(pDevicePrivateData),
	m_pDevicePrivateData(pDevicePrivateData)
{
}

XnStatus XnSensorFirmware::Init(XnBool bReset, XnBool bLeanInit)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// check current mode
	XnUInt16 nMode;
	nRetVal = XnHostProtocolGetMode(m_pDevicePrivateData, nMode);
	XN_IS_STATUS_OK(nRetVal);

	if (bReset)
	{
		// check if safe mode
		if (nMode == XN_HOST_PROTOCOL_MODE_SAFE_MODE)
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_SAFE_MODE, XN_MASK_DEVICE_SENSOR, "Device is in safe mode. Cannot start any stream!");
		}

		// check if device is alive
		XnUInt32 nCounter = 5;
		while (nCounter)
		{
			nRetVal = XnHostProtocolKeepAlive(m_pDevicePrivateData);
			if (nRetVal != XN_STATUS_OK)
			{
				nCounter--;
			}
			else
				nCounter = 0;
		}
		if (nRetVal != XN_STATUS_OK)
		{
			printf("Keep alive failed!\n");
			return nRetVal;
		}

		// perform a soft reset (to start clean)
		nRetVal = XnHostProtocolReset(m_pDevicePrivateData, XN_RESET_TYPE_SOFT_FIRST);
		if (nRetVal != XN_STATUS_OK)
		{
			printf("Couldn't reset the device!\n");
			return nRetVal;
		}

		// wait for sensor to recover from reset
		xnOSSleep(m_pDevicePrivateData->FWInfo.nUSBDelaySoftReset);

		// send keep alive again to see sensor is up
		nCounter = 10;
		while (nCounter)
		{
			nRetVal = XnHostProtocolKeepAlive(m_pDevicePrivateData);
			if (nRetVal != XN_STATUS_OK)
			{
				nCounter--;
				xnOSSleep(10);
			}
			else
				break;
		}
		if (nCounter == 0)
		{
			printf("10 keep alives is too much - stopping\n");
			return nRetVal;
		}

		nRetVal = XnHostProtocolGetMode(m_pDevicePrivateData, nMode);
		XN_IS_STATUS_OK(nRetVal);

		// check if safe mode
		if (nMode == XN_HOST_PROTOCOL_MODE_SAFE_MODE)
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_SAFE_MODE, XN_MASK_DEVICE_SENSOR, "Device is in safe mode. Cannot start any stream!");
		}
	}

	if (!bLeanInit)
	{
		nRetVal = m_FixedParams.Init();
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = m_Params.Init();
		XN_IS_STATUS_OK(nRetVal);

		if (nMode == XN_HOST_PROTOCOL_MODE_PS)
		{
			nRetVal = m_Params.UpdateAllProperties();
			XN_IS_STATUS_OK(nRetVal);
		}

		// Check if image is supported
		if (m_pInfo->bGetImageCmosTypeSupported)
		{
			m_pInfo->bImageSupported = (m_FixedParams.GetImageCmosType() != 0);
		}
		else
		{
			// This is an ugly patch to find out if this sensor has an image CMOS. It will be fixed
			// in future firmwares so we can just ask.
			XnUInt16 nLines;
			nRetVal = XnHostProtocolGetCmosBlanking(m_pDevicePrivateData, XN_CMOS_TYPE_IMAGE, &nLines);
			m_pInfo->bImageSupported = (nRetVal == XN_STATUS_OK && nLines > 0);
		}

		nRetVal = m_Streams.Init();
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

void XnSensorFirmware::Free()
{
	m_Params.Free();
}