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
#ifndef XNSENSORSTREAMHELPER_H
#define XNSENSORSTREAMHELPER_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "IXnSensorStream.h"
#include "XnSensorFirmware.h"
#include "XnSensorFixedParams.h"
#include <DDK/XnDeviceStream.h>
#include <DDK/XnDeviceModuleHolder.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnSensorStreamHelper
{
public:
	XnSensorStreamHelper(XnSensorObjects* pObjects);
	~XnSensorStreamHelper();

	XnStatus Init(IXnSensorStream* pSensorStream, XnDeviceStream* pStream);
	XnStatus Free();

	XnStatus Configure();
	XnStatus FinalOpen();
	XnStatus Open();
	XnStatus Close();

	/**
	* Registers a property which affects the data processor. When any of these properties
	* changes, the data processor will be recreated.
	*/
	XnStatus RegisterDataProcessorProperty(XnActualIntProperty& Property);

	typedef XnStatus (*ConvertCallback)(XnUInt64 nSource, XnUInt64* pnDest);

	/** 
	* Maps a stream property to a firmware property. Later on, such a property can be used
	* in calls to ConfigureFirmware or SetStreamFirmwareParam.
	*/
	XnStatus MapFirmwareProperty(XnActualIntProperty& Property, XnActualIntProperty& FirmwareProperty, XnBool bAllowChangeWhileOpen, ConvertCallback pStreamToFirmwareFunc = 0);

	/**
	* Configures the firmware according to the property. This can only be done for properties
	* which were previously attached via the MapFirmwareProperty function.
	*/
	XnStatus ConfigureFirmware(XnActualIntProperty& Property);

	XnStatus BeforeSettingFirmwareParam(XnActualIntProperty& Property, XnUInt16 nValue);
	XnStatus AfterSettingFirmwareParam(XnActualIntProperty& Property);

	XnStatus BeforeSettingDataProcessorProperty();
	XnStatus AfterSettingDataProcessorProperty();

	XnStatus SimpleSetFirmwareParam(XnActualIntProperty& Property, XnUInt16 nValue);

	XnStatus UpdateFromFirmware(XnActualIntProperty& Property);

	inline XnSensorFirmware* GetFirmware() const { return m_pObjects->pFirmware; }
	inline XnFWVer GetFirmwareVersion() const { return GetFirmware()->GetInfo()->nFWVer; }
	inline XnSensorFixedParams* GetFixedParams() const { return m_pObjects->pFirmware->GetFixedParams(); }
	inline XnDevicePrivateData* GetPrivateData() const { return m_pObjects->pDevicePrivateData; }
	inline XnSensorFPS* GetFPS() const { return m_pObjects->pFPS; }
	inline XnCmosInfo* GetCmosInfo() const { return m_pObjects->pCmosInfo; }
	inline IXnSensorStream* GetSensorStream() { return m_pSensorStream; }

	inline XnStatus StartFirmwareTransaction() { return GetFirmware()->GetParams()->StartTransaction(); }
	inline XnStatus CommitFirmwareTransaction() { return GetFirmware()->GetParams()->CommitTransaction(); }
	inline XnStatus CommitFirmwareTransactionAsBatch() { return GetFirmware()->GetParams()->CommitTransactionAsBatch(); }
	inline XnStatus RollbackFirmwareTransaction() { return GetFirmware()->GetParams()->RollbackTransaction(); }

	XnStatus BatchConfig(const XnActualPropertiesHash& props);

	XnFirmwareCroppingMode GetFirmwareCroppingMode(XnCroppingMode nValue, XnBool bEnabled);

private:
	IXnSensorStream* m_pSensorStream;
	XnDeviceStream* m_pStream;
	XnSensorObjects* m_pObjects;

	class XnSensorStreamHelperCookie
	{
	public:
		XnSensorStreamHelperCookie() {}
		XnSensorStreamHelperCookie(XnActualIntProperty* pStreamProp, XnActualIntProperty* pFirmwareProp, XnBool bAllowWhileOpen, XnSensorStreamHelper::ConvertCallback pStreamToFirmwareFunc) :
			pStreamProp(pStreamProp), pFirmwareProp(pFirmwareProp), bAllowWhileOpen(bAllowWhileOpen), pStreamToFirmwareFunc(pStreamToFirmwareFunc), bProcessorProp(FALSE)
		{}

		XnActualIntProperty* pStreamProp;
		XnActualIntProperty* pFirmwareProp;
		XnBool bAllowWhileOpen;
		XnSensorStreamHelper::ConvertCallback pStreamToFirmwareFunc;
		XnBool bProcessorProp;

		struct
		{
			XnBool bShouldOpen;
			XnBool bChooseProcessor;
		} CurrentTransaction;
	};

	typedef xnl::Hash<XnActualIntProperty*, XnSensorStreamHelperCookie> FirmareProperties;
	FirmareProperties m_FirmwareProperties;
};

class XnSensorStreamHolder : public XnDeviceModuleHolder
{
public:
	XnSensorStreamHolder(XnDeviceStream* pStream, XnSensorStreamHelper* pHelper) : 
		XnDeviceModuleHolder(pStream), m_pHelper(pHelper) 
	{}

	inline XnDeviceStream* GetStream() { return (XnDeviceStream*)GetModule(); }

	XnStatus Configure() { return m_pHelper->Configure(); }
	XnStatus FinalOpen() { return m_pHelper->FinalOpen(); }

private:
	XnSensorStreamHelper* m_pHelper;
};

#endif // XNSENSORSTREAMHELPER_H
