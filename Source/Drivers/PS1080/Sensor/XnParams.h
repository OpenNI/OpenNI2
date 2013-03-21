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
#ifndef XN_PARAMS_H
#define XN_PARAMS_H

typedef enum 
{
	//General,
	PARAM_GENERAL_CURRENT_MODE = 0,
	PARAM_GENERAL_FRAME_SYNC = 1,
	PARAM_GENERAL_REGISTRATION_ENABLE = 2,
	PARAM_GENERAL_STREAM_PRIORITY = 3,
	PARAM_GENERAL_TRIGGER_ACTION = 4,
	PARAM_GENERAL_STREAM0_MODE = 5,
	PARAM_GENERAL_STREAM1_MODE = 6,
	//Audio,
	PARAM_GENERAL_STREAM2_MODE = 7,
	PARAM_AUDIO_STEREO_MODE = 8,
	PARAM_AUDIO_SAMPLE_RATE = 9,
	PARAM_AUDIO_LEFT_CHANNEL_VOLUME_LEVEL = 10,
	PARAM_AUDIO_RIGHT_CHANNEL_VOLUME_LEVEL = 11,
	//Image,
	PARAM_IMAGE_FORMAT = 12,
	PARAM_IMAGE_RESOLUTION = 13,
	PARAM_IMAGE_FPS = 14,
	PARAM_IMAGE_AGC = 15,
	PARAM_IMAGE_QUALITY = 16,
	PARAM_IMAGE_FLICKER_DETECTION = 17,
	//Depth,
	PARAM_DEPTH_FORMAT = 18,
	PARAM_DEPTH_RESOLUTION = 19,
	PARAM_DEPTH_FPS = 20,
	PARAM_DEPTH_AGC = 21,
	PARAM_DEPTH_HOLE_FILTER = 22,
	PARAM_DEPTH_MIRROR = 23,
	PARAM_DEPTH_DECIMATION = 24,
	//IR,
	PARAM_IR_FORMAT = 25,
	PARAM_IR_RESOLUTION = 26,
	PARAM_IR_FPS = 27,
	PARAM_IR_AGC = 28,
	PARAM_IR_QUALITY = 29,
	//Misc,
	PARAM_MISC_LOG_FILTER = 30,
	PARAM_MISC_PACKET_TIMEOUT = 31,
	PARAM_MISC_PS_SESSION_TIMEOUT = 32,
	PARAM_AUDIO_LEFT_CHANNEL_MUTE = 33,
	PARAM_AUDIO_RIGHT_CHANNEL_MUTE = 34,
	PARAM_AUDIO_MICROPHONE_IN = 35,
	PARAM_DEPTH_GMC_MODE = 36,
	PARAM_DEPTH_GMC_REF_OFFSET = 37,
	PARAM_DEPTH_GMC_RICC = 38,
	PARAM_DEPTH_GMC_DX_CORRECTION = 39,
	PARAM_DEPTH_GMC_DX_CORRECTION_A = 40,
	PARAM_DEPTH_GMC_DX_CORRECTION_B_HIGH = 41,
	PARAM_DEPTH_GMC_DX_CORRECTION_B_LOW = 42,
	PARAM_DEPTH_GMC_DX_CORRECTION_DB_HIGH = 43,
	PARAM_DEPTH_GMC_DX_CORRECTION_DB_LOW = 44,
	PARAM_DEPTH_WHITE_BALANCE_ENABLE = 45,

	//Image Crop
	PARAM_IMAGE_CROP_SIZE_X = 46,
	PARAM_IMAGE_CROP_SIZE_Y = 47,
	PARAM_IMAGE_CROP_OFFSET_X = 48,
	PARAM_IMAGE_CROP_OFFSET_Y = 49,
	PARAM_IMAGE_CROP_MODE = 50,
	//Depth Crop
	PARAM_DEPTH_CROP_SIZE_X = 51,
	PARAM_DEPTH_CROP_SIZE_Y = 52,

	PARAM_DEPTH_CROP_OFFSET_X = 53,
	PARAM_DEPTH_CROP_OFFSET_Y = 54,
	PARAM_DEPTH_CROP_MODE = 55,
	//IR Crop
	PARAM_IR_CROP_SIZE_X = 56,
	PARAM_IR_CROP_SIZE_Y = 57,
	PARAM_IR_CROP_OFFSET_X = 58,
	PARAM_IR_CROP_OFFSET_Y = 59,
	PARAM_IR_CROP_MODE = 60,

	PARAM_GMC_DEBUG = 61,
	PARAM_APC_ENABLE = 62,

	PARAM_DEPTH_AGC_BIN0_LOW = 63,
	PARAM_DEPTH_AGC_BIN0_HIGH = 64,
	PARAM_DEPTH_AGC_BIN1_LOW = 65,
	PARAM_DEPTH_AGC_BIN1_HIGH = 66,
	PARAM_DEPTH_AGC_BIN2_LOW = 67,
	PARAM_DEPTH_AGC_BIN2_HIGH = 68,
	PARAM_DEPTH_AGC_BIN3_LOW = 69,
	PARAM_DEPTH_AGC_BIN3_HIGH = 70,

	PARAM_IMAGE_MIRROR = 71,
	PARAM_IR_MIRROR = 72,
	PARAM_WAVELENGTH_CORRECTION_ENABLED = 73,
	PARAM_WAVELENGTH_CORRECTION_DEBUG_ENABLED = 74,
	PARAM_GMC_CORRECTION_MODE = 75,
	PARAM_IMAGE_SHARPNESS = 76,
	PARAM_IMAGE_AUTO_WHITE_BALANCE_MODE = 77,
	PARAM_IMAGE_COLOR_TEMPERATURE = 78,
	PARAM_IMAGE_BACK_LIGHT_COMPENSATION = 79,
	PARAM_IMAGE_AUTO_EXPOSURE_MODE = 80,
	PARAM_IMAGE_EXPOSURE_BAR = 81,
	PARAM_IMAGE_LOW_LIGHT_COMPENSATION_MODE = 82,
	PARAM_DEPTH_CLOSE_RANGE = 84,
	PARAM_FILE_SYSTEM_LOCK = 85,
} EConfig_Params;

typedef enum XnExecuter
{
	XN_EXECUTER_NONE = 0,
	XN_EXECUTER_FW = 1,
	XN_EXECUTER_HOST = 2,
} XnExecuter;

#endif
