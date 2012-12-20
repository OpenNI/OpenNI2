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
#ifndef _XN_STREAM_PARAMS_H_
#define _XN_STREAM_PARAMS_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>
#include <OniTypes.h>
#include <PS1080.h>

//---------------------------------------------------------------------------
// Modules Names
//---------------------------------------------------------------------------
#define XN_MODULE_NAME_DEVICE			"Device"

#define XN_MODULE_NAME_FIXED_PARAMS		"FixedParams"
#define	XN_MODULE_NAME_SHIFTS			"Shifts"

//---------------------------------------------------------------------------
// Streams Types
//---------------------------------------------------------------------------
#define XN_STREAM_TYPE_DEPTH	"Depth"
#define XN_STREAM_TYPE_IMAGE	"Image"
#define XN_STREAM_TYPE_IR		"IR"
#define XN_STREAM_TYPE_AUDIO	"Audio"

//---------------------------------------------------------------------------
// Streams Names
//---------------------------------------------------------------------------
#define XN_STREAM_NAME_DEPTH	XN_STREAM_TYPE_DEPTH
#define XN_STREAM_NAME_IMAGE	XN_STREAM_TYPE_IMAGE
#define XN_STREAM_NAME_IR		XN_STREAM_TYPE_IR
#define XN_STREAM_NAME_AUDIO	XN_STREAM_TYPE_AUDIO

enum
{
	//---------------------------------------------------------------------------
	// General Properties
	//---------------------------------------------------------------------------
	XN_STREAM_PROPERTY_TYPE = 0x20000, //	"Type"
	XN_STREAM_PROPERTY_IS_STREAM, // "IsStream"
	XN_STREAM_PROPERTY_IS_FRAME_BASED, // "IsFrameBased"
	XN_STREAM_PROPERTY_IS_PIXEL_BASED, // "IsPixelBased"
	XN_STREAM_PROPERTY_IS_STREAMING, // "IsStreaming"

	/** Integer */
	XN_MODULE_PROPERTY_LOCK, // "Lock"
	XN_SENSOR_PROPERTY_INSTANCE_POINTER, // "InstancePointer"

	//---------------------------------------------------------------------------
	// General Stream Properties
	//---------------------------------------------------------------------------
	/** Integer */
	XN_STREAM_PROPERTY_STATE, // "State"
	/** Integer */
	XN_STREAM_PROPERTY_REQUIRED_DATA_SIZE, // "RequiredDataSize"
	/** Integer */
	XN_STREAM_PROPERTY_FRAME_ID, // "FrameId"
	/** Integer */
	XN_STREAM_PROPERTY_TIMESTAMP, // "Timestamp"
	/** Integer */
	XN_STREAM_PROPERTY_IS_NEW_DATA, // "IsNew"
	/** Integer (OniFormat) */
	XN_STREAM_PROPERTY_OUTPUT_FORMAT, // "OutputFormat"
	/** General (void*) */ 
	XN_STREAM_PROPERTY_BUFFER, // "Buffer"
	/** Integer */ 
	XN_STREAM_PROPERTY_BUFFER_SIZE, // "BufferSize"
	/** Integer */
	XN_STREAM_PROPERTY_COMPRESSION, // "Compression"
	/** String */
	XN_STREAM_PROPERTY_SHARED_BUFFER_NAME, // "SharedBufferName"
	/** Boolean */
	XN_STREAM_PROPERTY_ACTUAL_READ_DATA, // "ActualReadData"

	//---------------------------------------------------------------------------
	// Frame-Based Stream Properties (Depth, Image, IR)
	//---------------------------------------------------------------------------
	/** Integer */ 
	XN_STREAM_PROPERTY_FPS, // "FPS"
	/** Integer */
	XN_STREAM_PROPERTY_NUMBER_OF_FRAMES, // "NumberOfFrames"
	/** XnDynamicSizeBuffer */
	XN_STREAM_PROPERTY_LAST_RAW_FRAME, // "LastRawFrame"
	/** OniGeneralBuffer array */
	XN_STREAM_PROPERTY_EXTERNAL_BUFFER_POOL, // "ExternalBufferPool"

	//---------------------------------------------------------------------------
	// Pixel-Based Stream Properties (Depth, Image, IR)
	//---------------------------------------------------------------------------
	/** XnResolutions */
	XN_STREAM_PROPERTY_RESOLUTION, // "Resolution"
	/** Integer */ 
	XN_STREAM_PROPERTY_X_RES, // "XRes"
	/** Integer */ 
	XN_STREAM_PROPERTY_Y_RES, // "YRes"
	/** Integer */ 
	XN_STREAM_PROPERTY_BYTES_PER_PIXEL, // "BytesPerPixel"
	/** OniCropping */ 
	XN_STREAM_PROPERTY_CROPPING, // "Cropping"
	/** Integer */
	XN_STREAM_PROPERTY_SUPPORT_MODES_COUNT, // "SupportedModesCount"
	/** General (XnCmosPreset array) */
	XN_STREAM_PROPERTY_SUPPORT_MODES, // "SupportedModes"

	//---------------------------------------------------------------------------
	// Depth Specific Properties
	//---------------------------------------------------------------------------
	/** Integer */ 
	XN_STREAM_PROPERTY_SHADOW, // "ShadowValue"
	/** Integer */ 
	XN_STREAM_PROPERTY_NO_SAMPLE, // "NoSampleValue"
	/** Boolean */ 
	XN_STREAM_PROPERTY_REGISTRATION, // "Registration"
	/** Integer */
	XN_STREAM_PROPERTY_DEVICE_MAX_DEPTH, // "DeviceMaxDepth"
	/** Boolean */
	XN_STREAM_PROPERTY_GMC_MODE, // "GmcMode"
	/** XnUInt16* (general) */
	XN_STREAM_PROPERTY_SHIFTS_MAP, // "ShiftsMap"

	#ifndef XN_LEAN_SENSOR
	/** Boolean */
	XN_STREAM_PROPERTY_GMC_DEBUG, // "GmcDebug"
	/** Boolean */
	XN_STREAM_PROPERTY_WAVELENGTH_CORRECTION, // "WavelengthCorrection"
	/** Boolean */
	XN_STREAM_PROPERTY_WAVELENGTH_CORRECTION_DEBUG, // "WavelengthCorrectionDebug"
	#endif

	//---------------------------------------------------------------------------
	// Image Specific Properties
	//---------------------------------------------------------------------------
	/** Integer */ 
	XN_STREAM_PROPERTY_FLICKER, // "Flicker"
	/** Integer */ 
	XN_STREAM_PROPERTY_QUALITY, // "Quality"
	/** Float */
	XN_STREAM_PROPERTY_BRIGHTNESS, // "Brightness"
	/** Float */
	XN_STREAM_PROPERTY_CONTRAST, // "Contrast"
	/** Float */
	XN_STREAM_PROPERTY_SATURATION, // "Saturation"
	/** Float */
	XN_STREAM_PROPERTY_SHARPNESS, // "Sharpness"
	/** Float */
	XN_STREAM_PROPERTY_COLOR_TEMPERATURE, // "ColorTemperature"
	/** Float */
	XN_STREAM_PROPERTY_BACKLIGHT_COMPENSATION, // "BacklightCompensation"
	/** Float */
	XN_STREAM_PROPERTY_ZOOM, // "Zoom"
	/** Integer (in microseconds) */
	XN_STREAM_PROPERTY_EXPOSURE, // "Exposure"
	/** Float */
	XN_STREAM_PROPERTY_PAN, // "Pan"
	/** Float */
	XN_STREAM_PROPERTY_TILT, // "Tilt"
	/** Boolean */
	XN_STREAM_PROPERTY_LOW_LIGHT_COMPENSATION, // "LowLightCompensation"

	//---------------------------------------------------------------------------
	// Audio Specific Properties
	//---------------------------------------------------------------------------
	/** XnSampleRate */
	XN_STREAM_PROPERTY_SAMPLE_RATE, // "SampleRate"
	/** Integer */
	XN_STREAM_PROPERTY_LEFT_CHANNEL_VOLUME, // "LeftChannelVolume"
	/** Integer */
	XN_STREAM_PROPERTY_RIGHT_CHANNEL_VOLUME, // "RightChannelVolume"
	/** Integer */
	XN_STREAM_PROPERTY_NUMBER_OF_CHANNELS, // "NumOfChannels"
	/** Boolean */
	XN_STREAM_PROPERTY_IS_STEREO, // "IsStereo"
	/** Integer */
	XN_STREAM_PROPERTY_READ_MODE, // "ReadMode"
	/** Integer */
	XN_STREAM_PROPERTY_READ_CHUNK_SIZE, // "ReadChunkSize"
	/** Integer */
	XN_STREAM_PROPERTY_READ_SYNC, // "AudioReadSync"

	//---------------------------------------------------------------------------
	// DeviceParams Properties
	//---------------------------------------------------------------------------
	/** Integer */
	XN_MODULE_PROPERTY_NUMBER_OF_BUFFERS, // "NumberOfBuffers"
	/** Boolean */
	XN_MODULE_PROPERTY_READ_ENDPOINT_1, // "ReadEndpoint1"
	/** Boolean */
	XN_MODULE_PROPERTY_READ_ENDPOINT_2, // "ReadEndpoint2"
	/** Boolean */
	XN_MODULE_PROPERTY_READ_ENDPOINT_3, // "ReadEndpoint3"
	/** String */ 
	XN_MODULE_PROPERTY_PRIMARY_STREAM, // "PrimaryStream"
	/** Boolean */ 
	XN_MODULE_PROPERTY_MIRROR, // "Mirror"
	/** Boolean */
	XN_MODULE_PROPERTY_READ_DATA, // "ReadData"
	/** Integer */
	XN_MODULE_PROPERTY_READ_WRITE_MODE, // "ReadWriteMode"
	/** Integer */
	XN_MODULE_PROPERTY_SHARE_MODE, // "ShareMode"
	/** Boolean */
	XN_MODULE_PROPERTY_FRAME_DELAY, // "FrameDelay"
	/** Boolean */
	XN_MODULE_PROPERTY_FRAME_SYNC, // "FrameSync"
	/** XnCmosBlankingUnits */
	XN_MODULE_PROPERTY_CMOS_BLANKING_UNITS, // "CmosBlankingUnits"
	/** XnCmosBlankingTime */
	XN_MODULE_PROPERTY_CMOS_BLANKING_TIME, // "CmosBlankingTime"
	/* XnDynamicSizeBuffer */
	XN_MODULE_PROPERTY_FIXED_PARAMS, // "FixedParams"
	/** Integer */
	XN_MODULE_PROPERTY_FIRMWARE_MODE, // "FirmwareMode"
	/** Boolean */
	XN_MODULE_PROPERTY_HIGH_RES_TIMESTAMPS, // "HighResTimestamps"
	/** Boolean */
	XN_MODULE_PROPERTY_HOST_TIMESTAMPS, // "HostTimestamps"
	/** Boolean */
	XN_MODULE_PROPERTY_CLOSE_STREAMS_ON_SHUTDOWN, // "CloseStreamsOnShutdown"
	/** Integer */
	XN_MODULE_PROPERTY_SERVER_NO_CLIENTS_TIMEOUT, // "ServerNoClientsTimeout"
	/** Integer */
	XN_MODULE_PROPERTY_SERVER_START_NEW_LOG_FILE, // "ServerStartNewLogFile"
	/** String */
	XN_MODULE_PROPERTY_SERVER_LOG_FILE, // "ServerLogFile"
	/** Integer */
	XN_MODULE_PROPERTY_ERROR_STATE, // "ErrorState"
	/** Boolean */
	XN_MODULE_PROPERTY_ENABLE_MULTI_PROCESS, // "EnableMultiProcess"
	/** Boolean */
	XN_MODULE_PROPERTY_ENABLE_MULTI_USERS, // "EnableMultiUsers"
	/** String */
	XN_MODULE_PROPERTY_PHYSICAL_DEVICE_NAME, // "PhysicalDeviceName"
	/** String */
	XN_MODULE_PROPERTY_VENDOR_SPECIFIC_DATA, // "VendorSpecificData"
	/** String */
	XN_MODULE_PROPERTY_SENSOR_PLATFORM_STRING, // "SensorPlatformString"
	/** Boolean */
	XN_MODULE_PROPERTY_AUDIO_SUPPORTED, // "AudioSupported"
	/** Boolean */
	XN_MODULE_PROPERTY_IMAGE_SUPPORTED, // "ImageSupported"

#ifndef XN_LEAN_SENSOR
	/** Integer */
	XN_MODULE_PROPERTY_FIRMWARE_LOG_INTERVAL, // "FirmwareLogInterval"
	/** Boolean */
	XN_MODULE_PROPERTY_PRINT_FIRMWARE_LOG, // "FirmwareLogPrint"
	/** Integer */
	XN_MODULE_PROPERTY_FIRMWARE_LOG_FILTER, // "FirmwareLogFilter"
	/** Integer */
	XN_MODULE_PROPERTY_FIRMWARE_LOG, // "FirmwareLog"
	/** Integer */
	XN_MODULE_PROPERTY_FIRMWARE_CPU_INTERVAL, // "FirmwareCPUInterval"
	/** XnFlashFileList */
	XN_MODULE_PROPERTY_FILE_LIST, // "FileList"
	/** XnParamFlashData */
	XN_MODULE_PROPERTY_FLASH_CHUNK, // "FlashChunk"
	XN_MODULE_PROPERTY_FILE, // "FlashFile"
	/** Integer */
	XN_MODULE_PROPERTY_DELETE_FILE, // "DeleteFile"
	XN_MODULE_PROPERTY_FILE_ATTRIBUTES, // "FileAttributes"
	XN_MODULE_PROPERTY_TEC_SET_POINT, // "TecSetPoint"
	XN_MODULE_PROPERTY_TEC_STATUS, // "TecStatus"
	XN_MODULE_PROPERTY_TEC_FAST_CONVERGENCE_STATUS, // "TecFastConvergenceStatus"
	XN_MODULE_PROPERTY_EMITTER_SET_POINT, // "EmitterSetPoint"
	XN_MODULE_PROPERTY_EMITTER_STATUS, // "EmitterStatus"
	XN_MODULE_PROPERTY_I2C, // "I2C"
	/** Integer */
	XN_MODULE_PROPERTY_BIST, // "BIST"
	/** XnProjectorFaultData */
	XN_MODULE_PROPERTY_PROJECTOR_FAULT, // "ProjectorFault"
	/** Boolean */
	XN_MODULE_PROPERTY_APC_ENABLED, // "APCEnabled"
	/** Boolean */
	XN_MODULE_PROPERTY_FIRMWARE_TEC_DEBUG_PRINT, // "TecDebugPrint"
#endif

};

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_PRIMARY_STREAM_ANY	"Any"
#define XN_PRIMARY_STREAM_NONE	"None"

#define XN_MAX_LOG_SIZE	(6*1024)

#define XN_GAIN_AUTO	0U

#define XN_QVGA_X_RES	320
#define XN_QVGA_Y_RES	240
#define XN_VGA_X_RES	640
#define XN_VGA_Y_RES	480
#define XN_SXGA_X_RES	1280
#define XN_SXGA_Y_RES	1024
#define XN_UXGA_X_RES	1600
#define XN_UXGA_Y_RES	1200

#define XN_IO_MAX_I2C_BUFFER_SIZE 10

//---------------------------------------------------------------------------
// Enums - values of various properties
//---------------------------------------------------------------------------
typedef enum XnResolutions
{
	XN_RESOLUTION_CUSTOM = -1,
	XN_RESOLUTION_QVGA = 0, // 320x240
	XN_RESOLUTION_VGA = 1, // 640x480
	XN_RESOLUTION_SXGA = 2, // 1280x1024
	XN_RESOLUTION_UXGA = 3, // 1600x1200
	XN_RESOLUTION_QQVGA = 4, // 160x120
	XN_RESOLUTION_QCIF = 5, // 176x144
	XN_RESOLUTION_240P = 6, // 432x240
	XN_RESOLUTION_CIF = 7, // 352x288
	XN_RESOLUTION_WVGA = 8, // 640x360
	XN_RESOLUTION_480P = 9, // 864x480
	XN_RESOLUTION_800_448 = 10, // 800x448
	XN_RESOLUTION_SVGA = 11, // 800x600
	XN_RESOLUTION_576P = 12, // 1024x576
	XN_RESOLUTION_DV = 13, // 960x720
	XN_RESOLUTION_720P = 14, // 1280x720
	XN_RESOLUTION_1280_960 = 15, // 1280x960
} XnResolutions;

/*typedef enum XnSampleRate
{
        XN_SAMPLE_RATE_8K = 8000,
        XN_SAMPLE_RATE_11K = 11025,
        XN_SAMPLE_RATE_12K = 12000,
        XN_SAMPLE_RATE_16K = 16000,
        XN_SAMPLE_RATE_22K = 22050,
        XN_SAMPLE_RATE_24K = 24000,
        XN_SAMPLE_RATE_32K = 32000,
        XN_SAMPLE_RATE_44K = 44100,
        XN_SAMPLE_RATE_48K = 48000,
} XnSampleRate;*/

typedef enum
{
	XN_MODE_PS = 0,
	XN_MODE_MAINTENANCE = 1,
	XN_MODE_SAFE_MODE = 2,
} XnParamCurrentMode;

typedef enum
{
	XN_VIDEO_STREAM_OFF = 0,
	XN_VIDEO_STREAM_COLOR = 1,
	XN_VIDEO_STREAM_DEPTH = 2,
	XN_VIDEO_STREAM_IR = 3,
} XnVideoStreamMode;

typedef enum
{
	XN_AUDIO_STREAM_OFF = 0,
	XN_AUDIO_STREAM_ON = 1,
} XnAudioStreamMode;

typedef enum XnFirmwareCroppingMode
{
	XN_FIRMWARE_CROPPING_MODE_DISABLED = 0,
	XN_FIRMWARE_CROPPING_MODE_NORMAL = 1,
	XN_FIRMWARE_CROPPING_MODE_INCREASED_FPS = 2,
} XnFirmwareCroppingMode;

#ifndef XN_LEAN_SENSOR
typedef enum
{
	XnLogFilterDebug		= 0x0001,
	XnLogFilterInfo			= 0x0002,
	XnLogFilterError		= 0x0004,
	XnLogFilterProtocol		= 0x0008,
	XnLogFilterAssert		= 0x0010,
	XnLogFilterConfig		= 0x0020,
	XnLogFilterFrameSync	= 0x0040,
	XnLogFilterAGC			= 0x0080,
	XnLogFilterTelems		= 0x0100,

	XnLogFilterAll			= 0xFFFF
} XnLogFilter;

typedef enum
{
	XnFileAttributeReadOnly	= 0x8000
} XnFilePossibleAttributes;

typedef enum
{
	XnFlashFileTypeFileTable,
	XnFlashFileTypeScratchFile,
	XnFlashFileTypeBootSector,
	XnFlashFileTypeBootManager,
	XnFlashFileTypeCodeDownloader,
	XnFlashFileTypeMonitor,
	XnFlashFileTypeApplication,
	XnFlashFileTypeFixedParams,
	XnFlashFileTypeDescriptors,
	XnFlashFileTypeDefaultParams,
	XnFlashFileTypeImageCmos,
	XnFlashFileTypeDepthCmos,
	XnFlashFileTypeAlgorithmParams,
	XnFlashFileTypeReferenceQVGA,
	XnFlashFileTypeReferenceVGA,
	XnFlashFileTypeMaintenance,
	XnFlashFileTypeDebugParams,
	XnFlashFileTypePrimeProcessor,
	XnFlashFileTypeGainControl,
	XnFlashFileTypeRegistartionParams,
	XnFlashFileTypeIDParams,
	XnFlashFileTypeSensorTECParams,
	XnFlashFileTypeSensorAPCParams,
	XnFlashFileTypeSensorProjectorFaultParams,
	XnFlashFileTypeProductionFile,
	XnFlashFileTypeUpgradeInProgress,
	XnFlashFileTypeWavelengthCorrection,
	XnFlashFileTypeGMCReferenceOffset,
} XnFlashFileType;

typedef enum XnBistType
{
	XN_BIST_IMAGE_CMOS = 1 << 0,
	XN_BIST_IR_CMOS = 1 << 1,
	XN_BIST_POTENTIOMETER = 1 << 2,
	XN_BIST_FLASH = 1 << 3,
	XN_BIST_FULL_FLASH = 1 << 4,
	XN_BIST_PROJECTOR_TEST_MASK = 1 << 5,
	XN_BIST_TEC_TEST_MASK = 1 << 6,
	XN_BIST_ALL = 0xFFFFFFFF,
} XnBistType;

typedef enum XnBistError
{
	XN_BIST_RAM_TEST_FAILURE = 1 << 0,
	XN_BIST_IR_CMOS_CONTROL_BUS_FAILURE = 1 << 1,
	XN_BIST_IR_CMOS_DATA_BUS_FAILURE = 1 << 2,
	XN_BIST_IR_CMOS_BAD_VERSION = 1 << 3,
	XN_BIST_IR_CMOS_RESET_FAILUE = 1 << 4,
	XN_BIST_IR_CMOS_TRIGGER_FAILURE = 1 << 5,
	XN_BIST_IR_CMOS_STROBE_FAILURE = 1 << 6,
	XN_BIST_COLOR_CMOS_CONTROL_BUS_FAILURE = 1 << 7,
	XN_BIST_COLOR_CMOS_DATA_BUS_FAILURE = 1 << 8,
	XN_BIST_COLOR_CMOS_BAD_VERSION = 1 << 9,
	XN_BIST_COLOR_CMOS_RESET_FAILUE = 1 << 10,
	XN_BIST_FLASH_WRITE_LINE_FAILURE = 1 << 11,
	XN_BIST_FLASH_TEST_FAILURE = 1 << 12,
	XN_BIST_POTENTIOMETER_CONTROL_BUS_FAILURE = 1 << 13,
	XN_BIST_POTENTIOMETER_FAILURE = 1 << 14,
	XN_BIST_AUDIO_TEST_FAILURE = 1 << 15,
	XN_BIST_PROJECTOR_TEST_LD_FAIL = 1 << 16,
	XN_BIST_PROJECTOR_TEST_LD_FAILSAFE_TRIG_FAIL = 1 << 17,
	XN_BIST_PROJECTOR_TEST_FAILSAFE_HIGH_FAIL = 1 << 18,
	XN_BIST_PROJECTOR_TEST_FAILSAFE_LOW_FAIL = 1 << 19,
	XN_TEC_TEST_HEATER_CROSSED = 1 << 20,
	XN_TEC_TEST_HEATER_DISCONNETED = 1 << 21,
	XN_TEC_TEST_TEC_CROSSED = 1 << 22,
	XN_TEC_TEST_TEC_FAULT = 1 << 23,
} XnBistError;
#endif

//---------------------------------------------------------------------------
// Data Structures - structures that are arguments to properties
//---------------------------------------------------------------------------

#pragma pack (push, 1)

typedef struct XnCmosBlankingTime
{
	XnCMOSType nCmosID;
	XnFloat nTimeInMilliseconds;
	XnUInt16 nNumberOfFrames;
} XnCmosBlankingTime;

typedef struct XnCmosBlankingUnits
{
	XnCMOSType nCmosID;
	XnUInt16 nUnits;
	XnUInt16 nNumberOfFrames;
} XnCmosBlankingUnits;

typedef struct XnDynamicSizeBuffer
{
	void* pData;
	XnUInt32 nMaxSize;
	XnUInt32 nDataSize;
} XnDynamicSizeBuffer;

typedef struct XnCmosPreset
{
	XnUInt16 nFormat;
	XnUInt16 nResolution;
	XnUInt16 nFPS;
} XnCmosPreset;

typedef struct XnI2CWriteData
{
	XnUInt16 nBus;
	XnUInt16 nSlaveAddress;
	XnUInt16 cpWriteBuffer[XN_IO_MAX_I2C_BUFFER_SIZE];
	XnUInt16 nWriteSize;
} XnI2CWriteData;

typedef struct XnI2CReadData
{
	XnUInt16 nBus;
	XnUInt16 nSlaveAddress;
	XnUInt16 cpReadBuffer[XN_IO_MAX_I2C_BUFFER_SIZE];
	XnUInt16 cpWriteBuffer[XN_IO_MAX_I2C_BUFFER_SIZE];
	XnUInt16 nReadSize;
	XnUInt16 nWriteSize;
} XnI2CReadData;

#ifndef XN_LEAN_SENSOR
typedef struct XnTecData
{
	XnUInt16 m_SetPointVoltage;
	XnUInt16 m_CompensationVoltage;
	XnUInt16 m_TecDutyCycle; //duty cycle on heater/cooler
	XnUInt16 m_HeatMode; //TRUE - heat, FALSE - cool
	XnInt32 m_ProportionalError;
	XnInt32 m_IntegralError;
	XnInt32 m_DerivativeError;
	XnUInt16 m_ScanMode; //0 - crude, 1 - precise
} XnTecData;

typedef struct XnTecFastConvergenceData
{
    XnInt16     m_SetPointTemperature;  // set point temperature in celsius,
                                        // scaled by factor of 100 (extra precision)
    XnInt16     m_MeasuredTemperature;  // measured temperature in celsius,
                                        // scaled by factor of 100 (extra precision)
    XnInt32 	m_ProportionalError;    // proportional error in system clocks
    XnInt32 	m_IntegralError;        // integral error in system clocks
    XnInt32 	m_DerivativeError;      // derivative error in system clocks
    XnUInt16 	m_ScanMode; // 0 - initial, 1 - crude, 2 - precise
    XnUInt16    m_HeatMode; // 0 - idle, 1 - heat, 2 - cool
    XnUInt16    m_TecDutyCycle; // duty cycle on heater/cooler in percents
    XnUInt16	m_TemperatureRange;	// 0 - cool, 1 - room, 2 - warm
} XnTecFastConvergenceData;

typedef struct XnEmitterData
{
	XnUInt16 m_State; //idle, calibrating
	XnUInt16 m_SetPointVoltage; //this is what should be written to the XML
	XnUInt16 m_SetPointClocks; //target cross duty cycle
	XnUInt16 m_PD_Reading; //current cross duty cycle in system clocks(high time)
	XnUInt16 m_EmitterSet; //duty cycle on emitter set in system clocks (high time).
	XnUInt16 m_EmitterSettingLogic; //TRUE = positive logic, FALSE = negative logic
	XnUInt16 m_LightMeasureLogic; //TRUE - positive logic, FALSE - negative logic
	XnUInt16 m_IsAPCEnabled;
	XnUInt16 m_EmitterSetStepSize; // in MilliVolts
	XnUInt16 m_ApcTolerance; // in system clocks (only valid up till v5.2)
	XnUInt16 m_SubClocking; //in system clocks (only valid from v5.3)
	XnUInt16 m_Precision; // (only valid from v5.3)
} XnEmitterData;

typedef struct
{
	XnUInt16 nId;
	XnUInt16 nAttribs;
} XnFileAttributes;

typedef struct
{
	XnUInt32 nOffset;
	const XnChar* strFileName;
	XnUInt16 nAttributes;
} XnParamFileData;

typedef struct
{
	XnUInt32 nOffset;
	XnUInt32 nSize;
	XnUChar* pData;
} XnParamFlashData;

typedef struct  {
	XnUInt16 nId;
	XnUInt16 nType;
	XnUInt32 nVersion;
	XnUInt32 nOffset;
	XnUInt32 nSize;
	XnUInt16 nCrc;
	XnUInt16 nAttributes;
	XnUInt16 nReserve;
} XnFlashFile;

typedef struct  
{
	XnFlashFile* pFiles;
	XnUInt16 nFiles;
} XnFlashFileList;

typedef struct XnProjectorFaultData
{
	XnUInt16 nMinThreshold;
	XnUInt16 nMaxThreshold;
	XnBool bProjectorFaultEvent;
} XnProjectorFaultData;

typedef struct XnBist
{
	XnUInt32 nTestsMask;
	XnUInt32 nFailures;
} XnBist;
#endif

#pragma pack (pop)

#endif // _XN_STREAM_PARAMS_H_
