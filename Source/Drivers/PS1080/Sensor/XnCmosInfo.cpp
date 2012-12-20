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
#include "XnCmosInfo.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnCmosInfo::XnCmosInfo(XnSensorFirmware* pFirmware, XnDevicePrivateData* pDevicePrivateData) :
	m_pFirmware(pFirmware),
	m_pDevicePrivateData(pDevicePrivateData)
{
	xnOSMemSet(m_pCurrCmosBlankingInfo, 0, sizeof(m_pCurrCmosBlankingInfo));
}

XnCmosInfo::~XnCmosInfo()
{
}

XnStatus XnCmosInfo::SetCmosConfig(XnCMOSType nCmos, XnResolutions nResolution, XnUInt32 nFPS)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_pFirmware->GetInfo()->nFWVer >= XN_SENSOR_FW_VER_5_1)
	{
		// take blanking info
		XnCmosBlankingInformation* pInfo = NULL;

		// search the list if we already have this info
		for (XnCmosBlankingDataList::Iterator it = m_CmosBlankingInfo.Begin(); it != m_CmosBlankingInfo.End(); ++it)
		{
			XnCmosBlankingData& data = *it;
			if (data.nRes == nResolution && data.nFPS == nFPS)
			{
				pInfo = &data.BlankingInfo;
				break;
			}
		}

		if (pInfo == NULL)
		{
			// not found in list. fetch it from FW
			XnCmosBlankingData data;
			data.nRes = nResolution;
			data.nFPS = nFPS;

			nRetVal = XnHostProtocolAlgorithmParams(m_pDevicePrivateData, XN_HOST_PROTOCOL_ALGORITHM_BLANKING, &data.BlankingInfo, sizeof(XnCmosBlankingInformation), nResolution, (XnUInt16)nFPS);
			XN_IS_STATUS_OK(nRetVal);

			// add to list
			nRetVal = m_CmosBlankingInfo.AddFirst(data);
			XN_IS_STATUS_OK(nRetVal);

			// take its info (take a pointer to the object in the list, and not to the one on the stack)
			pInfo = &m_CmosBlankingInfo.Begin()->BlankingInfo;
		}

		m_pCurrCmosBlankingInfo[nCmos] = &pInfo->Coefficients[nCmos];
	}

	return (XN_STATUS_OK);
}
