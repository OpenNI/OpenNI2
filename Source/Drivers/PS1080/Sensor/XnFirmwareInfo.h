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
#ifndef XNFIRMWAREINFO_H
#define XNFIRMWAREINFO_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnStreamParams.h>
#include <XnArray.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnFirmwareInfo
{
public:
	XnFWVer nFWVer;
	XnUInt16 nHostMagic;
	XnUInt16 nFWMagic;
	XnUInt16 nProtocolHeaderSize;
	XnUInt16 nProtocolMaxPacketSize;

	XnParamCurrentMode nCurrMode;

	XnBool bAudioSupported;
	XnBool bGetPresetsSupported;
	XnBool bDeviceInfoSupported;
	XnBool bImageAdjustmentsSupported;

	XnUInt16 nOpcodeGetVersion;
	XnUInt16 nOpcodeKeepAlive;
	XnUInt16 nOpcodeGetParam;
	XnUInt16 nOpcodeSetParam;
	XnUInt16 nOpcodeGetFixedParams;
	XnUInt16 nOpcodeGetMode;
	XnUInt16 nOpcodeSetMode;
	XnUInt16 nOpcodeAlgorithmParams;
	XnUInt16 nOpcodeReset;
	XnUInt16 nOpcodeSetCmosBlanking;
	XnUInt16 nOpcodeGetCmosBlanking;
	XnUInt16 nOpcodeGetCmosPresets;
	XnUInt16 nOpcodeGetSerialNumber;
	XnUInt16 nOpcodeGetFastConvergenceTEC;
	XnUInt16 nOpcodeGetCMOSReg;
	XnUInt16 nOpcodeSetCMOSReg;
	XnUInt16 nOpcodeWriteI2C;
	XnUInt16 nOpcodeReadI2C;
	XnUInt16 nOpcodeReadAHB;
	XnUInt16 nOpcodeWriteAHB;
	XnUInt16 nOpcodeGetPlatformString;
	XnUInt16 nOpcodeGetUsbCore;
	XnUInt16 nOpcodeSetLedState;
	XnUInt16 nOpcodeEnableEmitter;

	XnUInt16 nOpcodeGetLog;
	XnUInt16 nOpcodeTakeSnapshot;
	XnUInt16 nOpcodeInitFileUpload;
	XnUInt16 nOpcodeWriteFileUpload;
	XnUInt16 nOpcodeFinishFileUpload;
	XnUInt16 nOpcodeDownloadFile;
	XnUInt16 nOpcodeDeleteFile;
	XnUInt16 nOpcodeGetFlashMap;
	XnUInt16 nOpcodeGetFileList;
	XnUInt16 nOpcodeSetFileAttribute;
	XnUInt16 nOpcodeExecuteFile;
	XnUInt16 nOpcodeReadFlash;
	XnUInt16 nOpcodeBIST;
	XnUInt16 nOpcodeSetGMCParams;
	XnUInt16 nOpcodeGetCPUStats;
	XnUInt16 nOpcodeCalibrateTec;
	XnUInt16 nOpcodeGetTecData;
	XnUInt16 nOpcodeCalibrateEmitter;
	XnUInt16 nOpcodeGetEmitterData;
	XnUInt16 nOpcodeCalibrateProjectorFault;

	XnUInt16 nLogStringType;
	XnUInt16 nLogOverflowType;

	XnBool bMirrorSupported;

	XnUInt16 nUSBDelayReceive;
	XnUInt16 nUSBDelayExecutePreSend;
	XnUInt16 nUSBDelayExecutePostSend;
	XnUInt16 nUSBDelaySoftReset;
	XnUInt16 nUSBDelaySetParamFlicker;
	XnUInt16 nUSBDelaySetParamStream0Mode;
	XnUInt16 nUSBDelaySetParamStream1Mode;
	XnUInt16 nUSBDelaySetParamStream2Mode;

	XnUInt8 nISOAlternativeInterface;
	XnUInt8 nBulkAlternativeInterface;
	XnUInt8 nISOLowDepthAlternativeInterface;

	XnBool bGetImageCmosTypeSupported;
	XnBool bImageSupported;
	XnBool bIncreasedFpsCropSupported;
	XnBool bHasFilesystemLock;

	xnl::Array<XnCmosPreset> depthModes;
	xnl::Array<XnCmosPreset> _imageBulkModes;
	xnl::Array<XnCmosPreset> _imageIsoModes;
	xnl::Array<XnCmosPreset> imageModes;
	xnl::Array<XnCmosPreset> irModes;
};

#endif // XNFIRMWAREINFO_H
