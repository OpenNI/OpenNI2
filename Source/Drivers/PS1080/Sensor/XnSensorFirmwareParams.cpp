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
#include "XnSensorFirmwareParams.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnSensorFirmwareParams::XnSensorFirmwareParams(XnFirmwareInfo* pInfo, XnFirmwareCommands* pCommands) :
	/* Member						   Name							Firmware Param								Min Valid Version     Max Valid Version     Value if wrong version */
	/* ====================		       ========================		=====================================		====================  ====================  ====================== */
	m_FrameSyncEnabled(0, "FrameSync"),
	m_RegistrationEnabled(0, "Registration"),
	m_Stream0Mode(0, "Stream0Mode"),
	m_Stream1Mode(0, "Stream1Mode"),
	m_Stream2Mode(0, "Stream2Mode"),
	m_AudioStereo(0, "AudioStereo"),
	m_AudioSampleRate(0, "AudioSampleRate"),
	m_AudioLeftChannelGain(0, "AudioLeftChannelGain"),
	m_AudioRightChannelGain(0, "AudioRightChannelGain"),
	m_ImageFormat(0, "ImageFormat"),
	m_ImageResolution(0, "ImageResolution"),
	m_ImageFPS(0, "ImageFPS"),
	m_ImageQuality(0, "ImageQuality"),
	m_ImageFlickerDetection(0, "ImageFlicker"),
	m_ImageCropSizeX(0, "ImageCropSizeX"),
	m_ImageCropSizeY(0, "ImageCropSizeY"),
	m_ImageCropOffsetX(0, "ImageCropOffsetX"),
	m_ImageCropOffsetY(0, "ImageCropOffsetY"),
	m_ImageCropMode(0, "ImageCropEnabled"),
	m_DepthFormat(0, "DepthFormat"),
	m_DepthResolution(0, "DepthResolution"),
	m_DepthFPS(0, "DepthFPS"),
	m_DepthGain(0, "DepthGain"),
	m_DepthHoleFilter(0, "DepthHoleFilter"),
	m_DepthMirror(0, "DepthMirror"),
	m_DepthDecimation(0, "DepthDecimation"),
	m_DepthCropSizeX(0, "DepthCropSizeX"),
	m_DepthCropSizeY(0, "DepthCropSizeY"),
	m_DepthCropOffsetX(0, "DepthCropOffsetX"),
	m_DepthCropOffsetY(0, "DepthCropOffsetY"),
	m_DepthCropMode(0, "DepthCropEnabled"),
	m_DepthWhiteBalance(0, "DepthWhiteBalance"),
	m_IRFormat(0, "IRFormat"),
	m_IRResolution(0, "IRResolution"),
	m_IRFPS(0, "IRFPS"),
	m_IRCropSizeX(0, "IRCropSizeX"),
	m_IRCropSizeY(0, "IRCropSizeY"),
	m_IRCropOffsetX(0, "IRCropOffsetX"),
	m_IRCropOffsetY(0, "IRCropOffsetY"),
	m_IRCropMode(0, "IRCropEnabled"),
	m_ImageMirror(0, "ImageMirror"),
	m_IRMirror(0, "IRMirror"),
	m_ReferenceResolution(0, "ReferenceResolution", 0, "Firmware"),
	m_GMCMode(0, "GMCMode"),
	m_ImageSharpness(0, "ImageSharpness"),
	m_ImageAutoWhiteBalance(0, "ImageAutoWhiteBalance"),
	m_ImageColorTemperature(0, "ImageColorTemperature"),
	m_ImageBacklightCompensation(0, "ImageBacklightCompensation"),
	m_ImageAutoExposure(0, "ImageAutoExposure"),
	m_ImageExposureBar(0, "ImageExposureBar"),
	m_ImageLowLightCompensation(0, "ImageLowLightCompensation"),
	m_ImageGain(0, "ImageGain"),
	m_DepthCloseRange(0, "CloseRange"),
	m_FastZoomCrop(0, "FastZoomCrop"),
	m_LogFilter(0, "LogFilter"),
	m_GMCDebug(0, "GMCDebug"),
	m_APCEnabled(0, "APCEnabled"),
	m_WavelengthCorrection(0, "WavelengthCorrection"),
	m_WavelengthCorrectionDebug(0, "WavelengthCorrectionDebug"),
	m_AllFirmwareParams(),
	m_pInfo(pInfo),
	m_pCommands(pCommands),
	m_bInTransaction(FALSE)
{
	m_ReferenceResolution.SetLogSeverity(XN_LOG_VERBOSE);
}

XnSensorFirmwareParams::~XnSensorFirmwareParams()
{
}

XnStatus XnSensorFirmwareParams::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	/*								Property					Param										MinVersion				MaxVersion					ValueIfNotSupported */
	/*								======================		=======================================		====================	====================		=================== */
	nRetVal = AddFirmwareParam(		m_FrameSyncEnabled,			PARAM_GENERAL_FRAME_SYNC);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_RegistrationEnabled,		PARAM_GENERAL_REGISTRATION_ENABLE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_Stream0Mode,				PARAM_GENERAL_STREAM0_MODE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_Stream1Mode,				PARAM_GENERAL_STREAM1_MODE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareAudioParam(m_Stream2Mode,				PARAM_GENERAL_STREAM2_MODE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareAudioParam(m_AudioStereo,				PARAM_AUDIO_STEREO_MODE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareAudioParam(m_AudioSampleRate,			PARAM_AUDIO_SAMPLE_RATE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareAudioParam(m_AudioLeftChannelGain,		PARAM_AUDIO_LEFT_CHANNEL_VOLUME_LEVEL);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareAudioParam(m_AudioRightChannelGain,	PARAM_AUDIO_RIGHT_CHANNEL_VOLUME_LEVEL);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageFormat,				PARAM_IMAGE_FORMAT);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageResolution,			PARAM_IMAGE_RESOLUTION);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageFPS,					PARAM_IMAGE_FPS);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageQuality,				PARAM_IMAGE_QUALITY);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageFlickerDetection,	PARAM_IMAGE_FLICKER_DETECTION);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageCropSizeX,			PARAM_IMAGE_CROP_SIZE_X,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageCropSizeY,			PARAM_IMAGE_CROP_SIZE_Y,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageCropOffsetX,			PARAM_IMAGE_CROP_OFFSET_X,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageCropOffsetY,			PARAM_IMAGE_CROP_OFFSET_Y,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageCropMode,			PARAM_IMAGE_CROP_MODE,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthFormat,				PARAM_DEPTH_FORMAT);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthResolution,			PARAM_DEPTH_RESOLUTION);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthFPS,					PARAM_DEPTH_FPS);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthGain,				PARAM_DEPTH_AGC);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthHoleFilter,			PARAM_DEPTH_HOLE_FILTER);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthMirror,				PARAM_DEPTH_MIRROR,							XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthDecimation,			PARAM_DEPTH_DECIMATION);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthCropSizeX,			PARAM_DEPTH_CROP_SIZE_X,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthCropSizeY,			PARAM_DEPTH_CROP_SIZE_Y,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthCropOffsetX,			PARAM_DEPTH_CROP_OFFSET_X,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthCropOffsetY,			PARAM_DEPTH_CROP_OFFSET_Y,					XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthCropMode,			PARAM_DEPTH_CROP_MODE,						XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRFormat,					PARAM_IR_FORMAT);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRResolution,				PARAM_IR_RESOLUTION);
	XN_IS_STATUS_OK(nRetVal);								
	nRetVal = AddFirmwareParam(		m_IRFPS,					PARAM_IR_FPS);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRCropSizeX,				PARAM_IR_CROP_SIZE_X,						XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRCropSizeY,				PARAM_IR_CROP_SIZE_Y,						XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRCropOffsetX,			PARAM_IR_CROP_OFFSET_X,						XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRCropOffsetY,			PARAM_IR_CROP_OFFSET_Y,						XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRCropMode,			PARAM_IR_CROP_MODE,						XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthWhiteBalance,		PARAM_DEPTH_WHITE_BALANCE_ENABLE,			XN_SENSOR_FW_VER_4_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageMirror,				PARAM_IMAGE_MIRROR,							XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_IRMirror,					PARAM_IR_MIRROR,							XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_GMCMode,					PARAM_DEPTH_GMC_MODE,						XN_SENSOR_FW_VER_3_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageSharpness,			PARAM_IMAGE_SHARPNESS,						XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	50);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageAutoWhiteBalance,	PARAM_IMAGE_AUTO_WHITE_BALANCE_MODE,		XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageColorTemperature,	PARAM_IMAGE_COLOR_TEMPERATURE,				XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageBacklightCompensation,PARAM_IMAGE_BACK_LIGHT_COMPENSATION,		XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageAutoExposure,		PARAM_IMAGE_AUTO_EXPOSURE_MODE,				XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageExposureBar,			PARAM_IMAGE_EXPOSURE_BAR,					XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageLowLightCompensation,PARAM_IMAGE_LOW_LIGHT_COMPENSATION_MODE,	XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_ImageGain,				PARAM_IMAGE_AGC,							XN_SENSOR_FW_VER_5_4,	XN_SENSOR_FW_VER_UNKNOWN,	0);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_DepthCloseRange,			PARAM_DEPTH_CLOSE_RANGE,					XN_SENSOR_FW_VER_5_6,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_FastZoomCrop,				PARAM_FAST_ZOOM_CROP,						XN_SENSOR_FW_VER_5_9,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_LogFilter,				PARAM_MISC_LOG_FILTER);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_GMCDebug,					PARAM_GMC_DEBUG,							XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_APCEnabled,				PARAM_APC_ENABLE,							XN_SENSOR_FW_VER_5_0,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_WavelengthCorrection,		PARAM_WAVELENGTH_CORRECTION_ENABLED,		XN_SENSOR_FW_VER_5_2,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = AddFirmwareParam(		m_WavelengthCorrectionDebug,PARAM_WAVELENGTH_CORRECTION_DEBUG_ENABLED,	XN_SENSOR_FW_VER_5_2,	XN_SENSOR_FW_VER_UNKNOWN,	FALSE);
	XN_IS_STATUS_OK(nRetVal);

	// override some props
	m_ImageFormat.UpdateSetCallback(SetImageFormatCallback, this);

	// register for some interesting changes
	XnCallbackHandle hCallbackDummy;
	nRetVal = m_Stream0Mode.OnChangeEvent().Register(ReferenceResolutionPropertyValueChanged, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Stream1Mode.OnChangeEvent().Register(ReferenceResolutionPropertyValueChanged, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_IRResolution.OnChangeEvent().Register(ReferenceResolutionPropertyValueChanged, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_DepthFPS.OnChangeEvent().Register(ReferenceResolutionPropertyValueChanged, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = RecalculateReferenceResolution();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

void XnSensorFirmwareParams::Free()
{
	m_AllFirmwareParams.Clear();
}

XnStatus XnSensorFirmwareParams::AddFirmwareParam(XnActualIntProperty& Property, XnUInt16 nFirmwareParam, XnFWVer nMinVer /* = XN_SENSOR_FW_VER_UNKNOWN */, XnFWVer nMaxVer /* = XN_SENSOR_FW_VER_UNKNOWN */, XnUInt16 nValueIfNotSupported /* = 0 */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnFirmwareParam param;
	param.pProperty = &Property;
	param.nFirmwareParam = nFirmwareParam;
	param.MinVer = nMinVer;
	param.MaxVer = nMaxVer;
	param.nValueIfNotSupported = nValueIfNotSupported;

	nRetVal = m_AllFirmwareParams.Set(&Property, param);
	XN_IS_STATUS_OK(nRetVal);

	XnChar csNewName[XN_DEVICE_MAX_STRING_LENGTH];
	sprintf(csNewName, "%s (%d)", Property.GetName(), nFirmwareParam);

	Property.UpdateName("Firmware", csNewName);
	Property.SetLogSeverity(XN_LOG_VERBOSE);
	Property.SetAlwaysSet(TRUE);
	Property.UpdateSetCallback(SetFirmwareParamCallback, this);

	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::AddFirmwareAudioParam(XnActualIntProperty& Property, XnUInt16 nFirmwareParam, XnFWVer nMinVer /* = XN_SENSOR_FW_VER_3_0 */, XnFWVer nMaxVer /* = XN_SENSOR_FW_VER_UNKNOWN */, XnUInt16 nValueIfNotSupported /* = 0 */)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = AddFirmwareParam(Property, nFirmwareParam, nMinVer, nMaxVer, nValueIfNotSupported);
	XN_IS_STATUS_OK(nRetVal);

	Property.UpdateSetCallback(SetFirmwareAudioParamCallback, this);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::UpdateAllProperties()
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Reading all params from firmware...");

	for (XnFirmwareParamsHash::Iterator it = m_AllFirmwareParams.Begin(); it != m_AllFirmwareParams.End(); ++it)
	{
		XnFirmwareParam& param = it->Value();
		nRetVal = UpdateProperty(&param);
		XN_IS_STATUS_OK(nRetVal);
	}

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Firmware params were updated.");

	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::StartTransaction()
{
	if (m_bInTransaction)
	{
		return XN_STATUS_ERROR;
	}

	m_bInTransaction = TRUE;
	m_Transaction.Clear();
	m_TransactionOrder.Clear();

	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::CommitTransaction()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (!m_bInTransaction)
	{
		return XN_STATUS_ERROR;
	}

	// we are no longer in transaction, even if we fail to commit.
	m_bInTransaction = FALSE;

	for (XnActualIntPropertyList::Iterator it = m_TransactionOrder.Begin(); it != m_TransactionOrder.End(); ++it)
	{
		XnActualIntProperty* pProp = *it;

		XnUInt32 nValue;
		nRetVal = m_Transaction.Get(pProp, nValue);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = SetFirmwareParamImpl(pProp, nValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	m_Transaction.Clear();
	m_TransactionOrder.Clear();
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::CommitTransactionAsBatch()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (!m_bInTransaction)
	{
		return XN_STATUS_ERROR;
	}

	// we are no longer in transaction, even if we fail to commit.
	m_bInTransaction = FALSE;

	if (m_TransactionOrder.Size() != 0)
	{
		XnUInt32 nMaxCount = m_TransactionOrder.Size();
		XnInnerParamData* pParams;
		XN_VALIDATE_CALLOC(pParams, XnInnerParamData, nMaxCount);

		XnChar strLogMessage[1024];
		XnUInt32 nMaxLength = 1024;
		XnUInt32 nLength = 0;
		XnUInt32 nChars;
		xnOSStrFormat(strLogMessage + nLength, nMaxLength - nLength, &nChars, "Setting firmware params:\n\t");
		nLength += nChars;

		XnUInt32 nCount = 0;

		for (XnActualIntPropertyList::Iterator it = m_TransactionOrder.Begin(); it != m_TransactionOrder.End(); ++it)
		{
			XnActualIntProperty* pProp = *it;

			XnUInt32 nValue;
			nRetVal = m_Transaction.Get(pProp, nValue);
			if (nRetVal != XN_STATUS_OK)
			{
				xnOSFree(pParams);
				return (nRetVal);
			}

			XnFirmwareParam* pParam;
			nRetVal = CheckFirmwareParam(pProp, nValue, &pParam);
			if (nRetVal != XN_STATUS_OK)
			{
				xnOSFree(pParams);
				return (nRetVal);
			}

			if (pParam != NULL)
			{
				xnOSStrFormat(strLogMessage + nLength, nMaxLength - nLength, &nChars, "%s = %u\n\t", pProp->GetName(), nValue);
				nLength += nChars;

				pParams[nCount].nParam = pParam->nFirmwareParam;
				pParams[nCount].nValue = (XnUInt16)nValue;
				nCount++;
			}
		}

		xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "%s", strLogMessage);

		// set all params
		nRetVal = m_pCommands->SetMultipleFirmwareParams(pParams, nCount);
		xnOSFree(pParams);
		XN_IS_STATUS_OK(nRetVal);

		// and update their props
		for (XnActualIntPropertyList::Iterator it = m_TransactionOrder.Begin(); it != m_TransactionOrder.End(); ++it)
		{
			XnActualIntProperty* pProp = *it;

			XnUInt32 nValue;
			nRetVal = m_Transaction.Get(pProp, nValue);
			XN_IS_STATUS_OK(nRetVal);

			nRetVal = pProp->UnsafeUpdateValue(nValue);
			XN_IS_STATUS_OK(nRetVal);
		}
	}

	m_Transaction.Clear();
	m_TransactionOrder.Clear();

	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::RollbackTransaction()
{
	if (!m_bInTransaction)
	{
		return XN_STATUS_ERROR;
	}

	m_Transaction.Clear();
	m_TransactionOrder.Clear();
	m_bInTransaction = FALSE;

	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::UpdateProperty(XnFirmwareParam* pParam)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnUInt16 nNewValue;

	// check version
	if ((pParam->MinVer != XN_SENSOR_FW_VER_UNKNOWN && m_pInfo->nFWVer < pParam->MinVer) ||
		(pParam->MaxVer != XN_SENSOR_FW_VER_UNKNOWN && m_pInfo->nFWVer > pParam->MaxVer))
	{
		// version not supported
		nNewValue = pParam->nValueIfNotSupported;
	}
	else
	{
		// Read value from firmware
		nRetVal = m_pCommands->GetFirmwareParam(pParam->nFirmwareParam, &nNewValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	// update value if needed
	if (nNewValue != pParam->pProperty->GetValue())
	{
		// update base (don't call our function, so that it won't update firmware)
		nRetVal = pParam->pProperty->UnsafeUpdateValue(nNewValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::SetFirmwareParam(XnActualIntProperty* pProperty, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (m_bInTransaction)
	{
		nRetVal = m_Transaction.Set(pProperty, (XnUInt32)nValue);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = m_TransactionOrder.AddLast(pProperty);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		nRetVal = SetFirmwareParamImpl(pProperty, nValue);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::SetFirmwareAudioParam(XnActualIntProperty* pProperty, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// check if audio is not supported, and trying to change the value
	if (!m_pInfo->bAudioSupported && nValue != pProperty->GetValue())
	{
		return (XN_STATUS_DEVICE_UNSUPPORTED_PARAMETER);
	}

	nRetVal = SetFirmwareParam(pProperty, nValue);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::SetImageFormat(XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

/*	
	if (nValue == XN_IO_IMAGE_FORMAT_UNCOMPRESSED_BAYER)
	{
		nValue = XN_IO_IMAGE_FORMAT_BAYER;
	}
*/

	nRetVal = SetFirmwareParam(&m_ImageFormat, nValue);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::CheckFirmwareParam(XnActualIntProperty* pProperty, XnUInt64 nValue, XnFirmwareParam** ppParam)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// find the property in the hash
	XnFirmwareParam* pParam;
	nRetVal = m_AllFirmwareParams.Get(pProperty, pParam);
	XN_IS_STATUS_OK(nRetVal);

	*ppParam = NULL;

	// check version
	if ((pParam->MinVer != XN_SENSOR_FW_VER_UNKNOWN && m_pInfo->nFWVer < pParam->MinVer) ||
		(pParam->MaxVer != XN_SENSOR_FW_VER_UNKNOWN && m_pInfo->nFWVer > pParam->MaxVer))
	{
		// we only raise an error when trying to change the value...
		if (nValue != pParam->nValueIfNotSupported)
		{
			return (XN_STATUS_DEVICE_UNSUPPORTED_PARAMETER);
		}
	}
	else
	{
		*ppParam = pParam;
	}

	return XN_STATUS_OK;
}

XnStatus XnSensorFirmwareParams::SetFirmwareParamImpl(XnActualIntProperty* pProperty, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnFirmwareParam* pParam;
	nRetVal = CheckFirmwareParam(pProperty, nValue, &pParam);
	XN_IS_STATUS_OK(nRetVal);

	if (pParam != NULL)
	{
		// update firmware
		nRetVal = m_pCommands->SetFirmwareParam(pParam->nFirmwareParam, (XnUInt16)nValue); 
		XN_IS_STATUS_OK(nRetVal);

		// update property
		nRetVal = pParam->pProperty->UnsafeUpdateValue(nValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::SetStreamMode(XnActualIntProperty* pProperty, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// we require that every change to mode will go through OFF
	if (nValue != XN_VIDEO_STREAM_OFF && pProperty->GetValue() != XN_VIDEO_STREAM_OFF)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Firmware stream is already in use!");
	}

	// OK, set it
	nRetVal = SetFirmwareParam(pProperty, nValue);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorFirmwareParams::RecalculateReferenceResolution()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// by default, the 1.3 MP reference is used
	XnResolutions nRes = XN_RESOLUTION_SXGA;

	// only in the following cases, VGA reference is used:
	// 1. Depth is running in 60 FPS
	// 2. IR stream is running in QVGA
	if ((m_Stream1Mode.GetValue() == XN_VIDEO_STREAM_DEPTH && m_DepthFPS.GetValue() == 60) ||
		(m_Stream0Mode.GetValue() == XN_VIDEO_STREAM_IR && m_IRResolution.GetValue() == XN_RESOLUTION_QVGA))
	{
		nRes = XN_RESOLUTION_VGA;
	}

	if (nRes != (XnResolutions)m_ReferenceResolution.GetValue())
	{
		nRetVal = m_ReferenceResolution.UnsafeUpdateValue(nRes);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XN_CALLBACK_TYPE XnSensorFirmwareParams::SetFirmwareParamCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie)
{
	XnSensorFirmwareParams* pThis = (XnSensorFirmwareParams*)pCookie;
	return pThis->SetFirmwareParam(pSender, nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensorFirmwareParams::SetFirmwareAudioParamCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie)
{
	XnSensorFirmwareParams* pThis = (XnSensorFirmwareParams*)pCookie;
	return pThis->SetFirmwareAudioParam(pSender, nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensorFirmwareParams::SetImageFormatCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensorFirmwareParams* pThis = (XnSensorFirmwareParams*)pCookie;
	return pThis->SetImageFormat(nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensorFirmwareParams::SetStreamModeCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie)
{
	XnSensorFirmwareParams* pThis = (XnSensorFirmwareParams*)pCookie;
	return pThis->SetStreamMode(pSender, nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensorFirmwareParams::ReferenceResolutionPropertyValueChanged(const XnProperty* /*pSender*/, void* pCookie)
{
	XnSensorFirmwareParams* pThis = (XnSensorFirmwareParams*)pCookie;
	return pThis->RecalculateReferenceResolution();
}
