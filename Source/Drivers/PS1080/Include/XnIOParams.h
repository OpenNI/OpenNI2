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
#ifndef _XN_IO_PARAMS_H_
#define _XN_IO_PARAMS_H_

#include <XnPlatform.h>
#include <XnStream.h>
#include <XnStreamParams.h>

#define XN_IO_PARAM_MIN_DEPTH_VALUE				"MinDepthValue"
#define XN_IO_PARAM_MAX_DEPTH_VALUE				"MaxDepthValue"
#define XN_IO_PARAM_FRAME_DELAY					"FrameDelay"
#define XN_IO_PARAM_DEPTH_COMPRESSION			"DepthCompression"
#define XN_IO_PARAM_IMAGE_COMPRESSION			"ImageCompression"
#define XN_IO_PARAM_MISC_COMPRESSION			"MiscCompression"
#define XN_IO_PARAM_AUDIO_COMPRESSION			"AudioCompression"
#define XN_IO_PARAM_RGB_REGISTRAR_STATE			"RGBRegistrarState"
#define XN_IO_PARAM_AUDIO_ENABLED				"AudioEnabled"
#define XN_IO_PARAM_AUDIO_LEFT_CHANNEL_VOLUME	"AudioLeftChannelVolume"
#define XN_IO_PARAM_AUDIO_RIGHT_CHANNEL_VOLUME	"AudioRightChannelVolume"
#define XN_IO_PARAM_AUDIO_SAMPLE_RATE			"AudioSampleRate"
#define XN_IO_PARAM_AUDIO_READ_MODE				"AudioReadMode"
#define XN_IO_PARAM_AUDIO_READ_CHUNK_SIZE		"AudioReadChunkSize"
#define XN_IO_PARAM_AUDIO_READ_SYNC				"AudioReadSync"
#define XN_IO_PARAM_DEPTH_WHITE_BALANCE			"DepthWhiteBalance"
#define XN_IO_PARAM_DEPTH_INPUT_FORMAT			"DepthInputFormat"
#define XN_IO_PARAM_IMAGE_INPUT_FORMAT			"ImageInputFormat"
#define XN_IO_PARAM_DEPTH_STATE					"DepthState"
#define XN_IO_PARAM_IMAGE_STATE 				"ImageState"
#define XN_IO_PARAM_HIGHRES_TIMESTAMPS			"HighresTimestamps"


#define XN_IO_REGMODE_NO_REGISTER		(XnUInt32)0
#define XN_IO_REGMODE_IMAGE_TO_DEPTH	(XnUInt32)1
#define XN_IO_REGMODE_DEPTH_TO_IMAGE	(XnUInt32)2

#ifdef _XN_DEPRECATE_OLD_IO
#pragma deprecated ("XN_IO_PARAM_MIN_DEPTH_VALUE", "XN_IO_PARAM_MAX_DEPTH_VALUE", "XN_IO_PARAM_FRAME_DELAY", "XN_IO_PARAM_DEPTH_COMPRESSION")
#pragma deprecated ("XN_IO_PARAM_IMAGE_COMPRESSION", "XN_IO_PARAM_MISC_COMPRESSION", "XN_IO_PARAM_AUDIO_COMPRESSION", "XN_IO_PARAM_RGB_REGISTRAR_STATE")
#pragma deprecated ("XN_IO_PARAM_AUDIO_ENABLED", "XN_IO_PARAM_AUDIO_LEFT_CHANNEL_VOLUME", "XN_IO_PARAM_AUDIO_RIGHT_CHANNEL_VOLUME")
#pragma deprecated ("XN_IO_PARAM_AUDIO_SAMPLE_RATE", "XN_IO_PARAM_AUDIO_READ_MODE", "XN_IO_PARAM_AUDIO_READ_CHUNK_SIZE")
#pragma deprecated ("XN_IO_PARAM_AUDIO_READ_SYNC", "XN_IO_PARAM_GMC_MODE", "XN_IO_PARAM_DEPTH_WHITE_BALANCE", "XN_IO_PARAM_DEPTH_INPUT_FORMAT")
#pragma deprecated ("XN_IO_PARAM_IMAGE_INPUT_FORMAT", "XN_IO_PARAM_DEPTH_STATE", "XN_IO_PARAM_IMAGE_STATE", "XN_IO_PARAM_READ_LOG_INTERVAL")
#pragma deprecated ("XN_IO_REGMODE_NO_REGISTER", "XN_IO_REGMODE_IMAGE_TO_DEPTH", "XN_IO_REGMODE_DEPTH_TO_IMAGE")
#endif

typedef XN_OLD_IO_API enum XnIOParams
{
	XN_IO_PARAM_IMAGE_RESOLUTION = 0,			// Get only for now
	XN_IO_PARAM_IMAGE_FPS = 1,					// Get only for now
	XN_IO_PARAM_IMAGE_AGC = 2,					// PARAM_RGB_AGC
	XN_IO_PARAM_IMAGE_CONTROL_PROCESSING = 3,	// I2C
	XN_IO_PARAM_IMAGE_FLICKER_DETECTION = 4,	// PARAM_IMAGE_FLICKER_DETECTION

	XN_IO_PARAM_DEPTH_RESOLUTION = 5,			// Get only for now
	XN_IO_PARAM_DEPTH_FPS = 6,					// Get only for now
	XN_IO_PARAM_DEPTH_AGC = 7,					// PARAM_DEPTH_AGC
	XN_IO_PARAM_DEPTH_HOLE_FILTER = 8,			// PARAM_HOLE_FILTER
	XN_IO_PARAM_DEPTH_CONTROL_PROCESSING = 9,	// I2C

	XN_IO_PARAM_IR_RESOLUTION = 10,				// Get only for now
	XN_IO_PARAM_IR_FPS = 11,					// Get only for now
	XN_IO_PARAM_IR_AGC = 12,

	XN_IO_PARAM_REGISTRATION = 13,				// PARAM_REGISTRATION_ENABLE
	XN_IO_PARAM_FRAME_SYNC = 14,				// PARAM_FRAME_SYNC_BY
	XN_IO_PARAM_DEPTH_MIRROR = 15,				// PARAM_MIRROR

	XN_IO_PARAM_INNER_PARAM = 17,				// PARAM
	XN_IO_PARAM_VERSION = 18,					// VERSION
	XN_IO_PARAM_PRIMARY = 25,					// 'Read' returns when new image, new depth or new any received 
	XN_IO_PARAM_READ_WRITE_MODE = 26,			// Get only

	XN_IO_PARAM_SHIFT2DEPTH = 27,
	XN_IO_PARAM_DEPTH2SHIFT = 28,

	XN_IO_PARAM_RESET = 29,						// Reset (either power or software)

	XN_IO_PARAM_CURRENT_MODE = 31,				// Maintenance/Normal

	XN_IO_PARAM_LAST_RAW_IMAGE = 32,			// Last RAW Image

	XN_IO_PARAM_IR_CROPPING = 33,				// IR Cropping

	XN_IO_PARAM_IMAGE_QUALITY = 38,				// Image Quality - 0-default, 1-low, 2-medium, 3-high
	XN_IO_PARAM_CMOS_BLANKING_UNITS = 46,		// XnCmosBlanking
	XN_IO_PARAM_CMOS_BLANKING_TIME = 47,		// XnCmosBlankingTime

	XN_IO_PARAM_DEPTH_AGC_BIN = 48,				// XnDepthAGCBin

	XN_IO_PARAM_SUPPORTS_MIRRORING = 49,		// Is mirror supported (bool, get only)
	XN_IO_PARAM_MIRROR = 50,
	XN_IO_PARAM_IMAGE_MIRROR = 51,
	XN_IO_PARAM_IR_MIRROR = 52,

} XnIOParams;

typedef XN_OLD_IO_API enum
{
	XN_IO_DEPTH_DECIMATION_DISABLED,
	XN_IO_DEPTH_DECIMATION_ENABLED
} XnIODepthDecimation;

// some structs here that are deprecated holds other deprecated, so disable warnings
#pragma warning (push)
#pragma warning (disable: XN_DEPRECATED_WARNING_IDS)

typedef XN_OLD_IO_API struct XnDeviceSensorParameters
{
	XnBool bConfigure;
	XnUInt8 VideoMode;
	XnBool bAudioOn;
	XnIOImageFormats ImageFormat;
	XnIODepthFormats DepthFormat;
	XnIODepthDecimation DepthDecimation;
} XnDeviceSensorParameters;

typedef XN_OLD_IO_API enum
{
	XN_PRIMARY_ANY,
	XN_PRIMARY_DEPTH,
	XN_PRIMARY_IMAGE
} XnParamPrimary;

#pragma pack (push, 1)

typedef XN_OLD_IO_API struct {
	XnUChar* pBuffer;
	XnUInt32 nBufferSize;
} XnDataBuffer;

#pragma pack (pop)

#pragma warning (pop)

// Set Param
// (xx)(yyy)(zzz) - byte comprised of mode(x), stream0(y), stream1(z)
#define XN_VIDEO_PS_MODE		0
#define XN_VIDEO_WEBCAM_MODE	1

#define XN_VIDEO_CURRENT_GET_MODE(x) (((x)>>6)&0x03)
#define XN_VIDEO_STREAM0_GET_MODE(x) (((x)>>3)&0x07)
#define XN_VIDEO_STREAM1_GET_MODE(x) ((x)&0x07)
#define XN_VIDEO_CURRENT_MODE(x) ((x)<<6)
#define XN_VIDEO_STREAM0_MODE(x) (((x)&0x7)<<3)
#define XN_VIDEO_STREAM1_MODE(x) ((x)&0x7)
#define XN_VIDEO_MODE(current, stream0, stream1) \
			(XN_VIDEO_CURRENT_MODE(current)|XN_VIDEO_STREAM0_MODE(stream0)|XN_VIDEO_STREAM1_MODE(stream1))

#define XN_VIDEO_MODE_OFF			XN_VIDEO_MODE(XN_VIDEO_PS_MODE,		XN_VIDEO_STREAM_OFF,	XN_VIDEO_STREAM_OFF)  // 0x00
#define XN_VIDEO_MODE_COLOR_PS		XN_VIDEO_MODE(XN_VIDEO_PS_MODE,		XN_VIDEO_STREAM_COLOR,	XN_VIDEO_STREAM_OFF)  // 0x08
#define XN_VIDEO_MODE_IR			XN_VIDEO_MODE(XN_VIDEO_PS_MODE,		XN_VIDEO_STREAM_IR,		XN_VIDEO_STREAM_OFF)  // 0x18
#define XN_VIDEO_MODE_DEPTH			XN_VIDEO_MODE(XN_VIDEO_PS_MODE,		XN_VIDEO_STREAM_OFF,	XN_VIDEO_STREAM_DEPTH)// 0x02
#define XN_VIDEO_MODE_COLOR_DEPTH	XN_VIDEO_MODE(XN_VIDEO_PS_MODE,		XN_VIDEO_STREAM_COLOR,	XN_VIDEO_STREAM_DEPTH)// 0x0a
#define XN_VIDEO_MODE_DEPTH_IR		XN_VIDEO_MODE(XN_VIDEO_PS_MODE,		XN_VIDEO_STREAM_IR,		XN_VIDEO_STREAM_DEPTH)// 0x1a
// Not supported for now?
#define XN_VIDEO_MODE_WEBCAM		XN_VIDEO_MODE(XN_VIDEO_WEBCAM_MODE,	XN_VIDEO_STREAM_COLOR,	XN_VIDEO_STREAM_OFF)  // 0x48
#define XN_VIDEO_MODE_COLOR_IR		XN_VIDEO_MODE(XN_VIDEO_PS_MODE,		XN_VIDEO_STREAM_COLOR,	XN_VIDEO_STREAM_IR)   // 0x0b

#ifdef _XN_DEPRECATE_OLD_IO
#pragma deprecated ("XN_VIDEO_PS_MODE", "XN_VIDEO_WEBCAM_MODE")
#pragma deprecated ("XN_VIDEO_CURRENT_GET_MODE", "XN_VIDEO_STREAM0_GET_MODE", "XN_VIDEO_STREAM1_GET_MODE")
#pragma deprecated ("XN_VIDEO_CURRENT_MODE", "XN_VIDEO_STREAM0_MODE", "XN_VIDEO_STREAM1_MODE", "XN_VIDEO_MODE")
#pragma deprecated ("XN_VIDEO_MODE_OFF", "XN_VIDEO_MODE_COLOR_PS", "XN_VIDEO_MODE_IR", "XN_VIDEO_MODE_DEPTH")
#pragma deprecated ("XN_VIDEO_MODE_COLOR_DEPTH", "XN_VIDEO_MODE_DEPTH_IR", "XN_VIDEO_MODE_WEBCAM", "XN_VIDEO_MODE_COLOR_IR")
#endif

#endif
