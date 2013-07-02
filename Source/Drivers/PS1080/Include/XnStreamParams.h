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

/* Internal properties - using same structure as private properties - 0x1080FFXX */
enum
{
	//---------------------------------------------------------------------------
	// General Properties
	//---------------------------------------------------------------------------
	XN_STREAM_PROPERTY_TYPE									= 0x1080FF00, //	"Type"
	XN_STREAM_PROPERTY_IS_STREAM							= 0x1080FF01, // "IsStream"
	XN_STREAM_PROPERTY_IS_FRAME_BASED						= 0x1080FF02, // "IsFrameBased"
	XN_STREAM_PROPERTY_IS_PIXEL_BASED						= 0x1080FF03, // "IsPixelBased"
	XN_STREAM_PROPERTY_IS_STREAMING							= 0x1080FF04, // "IsStreaming"

	/** Integer */
	XN_MODULE_PROPERTY_LOCK									= 0x1080FF05, // "Lock"

	//---------------------------------------------------------------------------
	// General Stream Properties
	//---------------------------------------------------------------------------
	/** Integer */
	XN_STREAM_PROPERTY_STATE								= 0x1080FF10, // "State"
	/** Integer */
	XN_STREAM_PROPERTY_REQUIRED_DATA_SIZE					= 0x1080FF11, // "RequiredDataSize"
	/** Integer (OniFormat) */
	XN_STREAM_PROPERTY_OUTPUT_FORMAT						= 0x1080FF12, // "OutputFormat"
	/** Integer */ 
	XN_STREAM_PROPERTY_BUFFER_SIZE							= 0x1080FF13, // "BufferSize"
	/** Boolean */
	XN_STREAM_PROPERTY_ACTUAL_READ_DATA						= 0x1080FF14, // "ActualReadData"

	//---------------------------------------------------------------------------
	// Frame-Based Stream Properties (Depth, Image, IR)
	//---------------------------------------------------------------------------
	/** Integer */ 
	XN_STREAM_PROPERTY_FPS									= 0x1080FF20, // "FPS"

	//---------------------------------------------------------------------------
	// Pixel-Based Stream Properties (Depth, Image, IR)
	//---------------------------------------------------------------------------
	/** XnResolutions */
	XN_STREAM_PROPERTY_RESOLUTION							= 0x1080FF30, // "Resolution"
	/** Integer */ 
	XN_STREAM_PROPERTY_X_RES								= 0x1080FF31, // "XRes"
	/** Integer */ 
	XN_STREAM_PROPERTY_Y_RES								= 0x1080FF32, // "YRes"
	/** Integer */ 
	XN_STREAM_PROPERTY_BYTES_PER_PIXEL						= 0x1080FF33, // "BytesPerPixel"
	/** Integer */
	XN_STREAM_PROPERTY_SUPPORT_MODES_COUNT					= 0x1080FF34, // "SupportedModesCount"
	/** General (XnCmosPreset array) */
	XN_STREAM_PROPERTY_SUPPORT_MODES						= 0x1080FF35, // "SupportedModes"
	/** OniCropping */ 
	XN_STREAM_PROPERTY_CROPPING								= 0x1080FF36, // "Cropping"

	//---------------------------------------------------------------------------
	// Depth Specific Properties
	//---------------------------------------------------------------------------
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_MIN_DEPTH							= 0x1080FF40, // "MinDepthValue"
	/** unsigned long long */ 
	XN_STREAM_PROPERTY_MAX_DEPTH							= 0x1080FF41, // "MaxDepthValue"
	/** Boolean */ 
	XN_STREAM_PROPERTY_REGISTRATION							= 0x1080FF42, // "Registration"
	/** Integer */
	XN_STREAM_PROPERTY_DEVICE_MAX_DEPTH						= 0x1080FF43, // "DeviceMaxDepth"

	//---------------------------------------------------------------------------
	// Image Specific Properties
	//---------------------------------------------------------------------------
	/** Integer */ 
	XN_STREAM_PROPERTY_QUALITY								= 0x1080FF51, // "Quality"

	//---------------------------------------------------------------------------
	// Audio Specific Properties
	//---------------------------------------------------------------------------
	/** XnSampleRate */
	XN_STREAM_PROPERTY_SAMPLE_RATE							= 0x1080FF60, // "SampleRate"
	/** Integer */
	XN_STREAM_PROPERTY_LEFT_CHANNEL_VOLUME					= 0x1080FF61, // "LeftChannelVolume"
	/** Integer */
	XN_STREAM_PROPERTY_RIGHT_CHANNEL_VOLUME					= 0x1080FF62, // "RightChannelVolume"
	/** Integer */
	XN_STREAM_PROPERTY_NUMBER_OF_CHANNELS					= 0x1080FF63, // "NumOfChannels"
	/** Boolean */
	XN_STREAM_PROPERTY_IS_STEREO							= 0x1080FF64, // "IsStereo"
	/** Integer */
	XN_STREAM_PROPERTY_READ_MODE							= 0x1080FF65, // "ReadMode"
	/** Integer */
	XN_STREAM_PROPERTY_READ_CHUNK_SIZE						= 0x1080FF66, // "ReadChunkSize"
	/** Integer */
	XN_STREAM_PROPERTY_READ_SYNC							= 0x1080FF67, // "AudioReadSync"

	//---------------------------------------------------------------------------
	// DeviceParams Properties
	//---------------------------------------------------------------------------
	/** Integer */
	XN_MODULE_PROPERTY_NUMBER_OF_BUFFERS					= 0x1080FF70, // "NumberOfBuffers"
	/** Boolean */
	XN_MODULE_PROPERTY_READ_DATA							= 0x1080FF71, // "ReadData"
	/** Integer */
	XN_MODULE_PROPERTY_READ_WRITE_MODE						= 0x1080FF72, // "ReadWriteMode"
	/** Boolean */
	XN_MODULE_PROPERTY_FRAME_SYNC							= 0x1080FF73, // "FrameSync"
	/* XnDynamicSizeBuffer */
	XN_MODULE_PROPERTY_FIXED_PARAMS							= 0x1080FF76, // "FixedParams"
	/** Integer */
	XN_MODULE_PROPERTY_ERROR_STATE							= 0x1080FF79, // "ErrorState"
	/** Boolean */
	XN_MODULE_PROPERTY_AUDIO_SUPPORTED						= 0x1080FF7D, // "AudioSupported"
	/** Boolean */
	XN_MODULE_PROPERTY_IMAGE_SUPPORTED						= 0x1080FF7E, // "ImageSupported"
};

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_GAIN_AUTO	0U

#define XN_QVGA_X_RES	320
#define XN_QVGA_Y_RES	240
#define XN_VGA_X_RES	640
#define XN_VGA_Y_RES	480
#define XN_SXGA_X_RES	1280
#define XN_SXGA_Y_RES	1024
#define XN_UXGA_X_RES	1600
#define XN_UXGA_Y_RES	1200

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

typedef enum XnSampleRate
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
} XnSampleRate;

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

//---------------------------------------------------------------------------
// Data Structures - structures that are arguments to properties
//---------------------------------------------------------------------------

#pragma pack (push, 1)

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

#pragma pack (pop)

#endif // _XN_STREAM_PARAMS_H_
