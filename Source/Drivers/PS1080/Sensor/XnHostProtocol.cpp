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
#include "XnDeviceSensorProtocol.h"
#include "XnHostProtocol.h"
#include <math.h>
#include <XnLog.h>
#include "XnSensorDepthStream.h"
#include "XnSensor.h"
#include <XnPsVersion.h>

// Control Protocol
#include "XnParams.h"

#define XN_RECEIVE_USB_DATA_TIMEOUT 20000
#define XN_USB_HOST_PROTOCOL_TIMEOUT 5000
#define XN_USB_HOST_PROTOCOL_TIMEOUT_KEEP_ALIVE 5000
#define XN_USB_HOST_PROTOCOL_TIMEOUT_GETVERSION 5000
#define XN_USB_HOST_PROTOCOL_TIMEOUT_SETPARAM 5000

#define XN_USB_HOST_PROTOCOL_TIMEOUT_UPLOAD 180000
#define XN_USB_HOST_PROTOCOL_TIMEOUT_FILE_OPS 180000
#define XN_USB_HOST_PROTOCOL_TIMEOUT_BIST 300000
#define XN_USB_HOST_PROTOCOL_TIMEOUT_EMITTER_DATA 60000

#define XN_USB_HOST_PROTOCOL_FILE_UPLOAD_PRE_DELAY 250
#define XN_USB_HOST_PROTOCOL_FILE_UPLOAD_POST_DELAY 0

#define XN_LOG_TEXT_MESSAGE_V1_2	0x1000 
#define XN_LOG_OVERFLOW_V1_2		0x1001

#define XN_LOG_TEXT_MESSAGE_V3_0	0x1200
#define XN_LOG_OVERFLOW_V3_0		0x1201

#define XN_LOG_TEXT_MESSAGE_V4_0	0x1200
#define XN_LOG_OVERFLOW_V4_0		0x1201

#define XN_LOG_TEXT_MESSAGE_V5_0	0x5400
#define XN_LOG_OVERFLOW_V5_0		0x5401

#define XN_USB_HOST_PROTOCOL_SEND_RETRIES	5
#define XN_HOST_PROTOCOL_NOT_READY_RETRIES	3

#define XN_PROTOCOL_MAX_PACKET_SIZE_V5_0	512
#define XN_PROTOCOL_MAX_PACKET_SIZE_V0_17	64

#define MAX_PACKET_SIZE 512

inline XnInt32 CompareVersion(XnUInt8 nMajor1, XnUInt8 nMinor1, XnUInt16 nBuild1, XnUInt8 nMajor2, XnUInt8 nMinor2, XnUInt16 nBuild2)
{
	XnInt32 nResult = nMajor1 - nMajor2;

	if (nResult == 0)
	{
		nResult = nMinor1 - nMinor2;
	}

	if (nResult == 0)
	{
		nResult = nBuild1 - nBuild2;
	}

	return (nResult);
}

static XnFWVer GetFWVersion(XnUInt8 nMajor, XnUInt8 nMinor, XnUInt16 nBuild)
{
	if (CompareVersion(nMajor, nMinor, nBuild, 5, 8, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_8;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 7, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_7;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 6, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_6;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 5, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_5;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 4, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_4;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_3;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 2, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_2;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 1, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_1;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 5, 0, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_5_0;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 4, 0, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_4_0;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 3, 0, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_3_0;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 1, 2, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_1_2;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 1, 2, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_1_2;
	}
	else if (CompareVersion(nMajor, nMinor, nBuild, 1, 1, 0) >= 0)
	{
		return XN_SENSOR_FW_VER_1_1;
	}
	else
	{
		return XN_SENSOR_FW_VER_0_17;
	}
}

XnStatus XnHostProtocolUpdateSupportedImageModes(XnDevicePrivateData* pDevicePrivateData)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if (pDevicePrivateData->FWInfo.bGetPresetsSupported)
	{
		// ask the firmware
		const XnUInt32 nAllocSize = 100;
		XnUInt32 nCount = nAllocSize;
		XnCmosPreset aSupportedModes[nAllocSize];
		nRetVal = XnHostProtocolGetCmosPresets(pDevicePrivateData, XN_CMOS_TYPE_IMAGE, aSupportedModes, nCount);
		XN_IS_STATUS_OK(nRetVal);

		if (nCount == 0)
		{
			xnLogError(XN_MASK_DEVICE_SENSOR, "Device does not support any image mode!");
			return XN_STATUS_DEVICE_UNSUPPORTED_PARAMETER;
		}

		nRetVal = pDevicePrivateData->FWInfo.imageModes.SetData(aSupportedModes, nCount);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		// old firmware. Just use what we know
		switch (pDevicePrivateData->pSensor->GetCurrentUsbInterface())
		{
		case XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS:
			nRetVal = pDevicePrivateData->FWInfo.imageModes.SetData(pDevicePrivateData->FWInfo._imageBulkModes.GetData(), pDevicePrivateData->FWInfo._imageBulkModes.GetSize());
			XN_IS_STATUS_OK(nRetVal);
			break;
		case XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS:
			nRetVal = pDevicePrivateData->FWInfo.imageModes.SetData(pDevicePrivateData->FWInfo._imageIsoModes.GetData(), pDevicePrivateData->FWInfo._imageIsoModes.GetSize());
			XN_IS_STATUS_OK(nRetVal);
			break;
		default:
			xnLogError(XN_MASK_DEVICE_SENSOR, "Unknown interface in old firmware (%d)", pDevicePrivateData->pSensor->GetCurrentUsbInterface());
			XN_ASSERT(FALSE);
			return XN_STATUS_ERROR;
		}
	}

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolInitFWParams(XnDevicePrivateData* pDevicePrivateData, XnUInt8 nMajor, XnUInt8 nMinor, XnUInt16 nBuild, XnHostProtocolUsbCore usb, XnBool bGuessed)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// we start with oldest settings (FW version 0.17), and change them for newer versions
	pDevicePrivateData->FWInfo.nFWMagic = XN_FW_MAGIC_25;
	pDevicePrivateData->FWInfo.nHostMagic = XN_HOST_MAGIC_25;
	pDevicePrivateData->FWInfo.nProtocolHeaderSize = sizeof(XnHostProtocolHeaderV25);
	pDevicePrivateData->FWInfo.nProtocolMaxPacketSize = XN_PROTOCOL_MAX_PACKET_SIZE_V0_17;
	pDevicePrivateData->FWInfo.bAudioSupported = FALSE;
	pDevicePrivateData->FWInfo.bMirrorSupported = FALSE;
	pDevicePrivateData->FWInfo.bGetPresetsSupported = FALSE;
	pDevicePrivateData->FWInfo.bDeviceInfoSupported = FALSE;
	pDevicePrivateData->FWInfo.bImageAdjustmentsSupported = FALSE;

	pDevicePrivateData->FWInfo.nOpcodeGetVersion = OPCODE_V017_GET_VERSION;
	pDevicePrivateData->FWInfo.nOpcodeKeepAlive = OPCODE_V017_KEEP_ALIVE;
	pDevicePrivateData->FWInfo.nOpcodeGetParam = OPCODE_V017_GET_PARAM;
	pDevicePrivateData->FWInfo.nOpcodeSetParam = OPCODE_V017_SET_PARAM;
	pDevicePrivateData->FWInfo.nOpcodeGetFixedParams = OPCODE_V017_GET_FIXED_PARAMS;
	pDevicePrivateData->FWInfo.nOpcodeGetMode = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeSetMode = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeAlgorithmParams = OPCODE_V017_ALGORITM_PARAMS;
	pDevicePrivateData->FWInfo.nOpcodeReset = OPCODE_V017_RESET;
	pDevicePrivateData->FWInfo.nOpcodeSetCmosBlanking = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetCmosBlanking = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetCmosPresets = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetSerialNumber = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetFastConvergenceTEC = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetCMOSReg = OPCODE_V017_GET_CMOS_REGISTER;
	pDevicePrivateData->FWInfo.nOpcodeSetCMOSReg = OPCODE_V017_SET_CMOS_REGISTER;
	pDevicePrivateData->FWInfo.nOpcodeWriteI2C = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeReadI2C = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeReadAHB = OPCODE_V017_READ_AHB;
	pDevicePrivateData->FWInfo.nOpcodeWriteAHB = OPCODE_V017_WRITE_AHB;
	pDevicePrivateData->FWInfo.nOpcodeGetPlatformString = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetUsbCore = OPCODE_GET_USB_CORE_TYPE;
	pDevicePrivateData->FWInfo.nOpcodeSetLedState = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeEnableEmitter = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetLog = OPCODE_V017_GET_LOG;
	pDevicePrivateData->FWInfo.nOpcodeTakeSnapshot = OPCODE_V017_TAKE_SNAPSHOT;
	pDevicePrivateData->FWInfo.nOpcodeInitFileUpload = OPCODE_V017_INIT_FILE_UPLOAD;
	pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload = OPCODE_V017_WRITE_FILE_UPLOAD;
	pDevicePrivateData->FWInfo.nOpcodeFinishFileUpload = OPCODE_V017_FINISH_FILE_UPLOAD;
	pDevicePrivateData->FWInfo.nOpcodeDownloadFile = OPCODE_V017_DOWNLOAD_FILE;
	pDevicePrivateData->FWInfo.nOpcodeDeleteFile = OPCODE_V017_DELETE_FILE;
	pDevicePrivateData->FWInfo.nOpcodeGetFlashMap = OPCODE_V017_GET_FLASH_MAP;
	pDevicePrivateData->FWInfo.nOpcodeGetFileList = OPCODE_V017_GET_FILE_LIST;
	pDevicePrivateData->FWInfo.nOpcodeSetFileAttribute = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeExecuteFile = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeReadFlash = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeBIST = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeSetGMCParams = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetCPUStats = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeCalibrateTec = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetTecData = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeCalibrateEmitter = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeGetEmitterData = OPCODE_INVALID;
	pDevicePrivateData->FWInfo.nOpcodeCalibrateProjectorFault = OPCODE_INVALID;

	pDevicePrivateData->FWInfo.nLogStringType = XN_LOG_TEXT_MESSAGE_V1_2;
	pDevicePrivateData->FWInfo.nLogOverflowType = XN_LOG_OVERFLOW_V1_2;

	pDevicePrivateData->FWInfo.nUSBDelayReceive = 100;
	pDevicePrivateData->FWInfo.nUSBDelayExecutePreSend = 1;
	pDevicePrivateData->FWInfo.nUSBDelayExecutePostSend = 10;
	pDevicePrivateData->FWInfo.nUSBDelaySoftReset = 800;
	pDevicePrivateData->FWInfo.nUSBDelaySetParamFlicker = 3000;
	pDevicePrivateData->FWInfo.nUSBDelaySetParamStream0Mode = 1;
	pDevicePrivateData->FWInfo.nUSBDelaySetParamStream1Mode = 300;
	pDevicePrivateData->FWInfo.nUSBDelaySetParamStream2Mode = 1;

	pDevicePrivateData->FWInfo.bGetImageCmosTypeSupported = FALSE;
	pDevicePrivateData->FWInfo.bImageSupported = TRUE;
	pDevicePrivateData->FWInfo.bIncreasedFpsCropSupported = FALSE;
	pDevicePrivateData->FWInfo.bHasFilesystemLock = FALSE;

	pDevicePrivateData->FWInfo.nISOLowDepthAlternativeInterface = (XnUInt8)(-1);

	// depth cmos modes
	pDevicePrivateData->FWInfo.depthModes.Clear();
	XnCmosPreset depthModes[] = 
	{
		{ XN_IO_DEPTH_FORMAT_COMPRESSED_PS, XN_RESOLUTION_QVGA, 30 },
		{ XN_IO_DEPTH_FORMAT_COMPRESSED_PS, XN_RESOLUTION_QVGA, 60 },
		{ XN_IO_DEPTH_FORMAT_COMPRESSED_PS, XN_RESOLUTION_VGA, 30 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_11_BIT, XN_RESOLUTION_QVGA, 30 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_11_BIT, XN_RESOLUTION_QVGA, 60 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_11_BIT, XN_RESOLUTION_VGA, 30 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_12_BIT, XN_RESOLUTION_QVGA, 30 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_12_BIT, XN_RESOLUTION_QVGA, 60 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_12_BIT, XN_RESOLUTION_VGA, 30 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_16_BIT, XN_RESOLUTION_QVGA, 30 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_16_BIT, XN_RESOLUTION_QVGA, 60 },
		{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_16_BIT, XN_RESOLUTION_VGA, 30 },
	};
	nRetVal = pDevicePrivateData->FWInfo.depthModes.AddLast(depthModes, sizeof(depthModes)/sizeof(depthModes[0]));
	XN_IS_STATUS_OK(nRetVal);

	// image cmos modes
	pDevicePrivateData->FWInfo._imageBulkModes.Clear();
	pDevicePrivateData->FWInfo._imageIsoModes.Clear();

	XnCmosPreset imageCommonModes[] =
	{
		{ XN_IO_IMAGE_FORMAT_YUV422, XN_RESOLUTION_QVGA, 30 },
		{ XN_IO_IMAGE_FORMAT_YUV422, XN_RESOLUTION_QVGA, 60 },
		{ XN_IO_IMAGE_FORMAT_YUV422, XN_RESOLUTION_VGA, 30 },
	};
	nRetVal = pDevicePrivateData->FWInfo._imageBulkModes.AddLast(imageCommonModes, sizeof(imageCommonModes)/sizeof(imageCommonModes[0]));
	XN_IS_STATUS_OK(nRetVal);
	nRetVal = pDevicePrivateData->FWInfo._imageIsoModes.AddLast(imageCommonModes, sizeof(imageCommonModes)/sizeof(imageCommonModes[0]));
	XN_IS_STATUS_OK(nRetVal);

	XnCmosPreset imageIsoModes[] =
	{
		{ XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_RESOLUTION_QVGA, 30 },
		{ XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_RESOLUTION_QVGA, 60 },
		{ XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_RESOLUTION_VGA, 30 },
	};
	nRetVal = pDevicePrivateData->FWInfo._imageIsoModes.AddLast(imageIsoModes, sizeof(imageIsoModes)/sizeof(imageIsoModes[0]));
	XN_IS_STATUS_OK(nRetVal);

	// IR cmos modes
	pDevicePrivateData->FWInfo.irModes.Clear();
	XnCmosPreset irModes[] = 
	{
		{ 0, XN_RESOLUTION_QVGA, 30 },
		{ 0, XN_RESOLUTION_QVGA, 60 },
		{ 0, XN_RESOLUTION_VGA, 30 },
	};
	nRetVal = pDevicePrivateData->FWInfo.irModes.AddLast(irModes, sizeof(irModes)/sizeof(irModes[0]));
	XN_IS_STATUS_OK(nRetVal);

	if (CompareVersion(nMajor, nMinor, nBuild, 1, 1, 0) >= 0)
	{
		// opcodes were changed
		pDevicePrivateData->FWInfo.nOpcodeGetVersion = OPCODE_V110_GET_VERSION;
		pDevicePrivateData->FWInfo.nOpcodeKeepAlive = OPCODE_V110_KEEP_ALIVE;
		pDevicePrivateData->FWInfo.nOpcodeGetParam = OPCODE_V110_GET_PARAM;
		pDevicePrivateData->FWInfo.nOpcodeSetParam = OPCODE_V110_SET_PARAM;
		pDevicePrivateData->FWInfo.nOpcodeGetFixedParams = OPCODE_V110_GET_FIXED_PARAMS;
		pDevicePrivateData->FWInfo.nOpcodeGetMode = OPCODE_V110_GET_MODE;
		pDevicePrivateData->FWInfo.nOpcodeSetMode = OPCODE_V110_SET_MODE;
		pDevicePrivateData->FWInfo.nOpcodeAlgorithmParams = OPCODE_V110_ALGORITHM_PARAMS;
		pDevicePrivateData->FWInfo.nOpcodeReset = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeSetCmosBlanking = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCmosBlanking = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCmosPresets = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetSerialNumber = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetFastConvergenceTEC = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCMOSReg = OPCODE_V110_GET_CMOS_REGISTER;
		pDevicePrivateData->FWInfo.nOpcodeSetCMOSReg = OPCODE_V110_SET_CMOS_REGISTER;
		pDevicePrivateData->FWInfo.nOpcodeWriteI2C = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeReadI2C = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeReadAHB = OPCODE_V110_READ_AHB;
		pDevicePrivateData->FWInfo.nOpcodeWriteAHB = OPCODE_V110_WRITE_AHB;
		pDevicePrivateData->FWInfo.nOpcodeGetLog = OPCODE_V110_GET_LOG;
		pDevicePrivateData->FWInfo.nOpcodeTakeSnapshot = OPCODE_V110_TAKE_SNAPSHOT;
		pDevicePrivateData->FWInfo.nOpcodeInitFileUpload = OPCODE_V110_INIT_FILE_UPLOAD;
		pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload = OPCODE_V110_WRITE_FILE_UPLOAD;
		pDevicePrivateData->FWInfo.nOpcodeFinishFileUpload = OPCODE_V110_FINISH_FILE_UPLOAD;
		pDevicePrivateData->FWInfo.nOpcodeDownloadFile = OPCODE_V110_DOWNLOAD_FILE;
		pDevicePrivateData->FWInfo.nOpcodeDeleteFile = OPCODE_V110_DELETE_FILE;
		pDevicePrivateData->FWInfo.nOpcodeGetFlashMap = OPCODE_V110_GET_FLASH_MAP;
		pDevicePrivateData->FWInfo.nOpcodeGetFileList = OPCODE_V110_GET_FILE_LIST;
		pDevicePrivateData->FWInfo.nOpcodeSetFileAttribute = OPCODE_V110_SET_FILE_ATTRIBUTES;
		pDevicePrivateData->FWInfo.nOpcodeExecuteFile = OPCODE_V110_EXECUTE_FILE;
		pDevicePrivateData->FWInfo.nOpcodeReadFlash = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeBIST = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeSetGMCParams = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCPUStats = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateTec = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetTecData = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateEmitter = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetEmitterData = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateProjectorFault = OPCODE_INVALID;

		// log system changed
		pDevicePrivateData->FWInfo.nLogStringType = XN_LOG_TEXT_MESSAGE_V1_2;
		pDevicePrivateData->FWInfo.nLogOverflowType = XN_LOG_OVERFLOW_V1_2;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 1, 2, 0) >= 0)
	{
		// protocol header was changed
		pDevicePrivateData->FWInfo.nFWMagic = XN_FW_MAGIC_26;
		pDevicePrivateData->FWInfo.nHostMagic = XN_HOST_MAGIC_26;
		pDevicePrivateData->FWInfo.nProtocolHeaderSize = sizeof(XnHostProtocolHeaderV26);
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 3, 0, 0) >= 0)
	{
		// audio support!
		pDevicePrivateData->FWInfo.bAudioSupported = TRUE;

		// opcodes were changed
		pDevicePrivateData->FWInfo.nOpcodeGetVersion = OPCODE_GET_VERSION;
		pDevicePrivateData->FWInfo.nOpcodeKeepAlive = OPCODE_KEEP_ALIVE;
		pDevicePrivateData->FWInfo.nOpcodeGetParam = OPCODE_GET_PARAM;
		pDevicePrivateData->FWInfo.nOpcodeSetParam = OPCODE_SET_PARAM;
		pDevicePrivateData->FWInfo.nOpcodeGetFixedParams = OPCODE_GET_FIXED_PARAMS;
		pDevicePrivateData->FWInfo.nOpcodeGetMode = OPCODE_GET_MODE;
		pDevicePrivateData->FWInfo.nOpcodeSetMode = OPCODE_SET_MODE;
		pDevicePrivateData->FWInfo.nOpcodeAlgorithmParams = OPCODE_ALGORITM_PARAMS;
		pDevicePrivateData->FWInfo.nOpcodeReset = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeSetCmosBlanking = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCmosBlanking = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCmosPresets = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetSerialNumber = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetFastConvergenceTEC = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCMOSReg = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeSetCMOSReg = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeWriteI2C = OPCODE_I2C_WRITE;
		pDevicePrivateData->FWInfo.nOpcodeReadI2C = OPCODE_I2C_READ;
		pDevicePrivateData->FWInfo.nOpcodeReadAHB = OPCODE_READ_AHB;
		pDevicePrivateData->FWInfo.nOpcodeWriteAHB = OPCODE_WRITE_AHB;
		pDevicePrivateData->FWInfo.nOpcodeGetLog = OPCODE_GET_LOG;
		pDevicePrivateData->FWInfo.nOpcodeTakeSnapshot = OPCODE_TAKE_SNAPSHOT;
		pDevicePrivateData->FWInfo.nOpcodeInitFileUpload = OPCODE_INIT_FILE_UPLOAD;
		pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload = OPCODE_WRITE_FILE_UPLOAD;
		pDevicePrivateData->FWInfo.nOpcodeFinishFileUpload = OPCODE_FINISH_FILE_UPLOAD;
		pDevicePrivateData->FWInfo.nOpcodeDownloadFile = OPCODE_DOWNLOAD_FILE;
		pDevicePrivateData->FWInfo.nOpcodeDeleteFile = OPCODE_DELETE_FILE;
		pDevicePrivateData->FWInfo.nOpcodeGetFlashMap = OPCODE_GET_FLASH_MAP;
		pDevicePrivateData->FWInfo.nOpcodeGetFileList = OPCODE_GET_FILE_LIST;
		pDevicePrivateData->FWInfo.nOpcodeSetFileAttribute = OPCODE_SET_FILE_ATTRIBUTES;
		pDevicePrivateData->FWInfo.nOpcodeExecuteFile = OPCODE_EXECUTE_FILE;
		pDevicePrivateData->FWInfo.nOpcodeReadFlash = OPCODE_READ_FLASH;
		pDevicePrivateData->FWInfo.nOpcodeBIST = OPCODE_V300_BIST;
		pDevicePrivateData->FWInfo.nOpcodeSetGMCParams = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetCPUStats = OPCODE_GET_CPU_STATS;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateTec = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetTecData = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateEmitter = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeGetEmitterData = OPCODE_INVALID;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateProjectorFault = OPCODE_INVALID;

		// log system changed
		pDevicePrivateData->FWInfo.nLogStringType = XN_LOG_TEXT_MESSAGE_V3_0;
		pDevicePrivateData->FWInfo.nLogOverflowType = XN_LOG_OVERFLOW_V3_0;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 4, 0, 0) >= 0)
	{
		// audio removed...
		pDevicePrivateData->FWInfo.bAudioSupported = FALSE;

		// opcodes added
		pDevicePrivateData->FWInfo.nOpcodeBIST = OPCODE_BIST;
		pDevicePrivateData->FWInfo.nOpcodeSetGMCParams = OPCODE_SET_GMC_PARAMS;

		// log system changed
		pDevicePrivateData->FWInfo.nLogStringType = XN_LOG_TEXT_MESSAGE_V4_0;
		pDevicePrivateData->FWInfo.nLogOverflowType = XN_LOG_OVERFLOW_V4_0;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 0, 0) >= 0)
	{
		// max packet size changed
		pDevicePrivateData->FWInfo.nProtocolMaxPacketSize = XN_PROTOCOL_MAX_PACKET_SIZE_V5_0;
		// audio is back on
		pDevicePrivateData->FWInfo.bAudioSupported = TRUE;
		// mirror supported
		pDevicePrivateData->FWInfo.bMirrorSupported = TRUE;

		// opcodes changes
		pDevicePrivateData->FWInfo.nOpcodeSetCmosBlanking = OPCODE_SET_CMOS_BLANKING;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateTec = OPCODE_CALIBRATE_TEC;
		pDevicePrivateData->FWInfo.nOpcodeGetTecData = OPCODE_GET_TEC_DATA;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateEmitter = OPCODE_CALIBRATE_EMITTER;
		pDevicePrivateData->FWInfo.nOpcodeGetEmitterData = OPCODE_GET_EMITTER_DATA;
		pDevicePrivateData->FWInfo.nOpcodeCalibrateProjectorFault = OPCODE_CALIBRATE_PROJECTOR_FAULT;

		// log system changed
		pDevicePrivateData->FWInfo.nLogStringType = XN_LOG_TEXT_MESSAGE_V5_0;
		pDevicePrivateData->FWInfo.nLogOverflowType = XN_LOG_OVERFLOW_V5_0;

		// ISO endpoints interface was added
		pDevicePrivateData->FWInfo.nISOAlternativeInterface = 0;
		pDevicePrivateData->FWInfo.nBulkAlternativeInterface = 1;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 1, 0) >= 0)
	{
		// added high-res IR
		XnCmosPreset irHighResMode = { 0, XN_RESOLUTION_SXGA, 30 };
		nRetVal = pDevicePrivateData->FWInfo.irModes.AddLast(irHighResMode);
		XN_IS_STATUS_OK(nRetVal);

		// opcode added
		pDevicePrivateData->FWInfo.nOpcodeGetCmosBlanking = OPCODE_GET_CMOS_BLANKING;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 2, 0) >= 0 && CompareVersion(nMajor, nMinor, nBuild, 5, 6, 0) < 0)
	{
		// 25 FPS modes were added
		XnCmosPreset depthModes25FPS[] = 
		{
			{ XN_IO_DEPTH_FORMAT_COMPRESSED_PS, XN_RESOLUTION_QVGA, 25 },
			{ XN_IO_DEPTH_FORMAT_COMPRESSED_PS, XN_RESOLUTION_VGA, 25 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_11_BIT, XN_RESOLUTION_QVGA, 25 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_11_BIT, XN_RESOLUTION_VGA, 25 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_12_BIT, XN_RESOLUTION_QVGA, 25 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_12_BIT, XN_RESOLUTION_VGA, 25 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_16_BIT, XN_RESOLUTION_QVGA, 25 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_16_BIT, XN_RESOLUTION_VGA, 25 },
		};
		nRetVal = pDevicePrivateData->FWInfo.depthModes.AddLast(depthModes25FPS, sizeof(depthModes25FPS)/sizeof(depthModes25FPS[0]));
		XN_IS_STATUS_OK(nRetVal);

		XnCmosPreset imageModes25FpsCommon[] =
		{
			{ XN_IO_IMAGE_FORMAT_YUV422, XN_RESOLUTION_QVGA, 25 },
			{ XN_IO_IMAGE_FORMAT_YUV422, XN_RESOLUTION_VGA, 25 },
		};
		nRetVal = pDevicePrivateData->FWInfo._imageBulkModes.AddLast(imageModes25FpsCommon, sizeof(imageModes25FpsCommon)/sizeof(imageModes25FpsCommon[0]));
		XN_IS_STATUS_OK(nRetVal);
		nRetVal = pDevicePrivateData->FWInfo._imageIsoModes.AddLast(imageModes25FpsCommon, sizeof(imageModes25FpsCommon)/sizeof(imageModes25FpsCommon[0]));
		XN_IS_STATUS_OK(nRetVal);

		XnCmosPreset imageModes25FpsIso[] =
		{
			{ XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_RESOLUTION_QVGA, 25 },
			{ XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_RESOLUTION_VGA, 25 },
		};
		nRetVal = pDevicePrivateData->FWInfo._imageIsoModes.AddLast(imageModes25FpsIso, sizeof(imageModes25FpsIso)/sizeof(imageModes25FpsIso[0]));
		XN_IS_STATUS_OK(nRetVal);

		XnCmosPreset irModes25Fps[] =
		{
			{ 0, XN_RESOLUTION_QVGA, 25 },
			{ 0, XN_RESOLUTION_VGA, 25 },
		};
		nRetVal = pDevicePrivateData->FWInfo.irModes.AddLast(irModes25Fps, sizeof(irModes25Fps)/sizeof(irModes25Fps[0]));
		XN_IS_STATUS_OK(nRetVal);
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 2, 0) >= 0)
	{
		// added high-res image modes (UXGA for 5.2, SXGA for 5.3 and newer)
		XnCmosPreset imageHighResBayerMode = { XN_IO_IMAGE_FORMAT_BAYER, XN_RESOLUTION_UXGA, 30 };
		if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 0) >= 0)
		{
			imageHighResBayerMode.nResolution = XN_RESOLUTION_SXGA;
		}
		nRetVal = pDevicePrivateData->FWInfo._imageBulkModes.AddLast(imageHighResBayerMode);
		XN_IS_STATUS_OK(nRetVal);
		nRetVal = pDevicePrivateData->FWInfo._imageIsoModes.AddLast(imageHighResBayerMode);
		XN_IS_STATUS_OK(nRetVal);
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 15) == 0)
	{
		pDevicePrivateData->FWInfo.nUSBDelaySetParamFlicker = 300;
	}	

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 16) >= 0 && 
		!pDevicePrivateData->pSensor->IsLowBandwidth())
	{
		if (usb == XN_USB_CORE_JANGO)
		{
			pDevicePrivateData->FWInfo.nUSBDelayReceive = 1;
			pDevicePrivateData->FWInfo.nUSBDelayExecutePreSend = 0;
			pDevicePrivateData->FWInfo.nUSBDelayExecutePostSend = 0;
		}

		pDevicePrivateData->FWInfo.nUSBDelaySoftReset = 1;
		pDevicePrivateData->FWInfo.nUSBDelaySetParamFlicker = 1;
		pDevicePrivateData->FWInfo.nUSBDelaySetParamStream0Mode = 1;
		pDevicePrivateData->FWInfo.nUSBDelaySetParamStream1Mode = 1;
		pDevicePrivateData->FWInfo.nUSBDelaySetParamStream2Mode = 1;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 25) >= 0)
	{
		pDevicePrivateData->FWInfo.bDeviceInfoSupported = TRUE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 28) >= 0)
	{
		// YUV is also supported in high-res
		XnCmosPreset imageHighResYuvMode = { XN_IO_IMAGE_FORMAT_YUV422, XN_RESOLUTION_SXGA, 30 };
		nRetVal = pDevicePrivateData->FWInfo._imageBulkModes.AddLast(imageHighResYuvMode);
		XN_IS_STATUS_OK(nRetVal);
		nRetVal = pDevicePrivateData->FWInfo._imageIsoModes.AddLast(imageHighResYuvMode);
		XN_IS_STATUS_OK(nRetVal);

		XnCmosPreset imageHighResYuvModeIso = { XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422, XN_RESOLUTION_SXGA, 30 };
		nRetVal = pDevicePrivateData->FWInfo._imageIsoModes.AddLast(imageHighResYuvModeIso);
		XN_IS_STATUS_OK(nRetVal);
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 29) >= 0)
	{
		pDevicePrivateData->FWInfo.nOpcodeGetCmosPresets = OPCODE_GET_CMOS_PRESETS;
		pDevicePrivateData->FWInfo.bGetPresetsSupported = TRUE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 3, 31) >= 0 && CompareVersion(nMajor, nMinor, nBuild, 5, 4, 0) < 0)
	{
		// file system lock was also added in 5.3.31 (a maintenance release), but it's not in newer versions (5.4 and above)
		pDevicePrivateData->FWInfo.bHasFilesystemLock = TRUE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 4, 0) >= 0)
	{
		pDevicePrivateData->FWInfo.nOpcodeGetSerialNumber = OPCODE_GET_SERIAL_NUMBER;
		pDevicePrivateData->FWInfo.nOpcodeGetFastConvergenceTEC = OPCODE_GET_FAST_CONVERGENCE_TEC;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 5, 0) >= 0)
	{
		// only difference is the interfaces order
		pDevicePrivateData->FWInfo.nBulkAlternativeInterface = 0;
		pDevicePrivateData->FWInfo.nISOAlternativeInterface = 1;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 6, 0) >= 0)
	{
		// audio is no longer supported - switched to UAC
		pDevicePrivateData->FWInfo.bAudioSupported = FALSE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 6, 2) >= 0)
	{
		// add QQVGA depth modes
		XnCmosPreset aQQmodes[] = 
		{
			{ XN_IO_DEPTH_FORMAT_COMPRESSED_PS, XN_RESOLUTION_QQVGA, 30 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_11_BIT, XN_RESOLUTION_QQVGA, 30 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_12_BIT, XN_RESOLUTION_QQVGA, 30 },
			{ XN_IO_DEPTH_FORMAT_UNCOMPRESSED_16_BIT, XN_RESOLUTION_QQVGA, 30 },
		};
		nRetVal = pDevicePrivateData->FWInfo.depthModes.AddLast(aQQmodes, sizeof(aQQmodes)/sizeof(aQQmodes[0]));
		XN_IS_STATUS_OK(nRetVal);
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 6, 9) >= 0)
	{
		pDevicePrivateData->FWInfo.bGetImageCmosTypeSupported = TRUE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 7, 0) >= 0)
	{
		pDevicePrivateData->FWInfo.nOpcodeGetPlatformString = OPCODE_GET_PLATFORM_STRING;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 7, 2) >= 0)
	{
		pDevicePrivateData->FWInfo.bIncreasedFpsCropSupported = TRUE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 8, 0) >= 0)
	{
		pDevicePrivateData->FWInfo.nOpcodeSetLedState = OPCODE_SET_LED_STATE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 8, 2) >= 0)
	{
		pDevicePrivateData->FWInfo.bHasFilesystemLock = TRUE;		
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 8, 9) >= 0)
	{
		pDevicePrivateData->FWInfo.bImageAdjustmentsSupported = TRUE;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 8, 15) >= 0)
	{
		pDevicePrivateData->FWInfo.nOpcodeEnableEmitter = OPCODE_ENABLE_EMITTER;	
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 8, 16) >= 0)
	{
		pDevicePrivateData->FWInfo.nISOLowDepthAlternativeInterface = 2;
	}

	if (CompareVersion(nMajor, nMinor, nBuild, 5, 9, 0) >= 0)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "Sensor version %d.%d.%x is newer than latest known. Trying to use 5.8 protocol...", nMajor, nMinor, nBuild);
	}

	// If FW is already known, update image modes
	if (!bGuessed)
	{
		nRetVal = XnHostProtocolUpdateSupportedImageModes(pDevicePrivateData);
		XN_IS_STATUS_OK(nRetVal);
	}

	pDevicePrivateData->FWInfo.nCurrMode = XN_MODE_PS;
	pDevicePrivateData->FWInfo.nFWVer = GetFWVersion(nMajor, nMinor, nBuild);

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolInitHeader(const XnDevicePrivateData* pDevicePrivateData, void* pBuffer, XnUInt32 nSize, XnUInt16 nOpcode)
{
	static XnUInt16 nId = 0;

	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_1_2)
	{
		XnHostProtocolHeaderV26* pHeader = (XnHostProtocolHeaderV26*)pBuffer;
		pHeader->nMagic = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->FWInfo.nHostMagic);
		pHeader->nSize = XN_PREPARE_VAR16_IN_BUFFER(XnUInt16(nSize/sizeof(XnUInt16)));
		pHeader->nOpcode = XN_PREPARE_VAR16_IN_BUFFER(nOpcode);
		pHeader->nId = XN_PREPARE_VAR16_IN_BUFFER(nId++);
	}
	else
	{
		XnHostProtocolHeaderV25* pHeader = (XnHostProtocolHeaderV25*)pBuffer;
		pHeader->nMagic = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->FWInfo.nHostMagic);
		pHeader->nSize = XN_PREPARE_VAR16_IN_BUFFER(XnUInt16(nSize/sizeof(XnUInt16)));
		pHeader->nOpcode = XN_PREPARE_VAR16_IN_BUFFER(nOpcode);
		pHeader->nId = XN_PREPARE_VAR16_IN_BUFFER(nId++);
		pHeader->nCRC16 = XN_PREPARE_VAR16_IN_BUFFER(0);
	}

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolUSBSend(const XnDevicePrivateData* pDevicePrivateData,
							   XnUChar* pBuffer, XnUInt16 nSize, XnUInt32 nTimeOut, XnBool bForceBulk)
{
	XnStatus nRetVal = XN_STATUS_OK;

	const XnUsbControlConnection* pCtrlConnection = &pDevicePrivateData->SensorHandle.ControlConnection;

	XnUInt32 nCounter = XN_USB_HOST_PROTOCOL_SEND_RETRIES;
	while (nCounter-- != 0)
	{
		if (pCtrlConnection->bIsBulk || bForceBulk)
			nRetVal = xnUSBWriteEndPoint(pCtrlConnection->ControlOutConnectionEp, pBuffer, nSize, nTimeOut);
		else
		{
			nRetVal = xnUSBSendControl(pDevicePrivateData->SensorHandle.USBDevice, XN_USB_CONTROL_TYPE_VENDOR, 0, 0, 0, pBuffer, nSize, nTimeOut);
		}

		if (nRetVal != XN_STATUS_USB_TRANSFER_TIMEOUT && nRetVal != XN_STATUS_USB_TRANSFER_STALL)
			break;

		xnOSSleep(100);
	}

	return nRetVal;
}

XnStatus XnHostProtocolUSBReceive(const XnDevicePrivateData* pDevicePrivateData,
								  XnUChar* pBuffer, XnUInt nSize, XnUInt32& nRead, XnUInt32 nTimeOut, XnBool bForceBulk, XnUInt32 nFailTimeout)
{
	XnStatus nRetVal;
	XnUInt64 nMaxTime;
	XnUInt64 nCurrTime;

	const XnUsbControlConnection* pCtrlConnection = &pDevicePrivateData->SensorHandle.ControlConnection;

	xnOSGetHighResTimeStamp(&nMaxTime);
	nMaxTime += (nTimeOut * 1000);

	for (;;)
	{
		xnOSGetHighResTimeStamp(&nCurrTime);
		if (nCurrTime > nMaxTime)
		{
			return (XN_STATUS_USB_TRANSFER_TIMEOUT);
		}

		if (pCtrlConnection->bIsBulk || bForceBulk)
			nRetVal = xnUSBReadEndPoint(pCtrlConnection->ControlInConnectionEp, pBuffer, nSize, &nRead, nTimeOut);
		else
			nRetVal = xnUSBReceiveControl(pDevicePrivateData->SensorHandle.USBDevice, XN_USB_CONTROL_TYPE_VENDOR, 0, 0, 0, pBuffer, nSize, &nRead, nTimeOut);

		if (nRetVal != XN_STATUS_USB_TRANSFER_TIMEOUT && nRetVal != XN_STATUS_USB_TRANSFER_STALL && nRetVal != XN_STATUS_USB_NOT_ENOUGH_DATA)
		{
			break;
		}

		if (nFailTimeout != 0)
		{
			XnUInt64 nNow;
			XnUInt64 nNow2;
			xnOSGetHighResTimeStamp(&nNow);
			xnOSGetHighResTimeStamp(&nNow2);
			while (nNow2 - nNow < nFailTimeout)
			{
				xnOSGetHighResTimeStamp(&nNow2);
			}
		}
		else
		{
			xnOSSleep(pDevicePrivateData->FWInfo.nUSBDelayReceive);
		}
	}

	return nRetVal;
}

XnStatus ValidateReplyV26(const XnDevicePrivateData* pDevicePrivateData, XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt16 nExpectedOpcode, XnUInt16 nRequestId, XnUInt16& nDataSize, XnUChar** pDataBuf)
{
	XnUInt16 nHeaderOffset = 0;
	XnHostProtocolHeaderV26* pHeader = (XnHostProtocolHeaderV26*)pBuffer;

	pHeader->nMagic = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nMagic);
	
	while (pHeader->nMagic !=  pDevicePrivateData->FWInfo.nFWMagic && nHeaderOffset < nBufferSize-pDevicePrivateData->FWInfo.nProtocolHeaderSize-sizeof(XnHostProtocolReplyHeader))
	{
		nHeaderOffset++;
		pHeader = (XnHostProtocolHeaderV26*)(pBuffer+nHeaderOffset);
		pHeader->nMagic = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nMagic);
	}

	pHeader->nId = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nId);
	pHeader->nOpcode = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nOpcode);
	pHeader->nSize = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nSize);

	if (pHeader->nMagic != pDevicePrivateData->FWInfo.nFWMagic)
	{
		return XN_STATUS_DEVICE_PROTOCOL_BAD_MAGIC;
	}

	if (pHeader->nId != nRequestId)
	{
//		printf("Response ID (%d) not the same as request id (%d)\n", pHeader->nId, nRequestId);
		return XN_STATUS_DEVICE_PROTOCOL_WRONG_ID;
	}

	if (pHeader->nOpcode != nExpectedOpcode)
	{
//		printf("Unexpected opcode %d (expected: %d)\n", pHeader->nOpcode, nExpectedOpcode);
		return XN_STATUS_DEVICE_PROTOCOL_WRONG_OPCODE;
	}
	// CRC?
	// ...

	XnHostProtocolReplyHeader* pReply = (XnHostProtocolReplyHeader*)(pBuffer+nHeaderOffset+pDevicePrivateData->FWInfo.nProtocolHeaderSize);
	pReply->nErrorCode = XN_PREPARE_VAR16_IN_BUFFER(pReply->nErrorCode);
	
	if (pReply->nErrorCode != ACK)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "Received NACK: %d", pReply->nErrorCode);

		switch (pReply->nErrorCode)
		{
		case NACK_INVALID_COMMAND:
			return XN_STATUS_DEVICE_PROTOCOL_INVALID_COMMAND;
		case NACK_BAD_PACKET_CRC:
			return XN_STATUS_DEVICE_PROTOCOL_BAD_PACKET_CRC;
		case NACK_BAD_PACKET_SIZE:
			return XN_STATUS_DEVICE_PROTOCOL_BAD_PACKET_SIZE;
		case NACK_BAD_PARAMS:
			return XN_STATUS_DEVICE_PROTOCOL_BAD_PARAMS;
		case NACK_I2C_TRANSACTION_FAILED:
			return XN_STATUS_DEVICE_PROTOCOL_I2C_TRANSACTION_FAILED;
		case NACK_FILE_NOT_FOUND:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_NOT_FOUND;
		case NACK_FILE_CREATE_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_CREATE_FAILURE;
		case NACK_FILE_WRITE_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_WRITE_FAILURE;
		case NACK_FILE_DELETE_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_DELETE_FAILURE;
		case NACK_FILE_READ_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_READ_FAILURE;
		case NACK_BAD_COMMAND_SIZE:
			return XN_STATUS_DEVICE_PROTOCOL_BAD_COMMAND_SIZE;
		case NACK_NOT_READY:
			return XN_STATUS_DEVICE_PROTOCOL_NOT_READY;
		case NACK_OVERFLOW:
			return XN_STATUS_DEVICE_PROTOCOL_OVERFLOW;
		case NACK_OVERLAY_NOT_LOADED:
			return XN_STATUS_DEVICE_PROTOCOL_OVERLAY_NOT_LOADED;
		case NACK_FILE_SYSTEM_LOCKED:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_SYSTEM_LOCKED;
		case NACK_UNKNOWN_ERROR:
		default:
			return XN_STATUS_DEVICE_PROTOCOL_UNKNOWN_ERROR;
		}
	}
	// Check reply length is reasonable for opcode

	nDataSize = pHeader->nSize - sizeof(XnHostProtocolReplyHeader)/sizeof(XnUInt16);

	if (pDataBuf)
		*pDataBuf = pBuffer + nHeaderOffset+pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnHostProtocolReplyHeader);
	return XN_STATUS_OK;
}

XnStatus ValidateReplyV25(const XnDevicePrivateData* pDevicePrivateData, XnUChar* pBuffer, XnUInt32 nBufferSize, XnUInt16 nExpectedOpcode, XnUInt16 nRequestId, XnUInt16& nDataSize, XnUChar** pDataBuf)
{
	XnUInt16 nHeaderOffset = 0;
	XnHostProtocolHeaderV25* pHeader = (XnHostProtocolHeaderV25*)pBuffer;

	pHeader->nMagic = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nMagic);

	while (pHeader->nMagic !=  pDevicePrivateData->FWInfo.nFWMagic && nHeaderOffset < nBufferSize-pDevicePrivateData->FWInfo.nProtocolHeaderSize-sizeof(XnHostProtocolReplyHeader))
	{
		nHeaderOffset++;
		pHeader = (XnHostProtocolHeaderV25*)(pBuffer+nHeaderOffset);
		pHeader->nMagic = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nMagic);
	}

	pHeader->nCRC16 = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nCRC16);
	pHeader->nId = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nId);
	pHeader->nOpcode = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nOpcode);
	pHeader->nSize = XN_PREPARE_VAR16_IN_BUFFER(pHeader->nSize);

	if (pHeader->nMagic != pDevicePrivateData->FWInfo.nFWMagic)
	{
		return XN_STATUS_DEVICE_PROTOCOL_BAD_MAGIC;
	}

	if (pHeader->nId != nRequestId)
	{
		//		printf("Response ID (%d) not the same as request id (%d)\n", pHeader->nId, nRequestId);
		return XN_STATUS_DEVICE_PROTOCOL_WRONG_ID;
	}

	if (pHeader->nOpcode != nExpectedOpcode)
	{
		//		printf("Unexpected opcode %d (expected: %d)\n", pHeader->nOpcode, nExpectedOpcode);
		return XN_STATUS_DEVICE_PROTOCOL_WRONG_OPCODE;
	}
	// CRC?
	// ...

	XnHostProtocolReplyHeader* pReply = (XnHostProtocolReplyHeader*)(pBuffer+nHeaderOffset+pDevicePrivateData->FWInfo.nProtocolHeaderSize);

	pReply->nErrorCode = XN_PREPARE_VAR16_IN_BUFFER(pReply->nErrorCode);

	if (pReply->nErrorCode != ACK)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "Received NACK: %d", pReply->nErrorCode);

		switch (pReply->nErrorCode)
		{
		case NACK_INVALID_COMMAND:
			return XN_STATUS_DEVICE_PROTOCOL_INVALID_COMMAND;
		case NACK_BAD_PACKET_CRC:
			return XN_STATUS_DEVICE_PROTOCOL_BAD_PACKET_CRC;
		case NACK_BAD_PACKET_SIZE:
			return XN_STATUS_DEVICE_PROTOCOL_BAD_PACKET_SIZE;
		case NACK_BAD_PARAMS:
			return XN_STATUS_DEVICE_PROTOCOL_BAD_PARAMS;
		case NACK_I2C_TRANSACTION_FAILED:
			return XN_STATUS_DEVICE_PROTOCOL_I2C_TRANSACTION_FAILED;
		case NACK_FILE_NOT_FOUND:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_NOT_FOUND;
		case NACK_FILE_CREATE_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_CREATE_FAILURE;
		case NACK_FILE_WRITE_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_WRITE_FAILURE;
		case NACK_FILE_DELETE_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_DELETE_FAILURE;
		case NACK_FILE_READ_FAILURE:
			return XN_STATUS_DEVICE_PROTOCOL_FILE_READ_FAILURE;
		case NACK_UNKNOWN_ERROR:
		default:
			return XN_STATUS_DEVICE_PROTOCOL_UNKNOWN_ERROR;
		}
	}
	// Check reply length is reasonable for opcode

	nDataSize = pHeader->nSize - sizeof(XnHostProtocolReplyHeader)/sizeof(XnUInt16);

	if (pDataBuf)
		*pDataBuf = pBuffer + nHeaderOffset+pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnHostProtocolReplyHeader);
	return XN_STATUS_OK;
}

XnUInt32 XnHostProtocolGetTimeOut(const XnDevicePrivateData* pDevicePrivateData, XnUInt16 nOpcode)
{
	if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeKeepAlive)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_KEEP_ALIVE;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeGetVersion)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_GETVERSION;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeSetParam)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_SETPARAM;
	else if (nOpcode == pDevicePrivateData->FWInfo.nOpcodeInitFileUpload)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_UPLOAD;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeDeleteFile)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_FILE_OPS;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeSetFileAttribute)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_FILE_OPS;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeDownloadFile)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_FILE_OPS;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeFinishFileUpload)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_FILE_OPS;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_FILE_OPS;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeBIST)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_BIST;
	else if  (nOpcode == pDevicePrivateData->FWInfo.nOpcodeGetEmitterData)
		return XN_USB_HOST_PROTOCOL_TIMEOUT_EMITTER_DATA;
	else
		return XN_USB_HOST_PROTOCOL_TIMEOUT;
}

XnUInt32 XnHostProtocolGetSetParamRecvTimeOut(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nParam)
{
	if (nParam == PARAM_IMAGE_FLICKER_DETECTION)
		return pDevicePrivateData->FWInfo.nUSBDelaySetParamFlicker;
	else if (nParam == PARAM_GENERAL_STREAM0_MODE)
		return pDevicePrivateData->FWInfo.nUSBDelaySetParamStream0Mode;
	else if (nParam == PARAM_GENERAL_STREAM1_MODE)
		return pDevicePrivateData->FWInfo.nUSBDelaySetParamStream1Mode;
	else if (nParam == PARAM_GENERAL_STREAM2_MODE)
		return pDevicePrivateData->FWInfo.nUSBDelaySetParamStream2Mode;
	else
		return 0;
}

XnStatus XnHostProtocolGetRequestID(const XnDevicePrivateData* pDevicePrivateData, XnUChar* pBuffer, XnUInt16* pnRequestId)
{
	XnUInt16 nRequestId;

	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_1_2)
	{
		nRequestId = ((XnHostProtocolHeaderV26*)(pBuffer))->nId;
	}
	else
	{
		nRequestId = ((XnHostProtocolHeaderV25*)(pBuffer))->nId;
	}

	*pnRequestId = XN_PREPARE_VAR16_IN_BUFFER(nRequestId);
	return XN_STATUS_OK;
}

XnStatus XnHostProtocolReceiveReply(const XnDevicePrivateData* pDevicePrivateData, XnUChar* pBuffer, XnUInt32 nTimeOut, XnUInt16 nOpcode, XnUInt16 nRequestId, XnUInt32* pnReadBytes, XnUInt16* pnDataSize, XnUChar** ppRelevantBuffer, XnBool bForceBulk, XnUInt32 nRecvTimeout, XnUInt32 nFailTimeout)
{
	XnStatus rc = XN_STATUS_OK;

	XnUInt64 nStartWaitingTime;
	xnOSGetTimeStamp(&nStartWaitingTime);

	for (;;) // loop until timeout expires
	{
		do // loop until right reply ID is received
		{
			// receive reply
			if (nRecvTimeout != 0)
			{
				xnOSSleep(nRecvTimeout);
			}

			rc = XnHostProtocolUSBReceive(pDevicePrivateData, pBuffer, pDevicePrivateData->FWInfo.nProtocolMaxPacketSize, *pnReadBytes, nTimeOut, bForceBulk, nFailTimeout);
			XN_IS_STATUS_OK(rc);

			// Validate the reply
			if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_1_2)
			{
				rc = ValidateReplyV26(pDevicePrivateData, pBuffer, *pnReadBytes, nOpcode, nRequestId, *pnDataSize, ppRelevantBuffer);
			}
			else
			{
				rc = ValidateReplyV25(pDevicePrivateData, pBuffer, *pnReadBytes, nOpcode, nRequestId, *pnDataSize, ppRelevantBuffer);
			}
		} while (rc == XN_STATUS_DEVICE_PROTOCOL_WRONG_ID);

		XnUInt64 nNow;
		xnOSGetTimeStamp(&nNow);

		if (rc != XN_STATUS_OK && rc != XN_STATUS_DEVICE_PROTOCOL_BAD_MAGIC)
		{
			return rc;
		}
		else if (rc == XN_STATUS_DEVICE_PROTOCOL_BAD_MAGIC && (nNow-nStartWaitingTime)>XN_RECEIVE_USB_DATA_TIMEOUT)
		{
			// Timeout expired
			return XN_STATUS_DEVICE_PROTOCOL_BAD_MAGIC;
		}
		else if (rc == XN_STATUS_DEVICE_PROTOCOL_BAD_MAGIC)
		{
			// Timeout not expired yet
			xnOSSleep(10);
		}
		else 
		{
			// OK
			break;
		}
	}

	return rc;
}

XnStatus XnHostProtocolExecute(const XnDevicePrivateData* pDevicePrivateData,
							   XnUChar* pBuffer, XnUInt16 nSize, XnUInt16 nOpcode,
							   XnUChar** ppRelevantBuffer, XnUInt16& nDataSize, XnUInt32 nRecvTimeout = 0)
{
	XnStatus rc;
	XnUInt32 nRead = 0;
	XnUInt32 nFailTimeout = 0;

	XnBool bForceBulk = FALSE;

	if (nOpcode == OPCODE_INVALID)
	{
		return (XN_STATUS_DEVICE_PROTOCOL_UNSUPPORTED_OPCODE);
	}

	// don't bother trying to communicate with the device if it was disconnected
	if (pDevicePrivateData->pSensor->GetErrorState() == XN_STATUS_DEVICE_NOT_CONNECTED)
	{
		return (XN_STATUS_DEVICE_NOT_CONNECTED);
	}

	XnUInt32 nTimeOut = XnHostProtocolGetTimeOut(pDevicePrivateData, nOpcode);

	// store request (in case we need to retry it)
	XnUChar request[MAX_PACKET_SIZE];
	xnOSMemCopy(request, pBuffer, nSize);

	XnUInt16 nRequestId;
	rc = XnHostProtocolGetRequestID(pDevicePrivateData, pBuffer, &nRequestId);
	XN_IS_STATUS_OK(rc);

	XnUInt16 nRetriesLeft = XN_HOST_PROTOCOL_NOT_READY_RETRIES;
	while (nRetriesLeft-- > 0) // loop until device is ready
	{
		rc = xnOSLockMutex(pDevicePrivateData->hExecuteMutex, XN_WAIT_INFINITE);
		XN_IS_STATUS_OK(rc);

		// Sleep before sending the control
		if (nOpcode == pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload)
		{
			XnUInt64 nNow;
			XnUInt64 nNow2;
			xnOSGetHighResTimeStamp(&nNow);
			xnOSGetHighResTimeStamp(&nNow2);
			while (nNow2 - nNow < XN_USB_HOST_PROTOCOL_FILE_UPLOAD_PRE_DELAY)
			{
				xnOSGetHighResTimeStamp(&nNow2);
			}
		}
		else
		{
			xnOSSleep(pDevicePrivateData->FWInfo.nUSBDelayExecutePreSend);
		}

		// Send request
		rc = XnHostProtocolUSBSend(pDevicePrivateData, request, nSize, nTimeOut, bForceBulk);
		if (rc != XN_STATUS_OK)
		{
			xnOSUnLockMutex(pDevicePrivateData->hExecuteMutex);
			return rc;
		}

		// Sleep before trying to read the reply
		if (nOpcode == pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload)
		{
			nFailTimeout = XN_USB_HOST_PROTOCOL_FILE_UPLOAD_PRE_DELAY;
		}
		else
		{
			xnOSSleep(pDevicePrivateData->FWInfo.nUSBDelayExecutePostSend);
		}

		// receive reply
		rc = XnHostProtocolReceiveReply(pDevicePrivateData, pBuffer, nTimeOut, nOpcode, nRequestId, &nRead, &nDataSize, ppRelevantBuffer, bForceBulk, nRecvTimeout, nFailTimeout);

		if (rc == XN_STATUS_DEVICE_PROTOCOL_NOT_READY || rc == XN_STATUS_OK)
		{
			XnStatus unlockRC = xnOSUnLockMutex(pDevicePrivateData->hExecuteMutex);
			XN_IS_STATUS_OK(unlockRC);
		}
		else
		{
			xnOSUnLockMutex(pDevicePrivateData->hExecuteMutex);
			return rc;
		}

		if (rc == XN_STATUS_OK)
			break;

		xnOSSleep(1000);
		xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Device not ready. %d more retries...", nRetriesLeft);
	}

	XN_IS_STATUS_OK(rc);

	if (ppRelevantBuffer == NULL)
		return XN_STATUS_OK;

	// Get rest of data
	XnInt32 nCur = nRead; // Read so far

	nRead -= (pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnHostProtocolReplyHeader)); // Data read so far

	while (nRead < nDataSize*2U)
	{
		XnUInt32 dummy = 0;

		rc = XnHostProtocolUSBReceive(pDevicePrivateData, pBuffer+nCur, pDevicePrivateData->FWInfo.nProtocolMaxPacketSize, dummy, nTimeOut, bForceBulk, 0);
		if (rc != XN_STATUS_OK)
		{
			return rc;
		}

		nCur += dummy;
		nRead += dummy;
	}

	return XN_STATUS_OK;
}

#pragma pack (push, 1)
typedef struct 
{
	XnUInt16 nEntrySize;
	XnUInt32 nTimeStamp;
	XnUInt16 nLogType;
} XnLogEntryHeader;

typedef struct
{
	XnUInt16 nLine;
	XnUInt32 nParam;
} XnLogDefaultData;
#pragma pack(pop)

XnStatus XnHostProtocolGetLog(XnDevicePrivateData* pDevicePrivateData, XnChar* csBuffer, XnUInt32 nBufferSize)
{
	XnStatus rc = XN_STATUS_OK;
	XnUChar buffer[MAX_PACKET_SIZE] = {0};

	XnUChar allLogBuffer[XN_MAX_LOG_SIZE];

	XnUInt nAllLogBytes = 0;

	// loop until no more log is available
	for (;;)
	{
		XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetLog);

		XnUChar* pRelevantBuffer;
		XnUInt16 nDataSize;

		rc = XnHostProtocolExecute(pDevicePrivateData,
			buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetLog,
			&pRelevantBuffer, nDataSize);

		XN_IS_STATUS_OK(rc);

		if (nDataSize == 0) // no more log
			break;

		// translate to bytes
		nDataSize *= sizeof(XnUInt16);

		if (nAllLogBytes + nDataSize > XN_MAX_LOG_SIZE)
		{
			xnLogError(XN_MASK_SENSOR_PROTOCOL, "Log Buffer is too small. received %d bytes, but buffer is %d long", nAllLogBytes + nDataSize, XN_MAX_LOG_SIZE);
			return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
		}

		xnOSMemCopy(allLogBuffer + nAllLogBytes, pRelevantBuffer, nDataSize);
		nAllLogBytes += nDataSize;
	}

	XnUChar* pCurrData = allLogBuffer;
	XnUChar* pEndData = allLogBuffer + nAllLogBytes;

	XnUInt32 nBufferUsed = 0;

	// now parse it
	while (pCurrData < pEndData)
	{
		XnLogEntryHeader* pLogEntryHeader = (XnLogEntryHeader*)pCurrData;
		pCurrData += sizeof(XnLogEntryHeader);

		pLogEntryHeader->nEntrySize = XN_PREPARE_VAR16_IN_BUFFER(pLogEntryHeader->nEntrySize);
		pLogEntryHeader->nLogType = XN_PREPARE_VAR16_IN_BUFFER(pLogEntryHeader->nLogType);
		pLogEntryHeader->nTimeStamp = XN_PREPARE_VAR32_IN_BUFFER(pLogEntryHeader->nTimeStamp);

		// lower byte contains error type and higher contains module ID
		XnUInt32 nCharsWritten = 0;

		if (pLogEntryHeader->nLogType == pDevicePrivateData->FWInfo.nLogStringType)
		{
			// text messages are in wide characters
			//XnWChar wcsMessage[600] = {0};

			if (pLogEntryHeader->nEntrySize*sizeof(XnUInt16) > 600)
			{
				xnLogError(XN_MASK_SENSOR_PROTOCOL, "Got a log entry with %d bytes!", pLogEntryHeader->nEntrySize*sizeof(XnUInt16));
				return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
			}

			rc = xnOSStrFormat((XnChar*)csBuffer + nBufferUsed, nBufferSize - nBufferUsed, &nCharsWritten, "%u:\t", pLogEntryHeader->nTimeStamp);
			XN_IS_STATUS_OK(rc);
			nBufferUsed += nCharsWritten;

			if (nBufferSize - nBufferUsed < pLogEntryHeader->nEntrySize)
			{
				xnLogError(XN_MASK_SENSOR_PROTOCOL, "Not enough space in user buffer!");
				return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
			}

			for (XnUInt32 i = 0; i < pLogEntryHeader->nEntrySize; ++i)
			{
				csBuffer[nBufferUsed++] = pCurrData[i*2];
			}

			csBuffer[nBufferUsed++] = '\n';
		}
		else if (pLogEntryHeader->nLogType == pDevicePrivateData->FWInfo.nLogOverflowType)
		{
			rc = xnOSStrFormat((XnChar*)csBuffer + nBufferUsed, nBufferSize - nBufferUsed, &nCharsWritten, "%u:\tLog Overflow\n", pLogEntryHeader->nTimeStamp);
			XN_IS_STATUS_OK(rc);

			nBufferUsed += nCharsWritten;
		}
		else
		{
			XnLogDefaultData* pData = (XnLogDefaultData*)pCurrData;
			
			pData->nLine = XN_PREPARE_VAR16_IN_BUFFER(pData->nLine);
			pData->nParam = XN_PREPARE_VAR16_IN_BUFFER(pData->nParam);

			rc = xnOSStrFormat((XnChar*)csBuffer + nBufferUsed, nBufferSize - nBufferUsed, &nCharsWritten, 
				"%u:\tModule: [0x%X], Error: [0x%X], Param: 0x%X, (Line: %d)\n", 
				pLogEntryHeader->nTimeStamp, XnChar(pLogEntryHeader->nLogType >> 8), 
				XnChar(pLogEntryHeader->nLogType), pData->nParam, pData->nLine);
			XN_IS_STATUS_OK(rc);

			nBufferUsed += nCharsWritten;
		}

		pCurrData += pLogEntryHeader->nEntrySize*sizeof(XnUInt16);
	}

	if (nBufferUsed > 0)
	{
		// add null termination
		csBuffer[nBufferUsed] = '\0';
		nBufferUsed++;
	}

	return XN_STATUS_OK;	
}

XnStatus XnHostProtocolGetVersion(const XnDevicePrivateData* pDevicePrivateData, XnVersions& Version)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUInt16 nDataSize;
	XnVersions *pVersion = NULL;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting hardware versions...");

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetVersion);
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetVersion,
										(XnUChar**)(&pVersion), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Get version failed: %s", xnGetStatusString(rc));

		return rc;
	}

	xnOSMemCopy(&Version, pVersion, sizeof(XnVersions));

	Version.nBuild = XN_PREPARE_VAR16_IN_BUFFER(Version.nBuild);
	Version.nChip = XN_PREPARE_VAR32_IN_BUFFER(Version.nChip);
	Version.nFPGA = XN_PREPARE_VAR16_IN_BUFFER(Version.nFPGA);
	Version.nSystemVersion = XN_PREPARE_VAR16_IN_BUFFER(Version.nSystemVersion);

	*((XnUInt16*)&Version) = xnOSEndianSwapUINT16(*((XnUInt16*)pVersion));

	if (Version.nMajor >= 5)
	{
		XnChar cpBuffer[XN_MAX_OS_NAME_LENGTH];
		sprintf(cpBuffer, "%x", Version.nBuild);
		Version.nBuild = (XnUInt16)atoi(cpBuffer);
	}

	Version.SDK.nMajor = XN_PS_MAJOR_VERSION;
	Version.SDK.nMinor = XN_PS_MINOR_VERSION;
	Version.SDK.nMaintenance = XN_PS_MAINTENANCE_VERSION;
	Version.SDK.nBuild = XN_PS_BUILD_VERSION;

	// find out hardware version
	if (Version.nFPGA == XN_FPGA_VER_FPDB_26)
	{
		Version.HWVer = XN_SENSOR_HW_VER_FPDB_10;
	}
	else if (Version.nFPGA == XN_FPGA_VER_FPDB_25)
	{
		Version.HWVer = XN_SENSOR_HW_VER_FPDB_10;
	}
	else if (Version.nFPGA == XN_FPGA_VER_CDB)
	{
		Version.HWVer = XN_SENSOR_HW_VER_CDB_10;
	}
	else if (Version.nFPGA == XN_FPGA_VER_CDB)
	{
		Version.HWVer = XN_SENSOR_HW_VER_CDB_10;
	}
	else if (Version.nFPGA == XN_FPGA_VER_RD3)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD_3;
	}
	else if (Version.nFPGA == XN_FPGA_VER_RD5)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD_5;
	}
	else if (Version.nFPGA == XN_FPGA_VER_RD1081)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD1081;
	}	
	else if (Version.nFPGA == XN_FPGA_VER_RD1082)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD1082;
	}	
	else if (Version.nFPGA == XN_FPGA_VER_RD109)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD109;
	}		
	else
	{
		Version.HWVer = XN_SENSOR_HW_VER_UNKNOWN;
	}

	// find out chip version
	if (Version.nChip == XN_CHIP_VER_PS1000)
	{
		Version.ChipVer = XN_SENSOR_CHIP_VER_PS1000;
	}
	else if (Version.nChip == XN_CHIP_VER_PS1080)
	{
		Version.ChipVer = XN_SENSOR_CHIP_VER_PS1080;
	}
	else if (Version.nChip == XN_CHIP_VER_PS1080A6)
	{
		Version.ChipVer = XN_SENSOR_CHIP_VER_PS1080A6;
	}	
	else
	{
		Version.ChipVer = XN_SENSOR_CHIP_VER_UNKNOWN;
	}

	// find out sensor version
	Version.SensorVer = XN_SENSOR_VER_UNKNOWN;

	// in some firmwares, the HWVer was incorrect. Override according to firmware number
	Version.FWVer = GetFWVersion(Version.nMajor, Version.nMinor, Version.nBuild);
	if (Version.FWVer == XN_SENSOR_FW_VER_5_0)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD_5;
	}
	else if (Version.FWVer == XN_SENSOR_FW_VER_5_1)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD_5;
	}
	else if (Version.FWVer == XN_SENSOR_FW_VER_5_2)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD_5;
	}
	else if (Version.FWVer == XN_SENSOR_FW_VER_5_3)
	{
		if (Version.nBuild < 28)
		{
			Version.HWVer = XN_SENSOR_HW_VER_RD1081;
		}
		else if (Version.nBuild == 28)
		{
			Version.HWVer = XN_SENSOR_HW_VER_RD1082;
		}
		// 5.3.29 and up returns valid HW versions, so no need to override anything
	}
	else if (Version.FWVer == XN_SENSOR_FW_VER_5_4)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD1082;
	}
	else if (Version.FWVer == XN_SENSOR_FW_VER_5_5)
	{
		Version.HWVer = XN_SENSOR_HW_VER_RD1082;
	}
	else if (Version.FWVer == XN_SENSOR_FW_VER_5_6)
	{
		if (CompareVersion(Version.nMajor, Version.nMinor, Version.nBuild, 5, 6, 6) >= 0) 
		{
			if (Version.nFPGA == 0)
			{
				Version.HWVer = XN_SENSOR_HW_VER_RD1081;
			}
			else if (Version.nFPGA == 1)
			{
				Version.HWVer = XN_SENSOR_HW_VER_RD1082;
			}		
		}
		else
		{
			Version.HWVer = XN_SENSOR_HW_VER_RD1082;
		}
	}

	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, 
		"Hardware versions: FW=%d.%d.%d (%d) HW=%d Chip=%d Sensor=%d SYS=%d", 
		Version.nMajor, Version.nMinor, Version.nBuild, 
		Version.FWVer, Version.HWVer, Version.ChipVer, 
		Version.SensorVer, Version.nSystemVersion);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolKeepAlive(XnDevicePrivateData* pDevicePrivateData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Requesting KeepAlive...");

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeKeepAlive);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeKeepAlive,
										NULL, nDataSize);

	if (rc == XN_STATUS_OK)
		xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Got KeepAlive Reply.");
	else
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "KeepAlive failed: %s", xnGetStatusString(rc));

	return rc;
}

XnStatus XnHostProtocolReadAHB(XnDevicePrivateData* pDevicePrivateData, XnUInt32 nAddress, XnUInt32 &nValue)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt32*)pDataBuf = XN_PREPARE_VAR32_IN_BUFFER(nAddress);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt32), pDevicePrivateData->FWInfo.nOpcodeReadAHB);

	XnUInt16 nDataSize;
	XnUInt32* pValue = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt32), pDevicePrivateData->FWInfo.nOpcodeReadAHB,
										(XnUChar**)(&pValue), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	nValue = XN_PREPARE_VAR32_IN_BUFFER(*pValue);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolWriteAHB(XnDevicePrivateData* pDevicePrivateData, XnUInt32 nAddress, XnUInt32 nValue,
								XnUInt32 nMask)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Write AHB: 0x%08x 0x%08x 0x%08x", nAddress, nValue, nMask);

	*(XnUInt32*)pDataBuf = XN_PREPARE_VAR32_IN_BUFFER(nAddress);
	*(((XnUInt32*)pDataBuf)+1) = XN_PREPARE_VAR32_IN_BUFFER(nValue);
	*(((XnUInt32*)pDataBuf)+2) = XN_PREPARE_VAR32_IN_BUFFER(nMask);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt32)*3, pDevicePrivateData->FWInfo.nOpcodeWriteAHB);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt32)*3, pDevicePrivateData->FWInfo.nOpcodeWriteAHB,
										NULL, nDataSize);
	return rc;
}

XnStatus XnHostProtocolGetParam(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nParam, XnUInt16& nValue)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

//	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Getting Parameter [%d]...", nParam);

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nParam);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetParam);

	XnUInt16 nDataSize;
	XnUInt16* pValue = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetParam,
										(XnUChar**)(&pValue), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed getting [%d]: %s", nParam, xnGetStatusString(rc));
		return rc;
	}

//	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Param[%d] = %d", nParam, *pValue);
	nValue = XN_PREPARE_VAR16_IN_BUFFER(*pValue);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolSetParam(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nParam, XnUInt16 nValue)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

//	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Setting Parameter [%d] to %d", nParam, nValue);

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nParam);
	*(((XnUInt16*)pDataBuf)+1) = XN_PREPARE_VAR16_IN_BUFFER(nValue);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16)*2, pDevicePrivateData->FWInfo.nOpcodeSetParam);

	XnUInt16 nDataSize;

	XnInt32 nTimesLeft = 5;
	XnStatus rc = XN_STATUS_ERROR;
	while (nTimesLeft > 0)
	{
		rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16)*2, pDevicePrivateData->FWInfo.nOpcodeSetParam,
										NULL, nDataSize, XnHostProtocolGetSetParamRecvTimeOut(pDevicePrivateData, nParam));
		nTimesLeft--;

		if (rc == XN_STATUS_OK ||
			rc == XN_STATUS_DEVICE_PROTOCOL_BAD_PARAMS || 
			rc == XN_STATUS_DEVICE_NOT_CONNECTED ||
			rc == XN_STATUS_DEVICE_PROTOCOL_INVALID_COMMAND)
		{
			break;
		}

		xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Retrying to set the param... rc=%d", rc);
	}

	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed setting [%d] to [%d]: %s", nParam, nValue, xnGetStatusString(rc));
	}
	else
	{
//		xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Done.");
	}

	return rc;
}

void XnHostPrototcolAdjustFixedParamsV26(XnFixedParamsV26* pFixedParamsV26, XnFixedParams* pFixedParams)
{
	// the only difference from V2.6 to V3.0 is the 4 last parameters
	xnOSMemCopy(pFixedParams, pFixedParamsV26, sizeof(XnFixedParamsV26));
	pFixedParams->nUseExtPhy = pFixedParamsV26->nUseExtPhy;
	pFixedParams->bProjectorProtectionEnabled = FALSE;
	pFixedParams->nProjectorDACOutputVoltage = FALSE;
	pFixedParams->nTecEmitterDelay = pFixedParamsV26->nTecEmitterDelay;
}

void XnHostPrototcolAdjustFixedParamsV20(XnFixedParamsV20* pFixedParamsV20, XnFixedParams* pFixedParams)
{
	// the only difference from V2.0 to V2.6 is the addition of nUseExtPhy
	XnFixedParamsV26 fixedParamsV26;
	xnOSMemCopy(&fixedParamsV26, pFixedParamsV20, sizeof(XnFixedParamsV20));

	// now adjust from V2.6 to current
	XnHostPrototcolAdjustFixedParamsV26(&fixedParamsV26, pFixedParams);
}

XnStatus XnHostProtocolGetFixedParams(XnDevicePrivateData* pDevicePrivateData, XnFixedParams& FixedParams)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUChar* pRelevantBuffer;
	XnUInt16 nFixedParamSize = 0;

	XnChar FixedParamsBuffer[2048] = {0};
	XnChar* pData = FixedParamsBuffer;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting the fixed params...");

	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_3_0)
	{
		nFixedParamSize = sizeof(XnFixedParams);
	}
	else if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_1_1)
	{
		nFixedParamSize = sizeof(XnFixedParamsV26);
	}
	else // v0.17
	{
		nFixedParamSize = sizeof(XnFixedParamsV20);
	}

	xnOSMemSet(&FixedParams, 0, sizeof(XnFixedParams));

	XnInt16 nDataRead = 0;

	XnStatus rc;
	while (nDataRead < nFixedParamSize)
	{
		*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(XnUInt16(nDataRead/sizeof(XnUInt32)));
		XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetFixedParams);
		
		XnUInt16 nDataSize;

		rc = XnHostProtocolExecute(pDevicePrivateData,
									buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetFixedParams,
									&pRelevantBuffer, nDataSize);

		if (rc != XN_STATUS_OK)
		{
			xnLogError(XN_MASK_SENSOR_PROTOCOL, "Get fixed params failed: %s", xnGetStatusString(rc));

			return rc;
		}

		XnUInt32 nReadNow = nDataSize*sizeof(XnUInt16);
		if (nReadNow == 0)
		{
			break;
		}

		xnOSMemCopy(pData + nDataRead, pRelevantBuffer, nReadNow);

		nDataRead += (XnUInt16)nReadNow;
	}

	for (XnUInt32 i = 0; i < nFixedParamSize/sizeof(XnUInt32); i ++)
	{
		XnUInt32 temp = *((XnUInt32*)(&FixedParams)+i);
		*((XnUInt32*)(&FixedParams)+i) = XN_PREPARE_VAR32_IN_BUFFER(temp);
	}

	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_3_0)
	{
		xnOSMemCopy(&FixedParams, FixedParamsBuffer, sizeof(XnFixedParams));
	}
	else if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_1_1)
	{
		XnFixedParamsV26 fixedParamsV26;
		xnOSMemCopy(&fixedParamsV26, FixedParamsBuffer, nFixedParamSize);
		XnHostPrototcolAdjustFixedParamsV26(&fixedParamsV26, &FixedParams);
	}
	else if (pDevicePrivateData->FWInfo.nFWVer == XN_SENSOR_FW_VER_0_17)
	{
		XnFixedParamsV20 fixedParamsV20;
		xnOSMemCopy(&fixedParamsV20, FixedParamsBuffer, nFixedParamSize);
		XnHostPrototcolAdjustFixedParamsV20(&fixedParamsV20, &FixedParams);
	}

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolGetMode(XnDevicePrivateData* pDevicePrivateData, XnUInt16& nMode)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetMode);

	XnUInt16 nDataSize;
	XnUInt16* pMode = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetMode,
										(XnUChar**)(&pMode), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Get mode failed: %s", xnGetStatusString(rc));

		return rc;
	}
	nMode = XN_PREPARE_VAR16_IN_BUFFER(*pMode);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolReset(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nResetType)
{
	XnStatus rc = XN_STATUS_OK;

	if (pDevicePrivateData->FWInfo.nFWVer == XN_SENSOR_FW_VER_0_17)
	{
		XnUChar buffer[MAX_PACKET_SIZE] = {0};
		XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

		*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nResetType);

		XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeReset);

		XnUInt16 nDataSize;

		rc = XnHostProtocolExecute(pDevicePrivateData, 
			buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeReset,
			NULL, nDataSize);

		// Power reset can't fail, and device won't have time to send ACK.
		if (nResetType == XN_RESET_TYPE_POWER)
			rc = XN_STATUS_OK;

		return rc;
	}
	else
	{
		XnUInt16 nActualValue;
		switch (nResetType)
		{
		case XN_RESET_TYPE_POWER:
			nActualValue = XN_HOST_PROTOCOL_MODE_REBOOT;
			break;
		case XN_RESET_TYPE_SOFT:
			{
				// also kill streams before (in FW < 5.2)
				if (pDevicePrivateData->FWInfo.nFWVer < XN_SENSOR_FW_VER_5_2)
				{
					XnSensorFirmwareParams* pParams = pDevicePrivateData->pSensor->GetFirmware()->GetParams();
					rc = pParams->m_Stream0Mode.SetValue(XN_VIDEO_STREAM_OFF);
					XN_IS_STATUS_OK(rc);
					rc = pParams->m_Stream1Mode.SetValue(XN_VIDEO_STREAM_OFF);
					XN_IS_STATUS_OK(rc);
					rc = pParams->m_Stream2Mode.SetValue(XN_AUDIO_STREAM_OFF);
					XN_IS_STATUS_OK(rc);
				}
			}

			nActualValue = XN_HOST_PROTOCOL_MODE_SOFT_RESET;
			break;
		case XN_RESET_TYPE_SOFT_FIRST:
			nActualValue = XN_HOST_PROTOCOL_MODE_SOFT_RESET;
			break;
		default:
			return XN_STATUS_DEVICE_UNSUPPORTED_MODE;
		}

		return XnHostProtocolSetMode(pDevicePrivateData, nActualValue);
	}
}

XnStatus XnHostProtocolSetMode(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nMode)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nMode);

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Setting mode to %d...", nMode);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeSetMode);

	XnUInt16 nDataSize;

	XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeSetMode,
										NULL, nDataSize);

	// Patch: always return OK when switching modes (since the firmware is changing there is nobody to ACK the request...)
	return XN_STATUS_OK;
}

XnStatus XnHostProtocolGetCMOSRegister(XnDevicePrivateData* pDevicePrivateData, XnCMOSType nCMOS, XnUInt16 nAddress,
									   XnUInt16& nValue)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)nCMOS);
	*(((XnUInt16*)pDataBuf)+1) = XN_PREPARE_VAR16_IN_BUFFER(nAddress);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16)*2, pDevicePrivateData->FWInfo.nOpcodeGetCMOSReg);

	XnUInt16 nDataSize;
	XnUInt16* pValue = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16)*2,
										pDevicePrivateData->FWInfo.nOpcodeGetCMOSReg, (XnUChar**)(&pValue), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	nValue = XN_PREPARE_VAR16_IN_BUFFER(*pValue);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolSetCMOSRegister(XnDevicePrivateData* pDevicePrivateData, XnCMOSType nCMOS, XnUInt16 nAddress,
									   XnUInt16 nValue)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)nCMOS);
	*(((XnUInt16*)pDataBuf)+1) = XN_PREPARE_VAR16_IN_BUFFER(nAddress);
	*(((XnUInt16*)pDataBuf)+2) = XN_PREPARE_VAR16_IN_BUFFER(nValue);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16)*3, pDevicePrivateData->FWInfo.nOpcodeSetCMOSReg);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16)*3,
										pDevicePrivateData->FWInfo.nOpcodeSetCMOSReg, NULL, nDataSize);

	return rc;
}

XnStatus XnHostProtocolReadI2C(XnDevicePrivateData* pDevicePrivateData, XnI2CReadData* pI2CReadData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(pI2CReadData->nBus);
	*(((XnUInt16*)pDataBuf)+1) = XN_PREPARE_VAR16_IN_BUFFER(pI2CReadData->nSlaveAddress);
	*(((XnUInt16*)pDataBuf)+2) = XN_PREPARE_VAR16_IN_BUFFER(pI2CReadData->nReadSize);
	for (int i = 0; i < pI2CReadData->nWriteSize; i++)
		*((XnUInt16*)pDataBuf+3+i) = XN_PREPARE_VAR16_IN_BUFFER(pI2CReadData->cpWriteBuffer[i]);

	XnUInt16 nOpSize = sizeof(XnUInt16)*3 + (pI2CReadData->nWriteSize * sizeof(XnUInt16));
	XnHostProtocolInitHeader(pDevicePrivateData, buffer, nOpSize, pDevicePrivateData->FWInfo.nOpcodeReadI2C);

	XnUInt16 nDataSize;
	XnUInt16* pValue = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+nOpSize,
		pDevicePrivateData->FWInfo.nOpcodeReadI2C, (XnUChar**)(&pValue), nDataSize);

	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	for (int i = 0; i < nDataSize; i++)
		pI2CReadData->cpReadBuffer[i] = XN_PREPARE_VAR16_IN_BUFFER(*(pValue+i));

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolWriteI2C(XnDevicePrivateData* pDevicePrivateData, const XnI2CWriteData* pI2CWriteData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(pI2CWriteData->nBus);
	*(((XnUInt16*)pDataBuf)+1) = XN_PREPARE_VAR16_IN_BUFFER(pI2CWriteData->nSlaveAddress);
	for (int i = 0; i < pI2CWriteData->nWriteSize; i++)
		*((XnUInt16*)pDataBuf+2+i) = XN_PREPARE_VAR16_IN_BUFFER(pI2CWriteData->cpWriteBuffer[i]);

	XnUInt16 nOpSize = sizeof(XnUInt16)*2 + (pI2CWriteData->nWriteSize * sizeof(XnUInt16));
	XnHostProtocolInitHeader(pDevicePrivateData, buffer, nOpSize, pDevicePrivateData->FWInfo.nOpcodeWriteI2C);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+nOpSize,
		pDevicePrivateData->FWInfo.nOpcodeWriteI2C, NULL, nDataSize);
	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolGetCMOSRegisterI2C(XnDevicePrivateData* pDevicePrivateData, XnCMOSType nCMOS, XnUInt16 nAddress,
										  XnUInt16& nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnI2CReadData I2CReadData;

	nValue = 0;
	I2CReadData.cpReadBuffer[0] = 0;
	I2CReadData.cpReadBuffer[1] = 0;

	I2CReadData.nReadSize = XN_PREPARE_VAR16_IN_BUFFER(2);
	I2CReadData.nWriteSize = XN_PREPARE_VAR16_IN_BUFFER(1);
	I2CReadData.cpWriteBuffer[0]=XN_PREPARE_VAR16_IN_BUFFER(nAddress);

	if (nCMOS == XN_CMOS_TYPE_IMAGE)
	{
		I2CReadData.nBus = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetImageCmosI2CBus());
		I2CReadData.nSlaveAddress = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetImageCmosI2CSlaveAddress());
	}
	else if (nCMOS == XN_CMOS_TYPE_DEPTH)
	{
		I2CReadData.nBus = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetDepthCmosI2CBus());
		I2CReadData.nSlaveAddress = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetDepthCmosI2CSlaveAddress());
	}
	else
	{
		return (XN_STATUS_ERROR);
	}

	nRetVal = XnHostProtocolReadI2C(pDevicePrivateData, &I2CReadData);
	XN_IS_STATUS_OK(nRetVal);

	nValue = XN_PREPARE_VAR16_IN_BUFFER((I2CReadData.cpReadBuffer[0] << 8) + I2CReadData.cpReadBuffer[1]);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolSetCMOSRegisterI2C(XnDevicePrivateData* pDevicePrivateData, XnCMOSType nCMOS, XnUInt16 nAddress,
									   XnUInt16 nValue)

{
	XnStatus nRetVal = XN_STATUS_OK;
	XnI2CWriteData I2CWriteData;

	I2CWriteData.cpWriteBuffer[0] = XN_PREPARE_VAR16_IN_BUFFER(nAddress);
	I2CWriteData.cpWriteBuffer[1] = XN_PREPARE_VAR16_IN_BUFFER((nValue >> 8) & 0xff);
	I2CWriteData.cpWriteBuffer[2] = XN_PREPARE_VAR16_IN_BUFFER(nValue & 0xff);
	I2CWriteData.nWriteSize = XN_PREPARE_VAR16_IN_BUFFER(3);

	if (nCMOS == XN_CMOS_TYPE_IMAGE)
	{
		I2CWriteData.nBus = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetImageCmosI2CBus());
		I2CWriteData.nSlaveAddress = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetImageCmosI2CSlaveAddress());
	}
	else if (nCMOS == XN_CMOS_TYPE_DEPTH)
	{
		I2CWriteData.nBus = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetDepthCmosI2CBus());
		I2CWriteData.nSlaveAddress = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->pSensor->GetFixedParams()->GetDepthCmosI2CSlaveAddress());
	}
	else
	{
		return (XN_STATUS_ERROR);
	}

	nRetVal = XnHostProtocolWriteI2C(pDevicePrivateData, &I2CWriteData);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolInitUpload(XnDevicePrivateData* pDevicePrivateData, XnUInt32 nOffset, XnUInt16 nAttributes,
								  XnUInt32 nSizeInWords, XN_FILE_HANDLE &FileToUpload, XnUInt32& nNextOffset)
{
	XnStatus rc = XN_STATUS_OK;

	if (pDevicePrivateData->FWInfo.bHasFilesystemLock)
	{
		rc = XnHostProtocolSetParam(pDevicePrivateData, PARAM_FILE_SYSTEM_LOCK, 0);
		if (rc != XN_STATUS_OK)
		{
			xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "Failed to unlock file system: %s", xnGetStatusString(rc));
			return rc;
		}
	}

	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt32*)pDataBuf = XN_PREPARE_VAR32_IN_BUFFER(nOffset);
	XnUInt32 *Size = (XnUInt32*)pDataBuf + 1;
	*Size = XN_PREPARE_VAR32_IN_BUFFER(nSizeInWords);
	XnUInt16 nHeaderSize;

	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_1_1)
	{
		*((XnUInt16*)((XnUInt32*)pDataBuf+2)) = XN_PREPARE_VAR16_IN_BUFFER(nAttributes);
		nHeaderSize = sizeof(XnUInt32)+sizeof(XnUInt32)+sizeof(XnUInt16);
	}
	else
	{
		nHeaderSize = sizeof(XnUInt32)+sizeof(XnUInt32);
	}

	XnUInt32 nReadFromFile = pDevicePrivateData->FWInfo.nProtocolMaxPacketSize - pDevicePrivateData->FWInfo.nProtocolHeaderSize - nHeaderSize;
	xnOSSeekFile64(FileToUpload, XN_OS_SEEK_SET, 0);
	xnOSReadFile(FileToUpload, pDataBuf + nHeaderSize, &nReadFromFile);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, nHeaderSize+nReadFromFile, pDevicePrivateData->FWInfo.nOpcodeInitFileUpload);

	XnUInt16 nDataSize;
	XnUInt32* pValue;
	
	XnHostProtocolExecute(pDevicePrivateData, 
						  buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize + nHeaderSize + (XnUInt16)nReadFromFile,
						  pDevicePrivateData->FWInfo.nOpcodeInitFileUpload, (XnUChar**)(&pValue), nDataSize);

	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	nNextOffset = XN_PREPARE_VAR32_IN_BUFFER(*pValue);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolWriteUpload(XnDevicePrivateData* pDevicePrivateData, XN_FILE_HANDLE &FileToUpload, 
								   XnUInt32 nOffset, XnUInt32 nFileSize, XnUInt32& nNextOffset)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	XnUInt32 nChunkSize = pDevicePrivateData->FWInfo.nProtocolMaxPacketSize - pDevicePrivateData->FWInfo.nProtocolHeaderSize - sizeof(XnUInt32);

	if (nFileSize-nNextOffset*sizeof(XnUInt16) < nChunkSize)
	{
		nChunkSize = nFileSize-nNextOffset*sizeof(XnUInt16);
	}

	*(XnUInt32*)pDataBuf = XN_PREPARE_VAR32_IN_BUFFER(nOffset);
	pDataBuf += sizeof(XnUInt32);

	XnStatus rc = xnOSSeekFile64(FileToUpload, XN_OS_SEEK_SET, nNextOffset*sizeof(XnUInt16));
	XN_IS_STATUS_OK(rc);

	rc = xnOSReadFile(FileToUpload, pDataBuf, &nChunkSize);
	XN_IS_STATUS_OK(rc);

	if (nChunkSize == 1)
	{
		pDataBuf[nChunkSize] = 0;
		nChunkSize++;
	}

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt32)+nChunkSize, pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload);

	XnUInt16 nDataSize;
	XnUInt32* pValue;

	rc = XnHostProtocolExecute(pDevicePrivateData, 
								buffer, (XnUInt16)(pDevicePrivateData->FWInfo.nProtocolHeaderSize + sizeof(XnUInt32) + nChunkSize),
								pDevicePrivateData->FWInfo.nOpcodeWriteFileUpload, (XnUChar**)(&pValue), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	nNextOffset = XN_PREPARE_VAR32_IN_BUFFER(*pValue);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolFinishUpload	(XnDevicePrivateData* pDevicePrivateData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeFinishFileUpload);

	XnUInt16 nDataSize;
	
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize,
		pDevicePrivateData->FWInfo.nOpcodeFinishFileUpload, NULL, nDataSize);

	return rc;
}

XnStatus XnHostProtocolFileUpload(XnDevicePrivateData* pDevicePrivateData, XnUInt32 nOffset,
								  const XnChar* strFileName, XnUInt16 nAttributes)
{
	XnStatus rc;
	XnUInt64 nFileSize;
	XN_FILE_HANDLE UploadFile;

	rc = xnOSGetFileSize64(strFileName, &nFileSize);
	XN_IS_STATUS_OK(rc);

	rc = xnOSOpenFile(strFileName, XN_OS_FILE_READ, &UploadFile);
	XN_IS_STATUS_OK(rc);

	if (nFileSize % 2 == 1)
		nFileSize++;

	XnUInt32 nNextOffset;

	XnUInt64 nBefore;
	xnOSGetTimeStamp(&nBefore);

	rc = XnHostProtocolInitUpload(pDevicePrivateData, nOffset, nAttributes, (XnUInt32)nFileSize/sizeof(XnUInt16), UploadFile, nNextOffset);
	if (rc != XN_STATUS_OK)
	{
		xnOSCloseFile(&UploadFile);
		return (rc);
	}

	XnUInt64 nNow;
	xnOSGetTimeStamp(&nNow);

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Initialized upload of %llu bytes in %llu ms", nFileSize, nNow - nBefore);

	xnOSGetTimeStamp(&nBefore);

	XnUInt32 nLastPrintBytes = 0;
	while (nNextOffset*sizeof(XnUInt16) < nFileSize)
	{
		while ((nNextOffset*sizeof(XnUInt16) - nLastPrintBytes) > 5000)
		{
			printf(".");
			nLastPrintBytes += 5000;
		}

		rc = XnHostProtocolWriteUpload(pDevicePrivateData, UploadFile, nNextOffset, (XnUInt32)nFileSize, nNextOffset);
		if (rc != XN_STATUS_OK)
		{
			xnOSCloseFile(&UploadFile);
			return (rc);
		}
	}
	printf("\n");

	xnOSGetTimeStamp(&nNow);

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Uploaded %llu bytes in %llu ms", nFileSize, nNow - nBefore);

	rc = XnHostProtocolFinishUpload(pDevicePrivateData);
	if (rc != XN_STATUS_OK)
	{
		xnOSCloseFile(&UploadFile);
		return (rc);
	}

	xnOSCloseFile(&UploadFile);

	return rc;
}

XnStatus XnHostProtocolDeleteFile(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nFileId)
{
	XnStatus rc = XN_STATUS_OK;

	if (pDevicePrivateData->FWInfo.bHasFilesystemLock)
	{
		rc = XnHostProtocolSetParam(pDevicePrivateData, PARAM_FILE_SYSTEM_LOCK, 0);
		if (rc != XN_STATUS_OK)
		{
			xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "Failed to unlock file system: %s", xnGetStatusString(rc));
			return rc;
		}
	}

	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nFileId);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeDeleteFile);

	XnUInt16 nDataSize;

	rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeDeleteFile,
										NULL, nDataSize);

	return rc;
}

XnStatus XnHostProtocolSetFileAttributes(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nFileId, XnUInt16 nAttributes)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nFileId);
	*(((XnUInt16*)pDataBuf)+1) = XN_PREPARE_VAR16_IN_BUFFER(nAttributes);


	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 2*sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeSetFileAttribute);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+2*sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeSetFileAttribute,
										NULL, nDataSize);

	return rc;
}

XnStatus XnHostProtocolExecuteFile(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nFileId)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nFileId);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeExecuteFile);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeExecuteFile,
										NULL, nDataSize);

	return rc;
}

XnStatus XnHostProtocolGetFlashMap(XnDevicePrivateData* pDevicePrivateData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetFlashMap);

	XnUInt nRead;
	XnUChar* pRelevantBuffer;
	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetFlashMap,
										&pRelevantBuffer, nDataSize);
	if (rc != XN_STATUS_OK)
	{
//		printf("Get Flash MapExecution failed\n");
		return rc;
	}

	// Deal with specific log reply
#pragma pack(push, 1)
	typedef struct
	{
		XnUInt16 nFileType;
		XnUInt32 nOffsetInFlash;
		XnUInt32 nSizeInWords;
		struct
		{
			XnUInt8 nMajor;
			XnUInt8 nMinor;
			XnUInt16 nBuild;
		} Version;
	} XnFlashEntry;
#pragma pack (pop)

	nRead = pDevicePrivateData->FWInfo.nProtocolHeaderSize+nDataSize*2;
	XnFlashEntry* pFlashEntry;

	// Go over log and print it out
	while (pRelevantBuffer < buffer + nRead)
	{
		pFlashEntry = (XnFlashEntry*)pRelevantBuffer;

		pFlashEntry->nFileType = XN_PREPARE_VAR16_IN_BUFFER(pFlashEntry->nFileType);
		pFlashEntry->nOffsetInFlash = XN_PREPARE_VAR32_IN_BUFFER(pFlashEntry->nOffsetInFlash);
		pFlashEntry->nSizeInWords = XN_PREPARE_VAR32_IN_BUFFER(pFlashEntry->nSizeInWords);
		pFlashEntry->Version.nBuild = XN_PREPARE_VAR16_IN_BUFFER(pFlashEntry->Version.nBuild);

		printf("File Type: %d\n", pFlashEntry->nFileType);
		printf("Offset: %u\n", pFlashEntry->nOffsetInFlash);
		printf("Size in Words: %u\n", pFlashEntry->nSizeInWords);
		printf("Version: %d.%d.%d\n", pFlashEntry->Version.nMajor, pFlashEntry->Version.nMinor,
			pFlashEntry->Version.nBuild);

		pRelevantBuffer += sizeof(XnFlashEntry);
	}

	return XN_STATUS_OK;
}

#pragma pack (push, 1)
typedef struct XnAlgorithmParamRequest
{
	XnUInt16 nParamID;
	XnUInt16 nFormat;
	XnUInt16 nResolution;
	XnUInt16 nFPS;
	XnUInt16 nOffset;
} XnAlgorithmParamRequest;

typedef struct XnAlgorithmParamRequestV4
{
	XnUInt8 nResolution;
	XnUInt8 nFPS;
	XnUInt8 nFormat;
	XnUInt8 nParamID;
	XnUInt16 nOffset;
} XnAlgorithmParamRequestV4;
#pragma pack (pop)

XnStatus XnHostProtocolAlgorithmParams(XnDevicePrivateData* pDevicePrivateData,
									   XnHostProtocolAlgorithmType eAlgorithmType,
									   void* pAlgorithmInformation, XnUInt16 nAlgInfoSize, XnResolutions nResolution, XnUInt16 nFPS)
{
	XnChar* pData = (XnChar*)pAlgorithmInformation;
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUChar* pRelevantBuffer;

	XnInt16 nDataRead = 0;
	XnUInt16 nRequestSize = 0;

	if (eAlgorithmType == XN_HOST_PROTOCOL_ALGORITHM_DEVICE_INFO && 
		!pDevicePrivateData->FWInfo.bDeviceInfoSupported)
	{
		XnDeviceInformation* pDeviceInfo = (XnDeviceInformation*)pAlgorithmInformation;
		strcpy(pDeviceInfo->strDeviceName, "PrimeSense Sensor");
		strcpy(pDeviceInfo->strVendorData, "");
		return XN_STATUS_OK;
	}

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting algorithm params 0x%x for resolution %d and fps %d....", eAlgorithmType, nResolution, nFPS);

	XnStatus rc;
	while (nDataRead < nAlgInfoSize)
	{
		if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_1)
		{
			XnAlgorithmParamRequest* pRequest = (XnAlgorithmParamRequest*)pDataBuf;
			pRequest->nParamID = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)eAlgorithmType);
			pRequest->nFormat = 0;
			pRequest->nResolution = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)nResolution);
			pRequest->nFPS = XN_PREPARE_VAR16_IN_BUFFER(nFPS);
			pRequest->nOffset = XN_PREPARE_VAR16_IN_BUFFER(nDataRead / sizeof(XnUInt16));
			nRequestSize = sizeof(XnAlgorithmParamRequest);
		}
		else
		{
			XnAlgorithmParamRequestV4* pRequest = (XnAlgorithmParamRequestV4*)pDataBuf;
			pRequest->nParamID = (XnUInt8)eAlgorithmType;
			pRequest->nFormat = 0;
			pRequest->nResolution = (XnUInt8)nResolution;
			pRequest->nFPS = 0;
			pRequest->nOffset = XN_PREPARE_VAR16_IN_BUFFER(nDataRead / sizeof(XnUInt16));
			nRequestSize = sizeof(XnAlgorithmParamRequestV4);
		}

		XnHostProtocolInitHeader(pDevicePrivateData, buffer, nRequestSize, pDevicePrivateData->FWInfo.nOpcodeAlgorithmParams);
		
		XnUInt16 nDataSize;
		rc = XnHostProtocolExecute(pDevicePrivateData, 
									buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+nRequestSize, pDevicePrivateData->FWInfo.nOpcodeAlgorithmParams,
									&pRelevantBuffer, nDataSize);


		if (rc != XN_STATUS_OK)
			return rc;

		XnUInt16 nReadNow = (XnUInt16)(nDataSize*sizeof(XnUInt16));
		if (nReadNow == 0)
		{
			break;
		}

		xnOSMemCopy(pData + nDataRead, pRelevantBuffer, nReadNow);

		nDataRead += nReadNow;
	}

	if (nDataRead != nAlgInfoSize)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_IO_DEVICE_INVALID_RESPONSE_SIZE, XN_MASK_SENSOR_PROTOCOL, "Failed getting algorithm params: expected %u bytes, but got only %u", nAlgInfoSize, nDataRead);
	}

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolTakeSnapshot(XnDevicePrivateData* pDevicePrivateData, XnCMOSType nCMOS)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)nCMOS);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeTakeSnapshot);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeTakeSnapshot,
										NULL, nDataSize);

	return rc;
}

XnStatus XnHostProtocolGetFileList(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nFirstFileId, XnFlashFile* pFileList, XnUInt16& nNumOfEntries)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUChar* pRelevantBuffer;
	XnUInt32 nBytesRead = 0;
	XnBool bDone = false;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting file list");

	for (;;)
	{
		*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nFirstFileId);

		XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetFileList);

		XnUInt16 nDataSize;

		XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
			buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetFileList,
			&pRelevantBuffer, nDataSize);
		if (rc != XN_STATUS_OK)
		{
			return rc;
		}

		XnUInt32 DataSizeInBytes = nDataSize*sizeof(XnUInt16);

		if (DataSizeInBytes == 0)
		{
			// Done
			break;
		}
		if (nBytesRead + DataSizeInBytes > nNumOfEntries*sizeof(XnFlashFile))
		{
			DataSizeInBytes = nNumOfEntries*sizeof(XnFlashFile) - nBytesRead;
			bDone = true;
		}

		xnOSMemCopy(((XnChar*)pFileList) + nBytesRead, pRelevantBuffer, DataSizeInBytes);

		nBytesRead += DataSizeInBytes;
		nFirstFileId = XN_PREPARE_VAR16_IN_BUFFER(pFileList[nBytesRead/sizeof(XnFlashFile)-1].nId)+1;

		if (bDone)
			break;
	}

	nNumOfEntries = (XnUInt16)(nBytesRead/sizeof(XnFlashFile));

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolFileDownloadChunk(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nFileType,
										 XnUInt32 nOffset, XnChar* pData, XnUInt16& nChunkSize)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUChar* pRelevantBuffer;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER(nFileType);
	*(XnUInt32*)((((XnUInt16*)pDataBuf)+1)) = XN_PREPARE_VAR32_IN_BUFFER(XnUInt32(nOffset/sizeof(XnUInt16)));

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16)+sizeof(XnUInt32), pDevicePrivateData->FWInfo.nOpcodeDownloadFile);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
										buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16)+sizeof(XnUInt32), pDevicePrivateData->FWInfo.nOpcodeDownloadFile,
										&pRelevantBuffer, nDataSize);
	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	if (nChunkSize < nDataSize*sizeof(XnUInt16))
	{
		// received too much.
		return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
	}
	
	nChunkSize = nDataSize*sizeof(XnUInt16);

	xnOSMemCopy(pData, pRelevantBuffer, nChunkSize);

	return XN_STATUS_OK;
	
}

XnStatus XnHostProtocolFileDownload(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nFileType,
									const XnChar* strFileName)
{
	XN_FILE_HANDLE File;
	XnStatus rc = XN_STATUS_OK;

	rc = xnOSOpenFile(strFileName, XN_OS_FILE_WRITE|XN_OS_FILE_TRUNCATE, &File);
	XN_IS_STATUS_OK(rc);

	XnChar Buffer[MAX_PACKET_SIZE];
	XnUInt16 nChunkSize = 0;
	XnUInt32 nOffset = 0;
	
	XnUInt32 nLastPrintBytes = 0;

	for (;;)
	{
		while ((nOffset - nLastPrintBytes) > 5000)
		{
			printf(".");
			nLastPrintBytes += 5000;
		}

		nChunkSize = MAX_PACKET_SIZE;

		rc = XnHostProtocolFileDownloadChunk(pDevicePrivateData, nFileType, nOffset, Buffer, nChunkSize);

		if (rc != XN_STATUS_OK || nChunkSize == 0)
		{
			break;
		}

		rc = xnOSWriteFile(File, Buffer, nChunkSize);
		if (rc != XN_STATUS_OK)
			break;
		nOffset += nChunkSize;
	}

	printf("\n");

	xnOSCloseFile(&File);

	return rc;
}

#define XN_HOST_PROTOCOL_INIT_BUFFER(pBuffer)	\
	XnUChar* __pBuffer = (XnUChar*)pBuffer;	\
	XnUInt16 __nBufferSize = 0;

#define XN_HOST_PROTOCOL_APPEND_PARAM(type, param)	\
	*(type*)__pBuffer = (type)param;				\
	__pBuffer += sizeof(type);						\
	__nBufferSize += sizeof(type);

#define XN_HOST_PROTOCOL_SIZE	__nBufferSize

XnStatus XnHostProtocolReadFlashChunk(XnDevicePrivateData* pDevicePrivateData, XnUInt32 nOffset, XnUChar* pData, XnUInt16* nChunkSize)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUChar* pRelevantBuffer;

	XN_HOST_PROTOCOL_INIT_BUFFER(pDataBuf);
	XN_HOST_PROTOCOL_APPEND_PARAM(XnUInt32, XN_PREPARE_VAR32_IN_BUFFER(nOffset));
	XN_HOST_PROTOCOL_APPEND_PARAM(XnUInt16, XN_PREPARE_VAR16_IN_BUFFER(*nChunkSize));

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, XN_HOST_PROTOCOL_SIZE, pDevicePrivateData->FWInfo.nOpcodeReadFlash);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+XN_HOST_PROTOCOL_SIZE, pDevicePrivateData->FWInfo.nOpcodeReadFlash,
		&pRelevantBuffer, nDataSize);

	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	// words to bytes
	if (*nChunkSize < nDataSize)
	{
		// received too much.
		return XN_STATUS_INTERNAL_BUFFER_TOO_SMALL;
	}

	*nChunkSize = nDataSize;

	// size is in words
	xnOSMemCopy(pData, pRelevantBuffer, nDataSize*sizeof(XnUInt16));

	return XN_STATUS_OK;

}

XnStatus XnHostProtocolReadFlash(XnDevicePrivateData* pDevicePrivateData, XnUInt32 nOffset, XnUInt32 nSize, XnUChar* pBuffer)
{
	XnStatus rc = XN_STATUS_OK;

	XnUInt32 nLoopOffset = nOffset;
	XnUInt16 nLoopChunkSize;
	XnUInt32 nReadSize = 0;

	XnUInt32 counter = 0;
	while (nReadSize < nSize)
	{
		if (counter % 100 == 0)
		{
			printf(".");
		}
		counter++;

		// don't ask for more than MAX UINT16
		nLoopChunkSize = (XnUInt16)XN_MIN(nSize - nReadSize, 0xFFFF);

		rc = XnHostProtocolReadFlashChunk(pDevicePrivateData, nLoopOffset, pBuffer + nReadSize*sizeof(XnUInt16), &nLoopChunkSize);

		if (rc != XN_STATUS_OK)
			return rc;

		if (nLoopChunkSize == 0)
			return XN_STATUS_ERROR;

		nLoopOffset += nLoopChunkSize;
		nReadSize += nLoopChunkSize;
	}

	printf("\n");

	return rc;
}

XnStatus XnHostProtocolRunBIST(XnDevicePrivateData* pDevicePrivateData, XnUInt32 nTestsMask, XnUInt32* pnFailures)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUInt32* pRelevantBuffer;

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)nTestsMask);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeBIST);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize + sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeBIST,
		(XnUChar**)&pRelevantBuffer, nDataSize);

	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	// the UINT32 received has a bit turned on for each failed module, so if all are off, everything is OK.
	*pnFailures = (*pRelevantBuffer);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolGetCPUStats(XnDevicePrivateData* pDevicePrivateData, XnTaskCPUInfo* pTasks, XnUInt32 *pnTimesCount)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUInt32* pRelevantBuffer;

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetCPUStats);

	XnUInt16 nDataSize;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetCPUStats,
		(XnUChar**)&pRelevantBuffer, nDataSize);

	if (rc != XN_STATUS_OK)
	{
		return rc;
	}

	// check how many numbers we got
	XnUInt32 nCount = nDataSize * sizeof(XnUInt16) / sizeof(XnTaskCPUInfo);

	// check if we have enough space in buffer
	if (nCount > *pnTimesCount)
	{
		xnLogWarning(XN_MASK_SENSOR_PROTOCOL, "CPUStats: no space in buffer for all tasks. Dropping last %d", nCount - *pnTimesCount);
		nCount = *pnTimesCount;
	}

	xnOSMemCopy(pTasks, pRelevantBuffer, nCount * sizeof(XnTaskCPUInfo));
	for (XnUInt32 i = 0; i < nCount; i++)
	{
		pTasks[i].nTimesExecuted = XN_PREPARE_VAR32_IN_BUFFER(pTasks[i].nTimesExecuted);
		pTasks[i].nTimeInMicroSeconds = XN_PREPARE_VAR32_IN_BUFFER(pTasks[i].nTimeInMicroSeconds);
	}

	*pnTimesCount = nCount;

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolSetAudioSampleRate(XnDevicePrivateData* pDevicePrivateData, XnSampleRate nSampleRate)
{
	EA2d_SampleRate nSample;

	switch (nSampleRate)
	{
	case XN_SAMPLE_RATE_8K:
		nSample = A2D_SAMPLE_RATE_8KHZ;
		break;
	case XN_SAMPLE_RATE_11K:
		nSample = A2D_SAMPLE_RATE_11KHZ;
		break;
	case XN_SAMPLE_RATE_12K:
		nSample = A2D_SAMPLE_RATE_12KHZ;
		break;
	case XN_SAMPLE_RATE_16K:
		nSample = A2D_SAMPLE_RATE_16KHZ;
		break;
	case XN_SAMPLE_RATE_22K:
		nSample = A2D_SAMPLE_RATE_22KHZ;
		break;
	case XN_SAMPLE_RATE_24K:
		nSample = A2D_SAMPLE_RATE_24KHZ;
		break;
	case XN_SAMPLE_RATE_32K:
		nSample = A2D_SAMPLE_RATE_32KHZ;
		break;
	case XN_SAMPLE_RATE_44K:
		nSample = A2D_SAMPLE_RATE_44KHZ;
		break;
	case XN_SAMPLE_RATE_48K:
		nSample = A2D_SAMPLE_RATE_48KHZ;
		break;
	default:
		return XN_STATUS_DEVICE_UNSUPPORTED_MODE;
	}

	return XnHostProtocolSetParam(pDevicePrivateData, PARAM_AUDIO_SAMPLE_RATE, (XnUInt16)nSample);
}

XnStatus XnHostProtocolGetAudioSampleRate(XnDevicePrivateData* pDevicePrivateData, XnSampleRate* pSampleRate)
{
	XnUInt16 nValue;
	XnHostProtocolGetParam(pDevicePrivateData, PARAM_AUDIO_SAMPLE_RATE, nValue);
	XnSampleRate nSample;

	switch (nValue)
	{
	case A2D_SAMPLE_RATE_8KHZ:
		nSample = XN_SAMPLE_RATE_8K;
		break;
	case A2D_SAMPLE_RATE_11KHZ:
		nSample = XN_SAMPLE_RATE_11K;
		break;
	case A2D_SAMPLE_RATE_12KHZ:
		nSample = XN_SAMPLE_RATE_12K;
		break;
	case A2D_SAMPLE_RATE_16KHZ:
		nSample = XN_SAMPLE_RATE_16K;
		break;
	case A2D_SAMPLE_RATE_22KHZ:
		nSample = XN_SAMPLE_RATE_22K;
		break;
	case A2D_SAMPLE_RATE_24KHZ:
		nSample = XN_SAMPLE_RATE_24K;
		break;
	case A2D_SAMPLE_RATE_32KHZ:
		nSample = XN_SAMPLE_RATE_32K;
		break;
	case A2D_SAMPLE_RATE_44KHZ:
		nSample = XN_SAMPLE_RATE_44K;
		break;
	case A2D_SAMPLE_RATE_48KHZ:
		nSample = XN_SAMPLE_RATE_48K;
		break;
	default:
		return XN_STATUS_DEVICE_UNSUPPORTED_MODE;
	}

	*pSampleRate = nSample;

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolSetMultipleParams(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nNumOfParams, XnInnerParamData* anParams)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	XnUInt16* pCurData = (XnUInt16*)pDataBuf;
	for (XnUInt16 nIndex = 0; nIndex < nNumOfParams; ++nIndex)
	{
		*pCurData++ = XN_PREPARE_VAR16_IN_BUFFER(anParams[nIndex].nParam);
		*pCurData++ = XN_PREPARE_VAR16_IN_BUFFER(anParams[nIndex].nValue);
	}

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16)*nNumOfParams*2, pDevicePrivateData->FWInfo.nOpcodeSetParam);

	XnUInt16 nDataSize;

	XnInt32 nTimesLeft = 5;
	XnStatus rc = XN_STATUS_ERROR;
	while (rc != XN_STATUS_OK && rc != XN_STATUS_DEVICE_PROTOCOL_BAD_PARAMS && 
		rc != XN_STATUS_DEVICE_PROTOCOL_INVALID_COMMAND && nTimesLeft > 0)
	{
		rc = XnHostProtocolExecute(pDevicePrivateData, 
			buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16)*nNumOfParams*2, pDevicePrivateData->FWInfo.nOpcodeSetParam,
			NULL, nDataSize);
		nTimesLeft--;
	}

	if (rc != XN_STATUS_OK)
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed: %s", xnGetStatusString(rc));

	return rc;
}

#pragma pack (push, 1)
typedef struct
{
	XnUInt16 nSetPoint;
} XnCalibrateTecRequest;
#pragma pack (pop)

XnStatus XnHostProtocolCalibrateTec(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nSetPoint)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Calibrating TEC. Set Point: %d", nSetPoint);

	XnCalibrateTecRequest* pRequest = (XnCalibrateTecRequest*)pDataBuf;
	pRequest->nSetPoint = XN_PREPARE_VAR16_IN_BUFFER(nSetPoint);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnCalibrateTecRequest), pDevicePrivateData->FWInfo.nOpcodeCalibrateTec);

	XnUInt16 nDataSize;
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
			buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnCalibrateTecRequest), pDevicePrivateData->FWInfo.nOpcodeCalibrateTec,
			NULL, nDataSize);

	if (rc != XN_STATUS_OK)
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed Calibrating TEC: %s", xnGetStatusString(rc));
	else
		xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Calibrating TEC succeeded.");

	return rc;
}

XnStatus XnHostProtocolGetTecData(XnDevicePrivateData* pDevicePrivateData, XnTecData* pTecData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUInt16 nDataSize;
	XnStatus rc;

	if (pDevicePrivateData->FWInfo.nFWVer < XN_SENSOR_FW_VER_5_4)
	{
		xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting TEC data...");

		XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetTecData);

		XnTecData* pResult;

		rc = XnHostProtocolExecute(pDevicePrivateData,
			buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetTecData,
			(XnUChar**)(&pResult), nDataSize);

		XN_IS_STATUS_OK(rc);

		pTecData->m_SetPointVoltage = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_SetPointVoltage);
		pTecData->m_CompensationVoltage = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_CompensationVoltage);
		pTecData->m_TecDutyCycle = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_TecDutyCycle);
		pTecData->m_HeatMode = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_HeatMode);
		pTecData->m_ProportionalError = XN_PREPARE_VAR32_IN_BUFFER(pResult->m_ProportionalError);
		pTecData->m_IntegralError = XN_PREPARE_VAR32_IN_BUFFER(pResult->m_IntegralError);
		pTecData->m_DerivativeError = XN_PREPARE_VAR32_IN_BUFFER(pResult->m_DerivativeError);
		pTecData->m_ScanMode = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_ScanMode);
	}
	else
	{
		XnTecFastConvergenceData TecFastConvergenceData;

		rc = XnHostProtocolGetTecFastConvergenceData(pDevicePrivateData, &TecFastConvergenceData);
		XN_IS_STATUS_OK(rc);

		pTecData->m_SetPointVoltage = 0;
		pTecData->m_CompensationVoltage = 0;
		pTecData->m_TecDutyCycle = TecFastConvergenceData.m_TecDutyCycle;
		pTecData->m_HeatMode = TecFastConvergenceData.m_HeatMode;
		pTecData->m_ProportionalError = TecFastConvergenceData.m_ProportionalError;
		pTecData->m_IntegralError = TecFastConvergenceData.m_IntegralError;
		pTecData->m_DerivativeError = TecFastConvergenceData.m_DerivativeError;

		// Convert the new modes (post FW 5.4) into the old modes (pre 5.4). 
		pTecData->m_ScanMode = (TecFastConvergenceData.m_ScanMode)-1;
	}

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolGetTecFastConvergenceData (XnDevicePrivateData* pDevicePrivateData, XnTecFastConvergenceData* pTecData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUInt16 nDataSize;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting TEC Fast Convergence data...");

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetFastConvergenceTEC);

	XnTecFastConvergenceData* pResult;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetFastConvergenceTEC,
		(XnUChar**)(&pResult), nDataSize);

	XN_IS_STATUS_OK(rc);

	pTecData->m_SetPointTemperature = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_SetPointTemperature);
	pTecData->m_MeasuredTemperature = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_MeasuredTemperature);
	pTecData->m_ProportionalError = XN_PREPARE_VAR32_IN_BUFFER(pResult->m_ProportionalError);
	pTecData->m_IntegralError = XN_PREPARE_VAR32_IN_BUFFER(pResult->m_IntegralError);
	pTecData->m_DerivativeError = XN_PREPARE_VAR32_IN_BUFFER(pResult->m_DerivativeError);
	pTecData->m_ScanMode = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_ScanMode);
	pTecData->m_HeatMode = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_HeatMode);
	pTecData->m_TecDutyCycle = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_TecDutyCycle);
	pTecData->m_TemperatureRange = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_TemperatureRange);

	return (XN_STATUS_OK);
}

#pragma pack (push, 1)
typedef struct
{
	XnUInt16 nSetPoint;
} XnCalibrateEmitterRequest;
#pragma pack (pop)

XnStatus XnHostProtocolCalibrateEmitter(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nSetPoint)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Calibrating Emitter. Set Point: %d", nSetPoint);

	XnCalibrateEmitterRequest* pRequest = (XnCalibrateEmitterRequest*)pDataBuf;
	pRequest->nSetPoint = XN_PREPARE_VAR16_IN_BUFFER(nSetPoint);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnCalibrateEmitterRequest), pDevicePrivateData->FWInfo.nOpcodeCalibrateEmitter);

	XnUInt16 nDataSize;
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnCalibrateEmitterRequest), pDevicePrivateData->FWInfo.nOpcodeCalibrateEmitter,
		NULL, nDataSize);

	if (rc != XN_STATUS_OK)
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed Calibrating Emitter: %s", xnGetStatusString(rc));
	else
		xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Calibrating Emitter succeeded.");

	return rc;
}

XnStatus XnHostProtocolGetEmitterData(XnDevicePrivateData* pDevicePrivateData, XnEmitterData* pEmitterData)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUInt16 nDataSize;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting Emitter data...");

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetEmitterData);

	XnEmitterData* pResult;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetEmitterData,
		(XnUChar**)(&pResult), nDataSize);

	XN_IS_STATUS_OK(rc);

	pEmitterData->m_State = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_State);
	pEmitterData->m_SetPointVoltage = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_SetPointVoltage);
	pEmitterData->m_SetPointClocks = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_SetPointClocks);
	pEmitterData->m_PD_Reading = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_PD_Reading);
	pEmitterData->m_EmitterSet = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_EmitterSet);
	pEmitterData->m_EmitterSettingLogic = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_EmitterSettingLogic);
	pEmitterData->m_LightMeasureLogic = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_LightMeasureLogic);
	pEmitterData->m_IsAPCEnabled = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_IsAPCEnabled);

	// set some version specific fields
	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_1)
	{
		pEmitterData->m_EmitterSetStepSize = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_EmitterSetStepSize);
		pEmitterData->m_ApcTolerance = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_ApcTolerance);
	}
	else
	{
		pEmitterData->m_EmitterSetStepSize = 0;
		pEmitterData->m_ApcTolerance = 0;
	}

	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_3)
	{
		pEmitterData->m_SubClocking = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_SubClocking);
		pEmitterData->m_Precision = XN_PREPARE_VAR16_IN_BUFFER(pResult->m_Precision);
	}
	else
	{
		pEmitterData->m_SubClocking = 0;
		pEmitterData->m_Precision = 0;
	}

	return (XN_STATUS_OK);
}

#pragma pack (push, 1)
typedef struct
{
	XnUInt16 nMinThreshold;
	XnUInt16 nMaxThreshold;
} XnProjectorFaultRequest;
#pragma pack (pop)

XnStatus XnHostProtocolCalibrateProjectorFault(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nMinThreshold, XnUInt16 nMaxThreshold, XnBool* pbProjectorFaultEvent)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUInt16 nDataSize;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Testing Projector Fault. Min Threshold: %u, Max Threshold: %u...", nMinThreshold, nMaxThreshold);

	XnProjectorFaultRequest* pRequest = (XnProjectorFaultRequest*)pDataBuf;
	pRequest->nMinThreshold = XN_PREPARE_VAR16_IN_BUFFER(nMinThreshold);
	pRequest->nMaxThreshold = XN_PREPARE_VAR16_IN_BUFFER(nMaxThreshold);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnProjectorFaultRequest), pDevicePrivateData->FWInfo.nOpcodeCalibrateProjectorFault);

	XnBool* pResult;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize + sizeof(XnProjectorFaultRequest), pDevicePrivateData->FWInfo.nOpcodeCalibrateProjectorFault,
		(XnUChar**)(&pResult), nDataSize);

	XN_IS_STATUS_OK(rc);

	*pbProjectorFaultEvent = *pResult;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Projector fault event: %d", *pResult);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceSensorGetDepthAGCParams(XnUInt16 nBin, XnUInt16* pnMinParam, XnUInt16* pnMaxParam)
{
	switch (nBin)
	{
	case 0:
		*pnMinParam = PARAM_DEPTH_AGC_BIN0_LOW;
		*pnMaxParam = PARAM_DEPTH_AGC_BIN0_HIGH;
		break;
	case 1:
		*pnMinParam = PARAM_DEPTH_AGC_BIN1_LOW;
		*pnMaxParam = PARAM_DEPTH_AGC_BIN1_HIGH;
		break;
	case 2:
		*pnMinParam = PARAM_DEPTH_AGC_BIN2_LOW;
		*pnMaxParam = PARAM_DEPTH_AGC_BIN2_HIGH;
		break;
	case 3:
		*pnMinParam = PARAM_DEPTH_AGC_BIN3_LOW;
		*pnMaxParam = PARAM_DEPTH_AGC_BIN3_HIGH;
		break;
	default:
		return XN_STATUS_DEVICE_BAD_PARAM;
	}

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolSetDepthAGCBin(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nBin, XnUInt16 nMinShift, XnUInt16 nMaxShift)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnUInt16 nMinParam;
	XnUInt16 nMaxParam;

	nRetVal = XnDeviceSensorGetDepthAGCParams(nBin, &nMinParam, &nMaxParam);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnHostProtocolSetParam(pDevicePrivateData, nMinParam, nMinShift);
	XN_IS_STATUS_OK(nRetVal);
	
	nRetVal = XnHostProtocolSetParam(pDevicePrivateData, nMaxParam, nMaxShift);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolGetDepthAGCBin(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nBin, XnUInt16* pnMinShift, XnUInt16* pnMaxShift)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnUInt16 nMinParam;
	XnUInt16 nMaxParam;

	nRetVal = XnDeviceSensorGetDepthAGCParams(nBin, &nMinParam, &nMaxParam);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnHostProtocolGetParam(pDevicePrivateData, nMinParam, *pnMinShift);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnHostProtocolGetParam(pDevicePrivateData, nMaxParam, *pnMaxShift);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

#pragma pack (push, 1)

typedef struct XnVSyncRequest
{
	XnUInt16 nUnits;
	XnUInt16 nCmosID;
	XnUInt16 nNumberOfFrames;
} XnVSyncRequest;

#pragma pack (pop)

XnStatus XnHostProtocolSetCmosBlanking(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nUnits, XnCMOSType nCMOSID, XnUInt16 nNumberOfFrames)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUInt32 nRequestSize;

	if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_5_1)
	{
		XnVSyncRequest* pRequest = (XnVSyncRequest*)pDataBuf;
		pRequest->nUnits = XN_PREPARE_VAR16_IN_BUFFER(nUnits);
		pRequest->nCmosID = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)nCMOSID);
		pRequest->nNumberOfFrames = XN_PREPARE_VAR16_IN_BUFFER(nNumberOfFrames);
		nRequestSize = sizeof(XnVSyncRequest);
	}
	else
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_IO_DEVICE_FUNCTION_NOT_SUPPORTED, XN_MASK_SENSOR_PROTOCOL, "Set Blanking is not supported by this firmware!");
	}

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Chaning CMOS %d Blanking to %hd (NumberOfFrames=%hu)...", nCMOSID, nUnits, nNumberOfFrames);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, nRequestSize, pDevicePrivateData->FWInfo.nOpcodeSetCmosBlanking);

	XnUInt16 nDataSize;
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize + (XnUInt16)nRequestSize, pDevicePrivateData->FWInfo.nOpcodeSetCmosBlanking,
		NULL, nDataSize);

	if (rc != XN_STATUS_OK)
	{
		XN_LOG_WARNING_RETURN(rc, XN_MASK_SENSOR_PROTOCOL, "Failed changing CMOS %d Blanking to %hd (NumberOfFrames=%hu): %s", nCMOSID, nUnits, nNumberOfFrames, xnGetStatusString(rc));
	}

	return (XN_STATUS_OK);
}

#pragma pack (push, 1)

typedef struct XnGetCmosBlankingRequest
{
	XnUInt16 nCmosID;
} XnGetCmosBlankingRequest;

typedef struct XnGetCmosBlankingReply
{
	XnUInt32 nUnits;
} XnGetCmosBlankingReply;

#pragma pack (pop)

XnStatus XnHostProtocolGetCmosBlanking(XnDevicePrivateData* pDevicePrivateData, XnCMOSType nCMOSID, XnUInt16* pnLines)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	XnGetCmosBlankingRequest* pRequest = (XnGetCmosBlankingRequest*)pDataBuf;
	pRequest->nCmosID = (XnUInt16)nCMOSID;

	xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "Getting Cmos %d VBlanking...", nCMOSID);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnGetCmosBlankingRequest), pDevicePrivateData->FWInfo.nOpcodeGetCmosBlanking);

	XnGetCmosBlankingReply* pReply;
	XnUInt16 nDataSize;
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize + sizeof(XnGetCmosBlankingRequest), pDevicePrivateData->FWInfo.nOpcodeGetCmosBlanking,
		(XnUChar**)&pReply, nDataSize);

	if (rc != XN_STATUS_OK)
	{
		XN_LOG_WARNING_RETURN(rc, XN_MASK_SENSOR_PROTOCOL, "Failed getting Cmos %d Blanking: %s", nCMOSID, xnGetStatusString(rc));
	}

	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Cmos %d VBlanking: %u", nCMOSID, pReply->nUnits);

	*pnLines = (XnUInt16)pReply->nUnits;

	return (XN_STATUS_OK);
}

XnStatus XnHostProtocolGetCmosPresets(XnDevicePrivateData* pDevicePrivateData, XnCMOSType nCMOSID, XnCmosPreset* aPresets, XnUInt32& nCount)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;

	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Reading CMOS %d supported presets...", nCMOSID);

	*(XnUInt16*)pDataBuf = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)nCMOSID);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetCmosPresets);

	XnUInt16 nDataSize;
	XnCmosPreset* pValue = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize+sizeof(XnUInt16), pDevicePrivateData->FWInfo.nOpcodeGetCmosPresets,
		(XnUChar**)(&pValue), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed getting CMOS %d presets: %s", nCMOSID, xnGetStatusString(rc));
		return rc;
	}

	XnUInt32 nReturnedCount = nDataSize * 2 / sizeof(XnCmosPreset);
	if (nReturnedCount > nCount)
	{
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	XnCmosPreset* pValueEnd = pValue + nReturnedCount;

	nCount = 0;

	while (pValue < pValueEnd)
	{
		// workaround a FW bug - an extra preset arrives with FPS 0. Ignore it.
		if (pValue->nFPS != 0)
		{
			aPresets[nCount].nFormat = XN_PREPARE_VAR16_IN_BUFFER(pValue->nFormat);
			aPresets[nCount].nResolution = XN_PREPARE_VAR16_IN_BUFFER(pValue->nResolution);
			aPresets[nCount].nFPS = XN_PREPARE_VAR16_IN_BUFFER(pValue->nFPS);
			++nCount;
		}

		++pValue;
	}

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolGetSerialNumber (XnDevicePrivateData* pDevicePrivateData, XnChar* cpSerialNumber)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};

	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Reading sensor serial number...");

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetSerialNumber);

	XnUInt16 nDataSize;
	XnUChar *serialNumberBuffer = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetSerialNumber,
		(XnUChar**)(&serialNumberBuffer), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed getting the sensor serial number: %s", xnGetStatusString(rc));
		return rc;
	}

	serialNumberBuffer[nDataSize*2]=0;

	strcpy(cpSerialNumber, (XnChar*)serialNumberBuffer);

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolGetPlatformString(XnDevicePrivateData* pDevicePrivateData, XnChar* cpPlatformString)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};

	cpPlatformString[0] = '\0';

	if (pDevicePrivateData->FWInfo.nOpcodeGetPlatformString == OPCODE_INVALID)
	{
		// for FW that doesn't support this opcode, we just return an empty string
		return XN_STATUS_OK;
	}

	xnLogInfo(XN_MASK_SENSOR_PROTOCOL, "Reading sensor platform string...");

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetPlatformString);

	XnUInt16 nDataSize;
	XnChar *platformStringBuffer = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetPlatformString,
		(XnUChar**)(&platformStringBuffer), nDataSize);
	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_SENSOR_PROTOCOL, "Failed getting the sensor platform string: %s", xnGetStatusString(rc));
		return rc;
	}

	XnUInt32 nBufferUsed = 0;
	for (XnUInt32 i = 0; i < (XnUInt32)nDataSize*2; ++i)
	{
		cpPlatformString[nBufferUsed++] = platformStringBuffer[i*2];
	}

	cpPlatformString[nBufferUsed++] = '\0';

	return XN_STATUS_OK;
}

XnStatus XnHostProtocolGetUsbCoreType(XnDevicePrivateData* pDevicePrivateData, XnHostProtocolUsbCore& nValue)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	
	XnHostProtocolInitHeader(pDevicePrivateData, buffer, 0, pDevicePrivateData->FWInfo.nOpcodeGetUsbCore);

	XnUInt16 nDataSize;
	XnUInt16* pValue = NULL;

	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData, 
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize, pDevicePrivateData->FWInfo.nOpcodeGetUsbCore,
		(XnUChar**)(&pValue), nDataSize);
	XN_IS_STATUS_OK(rc);

	nValue = (XnHostProtocolUsbCore)XN_PREPARE_VAR16_IN_BUFFER(*pValue);

	return XN_STATUS_OK;
}

#pragma pack (push, 1)

typedef struct XnVSetLedStateRequest
{
	XnUInt16 nLedId;
	XnUInt16 nState;
} XnVSetLedStateRequest;

#pragma pack (pop)

XnStatus XnHostProtocolSetLedState(XnDevicePrivateData* pDevicePrivateData, XnUInt16 nLedId, XnUInt16 nState)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUInt32 nRequestSize;

	XnVSetLedStateRequest* pRequest = (XnVSetLedStateRequest*)pDataBuf;
	pRequest->nLedId = XN_PREPARE_VAR16_IN_BUFFER(nLedId);
	pRequest->nState = XN_PREPARE_VAR16_IN_BUFFER(nState);
	nRequestSize = sizeof(XnVSetLedStateRequest);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, nRequestSize, pDevicePrivateData->FWInfo.nOpcodeSetLedState);

	XnUInt16 nDataSize;
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize + (XnUInt16)nRequestSize, pDevicePrivateData->FWInfo.nOpcodeSetLedState,
		NULL, nDataSize);
	XN_IS_STATUS_OK(rc);

	return (XN_STATUS_OK);
}

#pragma pack (push, 1)

typedef struct XnVSetEmitterStateRequest
{
	XnUInt16 nActive;
} XnVSetEmitterStateRequest;

#pragma pack (pop)

XnStatus XnHostProtocolSetEmitterState(XnDevicePrivateData* pDevicePrivateData, XnBool bActive)
{
	XnUChar buffer[MAX_PACKET_SIZE] = {0};
	XnUChar* pDataBuf = buffer + pDevicePrivateData->FWInfo.nProtocolHeaderSize;
	XnUInt32 nRequestSize;

	XnVSetEmitterStateRequest* pRequest = (XnVSetEmitterStateRequest*)pDataBuf;
	pRequest->nActive = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)bActive);
	nRequestSize = sizeof(XnVSetEmitterStateRequest);

	XnHostProtocolInitHeader(pDevicePrivateData, buffer, nRequestSize, pDevicePrivateData->FWInfo.nOpcodeEnableEmitter);

	XnUInt16 nDataSize;
	XnStatus rc = XnHostProtocolExecute(pDevicePrivateData,
		buffer, pDevicePrivateData->FWInfo.nProtocolHeaderSize + (XnUInt16)nRequestSize, pDevicePrivateData->FWInfo.nOpcodeEnableEmitter,
		NULL, nDataSize);
	XN_IS_STATUS_OK(rc);

	return (XN_STATUS_OK);
}
