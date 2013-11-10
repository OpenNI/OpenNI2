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
#ifndef XNSENSORFIRMWAREPARAMS_H
#define XNSENSORFIRMWAREPARAMS_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnActualIntProperty.h>
#include "XnParams.h"
#include <XnHash.h>
#include "XnFirmwareInfo.h"
#include "XnFirmwareCommands.h"
#include <XnList.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

/**
* Holds all firmware params
*/
class XnSensorFirmwareParams
{
public:
	XnSensorFirmwareParams(XnFirmwareInfo* pInfo, XnFirmwareCommands* pCommands);
	~XnSensorFirmwareParams();

	//---------------------------------------------------------------------------
	// Methods
	//---------------------------------------------------------------------------
	XnStatus Init();
	void Free();
	XnStatus UpdateAllProperties();
	XnStatus StartTransaction();
	XnStatus CommitTransaction();
	XnStatus CommitTransactionAsBatch();
	XnStatus RollbackTransaction();

	XnActualIntProperty m_FrameSyncEnabled;
	XnActualIntProperty m_RegistrationEnabled;
	XnActualIntProperty m_Stream0Mode;
	XnActualIntProperty m_Stream1Mode;
	XnActualIntProperty m_Stream2Mode;
	XnActualIntProperty m_AudioStereo;
	XnActualIntProperty m_AudioSampleRate;
	XnActualIntProperty m_AudioLeftChannelGain;
	XnActualIntProperty m_AudioRightChannelGain;
	XnActualIntProperty m_ImageFormat;
	XnActualIntProperty m_ImageResolution;
	XnActualIntProperty m_ImageFPS;
	XnActualIntProperty m_ImageQuality;
	XnActualIntProperty m_ImageFlickerDetection;
	XnActualIntProperty m_ImageCropSizeX;
	XnActualIntProperty m_ImageCropSizeY;
	XnActualIntProperty m_ImageCropOffsetX;
	XnActualIntProperty m_ImageCropOffsetY;
	XnActualIntProperty m_ImageCropMode;
	XnActualIntProperty m_DepthFormat;
	XnActualIntProperty m_DepthResolution;
	XnActualIntProperty m_DepthFPS;
	XnActualIntProperty m_DepthGain;
	XnActualIntProperty m_DepthHoleFilter;
	XnActualIntProperty m_DepthMirror;
	XnActualIntProperty m_DepthDecimation;
	XnActualIntProperty m_DepthCropSizeX;
	XnActualIntProperty m_DepthCropSizeY;
	XnActualIntProperty m_DepthCropOffsetX;
	XnActualIntProperty m_DepthCropOffsetY;
	XnActualIntProperty m_DepthCropMode;
	XnActualIntProperty m_DepthWhiteBalance;
	XnActualIntProperty m_IRFormat;
	XnActualIntProperty m_IRResolution;
	XnActualIntProperty m_IRFPS;
	XnActualIntProperty m_IRCropSizeX;
	XnActualIntProperty m_IRCropSizeY;
	XnActualIntProperty m_IRCropOffsetX;
	XnActualIntProperty m_IRCropOffsetY;
	XnActualIntProperty m_IRCropMode;
	XnActualIntProperty m_ImageMirror;
	XnActualIntProperty m_IRMirror;
	XnActualIntProperty m_ReferenceResolution;
	XnActualIntProperty m_GMCMode;
	XnActualIntProperty m_ImageSharpness;
	XnActualIntProperty m_ImageAutoWhiteBalance;
	XnActualIntProperty m_ImageColorTemperature;
	XnActualIntProperty m_ImageBacklightCompensation;
	XnActualIntProperty m_ImageAutoExposure;
	XnActualIntProperty m_ImageExposureBar;
	XnActualIntProperty m_ImageLowLightCompensation;
	XnActualIntProperty m_ImageGain;
	XnActualIntProperty m_DepthCloseRange;
	XnActualIntProperty m_FastZoomCrop;
	XnActualIntProperty m_LogFilter;
	XnActualIntProperty m_GMCDebug;
	XnActualIntProperty m_APCEnabled;
	XnActualIntProperty m_WavelengthCorrection;
	XnActualIntProperty m_WavelengthCorrectionDebug;

private:
	typedef struct XnFirmwareParam
	{
		XnActualIntProperty* pProperty;
		XnUInt16 nFirmwareParam;
		XnFWVer MinVer;
		XnFWVer MaxVer;
		XnUInt16 nValueIfNotSupported;
	} XnFirmwareParam;

	typedef xnl::Hash<XnActualIntProperty*, XnFirmwareParam> XnFirmwareParamsHash;
	typedef xnl::List<XnActualIntProperty*> XnActualIntPropertyList;
	typedef xnl::Hash<XnActualIntProperty*, XnUInt32> XnPropertyToValueHash;

	XnStatus AddFirmwareParam(XnActualIntProperty& Property, XnUInt16 nFirmwareParam, XnFWVer nMinVer = XN_SENSOR_FW_VER_UNKNOWN, XnFWVer nMaxVer = XN_SENSOR_FW_VER_UNKNOWN, XnUInt16 nValueIfNotSupported = 0);
	XnStatus AddFirmwareAudioParam(XnActualIntProperty& Property, XnUInt16 nFirmwareParam, XnFWVer nMinVer = XN_SENSOR_FW_VER_3_0, XnFWVer nMaxVer = XN_SENSOR_FW_VER_UNKNOWN, XnUInt16 nValueIfNotSupported = 0);

	XnStatus UpdateProperty(XnFirmwareParam* pParam);

	XnStatus SetFirmwareParam(XnActualIntProperty* pProperty, XnUInt64 nValue);
	XnStatus SetFirmwareAudioParam(XnActualIntProperty* pProperty, XnUInt64 nValue);
	XnStatus SetImageFormat(XnUInt64 nValue);
	XnStatus SetStreamMode(XnActualIntProperty* pProperty, XnUInt64 nValue);
	XnStatus RecalculateReferenceResolution();
	XnStatus GetFirmwareParam(XnActualIntProperty* pProperty, XnFirmwareParam** ppParam);

	XnStatus SetFirmwareParamImpl(XnActualIntProperty* pProperty, XnUInt64 nValue);
	XnStatus CheckFirmwareParam(XnActualIntProperty* pProperty, XnUInt64 nValue, XnFirmwareParam** ppParam);

	static XnStatus XN_CALLBACK_TYPE SetFirmwareParamCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareAudioParamCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetImageFormatCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetStreamModeCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ReferenceResolutionPropertyValueChanged(const XnProperty* pSender, void* pCookie);

	XnFirmwareParamsHash m_AllFirmwareParams;

	XnFirmwareInfo* m_pInfo;
	XnFirmwareCommands* m_pCommands;
	XnBool m_bInTransaction;
	XnActualIntPropertyList m_TransactionOrder; // the transaction according to the order in which it was set
	XnPropertyToValueHash m_Transaction; // maps a property to its new value
};

#endif // XNSENSORFIRMWAREPARAMS_H
