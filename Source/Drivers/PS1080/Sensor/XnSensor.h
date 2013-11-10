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
#ifndef XNSENSOR_H
#define XNSENSOR_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnDeviceBase.h>
#include "XnDeviceSensorIO.h"
#include "XnParams.h"
#include "XnDeviceSensor.h"
#include "XnSensorFixedParams.h"
#include "XnSensorFirmwareParams.h"
#include <DDK/XnDeviceStream.h>
#include "XnSensorFirmware.h"
#include "XnCmosInfo.h"
#include "IXnSensorStream.h"
#include <DDK/XnIntPropertySynchronizer.h>
#include "XnArray.h"

//---------------------------------------------------------------------------
// XnSensor class
//---------------------------------------------------------------------------
class XnSensor : public XnDeviceBase
{
	friend class XnServerSensorInvoker;

public:
	XnSensor(XnBool bResetOnStartup = TRUE, XnBool bLeanInit = FALSE);
	~XnSensor();

	virtual XnStatus InitImpl(const XnDeviceConfig* pDeviceConfig);
	virtual XnStatus Destroy();
	virtual XnStatus OpenAllStreams();
	virtual XnStatus LoadConfigFromFile(const XnChar* csINIFilePath, const XnChar* csSectionName);

public:
	inline XnSensorFixedParams* GetFixedParams() { return GetFirmware()->GetFixedParams(); }
	inline XnSensorFirmware* GetFirmware() { return &m_Firmware; }
	inline XnSensorFPS* GetFPSCalculator() { return &m_FPS; }

	XnStatus SetCmosConfiguration(XnCMOSType nCmos, XnResolutions nRes, XnUInt32 nFPS);

	inline XnDevicePrivateData* GetDevicePrivateData() { return &m_DevicePrivateData; }

	XnStatus ConfigPropertyFromFile(XnStringProperty* pProperty, const XnChar* csINIFilePath, const XnChar* csSectionName);
	XnStatus ConfigPropertyFromFile(XnIntProperty* pProperty, const XnChar* csINIFilePath, const XnChar* csSectionName);

	inline XnBool IsMiscSupported() const { return m_SensorIO.IsMiscEndpointSupported(); }
	inline XnBool IsLowBandwidth() const { return m_SensorIO.IsLowBandwidth(); }
	inline XnSensorUsbInterface GetCurrentUsbInterface() const { return m_SensorIO.GetCurrentInterface(*m_Firmware.GetInfo()); }

	XnStatus GetStream(const XnChar* strStream, XnDeviceStream** ppStream);

	inline XnStatus GetErrorState() { return (XnStatus)m_ErrorState.GetValue(); }
	XnStatus SetErrorState(XnStatus errorState);

	/**
	 * Resolves the config file's path.
	 * Specify NULL to strConfigDir to resolve it based on the driver's directory.
	 */
	static XnStatus ResolveGlobalConfigFileName(XnChar* strConfigFile, XnUInt32 nBufSize, const XnChar* strConfigDir);

	XnStatus SetGlobalConfigFile(const XnChar* strConfigFile);
	XnStatus ConfigureModuleFromGlobalFile(const XnChar* strModule, const XnChar* strSection = NULL);

	const XnChar* GetUSBPath() { return m_SensorIO.GetDevicePath(); }
	XnBool ShouldUseHostTimestamps() { return (m_HostTimestamps.GetValue() == TRUE); }
	XnBool HasReadingStarted() { return (m_ReadData.GetValue() == TRUE); }
	inline XnBool IsTecDebugPring() const { return (XnBool)m_FirmwareTecDebugPrint.GetValue(); }

	XnStatus SetFrameSyncStreamGroup(XnDeviceStream** ppStreamList, XnUInt32 numStreams);

protected:
	virtual XnStatus CreateStreamImpl(const XnChar* strType, const XnChar* strName, const XnActualPropertiesHash* pInitialSet);

	XnStatus CreateDeviceModule(XnDeviceModuleHolder** ppModuleHolder);
	XnStatus CreateStreamModule(const XnChar* StreamType, const XnChar* StreamName, XnDeviceModuleHolder** ppStream);
	void DestroyStreamModule(XnDeviceModuleHolder* pStreamHolder);

	virtual void OnNewStreamData(XnDeviceStream* pStream, OniFrame* pFrame);

private:
	XnStatus InitSensor(const XnDeviceConfig* pDeviceConfig);
	XnStatus ValidateSensorID(XnChar* csSensorID);
	XnStatus SetMirrorForModule(XnDeviceModule* pModule, XnUInt64 nValue);
	XnStatus FindSensorStream(const XnChar* StreamName, IXnSensorStream** ppStream);
	XnStatus InitReading();
	XnStatus OnFrameSyncPropertyChanged();

	static XnStatus XN_CALLBACK_TYPE GetInstanceCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);

	XnStatus ChangeTaskInterval(XnScheduledTask** ppTask, XnTaskCallbackFuncPtr pCallback, XnUInt32 nInterval);
	void ReadFirmwareLog();
	void ReadFirmwareCPU();

	//---------------------------------------------------------------------------
	// Getters
	//---------------------------------------------------------------------------
	XnStatus GetFirmwareParam(XnInnerParamData* pParam);
	XnStatus GetCmosBlankingUnits(XnCmosBlankingUnits* pBlanking);
	XnStatus GetCmosBlankingTime(XnCmosBlankingTime* pBlanking);
	XnStatus GetFirmwareMode(XnParamCurrentMode* pnMode);
	XnStatus GetLastRawFrame(const XnChar* strStream, XnUChar* pBuffer, XnUInt32 nDataSize);
	XnStatus GetFixedParams(XnDynamicSizeBuffer* pBuffer);
	XnStatus GetDepthCmosRegister(XnControlProcessingData* pRegister);
	XnStatus GetImageCmosRegister(XnControlProcessingData* pRegister);
	XnStatus ReadAHB(XnAHBData* pData);
	XnStatus GetI2C(XnI2CReadData* pI2CReadData);
	XnStatus GetTecStatus(XnTecData* pTecData);
	XnStatus GetTecFastConvergenceStatus(XnTecFastConvergenceData* pTecData);
	XnStatus GetEmitterStatus(XnEmitterData* pEmitterData);
	XnStatus ReadFlashFile(const XnParamFileData* pFile);
	XnStatus ReadFlashChunk(XnParamFlashData* pFlash);
	XnStatus GetFirmwareLog(XnChar* csLog, XnUInt32 nSize);
	XnStatus GetFileList(XnFlashFileList* pFileList);

	//---------------------------------------------------------------------------
	// Setters
	//---------------------------------------------------------------------------
	XnStatus SetInterface(XnSensorUsbInterface nInterface);
	XnStatus SetHostTimestamps(XnBool bHostTimestamps);
	XnStatus SetNumberOfBuffers(XnUInt32 nCount);
	XnStatus SetReadData(XnBool bRead);
	XnStatus SetFirmwareParam(const XnInnerParamData* pParam);
	XnStatus SetCmosBlankingUnits(const XnCmosBlankingUnits* pBlanking);
	XnStatus SetCmosBlankingTime(const XnCmosBlankingTime* pBlanking);
	XnStatus Reset(XnParamResetType nType);
	XnStatus SetFirmwareMode(XnParamCurrentMode nMode);
	XnStatus SetDepthCmosRegister(const XnControlProcessingData* pRegister);
	XnStatus SetImageCmosRegister(const XnControlProcessingData* pRegister);
	XnStatus WriteAHB(const XnAHBData* pData);
	XnStatus SetLedState(XnUInt16 nLedId, XnUInt16 nState);
	XnStatus SetEmitterState(XnBool bActive);
	XnStatus SetFirmwareFrameSync(XnBool bOn);
	XnStatus SetI2C(const XnI2CWriteData* pI2CWriteData);
	XnStatus SetFirmwareLogFilter(XnUInt32 nFilter);
	XnStatus SetFirmwareLogInterval(XnUInt32 nMilliSeconds);
	XnStatus SetFirmwareLogPrint(XnBool bPrint);
	XnStatus SetFirmwareCPUInterval(XnUInt32 nMilliSeconds);
	XnStatus SetAPCEnabled(XnBool bEnabled);
	XnStatus DeleteFile(XnUInt16 nFileID);
	XnStatus SetTecSetPoint(XnUInt16 nSetPoint);
	XnStatus SetEmitterSetPoint(XnUInt16 nSetPoint);
	XnStatus SetFileAttributes(const XnFileAttributes* pAttributes);
	XnStatus WriteFlashFile(const XnParamFileData* pFile);
	XnStatus SetProjectorFault(XnProjectorFaultData* pProjectorFaultData);
	XnStatus RunBIST(XnUInt32 nTestsMask, XnUInt32* pnFailures);
	XnStatus SetReadAllEndpoints(XnBool bEnabled);

	//---------------------------------------------------------------------------
	// Callbacks
	//---------------------------------------------------------------------------
	static void XN_CALLBACK_TYPE OnDeviceDisconnected(const OniDeviceInfo& deviceInfo, void* pCookie);

	static XnStatus XN_CALLBACK_TYPE SetInterfaceCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetHostTimestampsCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetNumberOfBuffersCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetReadDataCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareParamCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetCmosBlankingUnitsCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetCmosBlankingTimeCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ResetCallback(XnIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareModeCallback(XnIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetFixedParamsCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE FrameSyncPropertyChangedCallback(const XnProperty* pSender, void* pCookie);
	static XnBool XN_CALLBACK_TYPE HasSynchedFrameArrived(void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetFirmwareParamCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetCmosBlankingUnitsCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetCmosBlankingTimeCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetFirmwareModeCallback(const XnIntProperty* pSender, XnUInt64* pnValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetAudioSupportedCallback(const XnIntProperty* pSender, XnUInt64* pnValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetImageSupportedCallback(const XnIntProperty* pSender, XnUInt64* pnValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetDepthCmosRegisterCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetImageCmosRegisterCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetDepthCmosRegisterCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetImageCmosRegisterCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ReadAHBCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE WriteAHBCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetLedStateCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetEmitterStateCallback(XnIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareFrameSyncCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareLogFilterCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareLogIntervalCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareLogPrintCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFirmwareCPUIntervalCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetReadAllEndpointsCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetAPCEnabledCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetI2CCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE DeleteFileCallback(XnIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetTecSetPointCallback(XnIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetEmitterSetPointCallback(XnIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetFileAttributesCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE WriteFlashFileCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetProjectorFaultCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE RunBISTCallback(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetFileListCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static void XN_CALLBACK_TYPE ExecuteFirmwareLogTask(void* pCookie);
	static void XN_CALLBACK_TYPE ExecuteFirmwareCPUTask(void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetI2CCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetTecStatusCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetTecFastConvergenceStatusCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetEmitterStatusCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ReadFlashFileCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetFirmwareLogCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ReadFlashChunkCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);

	//---------------------------------------------------------------------------
	// Members
	//---------------------------------------------------------------------------
	XnCallbackHandle m_hDisconnectedCallback;
	XnActualIntProperty m_ErrorState;
	XnActualIntProperty m_ResetSensorOnStartup;
	XnActualIntProperty m_LeanInit;
	XnActualIntProperty m_Interface;
	XnActualIntProperty m_ReadData;
	XnActualIntProperty m_FrameSync;
	XnActualIntProperty m_FirmwareFrameSync;
	XnActualIntProperty m_CloseStreamsOnShutdown;
	XnActualIntProperty m_HostTimestamps;
	XnGeneralProperty m_FirmwareParam;
	XnGeneralProperty m_CmosBlankingUnits;
	XnGeneralProperty m_CmosBlankingTime;
	XnIntProperty m_Reset;
	XnVersions m_VersionData;
	XnActualGeneralProperty m_Version;
	XnGeneralProperty m_FixedParam;
	XnActualStringProperty m_ID;
	XnActualStringProperty m_DeviceName;
	XnActualStringProperty m_VendorSpecificData;
	XnActualStringProperty m_PlatformString;
	XnIntProperty m_AudioSupported;
	XnIntProperty m_ImageSupported;
	XnGeneralProperty m_ImageControl;
	XnGeneralProperty m_DepthControl;
	XnGeneralProperty m_AHB;
	XnGeneralProperty m_LedState;
	XnIntProperty m_EmitterEnabled;
	XnActualIntProperty m_FirmwareLogFilter;
	XnActualIntProperty m_FirmwareLogInterval;
	XnActualIntProperty m_FirmwareLogPrint;
	XnActualIntProperty m_FirmwareCPUInterval;
	XnActualIntProperty m_APCEnabled;
	XnActualIntProperty m_FirmwareTecDebugPrint;
	XnActualIntProperty m_ReadAllEndpoints;
	XnGeneralProperty m_I2C;
	XnIntProperty m_DeleteFile;
	XnIntProperty m_TecSetPoint;
	XnGeneralProperty m_TecStatus;
	XnGeneralProperty m_TecFastConvergenceStatus;
	XnIntProperty m_EmitterSetPoint;
	XnGeneralProperty m_EmitterStatus;
	XnGeneralProperty m_FileAttributes;
	XnGeneralProperty m_FlashFile;
	XnGeneralProperty m_FirmwareLog;
	XnGeneralProperty m_FlashChunk;
	XnGeneralProperty m_FileList;
	XnGeneralProperty m_BIST;
	XnGeneralProperty m_ProjectorFault;
	XnSensorFirmware m_Firmware;
	XnDevicePrivateData m_DevicePrivateData;
	XnSensorFPS m_FPS;
	XnCmosInfo m_CmosInfo;
	XnSensorIO m_SensorIO;

	XnSensorObjects m_Objects;

	/** A scheduler to be used for performing periodic tasks. */
	XnScheduler* m_pScheduler;
	XnScheduledTask* m_pLogTask;
	XnScheduledTask* m_pCPUTask;
	XnDumpFile* m_FirmwareLogDump;
	XnDumpFile* m_FrameSyncDump;
	XnBool m_nFrameSyncEnabled;
	typedef struct  
	{
		XnDeviceStream* pStream;
		OniFrame* pFrame;
	} FrameSyncedStream;
	xnl::Array<FrameSyncedStream> m_FrameSyncedStreams;
	int m_nFrameSyncLastFrameID;
	xnl::CriticalSection m_frameSyncCs;

	XnBool m_bInitialized;

	XnIntPropertySynchronizer m_PropSynchronizer;

	XnChar m_strGlobalConfigFile[XN_FILE_MAX_PATH];
};

#endif // XNSENSOR_H
