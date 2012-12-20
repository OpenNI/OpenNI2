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
#ifndef _PS1080_H_
#define _PS1080_H_

#include <OniCTypes.h>

/** The maximum permitted Xiron device name string length. */ 
#define XN_DEVICE_MAX_STRING_LENGTH 200

/* 
 * private properties of PS1080 devices.
 *
 * @remarks 
 * properties structure is 0x1080XXYY where XX is range and YY is code.
 * range values:
 * F0 - device properties
 * E0 - device commands
 * 00 - common stream properties
 * 10 - depth stream properties
 */
enum
{
	/*******************************************************************/
	/* Device properties                                               */
	/*******************************************************************/

	/** unsigned long long (XnSensorUsbInterface) */
	XN_MODULE_PROPERTY_USB_INTERFACE = 0x1080F001, // "UsbInterface"
	/** Boolean */ 
	XN_MODULE_PROPERTY_MIRROR = 0x1080F002, // "Mirror"
	/** unsigned long long */
	XN_STREAM_PROPERTY_CLOSE_RANGE = 0x1080F003, // "CloseRange"
	/** unsigned long long */
	XN_MODULE_PROPERTY_RESET_SENSOR_ON_STARTUP = 0x1080F004, // "ResetSensorOnStartup"
	/** unsigned long long */
	XN_MODULE_PROPERTY_LEAN_INIT = 0x1080F005, // "LeanInit"
	/** char[XN_DEVICE_MAX_STRING_LENGTH] */
	XN_MODULE_PROPERTY_SERIAL_NUMBER = 0x1080F006, // "ID"
	/** XnVersions */
	XN_MODULE_PROPERTY_VERSION = 0x1080F007, // "Version"

	/*******************************************************************/
	/* Device commands (activated via SetProperty)                     */
	/*******************************************************************/

	/** XnInnerParam */
	XN_MODULE_PROPERTY_FIRMWARE_PARAM = 0x1080E001, // "FirmwareParam"
	/** unsigned long long */
	XN_MODULE_PROPERTY_RESET = 0x1080E002, // "Reset"
	/** XnControlProcessingData */
	XN_MODULE_PROPERTY_IMAGE_CONTROL = 0x1080E003, // "ImageControl"
	/** XnControlProcessingData */
	XN_MODULE_PROPERTY_DEPTH_CONTROL = 0x1080E004, // "DepthControl"
	/** XnAHBData */
	XN_MODULE_PROPERTY_AHB = 0x1080E005, // "AHB"
	/** XnLedState */
	XN_MODULE_PROPERTY_LED_STATE = 0x1080E006, // "LedState"


	/*******************************************************************/
	/* Common stream properties                                        */
	/*******************************************************************/

	/** unsigned long long */ 
	XN_STREAM_PROPERTY_INPUT_FORMAT = 0x10800001, // "InputFormat"
	/** unsigned long long (XnCroppingMode) */
	XN_STREAM_PROPERTY_CROPPING_MODE = 0x10800002, // "CroppingMode"

	/*******************************************************************/
	/* Depth stream properties                                         */
	/*******************************************************************/

	/** XnPixelRegistration - get only */
	XN_STREAM_PROPERTY_PIXEL_REGISTRATION = 0x10801001, // "PixelRegistration"
	/** unsigned long long */
	XN_STREAM_PROPERTY_WHITE_BALANCE_ENABLED = 0x10801002, // "WhiteBalancedEnabled"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_GAIN = 0x10801003, // "Gain"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_HOLE_FILTER = 0x10801004, // "HoleFilter"
	/** unsigned long long (XnProcessingType) */ 
	XN_STREAM_PROPERTY_REGISTRATION_TYPE = 0x10801005, // "RegistrationType"
	/** XnDepthAGCBin* */
	XN_STREAM_PROPERTY_AGC_BIN = 0x10801006, // "AGCBin"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_CONST_SHIFT = 0x10801007, // "ConstShift"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_PIXEL_SIZE_FACTOR = 0x10801008, // "PixelSizeFactor"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_MAX_SHIFT = 0x10801009, // "MaxShift"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_PARAM_COEFF = 0x1080100A, // "ParamCoeff"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_SHIFT_SCALE = 0x1080100B, // "ShiftScale"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE = 0x1080100C, // "ZPD"
	/** double */ 
	XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE = 0x1080100D, // "ZPPS"
	/** double */ 
	XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE = 0x1080100E, // "LDDIS"
	/** double */ 
	XN_STREAM_PROPERTY_DCMOS_RCMOS_DISTANCE = 0x1080100F, // "DCRCDIS"
	/** OniDepthPixel[] */ 
	XN_STREAM_PROPERTY_S2D_TABLE = 0x10801010, // "S2D"
	/** unsigned short[] */ 
	XN_STREAM_PROPERTY_D2S_TABLE = 0x10801011, // "D2S"
};

typedef enum 
{
	XN_SENSOR_FW_VER_UNKNOWN = 0,
	XN_SENSOR_FW_VER_0_17 = 1,
	XN_SENSOR_FW_VER_1_1 = 2,
	XN_SENSOR_FW_VER_1_2 = 3,
	XN_SENSOR_FW_VER_3_0 = 4,
	XN_SENSOR_FW_VER_4_0 = 5,
	XN_SENSOR_FW_VER_5_0 = 6,
	XN_SENSOR_FW_VER_5_1 = 7,
	XN_SENSOR_FW_VER_5_2 = 8,
	XN_SENSOR_FW_VER_5_3 = 9,
	XN_SENSOR_FW_VER_5_4 = 10,
	XN_SENSOR_FW_VER_5_5 = 11,
	XN_SENSOR_FW_VER_5_6 = 12,
	XN_SENSOR_FW_VER_5_7 = 13,
	XN_SENSOR_FW_VER_5_8 = 14,
} XnFWVer;

typedef enum {
	XN_SENSOR_VER_UNKNOWN = 0,
	XN_SENSOR_VER_2_0 = 1,
	XN_SENSOR_VER_3_0 = 2,
	XN_SENSOR_VER_4_0 = 3,
	XN_SENSOR_VER_5_0 = 4
} XnSensorVer;

typedef enum {
	XN_SENSOR_HW_VER_UNKNOWN = 0,
	XN_SENSOR_HW_VER_FPDB_10 = 1,
	XN_SENSOR_HW_VER_CDB_10  = 2,
	XN_SENSOR_HW_VER_RD_3  = 3,
	XN_SENSOR_HW_VER_RD_5  = 4,
	XN_SENSOR_HW_VER_RD1081  = 5,
	XN_SENSOR_HW_VER_RD1082  = 6,
	XN_SENSOR_HW_VER_RD109  = 7	
} XnHWVer;

typedef enum {
	XN_SENSOR_CHIP_VER_UNKNOWN = 0,
	XN_SENSOR_CHIP_VER_PS1000 = 1,
	XN_SENSOR_CHIP_VER_PS1080 = 2,
	XN_SENSOR_CHIP_VER_PS1080A6 = 3
} XnChipVer;

typedef enum
{
	XN_CMOS_TYPE_IMAGE = 0,
	XN_CMOS_TYPE_DEPTH = 1,

	XN_CMOS_COUNT
} XnCMOSType;

typedef enum
{
	XN_IO_IMAGE_FORMAT_BAYER = 0,
	XN_IO_IMAGE_FORMAT_YUV422 = 1,
	XN_IO_IMAGE_FORMAT_JPEG = 2,
	XN_IO_IMAGE_FORMAT_JPEG_420 = 3,
	XN_IO_IMAGE_FORMAT_JPEG_MONO = 4,
	XN_IO_IMAGE_FORMAT_UNCOMPRESSED_YUV422 = 5,
	XN_IO_IMAGE_FORMAT_UNCOMPRESSED_BAYER = 6,
	XN_IO_IMAGE_FORMAT_UNCOMPRESSED_GRAY8 = 7,
} XnIOImageFormats;

typedef enum
{
	XN_IO_DEPTH_FORMAT_UNCOMPRESSED_16_BIT = 0,
	XN_IO_DEPTH_FORMAT_COMPRESSED_PS = 1,
	XN_IO_DEPTH_FORMAT_UNCOMPRESSED_10_BIT = 2,
	XN_IO_DEPTH_FORMAT_UNCOMPRESSED_11_BIT = 3,
	XN_IO_DEPTH_FORMAT_UNCOMPRESSED_12_BIT = 4,
} XnIODepthFormats;

typedef enum
{
	XN_RESET_TYPE_POWER = 0,
	XN_RESET_TYPE_SOFT = 1,
	XN_RESET_TYPE_SOFT_FIRST = 2,
} XnParamResetType;

typedef enum XnSensorUsbInterface
{
	XN_SENSOR_USB_INTERFACE_DEFAULT = 0,
	XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS = 1,
	XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS = 2,
} XnSensorUsbInterface;

typedef enum XnProcessingType
{
	XN_PROCESSING_DONT_CARE = 0,
	XN_PROCESSING_HARDWARE = 1,
	XN_PROCESSING_SOFTWARE = 2,
} XnProcessingType;

typedef enum XnCroppingMode
{
	XN_CROPPING_MODE_NORMAL = 1,
	XN_CROPPING_MODE_INCREASED_FPS = 2,
	XN_CROPPING_MODE_SOFTWARE_ONLY = 3,
} XnCroppingMode;

enum
{
	XN_ERROR_STATE_OK = 0,
	XN_ERROR_STATE_DEVICE_PROJECTOR_FAULT = 1,
	XN_ERROR_STATE_DEVICE_OVERHEAT = 2,
};

#pragma pack (push, 1)

typedef struct XnSDKVersion
{
	unsigned char nMajor;
	unsigned char nMinor;
	unsigned char nMaintenance;
	unsigned short nBuild;
} XnSDKVersion;

typedef struct {
	unsigned char nMajor;
	unsigned char nMinor;
	unsigned short nBuild;
	unsigned int nChip;
	unsigned short nFPGA;
	unsigned short nSystemVersion;

	XnSDKVersion SDK;

	XnHWVer		HWVer;
	XnFWVer		FWVer;
	XnSensorVer SensorVer;
	XnChipVer	ChipVer;
} XnVersions;

typedef struct
{
	unsigned short nParam;
	unsigned short nValue;
} XnInnerParamData;

typedef struct XnDepthAGCBin 
{
	unsigned short nBin;
	unsigned short nMin;
	unsigned short nMax;
} XnDepthAGCBin;

typedef struct XnControlProcessingData
{
	unsigned short nRegister;
	unsigned short nValue;
} XnControlProcessingData;

typedef struct XnAHBData
{
	unsigned int nRegister;
	unsigned int nValue;
	unsigned int nMask;
} XnAHBData;

typedef struct XnPixelRegistration
{
	unsigned int nDepthX;
	unsigned int nDepthY;
	uint16_t nDepthValue;
	unsigned int nImageXRes;
	unsigned int nImageYRes;
	unsigned int nImageX; // out
	unsigned int nImageY; // out
} XnPixelRegistration;

typedef struct XnLedState
{
	uint16_t nLedID;
	uint16_t nState;
} XnLedState;

#pragma pack (pop)

#endif //_PS1080_H_