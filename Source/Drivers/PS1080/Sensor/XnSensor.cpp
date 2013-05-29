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
#include "XnSensor.h"

#include "XnSensorDepthStream.h"
#include "XnSensorImageStream.h"
#include "XnSensorIRStream.h"
#include "XnSensorAudioStream.h"
#include "XnDeviceSensor.h"
#include "XnHostProtocol.h"
#include "XnDeviceSensorInit.h"
#include "XnDeviceEnumeration.h"
#include <XnPsVersion.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_SENSOR_MAX_STREAM_COUNT						5
#define XN_SENSOR_FRAME_SYNC_MAX_DIFF					3
#define XN_SENSOR_DEFAULT_CLOSE_STREAMS_ON_SHUTDOWN		TRUE
#define XN_SENSOR_DEFAULT_HOST_TIMESTAMPS				FALSE
#define XN_GLOBAL_CONFIG_FILE_NAME						"PS1080.ini"

#define FRAME_SYNC_MAX_FRAME_TIME_DIFF					3000

#if (XN_PLATFORM == XN_PLATFORM_WIN32)
	#define XN_SENSOR_DEFAULT_USB_INTERFACE				XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS
#else
	// on weak platforms (Arm), we prefer to use BULK
	// BULK seems to be more stable on linux
	#define XN_SENSOR_DEFAULT_USB_INTERFACE				XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS
#endif

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
typedef struct XnWaitForSycnhedFrameData
{
	XnSensor* pThis;
	const XnChar* strDepthStream;
	const XnChar* strImageStream;
} XnWaitForSycnhedFrameData;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnSensor::XnSensor(XnBool bResetOnStartup /* = TRUE */, XnBool bLeanInit /* = FALSE */) :
	XnDeviceBase(),
	m_hDisconnectedCallback(NULL),
	m_ErrorState(XN_MODULE_PROPERTY_ERROR_STATE, "ErrorState", XN_STATUS_OK),
	m_ResetSensorOnStartup(XN_MODULE_PROPERTY_RESET_SENSOR_ON_STARTUP, "ResetOnStartup", bResetOnStartup),
	m_LeanInit(XN_MODULE_PROPERTY_LEAN_INIT, "LeanInit", bLeanInit),
	m_Interface(XN_MODULE_PROPERTY_USB_INTERFACE, "UsbInterface", bResetOnStartup ? XN_SENSOR_DEFAULT_USB_INTERFACE : XN_SENSOR_USB_INTERFACE_DEFAULT),
	m_ReadData(0, "ReadData", FALSE),
	m_FrameSync(XN_MODULE_PROPERTY_FRAME_SYNC, "FrameSync", FALSE),
	m_FirmwareFrameSync(XN_MODULE_PROPERTY_FIRMWARE_FRAME_SYNC, "FirmwareFrameSync", FALSE),
	m_CloseStreamsOnShutdown(XN_MODULE_PROPERTY_CLOSE_STREAMS_ON_SHUTDOWN, "CloseStreamsOnShutdown", XN_SENSOR_DEFAULT_CLOSE_STREAMS_ON_SHUTDOWN),
	m_HostTimestamps(XN_MODULE_PROPERTY_HOST_TIMESTAMPS, "HostTimestamps", XN_SENSOR_DEFAULT_HOST_TIMESTAMPS),
	m_FirmwareParam(XN_MODULE_PROPERTY_FIRMWARE_PARAM, "FirmwareParam", NULL),
	m_CmosBlankingUnits(XN_MODULE_PROPERTY_CMOS_BLANKING_UNITS, "BlankingUnits", NULL),
	m_CmosBlankingTime(XN_MODULE_PROPERTY_CMOS_BLANKING_TIME, "BlankingTime", NULL),
	m_Reset(XN_MODULE_PROPERTY_RESET, "Reset"),
	m_Version(XN_MODULE_PROPERTY_VERSION, "Version", &m_DevicePrivateData.Version, sizeof(m_DevicePrivateData.Version), NULL),
	m_FixedParam(XN_MODULE_PROPERTY_FIXED_PARAMS, "FixedParams", NULL),
	m_ID(XN_MODULE_PROPERTY_SERIAL_NUMBER, "ID"),
	m_DeviceName(XN_MODULE_PROPERTY_PHYSICAL_DEVICE_NAME, "PhysicalDeviceName"),
	m_VendorSpecificData(XN_MODULE_PROPERTY_VENDOR_SPECIFIC_DATA, "VendorData"),
	m_PlatformString(XN_MODULE_PROPERTY_SENSOR_PLATFORM_STRING, "SensorPlatformString"),
	m_AudioSupported(XN_MODULE_PROPERTY_AUDIO_SUPPORTED, "IsAudioSupported"),
	m_ImageSupported(XN_MODULE_PROPERTY_IMAGE_SUPPORTED, "IsImageSupported"),
	m_ImageControl(XN_MODULE_PROPERTY_IMAGE_CONTROL, "ImageControl", NULL),
	m_DepthControl(XN_MODULE_PROPERTY_DEPTH_CONTROL, "DepthControl", NULL),
	m_AHB(XN_MODULE_PROPERTY_AHB, "AHB", NULL),
	m_LedState(XN_MODULE_PROPERTY_LED_STATE, "LedState", NULL),
	m_EmitterEnabled(XN_MODULE_PROPERTY_EMITTER_STATE, "EmitterState", NULL),
	m_FirmwareLogFilter(XN_MODULE_PROPERTY_FIRMWARE_LOG_FILTER, "FirmwareLogFilter", 0),
	m_FirmwareLogInterval(XN_MODULE_PROPERTY_FIRMWARE_LOG_INTERVAL, "FirmwareLogInterval", 0),
	m_FirmwareLogPrint(XN_MODULE_PROPERTY_PRINT_FIRMWARE_LOG, "PrintFirmwareLog", FALSE),
	m_FirmwareCPUInterval(XN_MODULE_PROPERTY_FIRMWARE_CPU_INTERVAL, "FirmwareCPUInterval", 0),
	m_APCEnabled(XN_MODULE_PROPERTY_APC_ENABLED, "APCEnabled", TRUE),
	m_FirmwareTecDebugPrint(XN_MODULE_PROPERTY_FIRMWARE_TEC_DEBUG_PRINT, "TecDebugPrint", FALSE),
	m_I2C(XN_MODULE_PROPERTY_I2C, "I2C", NULL),
	m_DeleteFile(XN_MODULE_PROPERTY_DELETE_FILE, "DeleteFile"),
	m_TecSetPoint(XN_MODULE_PROPERTY_TEC_SET_POINT, "TecSetPoint"),
	m_TecStatus(XN_MODULE_PROPERTY_TEC_STATUS, "TecStatus", NULL),
	m_TecFastConvergenceStatus(XN_MODULE_PROPERTY_TEC_FAST_CONVERGENCE_STATUS, "TecFastConvergenceStatus", NULL),
	m_EmitterSetPoint(XN_MODULE_PROPERTY_EMITTER_SET_POINT, "EmitterSetPoint"),
	m_EmitterStatus(XN_MODULE_PROPERTY_EMITTER_STATUS, "EmitterStatus", NULL),
	m_FileAttributes(XN_MODULE_PROPERTY_FILE_ATTRIBUTES, "FileAttributes", NULL),
	m_FlashFile(XN_MODULE_PROPERTY_FILE, "File", NULL),
	m_FirmwareLog(XN_MODULE_PROPERTY_FIRMWARE_LOG, "FirmwareLog", NULL),
	m_FlashChunk(XN_MODULE_PROPERTY_FLASH_CHUNK, "FlashChunk", NULL),
	m_FileList(XN_MODULE_PROPERTY_FILE_LIST, "FileList", NULL),
	m_BIST(XN_MODULE_PROPERTY_BIST, "BIST", NULL),
	m_ProjectorFault(XN_MODULE_PROPERTY_PROJECTOR_FAULT, "ProjectorFault", NULL),
	m_Firmware(&m_DevicePrivateData),
	m_FPS(),
	m_CmosInfo(&m_Firmware, &m_DevicePrivateData),
	m_SensorIO(&m_DevicePrivateData.SensorHandle),
	m_Objects(&m_Firmware, &m_DevicePrivateData, &m_FPS, &m_CmosInfo),
	m_pScheduler(NULL),
	m_pLogTask(NULL),
	m_pCPUTask(NULL),
	m_FirmwareLogDump(NULL),
	m_FrameSyncDump(NULL),
	m_bInitialized(FALSE)
{
	// reset all data
	xnOSMemSet(&m_DevicePrivateData, 0, sizeof(XnDevicePrivateData));
	ResolveGlobalConfigFileName(m_strGlobalConfigFile, sizeof(m_strGlobalConfigFile), NULL);

	m_ResetSensorOnStartup.UpdateSetCallbackToDefault();
	m_LeanInit.UpdateSetCallbackToDefault();
	m_Interface.UpdateSetCallback(SetInterfaceCallback, this);
	m_ReadData.UpdateSetCallback(SetReadDataCallback, this);
	m_FrameSync.UpdateSetCallbackToDefault();
	m_FirmwareFrameSync.UpdateSetCallback(SetFirmwareFrameSyncCallback, this);
	m_FirmwareParam.UpdateSetCallback(SetFirmwareParamCallback, this);
	m_FirmwareParam.UpdateGetCallback(GetFirmwareParamCallback, this);
	m_CmosBlankingUnits.UpdateSetCallback(SetCmosBlankingUnitsCallback, this);
	m_CmosBlankingUnits.UpdateGetCallback(GetCmosBlankingUnitsCallback, this);
	m_CmosBlankingTime.UpdateSetCallback(SetCmosBlankingTimeCallback, this);
	m_CmosBlankingTime.UpdateGetCallback(GetCmosBlankingTimeCallback, this);
	m_Reset.UpdateSetCallback(ResetCallback, this);
	m_FixedParam.UpdateGetCallback(GetFixedParamsCallback, this);
	m_CloseStreamsOnShutdown.UpdateSetCallbackToDefault();
	m_HostTimestamps.UpdateSetCallbackToDefault();
	m_AudioSupported.UpdateGetCallback(GetAudioSupportedCallback, this);
	m_ImageSupported.UpdateGetCallback(GetImageSupportedCallback, this);
	m_ImageControl.UpdateSetCallback(SetImageCmosRegisterCallback, this);
	m_ImageControl.UpdateGetCallback(GetImageCmosRegisterCallback, this);
	m_DepthControl.UpdateSetCallback(SetDepthCmosRegisterCallback, this);
	m_DepthControl.UpdateGetCallback(GetDepthCmosRegisterCallback, this);
	m_AHB.UpdateSetCallback(WriteAHBCallback, this);
	m_AHB.UpdateGetCallback(ReadAHBCallback, this);
	m_LedState.UpdateSetCallback(SetLedStateCallback, this);
	m_EmitterEnabled.UpdateSetCallback(SetEmitterStateCallback, this);
	m_FirmwareLogInterval.UpdateSetCallback(SetFirmwareLogIntervalCallback, this);
	m_FirmwareLogPrint.UpdateSetCallback(SetFirmwareLogPrintCallback, this);
	m_FirmwareCPUInterval.UpdateSetCallback(SetFirmwareCPUIntervalCallback, this);
	m_DeleteFile.UpdateSetCallback(DeleteFileCallback, this);
	m_FirmwareLogFilter.UpdateSetCallback(SetFirmwareLogFilterCallback, this);
	m_APCEnabled.UpdateSetCallback(SetAPCEnabledCallback, this);
	m_TecSetPoint.UpdateSetCallback(SetTecSetPointCallback, this);
	m_TecStatus.UpdateGetCallback(GetTecStatusCallback, this);
	m_TecFastConvergenceStatus.UpdateGetCallback(GetTecFastConvergenceStatusCallback, this);
	m_EmitterSetPoint.UpdateSetCallback(SetEmitterSetPointCallback, this);
	m_EmitterStatus.UpdateGetCallback(GetEmitterStatusCallback, this);
	m_I2C.UpdateSetCallback(SetI2CCallback, this);
	m_I2C.UpdateGetCallback(GetI2CCallback, this);
	m_FileAttributes.UpdateSetCallback(SetFileAttributesCallback, this);
	m_FlashFile.UpdateSetCallback(WriteFlashFileCallback, this);
	m_FlashFile.UpdateGetCallback(ReadFlashFileCallback, this);
	m_FirmwareLog.UpdateGetCallback(GetFirmwareLogCallback, this);
	m_FlashChunk.UpdateGetCallback(ReadFlashChunkCallback, this);
	m_FileList.UpdateGetCallback(GetFileListCallback, this);
	m_BIST.UpdateSetCallback(RunBISTCallback, this);
	m_ProjectorFault.UpdateSetCallback(SetProjectorFaultCallback, this);
	m_FirmwareTecDebugPrint.UpdateSetCallbackToDefault();

	// Clear the frame-synced streams.
	m_nFrameSyncEnabled = FALSE;
	m_nFrameSyncLastFrameID = 0;
}

XnSensor::~XnSensor()
{
	XnSensor::Destroy();
}

XnStatus XnSensor::InitImpl(const XnDeviceConfig *pDeviceConfig)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Initializing device sensor...");

	nRetVal = xnSchedulerStart(&m_pScheduler);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_PropSynchronizer.RegisterSynchronization(&m_Firmware.GetParams()->m_APCEnabled, &m_APCEnabled);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_PropSynchronizer.RegisterSynchronization(&m_Firmware.GetParams()->m_LogFilter, &m_FirmwareLogFilter);
	XN_IS_STATUS_OK(nRetVal);

	// Frame Sync
	XnCallbackHandle hCallbackDummy;
	nRetVal = m_FrameSync.OnChangeEvent().Register(FrameSyncPropertyChangedCallback, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = GetFirmware()->GetParams()->m_Stream0Mode.OnChangeEvent().Register(FrameSyncPropertyChangedCallback, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = GetFirmware()->GetParams()->m_Stream1Mode.OnChangeEvent().Register(FrameSyncPropertyChangedCallback, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	// other stuff
	m_FrameSyncDump = xnDumpFileOpen(XN_DUMP_FRAME_SYNC, "FrameSync.csv");
	xnDumpFileWriteString(m_FrameSyncDump, "HostTime(us),DepthNewData,DepthTimestamp(ms),ImageNewData,ImageTimestamp(ms),Diff(ms),Action\n");

	nRetVal = XnDeviceBase::InitImpl(pDeviceConfig);
	XN_IS_STATUS_OK(nRetVal);

	// now that everything is configured, open the sensor
	nRetVal = InitSensor(pDeviceConfig);
	if (nRetVal != XN_STATUS_OK)
	{
		Destroy();
		return (nRetVal);
	}

	nRetVal = XnDeviceEnumeration::DisconnectedEvent().Register(OnDeviceDisconnected, this, m_hDisconnectedCallback);
	XN_IS_STATUS_OK(nRetVal);

	xnLogInfo(XN_MASK_DEVICE_SENSOR, "Device sensor initialized");

	return (XN_STATUS_OK);
}

XnStatus XnSensor::InitSensor(const XnDeviceConfig* pDeviceConfig)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnDevicePrivateData* pDevicePrivateData = GetDevicePrivateData();

	pDevicePrivateData->pSensor = this;

	// open IO
	nRetVal = m_SensorIO.OpenDevice(pDeviceConfig->cpConnectionString);
	XN_IS_STATUS_OK(nRetVal);

	// initialize
	nRetVal = XnDeviceSensorInit(pDevicePrivateData);
	XN_IS_STATUS_OK(nRetVal);

	// init firmware
	nRetVal = m_Firmware.Init((XnBool)m_ResetSensorOnStartup.GetValue(), (XnBool)m_LeanInit.GetValue());
	XN_IS_STATUS_OK(nRetVal);
	m_bInitialized = TRUE;

	m_ResetSensorOnStartup.UpdateSetCallback(NULL, NULL);
	m_LeanInit.UpdateSetCallback(NULL, NULL);

	// update device info properties
	nRetVal = m_DeviceName.UnsafeUpdateValue(GetFixedParams()->GetDeviceName());
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = m_VendorSpecificData.UnsafeUpdateValue(GetFixedParams()->GetVendorData());
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = m_ID.UnsafeUpdateValue(GetFixedParams()->GetSensorSerial());
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = m_PlatformString.UnsafeUpdateValue(GetFixedParams()->GetPlatformString());
	XN_IS_STATUS_OK(nRetVal);

	// Add supported streams
	AddSupportedStream(XN_STREAM_TYPE_DEPTH);
	AddSupportedStream(XN_STREAM_TYPE_IR);

	if (GetFirmware()->GetInfo()->bImageSupported)
	{
		AddSupportedStream(XN_STREAM_TYPE_IMAGE);
	}

	if (GetFirmware()->GetInfo()->bAudioSupported)
	{
		AddSupportedStream(XN_STREAM_TYPE_AUDIO);
	}

	return XN_STATUS_OK;
}

XnStatus XnSensor::Destroy()
{
	XnDevicePrivateData* pDevicePrivateData = GetDevicePrivateData();

	if (m_hDisconnectedCallback != NULL)
	{
		XnDeviceEnumeration::DisconnectedEvent().Unregister(m_hDisconnectedCallback);
		m_hDisconnectedCallback = NULL;
	}

	// close Commands.txt thread
	if (pDevicePrivateData->LogThread.hThread != NULL)
	{
		pDevicePrivateData->LogThread.bKillThread = TRUE;

		xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Shutting down Sensor commands.txt thread...");
		xnOSWaitAndTerminateThread(&pDevicePrivateData->LogThread.hThread, XN_DEVICE_SENSOR_THREAD_KILL_TIMEOUT);
		pDevicePrivateData->LogThread.hThread = NULL;
	}

	// if needed, close the streams
	if (m_bInitialized && m_CloseStreamsOnShutdown.GetValue() == TRUE && 
		m_ReadData.GetValue() == TRUE && m_ErrorState.GetValue() != XN_STATUS_DEVICE_NOT_CONNECTED)
	{
		m_Firmware.GetParams()->m_Stream0Mode.SetValue(XN_VIDEO_STREAM_OFF);
		m_Firmware.GetParams()->m_Stream1Mode.SetValue(XN_VIDEO_STREAM_OFF);
		m_Firmware.GetParams()->m_Stream2Mode.SetValue(XN_AUDIO_STREAM_OFF);
	}

	// close IO (including all reading threads)
	m_SensorIO.CloseDevice();
	m_bInitialized = FALSE;

	// shutdown scheduler
	if (m_pScheduler != NULL)
	{
		xnSchedulerShutdown(&m_pScheduler);
		m_pScheduler = NULL;
	}

	if (pDevicePrivateData->hEndPointsCS != NULL)
	{
		xnOSCloseCriticalSection(&pDevicePrivateData->hEndPointsCS);
		pDevicePrivateData->hEndPointsCS = NULL;
	}

	// free buffers
	XnDeviceSensorFreeBuffers(pDevicePrivateData);

	if (pDevicePrivateData->hExecuteMutex != NULL)
	{
		xnOSCloseMutex(&pDevicePrivateData->hExecuteMutex);
		pDevicePrivateData->hExecuteMutex = NULL;
	}

	XnDeviceBase::Destroy();

	// close dumps
	xnDumpFileClose(pDevicePrivateData->TimestampsDump);
	xnDumpFileClose(pDevicePrivateData->BandwidthDump);
	xnDumpFileClose(pDevicePrivateData->MiniPacketsDump);
	xnDumpFileClose(m_FrameSyncDump);
	xnDumpFileClose(m_FirmwareLogDump);

	m_Firmware.Free();

	return (XN_STATUS_OK);
}

XnStatus XnSensor::CreateDeviceModule(XnDeviceModuleHolder** ppModuleHolder)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnDeviceBase::CreateDeviceModule(ppModuleHolder);
	XN_IS_STATUS_OK(nRetVal);

	// add sensor properties
	XnDeviceModule* pModule = (*ppModuleHolder)->GetModule();
	XnProperty* pProps[] = 
	{ 
		&m_ErrorState, &m_ResetSensorOnStartup, &m_LeanInit, &m_Interface, &m_ReadData, &m_FirmwareParam, 
		&m_CmosBlankingUnits, &m_CmosBlankingTime, &m_Reset, &m_Version, 
		&m_FixedParam, &m_FrameSync, &m_FirmwareFrameSync, &m_CloseStreamsOnShutdown, &m_ID,
		&m_VendorSpecificData, &m_AudioSupported, &m_ImageSupported,
		&m_ImageControl, &m_DepthControl, &m_AHB, &m_LedState, &m_EmitterEnabled, &m_HostTimestamps, &m_PlatformString,
		&m_FirmwareLogInterval, &m_FirmwareLogPrint, &m_FirmwareCPUInterval, &m_DeleteFile, 
		&m_APCEnabled, &m_TecSetPoint, &m_TecStatus, &m_TecFastConvergenceStatus, &m_EmitterSetPoint, &m_EmitterStatus, &m_I2C,
		&m_FileAttributes, &m_FlashFile, &m_FirmwareLogFilter, &m_FirmwareLog, &m_FlashChunk, &m_FileList, 
		&m_ProjectorFault, &m_BIST, &m_FirmwareTecDebugPrint, &m_DeviceName 
	};

	nRetVal = pModule->AddProperties(pProps, sizeof(pProps)/sizeof(XnProperty*));
	if (nRetVal != XN_STATUS_OK)
	{
		DestroyModule(*ppModuleHolder);
		*ppModuleHolder = NULL;
		return (nRetVal);
	}

	// configure it from global file
	if (m_strGlobalConfigFile[0] != '\0')
	{
		nRetVal = pModule->LoadConfigFromFile(m_strGlobalConfigFile);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::CreateStreamImpl(const XnChar* strType, const XnChar* strName, const XnActualPropertiesHash* pInitialSet)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnDeviceBase::CreateStreamImpl(strType, strName, pInitialSet);
	XN_IS_STATUS_OK(nRetVal);

	// and configure it from global config file
	nRetVal = ConfigureModuleFromGlobalFile(strName, strType);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::CreateStreamModule(const XnChar* StreamType, const XnChar* StreamName, XnDeviceModuleHolder** ppStreamHolder)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// make sure reading from streams is turned on
	if (!m_ReadData.GetValue())
	{
		nRetVal = m_ReadData.SetValue(TRUE);
		XN_IS_STATUS_OK(nRetVal);
	}

	XnDeviceStream* pStream;
	XnSensorStreamHelper* pHelper;

	// create stream
	if (strcmp(StreamType, XN_STREAM_TYPE_DEPTH) == 0)
	{
		XnSensorDepthStream* pDepthStream;
		XN_VALIDATE_NEW(pDepthStream, XnSensorDepthStream, StreamName, &m_Objects);
		pStream = pDepthStream;
		pHelper = pDepthStream->GetHelper();
	}
	else if (strcmp(StreamType, XN_STREAM_TYPE_IMAGE) == 0)
	{
		XnSensorImageStream* pImageStream;
		XN_VALIDATE_NEW(pImageStream, XnSensorImageStream, StreamName, &m_Objects);
		pStream = pImageStream;
		pHelper = pImageStream->GetHelper();
	}
	else if (strcmp(StreamType, XN_STREAM_TYPE_IR) == 0)
	{
		XnSensorIRStream* pIRStream;
		XN_VALIDATE_NEW(pIRStream, XnSensorIRStream, StreamName, &m_Objects);
		pStream = pIRStream;
		pHelper = pIRStream->GetHelper();
	}
	else if (strcmp(StreamType, XN_STREAM_TYPE_AUDIO) == 0)
	{
		// TODO: enable
		XN_ASSERT(FALSE);
		pStream = NULL;
		pHelper = NULL;
/*		if (!m_Firmware.GetInfo()->bAudioSupported)
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_UNSUPPORTED_STREAM, XN_MASK_DEVICE_SENSOR, "Audio is not supported by this FW!");
		}

		// TODO: use the allow other users property when constructing the audio stream
		XnSensorAudioStream* pAudioStream;
		XN_VALIDATE_NEW(pAudioStream, XnSensorAudioStream, GetUSBPath(), StreamName, &m_Objects, FALSE);
		pStream = pAudioStream;
		pHelper = pAudioStream->GetHelper();
*/
	}
	else
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_UNSUPPORTED_STREAM, XN_MASK_DEVICE_SENSOR, "Unsupported stream type: %s", StreamType);
	}

	*ppStreamHolder = XN_NEW(XnSensorStreamHolder, pStream, pHelper);

	return (XN_STATUS_OK);
}

void XnSensor::DestroyStreamModule(XnDeviceModuleHolder* pStreamHolder)
{
	XN_DELETE(pStreamHolder->GetModule());
	XN_DELETE(pStreamHolder);
}

XnStatus XnSensor::OpenAllStreams()
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Opening all streams...");

	// take a list of all the streams
	const XnChar* astrStreams[XN_SENSOR_MAX_STREAM_COUNT];
	XnUInt32 nStreamCount = XN_SENSOR_MAX_STREAM_COUNT;
	XnDeviceStream* apStreams[XN_SENSOR_MAX_STREAM_COUNT];
	XnSensorStreamHolder* apSensorStreams[XN_SENSOR_MAX_STREAM_COUNT];

	nRetVal = GetStreamNames(astrStreams, &nStreamCount);
	XN_IS_STATUS_OK(nRetVal);

	for (XnUInt32 i = 0; i < nStreamCount; ++i)
	{
		XnDeviceModuleHolder* pHolder;
		nRetVal = FindStream(astrStreams[i], &pHolder);
		XN_IS_STATUS_OK(nRetVal);

		apSensorStreams[i] = (XnSensorStreamHolder*)(pHolder);
		apStreams[i] = apSensorStreams[i]->GetStream();
	}

	// NOTE: the following is an ugly patch. When depth and IR both exist, Depth stream MUST be configured
	// and opened BEFORE IR stream. So, generally, if one of the streams is depth, we move it to be first.
	for (XnUInt32 i = 1; i < nStreamCount; ++i)
	{
		if (strcmp(apStreams[i]->GetType(), XN_STREAM_TYPE_DEPTH) == 0)
		{
			// switch it with the one in location 0
			const XnChar* strTempName = astrStreams[0];
			XnDeviceStream* pTempStream = apStreams[0];
			XnSensorStreamHolder* pTempHolder = apSensorStreams[0];

			astrStreams[0] = astrStreams[i];
			apStreams[0] = apStreams[i];
			apSensorStreams[0] = apSensorStreams[i];

			astrStreams[i] = strTempName;
			apStreams[i] = pTempStream;
			apSensorStreams[i] = pTempHolder;
			break;
		}
	}

	// now configure them all
	for (XnUInt32 i = 0; i < nStreamCount; ++i)
	{
		if (!apStreams[i]->IsOpen())
		{
			xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Configuring stream %s...", apStreams[i]->GetName());
			nRetVal = apSensorStreams[i]->Configure();
			XN_IS_STATUS_OK(nRetVal);
			xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Stream %s is configured", apStreams[i]->GetName());
		}
		else
		{
			xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Stream %s is already open.", apStreams[i]->GetName());
		}
	}

	// and open them all
	for (XnUInt32 i = 0; i < nStreamCount; ++i)
	{
		if (!apStreams[i]->IsOpen())
		{
			nRetVal = apSensorStreams[i]->FinalOpen();
			XN_IS_STATUS_OK(nRetVal);
		}
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetStream(const XnChar* strStream, XnDeviceStream** ppStream)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModuleHolder* pHolder;
	nRetVal = FindStream(strStream, &pHolder);
	XN_IS_STATUS_OK(nRetVal);

	XnSensorStreamHolder* pSensorStreamHolder = (XnSensorStreamHolder*)(pHolder);
	*ppStream = pSensorStreamHolder->GetStream();

	return XN_STATUS_OK;
}

XnStatus XnSensor::LoadConfigFromFile(const XnChar* csINIFilePath, const XnChar* csSectionName)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(csINIFilePath);
	XN_VALIDATE_INPUT_PTR(csSectionName);

	// we first need to configure the USB interface (we want to do so BEFORE creating streams)
	nRetVal = m_Interface.ReadValueFromFile(csINIFilePath, XN_MODULE_NAME_DEVICE);
	XN_IS_STATUS_OK(nRetVal);

	// now configure DEVICE module (primary stream, global mirror, etc.)
	nRetVal = DeviceModule()->LoadConfigFromFile(csINIFilePath, XN_MODULE_NAME_DEVICE);
	XN_IS_STATUS_OK(nRetVal);

	// and now configure the streams
	XnDeviceModuleHolderList streams;
	nRetVal = GetStreamsList(streams);
	XN_IS_STATUS_OK(nRetVal);

	for (XnDeviceModuleHolderList::Iterator it = streams.Begin(); it != streams.End(); ++it)
	{
		XnDeviceModuleHolder* pHolder = *it;
		nRetVal = pHolder->GetModule()->LoadConfigFromFile(csINIFilePath);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::InitReading()
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnSensorUsbInterface prevInterface = GetCurrentUsbInterface();

	// open data endpoints
	nRetVal = m_SensorIO.OpenDataEndPoints((XnSensorUsbInterface)m_Interface.GetValue(), *m_Firmware.GetInfo());
	XN_IS_STATUS_OK(nRetVal);

	XnSensorUsbInterface currInterface = GetCurrentUsbInterface();
	nRetVal = m_Interface.UnsafeUpdateValue(currInterface);
	XN_IS_STATUS_OK(nRetVal);

	if (prevInterface != currInterface)
	{
		nRetVal = XnHostProtocolUpdateSupportedImageModes(&m_DevicePrivateData);
		XN_IS_STATUS_OK(nRetVal);
	}

	// take frequency information
	XnFrequencyInformation FrequencyInformation;

	nRetVal = XnHostProtocolAlgorithmParams(&m_DevicePrivateData, XN_HOST_PROTOCOL_ALGORITHM_FREQUENCY, &FrequencyInformation, sizeof(XnFrequencyInformation), (XnResolutions)0, 0);
	if (nRetVal != XN_STATUS_OK)
		return nRetVal;

	m_DevicePrivateData.fDeviceFrequency = XN_PREPARE_VAR_FLOAT_IN_BUFFER(FrequencyInformation.fDeviceFrequency);

	// Init Dumps
	m_DevicePrivateData.BandwidthDump = xnDumpFileOpen(XN_DUMP_BANDWIDTH, "Bandwidth.csv");
	xnDumpFileWriteString(m_DevicePrivateData.BandwidthDump, "Timestamp,Frame Type,Frame ID,Size\n");
	m_DevicePrivateData.TimestampsDump = xnDumpFileOpen(XN_DUMP_TIMESTAMPS, "Timestamps.csv");
	xnDumpFileWriteString(m_DevicePrivateData.TimestampsDump, "Host Time (us),Stream,Device TS,Time (ms),Comments\n");
	m_DevicePrivateData.MiniPacketsDump = xnDumpFileOpen(XN_DUMP_MINI_PACKETS, "MiniPackets.csv");
	xnDumpFileWriteString(m_DevicePrivateData.MiniPacketsDump, "HostTS,Type,ID,Size,Timestamp\n");

	m_DevicePrivateData.nGlobalReferenceTS = 0;
	nRetVal = xnOSCreateCriticalSection(&m_DevicePrivateData.hEndPointsCS);
	XN_IS_STATUS_OK(nRetVal);

	// NOTE: when we go up, some streams might be open, and so we'll receive lots of garbage.
	// wait till streams are turned off, and then start reading.
//	pDevicePrivateData->bIgnoreDataPackets = TRUE;

	// open input threads
	nRetVal = XnDeviceSensorOpenInputThreads(GetDevicePrivateData());
	XN_IS_STATUS_OK(nRetVal);

	// open 'commands.txt' thread
	nRetVal = xnOSCreateThread(XnDeviceSensorProtocolScriptThread, (XN_THREAD_PARAM)&m_DevicePrivateData, &m_DevicePrivateData.LogThread.hThread);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnSensor::ChangeTaskInterval(XnScheduledTask** ppTask, XnTaskCallbackFuncPtr pCallback, XnUInt32 nInterval)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (*ppTask == NULL)
	{
		nRetVal = xnSchedulerAddTask(m_pScheduler, nInterval, pCallback, this, ppTask);
		XN_IS_STATUS_OK(nRetVal);
	}
	else // already scheduled
	{
		if (nInterval == 0)
		{
			nRetVal = xnSchedulerRemoveTask(m_pScheduler, ppTask);
			XN_IS_STATUS_OK(nRetVal);

			*ppTask = NULL;
		}
		else
		{
			nRetVal = xnSchedulerRescheduleTask(m_pScheduler, *ppTask, nInterval);
			XN_IS_STATUS_OK(nRetVal);
		}
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::ValidateSensorID(XnChar* csSensorID)
{
	if (strcmp(csSensorID, XN_DEVICE_SENSOR_DEFAULT_ID) != 0)
	{
		if (strcmp(csSensorID, GetFixedParams()->GetSensorSerial()) != 0)
		{
			return (XN_STATUS_IO_DEVICE_WRONG_SERIAL);
		}
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::ResolveGlobalConfigFileName(XnChar* strConfigFile, XnUInt32 nBufSize, const XnChar* strConfigDir)
{
	// If strConfigDir is NULL, tries to resolve the config file based on the driver's directory
	XnChar strBaseDir[XN_FILE_MAX_PATH];
	if (strConfigDir == NULL)
	{
		if (xnOSGetModulePathForProcAddress(reinterpret_cast<void*>(&XnSensor::ResolveGlobalConfigFileName), strBaseDir) == XN_STATUS_OK &&
				xnOSGetDirName(strBaseDir, strBaseDir, XN_FILE_MAX_PATH) == XN_STATUS_OK)
		{
			// Successfully obtained the driver's path
			strConfigDir = strBaseDir;
		}
		else
		{
			// Something wrong happened. Use the current directory as the fallback.
			strConfigDir = ".";
		}
	}

	XnStatus rc;
	XN_VALIDATE_STR_COPY(strConfigFile, strConfigDir, nBufSize, rc);
	return xnOSAppendFilePath(strConfigFile, XN_GLOBAL_CONFIG_FILE_NAME, nBufSize);
}

XnStatus XnSensor::SetGlobalConfigFile(const XnChar* strConfigFile)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = xnOSStrCopy(m_strGlobalConfigFile, strConfigFile, XN_FILE_MAX_PATH);
	XN_IS_STATUS_OK(nRetVal);

	XnBool bExists;
	nRetVal = xnOSDoesFileExist(m_strGlobalConfigFile, &bExists);
	XN_IS_STATUS_OK(nRetVal);

	if (!bExists)
	{
		xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Global configuration file '%s' was not found.", m_strGlobalConfigFile);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::ConfigureModuleFromGlobalFile(const XnChar* strModule, const XnChar* strSection /* = NULL */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(strModule, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->LoadConfigFromFile(m_strGlobalConfigFile, strSection);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetFirmwareParam(XnInnerParamData* pParam)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnHostProtocolGetParam(&m_DevicePrivateData, pParam->nParam, pParam->nValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::ReadAHB(XnAHBData* pAHB)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnHostProtocolReadAHB(&m_DevicePrivateData, pAHB->nRegister, pAHB->nValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::WriteAHB(const XnAHBData* pAHB)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnHostProtocolWriteAHB(&m_DevicePrivateData, pAHB->nRegister, pAHB->nValue, pAHB->nMask);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetLedState(XnUInt16 nLedId, XnUInt16 nState)
{
	return XnHostProtocolSetLedState(&m_DevicePrivateData, nLedId, nState);
}

XnStatus XnSensor::SetEmitterState(XnBool bActive)
{
	return XnHostProtocolSetEmitterState(&m_DevicePrivateData, bActive);
}

XnStatus XnSensor::SetFirmwareFrameSync(XnBool bOn)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
 	nRetVal = GetFirmware()->GetParams()->m_FrameSyncEnabled.SetValue(bOn);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_FirmwareFrameSync.UnsafeUpdateValue(bOn);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

void XnSensor::ReadFirmwareLog()
{
	// get log
	XnChar LogBuffer[XN_MAX_LOG_SIZE] = "";
	XnHostProtocolGetLog(&m_DevicePrivateData, LogBuffer, XN_MAX_LOG_SIZE);

	// write sensor log to dump
	xnDumpFileWriteString(m_FirmwareLogDump, LogBuffer);

	// print
	if (m_FirmwareLogPrint.GetValue())
	{
		printf("%s", LogBuffer);
	}
}

void XnSensor::ReadFirmwareCPU()
{
	XnTaskCPUInfo aTasks[100];
	XnUInt32 nTasksCount = 100;
	XnStatus nRetVal = XnHostProtocolGetCPUStats(&m_DevicePrivateData, aTasks, &nTasksCount);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "GetCPUStats failed execution: %s", xnGetStatusString(nRetVal));
		return;
	}

	// sum it all up
	XnUInt64 nSum = 0;
	for (XnUInt32 nIndex = 0; nIndex < nTasksCount; ++nIndex)
		nSum += aTasks[nIndex].nTimeInMicroSeconds;

	// print
	printf("Task ID  Total Time (us)  Percentage  Times    Avg. Time Per Call\n");
	printf("=======  ===============  ==========  =======  ==================\n");
	for (XnUInt32 nIndex = 0; nIndex < nTasksCount; ++nIndex)
	{
		printf("%7u  %15u  %10.3f  %7u  %18.3f\n", 
			nIndex, aTasks[nIndex].nTimeInMicroSeconds, 
			aTasks[nIndex].nTimeInMicroSeconds * 100.0 / nSum,
			aTasks[nIndex].nTimesExecuted,
			(double)aTasks[nIndex].nTimeInMicroSeconds / (double)aTasks[nIndex].nTimesExecuted);
	}
}

XnStatus XnSensor::GetI2C(XnI2CReadData* pI2CReadData)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolReadI2C(&m_DevicePrivateData, pI2CReadData);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetTecStatus(XnTecData* pTecData)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolGetTecData(&m_DevicePrivateData, pTecData);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetTecFastConvergenceStatus(XnTecFastConvergenceData* pTecData)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnHostProtocolGetTecFastConvergenceData(&m_DevicePrivateData, pTecData);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetEmitterStatus(XnEmitterData* pEmitterData)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnHostProtocolGetEmitterData(&m_DevicePrivateData, pEmitterData);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::ReadFlashFile(const XnParamFileData* pFile)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolFileDownload(&m_DevicePrivateData, (XnUInt16)pFile->nOffset, pFile->strFileName);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::ReadFlashChunk(XnParamFlashData* pFlash)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolReadFlash(&m_DevicePrivateData, pFlash->nOffset, pFlash->nSize, pFlash->pData);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetCmosBlankingUnits(XnCmosBlankingUnits* pBlanking)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (m_Firmware.GetInfo()->nFWVer < XN_SENSOR_FW_VER_5_1)
	{
		return (XN_STATUS_IO_DEVICE_FUNCTION_NOT_SUPPORTED);
	}

	nRetVal = XnHostProtocolGetCmosBlanking(&m_DevicePrivateData, pBlanking->nCmosID, &pBlanking->nUnits);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetCmosBlankingTime(XnCmosBlankingTime* pBlanking)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// check version
	if (m_Firmware.GetInfo()->nFWVer < XN_SENSOR_FW_VER_5_1)
	{
		return (XN_STATUS_IO_DEVICE_FUNCTION_NOT_SUPPORTED);
	}

	// get value in units
	XnCmosBlankingUnits blankingUnits;
	blankingUnits.nCmosID = pBlanking->nCmosID;
	nRetVal = GetCmosBlankingUnits(&blankingUnits);
	XN_IS_STATUS_OK(nRetVal);

	// get coefficients
	const XnCmosBlankingCoefficients* pCoeffs = m_CmosInfo.GetBlankingCoefficients(pBlanking->nCmosID);

	// translate to time
	pBlanking->nTimeInMilliseconds = (pCoeffs->fA * blankingUnits.nUnits + pCoeffs->fB)/1000;
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetFirmwareMode(XnParamCurrentMode* pnMode)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (m_Firmware.GetInfo()->nFWVer == XN_SENSOR_FW_VER_0_17)
	{
		*pnMode = m_Firmware.GetInfo()->nCurrMode;
	}
	else
	{
		XnUInt16 nMode;
		nRetVal = XnHostProtocolGetMode(&m_DevicePrivateData, nMode);
		XN_IS_STATUS_OK(nRetVal);

		switch (nMode)
		{
		case XN_HOST_PROTOCOL_MODE_PS:
			*pnMode = XN_MODE_PS;
			break;
		case XN_HOST_PROTOCOL_MODE_MAINTENANCE:
			*pnMode = XN_MODE_MAINTENANCE;
			break;
		case XN_HOST_PROTOCOL_MODE_SAFE_MODE:
			*pnMode = XN_MODE_SAFE_MODE;
			break;
		default:
			printf("Got Unknown Firmware Mode %d\n", nMode);
			return XN_STATUS_DEVICE_BAD_PARAM;
		}
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetDepthCmosRegister(XnControlProcessingData* pRegister)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_Firmware.GetInfo()->nFWVer >= XN_SENSOR_FW_VER_3_0)
	{
		nRetVal = XnHostProtocolGetCMOSRegisterI2C(&m_DevicePrivateData, XN_CMOS_TYPE_DEPTH, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		nRetVal = XnHostProtocolGetCMOSRegister(&m_DevicePrivateData, XN_CMOS_TYPE_DEPTH, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetImageCmosRegister(XnControlProcessingData* pRegister)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_Firmware.GetInfo()->nFWVer >= XN_SENSOR_FW_VER_3_0)
	{
		nRetVal = XnHostProtocolGetCMOSRegisterI2C(&m_DevicePrivateData, XN_CMOS_TYPE_IMAGE, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		nRetVal = XnHostProtocolGetCMOSRegister(&m_DevicePrivateData, XN_CMOS_TYPE_IMAGE, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetFirmwareLog(XnChar* csLog, XnUInt32 nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolGetLog(&m_DevicePrivateData, csLog, nSize);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetFileList(XnFlashFileList* pFileList)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolGetFileList(&m_DevicePrivateData, 0, pFileList->pFiles,  pFileList->nFiles);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::GetFixedParams(XnDynamicSizeBuffer* pBuffer)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (pBuffer->nMaxSize < sizeof(XnFixedParams))
	{
		return (XN_STATUS_OUTPUT_BUFFER_OVERFLOW);
	}

	XnFixedParams fixed;
	nRetVal = XnHostProtocolGetFixedParams(GetDevicePrivateData(), fixed);
	XN_IS_STATUS_OK(nRetVal);

	xnOSMemCopy(pBuffer->pData, &fixed, sizeof(XnFixedParams));
	pBuffer->nDataSize = sizeof(XnFixedParams);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::RunBIST(XnUInt32 nTestsMask, XnUInt32* pnFailures)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// always perform soft reset before running bist
	nRetVal = XnHostProtocolReset(&m_DevicePrivateData, XN_RESET_TYPE_SOFT);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnHostProtocolRunBIST(&m_DevicePrivateData, nTestsMask, pnFailures);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetErrorState(XnStatus errorState)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (errorState != GetErrorState())
	{
		if (errorState == XN_STATUS_OK)
		{
			xnLogInfo(XN_MASK_DEVICE_SENSOR, "Device is back to normal state.");
		}
		else
		{
			xnLogError(XN_MASK_DEVICE_SENSOR, "Device has entered error mode: %s", xnGetStatusString(errorState));
		}

		nRetVal = m_ErrorState.UnsafeUpdateValue((XnUInt64)errorState);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetInterface(XnSensorUsbInterface nInterface)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// we don't allow change if requested value is specific and different than current
	if (m_ReadData.GetValue() == TRUE &&
		nInterface != XN_SENSOR_USB_INTERFACE_DEFAULT &&
		nInterface != GetCurrentUsbInterface())
	{
		return (XN_STATUS_DEVICE_PROPERTY_READ_ONLY);
	}

	nRetVal = m_Interface.UnsafeUpdateValue(nInterface);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetHostTimestamps(XnBool bHostTimestamps)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// we don't allow change if requested value is specific and different than current
	if (m_ReadData.GetValue() == TRUE &&
		bHostTimestamps != (XnBool)m_HostTimestamps.GetValue())
	{
		return (XN_STATUS_DEVICE_PROPERTY_READ_ONLY);
	}

	nRetVal = m_HostTimestamps.UnsafeUpdateValue(bHostTimestamps);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetReadData(XnBool bRead)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (!bRead)
	{
		return XN_STATUS_ERROR;
	}
	else
	{
		nRetVal = InitReading();
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = m_ReadData.UnsafeUpdateValue(TRUE);
		XN_IS_STATUS_OK(nRetVal);

		// no longer needed
		m_ReadData.UpdateSetCallback(NULL, NULL);

		XnHostProtocolUpdateSupportedImageModes(&m_DevicePrivateData);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetDepthCmosRegister(const XnControlProcessingData* pRegister)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_Firmware.GetInfo()->nFWVer >= XN_SENSOR_FW_VER_3_0)
	{
		nRetVal = XnHostProtocolSetCMOSRegisterI2C(&m_DevicePrivateData, XN_CMOS_TYPE_DEPTH, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		nRetVal = XnHostProtocolSetCMOSRegister(&m_DevicePrivateData, XN_CMOS_TYPE_DEPTH, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetImageCmosRegister(const XnControlProcessingData* pRegister)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_Firmware.GetInfo()->nFWVer >= XN_SENSOR_FW_VER_3_0)
	{
		nRetVal = XnHostProtocolSetCMOSRegisterI2C(&m_DevicePrivateData, XN_CMOS_TYPE_IMAGE, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		nRetVal = XnHostProtocolSetCMOSRegister(&m_DevicePrivateData, XN_CMOS_TYPE_IMAGE, pRegister->nRegister, pRegister->nValue);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetFirmwareLogFilter(XnUInt32 nFilter)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// set the firmware param (this prop will be updated accordingly)
	nRetVal = m_Firmware.GetParams()->m_LogFilter.SetValue(nFilter);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetFirmwareLogInterval(XnUInt32 nMilliSeconds)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = ChangeTaskInterval(&m_pLogTask, ExecuteFirmwareLogTask, nMilliSeconds);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_FirmwareLogInterval.UnsafeUpdateValue(nMilliSeconds);
	XN_IS_STATUS_OK(nRetVal);

	if (nMilliSeconds == 0)
	{
		xnDumpFileClose(m_FirmwareLogDump);
	}
	else
	{
		m_FirmwareLogDump = xnDumpFileOpenEx("FirmwareLog", TRUE, TRUE, "Sensor.log");
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetFirmwareLogPrint(XnBool bPrint)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_FirmwareLogPrint.UnsafeUpdateValue(bPrint);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetFirmwareCPUInterval(XnUInt32 nMilliSeconds)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = ChangeTaskInterval(&m_pCPUTask, ExecuteFirmwareCPUTask, nMilliSeconds);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_FirmwareCPUInterval.UnsafeUpdateValue(nMilliSeconds);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetAPCEnabled(XnBool bEnabled)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// set firmware param (we will be updated via synch mechanism)
	nRetVal = m_Firmware.GetParams()->m_APCEnabled.SetValue(bEnabled);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetI2C(const XnI2CWriteData* pI2CWriteData)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolWriteI2C(&m_DevicePrivateData, pI2CWriteData);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::DeleteFile(XnUInt16 nFileID)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolDeleteFile(&m_DevicePrivateData, nFileID);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetTecSetPoint(XnUInt16 nSetPoint)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolCalibrateTec(&m_DevicePrivateData, nSetPoint);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetEmitterSetPoint(XnUInt16 nSetPoint)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnHostProtocolCalibrateEmitter(&m_DevicePrivateData, nSetPoint);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetFileAttributes(const XnFileAttributes* pAttributes)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolSetFileAttributes(&m_DevicePrivateData, pAttributes->nId, pAttributes->nAttribs);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetFirmwareParam(const XnInnerParamData* pParam)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolSetParam(&m_DevicePrivateData, pParam->nParam, pParam->nValue);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::WriteFlashFile(const XnParamFileData* pFile)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Upload file %s (offset %d)", pFile->strFileName, pFile->nOffset);

	nRetVal = XnHostProtocolFileUpload(&m_DevicePrivateData, pFile->nOffset, pFile->strFileName, pFile->nAttributes);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetCmosBlankingUnits(const XnCmosBlankingUnits* pBlanking)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_Firmware.GetInfo()->nFWVer < XN_SENSOR_FW_VER_5_1)
	{
		return (XN_STATUS_IO_DEVICE_FUNCTION_NOT_SUPPORTED);
	}

	nRetVal = XnHostProtocolSetCmosBlanking(&m_DevicePrivateData, pBlanking->nUnits, pBlanking->nCmosID, pBlanking->nNumberOfFrames);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetCmosBlankingTime(const XnCmosBlankingTime* pBlanking)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// check version
	if (m_Firmware.GetInfo()->nFWVer < XN_SENSOR_FW_VER_5_1)
	{
		return (XN_STATUS_IO_DEVICE_FUNCTION_NOT_SUPPORTED);
	}

	// get coefficients
	const XnCmosBlankingCoefficients* pCoeffs = m_CmosInfo.GetBlankingCoefficients(pBlanking->nCmosID);

	// translate to units request
	XnCmosBlankingUnits blankingUnits;
	blankingUnits.nCmosID = pBlanking->nCmosID;
	blankingUnits.nNumberOfFrames = pBlanking->nNumberOfFrames;
	blankingUnits.nUnits = XnUInt16((pBlanking->nTimeInMilliseconds*1000 - pCoeffs->fB)/pCoeffs->fA);

	nRetVal = SetCmosBlankingUnits(&blankingUnits);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensor::Reset(XnParamResetType nType)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolReset(&m_DevicePrivateData, (XnUInt16)nType);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetFirmwareMode(XnParamCurrentMode nMode)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (m_Firmware.GetInfo()->nFWVer == XN_SENSOR_FW_VER_0_17)
	{
		m_Firmware.GetInfo()->nCurrMode = nMode;
		return (XN_STATUS_OK);
	}

	XnHostProtocolModeType nActualValue;

	switch (nMode)
	{
	case XN_MODE_PS:
		nActualValue = XN_HOST_PROTOCOL_MODE_PS;
		break;
	case XN_MODE_MAINTENANCE:
		nActualValue = XN_HOST_PROTOCOL_MODE_MAINTENANCE;
		break;
	default:
		return XN_STATUS_DEVICE_UNSUPPORTED_MODE;
	}

	nRetVal = XnHostProtocolSetMode(&m_DevicePrivateData, (XnUInt16)nActualValue);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensor::SetProjectorFault(XnProjectorFaultData* pProjectorFaultData)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = XnHostProtocolCalibrateProjectorFault(&m_DevicePrivateData, pProjectorFaultData->nMinThreshold, pProjectorFaultData->nMaxThreshold, &pProjectorFaultData->bProjectorFaultEvent);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

void XnSensor::ExecuteFirmwareLogTask(void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	pThis->ReadFirmwareLog();
}

void XnSensor::ExecuteFirmwareCPUTask(void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	pThis->ReadFirmwareCPU();
}

XnStatus XnSensor::OnFrameSyncPropertyChanged()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (m_ReadData.GetValue() == TRUE)
	{
		// decide firmware frame sync - both streams are on, and user asked for it
		XnBool bFrameSync = (
			m_FrameSync.GetValue() == TRUE &&
			GetFirmware()->GetParams()->m_Stream0Mode.GetValue() == XN_VIDEO_STREAM_COLOR &&
			GetFirmware()->GetParams()->m_Stream1Mode.GetValue() == XN_VIDEO_STREAM_DEPTH
			);

		nRetVal = SetFirmwareFrameSync(bFrameSync);
		XN_IS_STATUS_OK(nRetVal);

		// Set frame sync enabled flag (so mechanism will not be activated in case a stream is turned off).
		m_frameSyncCs.Lock();
		m_nFrameSyncEnabled = bFrameSync;
		m_frameSyncCs.Unlock();
	}
	
	return (XN_STATUS_OK);
}

void XnSensor::OnNewStreamData(XnDeviceStream* pStream, OniFrame* pFrame)
{
	// Lock critical section.
	m_frameSyncCs.Lock();

	// Find the relevant stream in the frame-synced streams.
	FrameSyncedStream* pFrameSyncedStream = NULL;
	XnUInt32 nValidFrameCount = 0; // received frame
	XnUInt32 nFrameSyncStreamCount = m_FrameSyncedStreams.GetSize();
	int receivedFrameId = pFrame->frameIndex;
	for (XnUInt32 i = 0; m_nFrameSyncEnabled && (i < nFrameSyncStreamCount); ++i)
	{
		if (pStream == m_FrameSyncedStreams[i].pStream)
		{
			// Verify frame is valid.
			if (pFrame != NULL)
			{
				// Release old frame and assign new frame.
				if (m_FrameSyncedStreams[i].pFrame != NULL)
				{
					m_FrameSyncedStreams[i].pStream->ReleaseFrame(m_FrameSyncedStreams[i].pFrame);
				}

				// Store the frame, timestamp and frame ID.
				m_FrameSyncedStreams[i].pFrame = pFrame;
				pStream->AddRefToFrame(pFrame);
				pFrameSyncedStream = &m_FrameSyncedStreams[i];
				nValidFrameCount++;
			}
		}
		else if (m_FrameSyncedStreams[i].pFrame != NULL)
		{
			// Check if there is a stored frame which has older timestamp than allowed.
			XnUInt64 diff = (pFrame->timestamp > m_FrameSyncedStreams[i].pFrame->timestamp) ?
							 pFrame->timestamp - m_FrameSyncedStreams[i].pFrame->timestamp  :
							 m_FrameSyncedStreams[i].pFrame->timestamp - pFrame->timestamp;
			if (diff > FRAME_SYNC_MAX_FRAME_TIME_DIFF)
			{
				// Check if received frame has newer timestamp than stored one.
				if (pFrame->timestamp > m_FrameSyncedStreams[i].pFrame->timestamp)
				{
					// Release the last frame.
					m_FrameSyncedStreams[i].pStream->ReleaseFrame(m_FrameSyncedStreams[i].pFrame);
					m_FrameSyncedStreams[i].pFrame = NULL;
				}
				// Newest frame should be released.
				else
				{
					// Check whether frame was updated in the relevant frame synced stream.
					if (pFrameSyncedStream != NULL)
					{
						// Release the stored new frame.
						pFrameSyncedStream->pStream->ReleaseFrame(pFrameSyncedStream->pFrame);
						pFrameSyncedStream->pFrame = NULL;
						nValidFrameCount--;
					}
					else
					{
						// Release the new frame.
						pFrame = NULL;
					}
				}
				break;
			}
			else
			{
				++nValidFrameCount;
			}
		}
	}

	// Check whether stream is frame synced.
	if (m_nFrameSyncEnabled && (pFrameSyncedStream != NULL))
	{
		// Check if all the frames arrived.
		if (nValidFrameCount == nFrameSyncStreamCount)
		{
			// Send all the frames.
			++m_nFrameSyncLastFrameID;
			for (XnUInt32 i = 0; i < nFrameSyncStreamCount; ++i)
			{
				// Send the frame.
				m_FrameSyncedStreams[i].pFrame->frameIndex = m_nFrameSyncLastFrameID;
				XnDeviceBase::OnNewStreamData(m_FrameSyncedStreams[i].pStream, m_FrameSyncedStreams[i].pFrame);
				m_FrameSyncedStreams[i].pStream->ReleaseFrame(m_FrameSyncedStreams[i].pFrame);
				m_FrameSyncedStreams[i].pFrame = NULL;
			}
		}

		// Unlock critical section.
		m_frameSyncCs.Unlock();
	}
	else
	{
		// Unlock critical section.
		m_frameSyncCs.Unlock();

		// Set frame ID to higher of self and frame ID, to make sure IDs are incremental.
		m_nFrameSyncLastFrameID = (m_nFrameSyncLastFrameID < receivedFrameId) ? receivedFrameId : m_nFrameSyncLastFrameID;

		if (pFrame != NULL)
		{
			// Send the frame (it is not frame-synced).
			XnDeviceBase::OnNewStreamData(pStream, pFrame);
		}
	}
}

XnStatus XnSensor::SetFrameSyncStreamGroup(XnDeviceStream** ppStreamList, XnUInt32 numStreams)
{
	// Lock critical section.
	m_frameSyncCs.Lock();

	// Set the frame sync property in the device.
	XnStatus rc = SetProperty(XN_MODULE_NAME_DEVICE, XN_MODULE_PROPERTY_FRAME_SYNC, 
							  (XnUInt64)((numStreams > 0) ? TRUE : FALSE));
	if (rc != XN_STATUS_OK)
	{
		// Unlock critical section.
		m_frameSyncCs.Unlock();
		return rc;
	}

	// Clear all the streams from frame-sync list.
	XnUInt32 nFrameSyncStreamCount = m_FrameSyncedStreams.GetSize();
	for (XnUInt32 i = 0; i < nFrameSyncStreamCount; ++i)
	{
		// Release stored frame.
		if (m_FrameSyncedStreams[i].pFrame != NULL)
		{
			// Release the frame.
			m_FrameSyncedStreams[i].pStream->ReleaseFrame(m_FrameSyncedStreams[i].pFrame);
			m_FrameSyncedStreams[i].pFrame = NULL;
		}

		// Clear pointer to stream, timestamp and frame ID.
		m_FrameSyncedStreams[i].pStream = NULL;
	}

	// Check if creating group.
	if (numStreams > 0)
	{
		m_FrameSyncedStreams.SetSize(numStreams);
		
		// Add streams to frame-sync list.
		for (XnUInt32 i = 0; i < numStreams; ++i)
		{
			m_FrameSyncedStreams[i].pStream = ppStreamList[i];
			m_FrameSyncedStreams[i].pFrame = NULL;
		}
	}
	else
	{
		// Zero the size of the frame sync group.
		m_FrameSyncedStreams.SetSize(0);
	}

	// Unlock critical section.
	m_frameSyncCs.Unlock();
	
	return XN_STATUS_OK;
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetInterfaceCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->XnSensor::SetInterface((XnSensorUsbInterface)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetHostTimestampsCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->XnSensor::SetHostTimestamps(nValue == 1);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetReadDataCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->XnSensor::SetReadData((XnBool)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFirmwareParamCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnInnerParamData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFirmwareParam((const XnInnerParamData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetCmosBlankingUnitsCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnCmosBlankingUnits);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetCmosBlankingUnits((const XnCmosBlankingUnits*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetCmosBlankingTimeCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnCmosBlankingTime);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetCmosBlankingTime((const XnCmosBlankingTime*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::ResetCallback(XnIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->Reset((XnParamResetType)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFirmwareModeCallback(XnIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFirmwareMode((XnParamCurrentMode)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetFirmwareParamCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnInnerParamData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetFirmwareParam((XnInnerParamData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetCmosBlankingUnitsCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnCmosBlankingUnits);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetCmosBlankingUnits((XnCmosBlankingUnits*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetCmosBlankingTimeCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnCmosBlankingTime);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetCmosBlankingTime((XnCmosBlankingTime*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetFirmwareModeCallback(const XnIntProperty* /*pSender*/, XnUInt64* pnValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	XnParamCurrentMode nMode;
	XnStatus nRetVal = pThis->GetFirmwareMode(&nMode);
	XN_IS_STATUS_OK(nRetVal);

	*pnValue = nMode;
	return XN_STATUS_OK;
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetAudioSupportedCallback(const XnIntProperty* /*pSender*/, XnUInt64* pnValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	*pnValue = pThis->m_Firmware.GetInfo()->bAudioSupported;
	return XN_STATUS_OK;
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetImageSupportedCallback(const XnIntProperty* /*pSender*/, XnUInt64* pnValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	*pnValue = pThis->m_Firmware.GetInfo()->bImageSupported;
	return XN_STATUS_OK;
}

XnStatus XN_CALLBACK_TYPE XnSensor::FrameSyncPropertyChangedCallback(const XnProperty* /*pSender*/, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->OnFrameSyncPropertyChanged();
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetFixedParamsCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnDynamicSizeBuffer);
	XnSensor* pThis = (XnSensor*)pCookie;
	XnDynamicSizeBuffer* pBuffer = (XnDynamicSizeBuffer*)gbValue.data;
	return pThis->GetFixedParams(pBuffer);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetInstanceCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	if (gbValue.dataSize != sizeof(void*))
	{
		return XN_STATUS_DEVICE_PROPERTY_SIZE_DONT_MATCH;
	}

	*(void**)gbValue.data = pCookie;
	return XN_STATUS_OK;
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetDepthCmosRegisterCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnControlProcessingData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetDepthCmosRegister((const XnControlProcessingData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetImageCmosRegisterCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnControlProcessingData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetImageCmosRegister((const XnControlProcessingData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetDepthCmosRegisterCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnControlProcessingData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetDepthCmosRegister((XnControlProcessingData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetImageCmosRegisterCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnControlProcessingData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetImageCmosRegister((XnControlProcessingData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::WriteAHBCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnAHBData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->WriteAHB((const XnAHBData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::ReadAHBCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnAHBData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->ReadAHB((XnAHBData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetLedStateCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnLedState);
	XnSensor* pThis = (XnSensor*)pCookie;
	const XnLedState* pLedState = (const XnLedState*)gbValue.data;
	return pThis->SetLedState(pLedState->nLedID, pLedState->nState);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetEmitterStateCallback(XnIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetEmitterState(nValue == TRUE);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFirmwareFrameSyncCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFirmwareFrameSync(nValue == TRUE);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFirmwareLogFilterCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFirmwareLogFilter((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFirmwareLogIntervalCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFirmwareLogInterval((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFirmwareLogPrintCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFirmwareLogPrint((XnBool)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFirmwareCPUIntervalCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFirmwareCPUInterval((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetAPCEnabledCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetAPCEnabled((XnBool)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetI2CCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnI2CWriteData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetI2C((const XnI2CWriteData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::DeleteFileCallback(XnIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->DeleteFile((XnUInt16)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetTecSetPointCallback(XnIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetTecSetPoint((XnUInt16)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetEmitterSetPointCallback(XnIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetEmitterSetPoint((XnUInt16)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetFileAttributesCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnFileAttributes);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetFileAttributes((const XnFileAttributes*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::WriteFlashFileCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnParamFileData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->WriteFlashFile((const XnParamFileData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::SetProjectorFaultCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnProjectorFaultData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->SetProjectorFault((XnProjectorFaultData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetI2CCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnI2CReadData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetI2C((XnI2CReadData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetTecStatusCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnTecData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetTecStatus((XnTecData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetTecFastConvergenceStatusCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnTecFastConvergenceData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetTecFastConvergenceStatus((XnTecFastConvergenceData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetEmitterStatusCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnEmitterData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetEmitterStatus((XnEmitterData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::ReadFlashFileCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnParamFileData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->ReadFlashFile((const XnParamFileData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetFirmwareLogCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetFirmwareLog((XnChar*)gbValue.data, gbValue.dataSize);
}

XnStatus XN_CALLBACK_TYPE XnSensor::ReadFlashChunkCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnParamFlashData);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->ReadFlashChunk((XnParamFlashData*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::GetFileListCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnFlashFileList);
	XnSensor* pThis = (XnSensor*)pCookie;
	return pThis->GetFileList((XnFlashFileList*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnSensor::RunBISTCallback(XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XN_VALIDATE_GENERAL_BUFFER_TYPE(gbValue, XnBist);
	XnSensor* pThis = (XnSensor*)pCookie;
	XnBist* pBist = (XnBist*)gbValue.data;
	XnStatus nRetVal = pThis->RunBIST(pBist->nTestsMask, &pBist->nFailures);
	XN_IS_STATUS_OK(nRetVal);
	return XN_STATUS_OK;
}

void XN_CALLBACK_TYPE XnSensor::OnDeviceDisconnected(const OniDeviceInfo& deviceInfo, void* pCookie)
{
	XnSensor* pThis = (XnSensor*)pCookie;
	if (xnOSStrCmp(deviceInfo.uri, pThis->GetUSBPath()) == 0)
	{
		pThis->SetErrorState(XN_STATUS_DEVICE_NOT_CONNECTED);
	}
}

