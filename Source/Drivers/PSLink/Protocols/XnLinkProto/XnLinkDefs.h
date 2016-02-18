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
#ifndef XNLINKDEFS_H
#define XNLINKDEFS_H

/* Version */
#define XN_LINK_PROTOCOL_MAJOR_VERSION				0
#define XN_LINK_PROTOCOL_MINOR_VERSION				56

/* Magic numbers */
#define XN_LINK_MAGIC								0x5350 //"PS"
#define XN_LINK_STREAM_ID_NONE						0x0
#define XN_LINK_STREAM_ID_INVALID					0xFFFF

/* Max sizes */
#define XN_LINK_MAX_STREAMS							32 //TODO: This is not really the max number of streams! Stream ID is 14 bit, so theoretically max id is 0x3FFF
#define XN_LINK_MAX_CREATION_INFO_LENGTH			80
#define XN_LINK_MAX_LOG_MASK_LENGTH					16
#define XN_LINK_SERIAL_NUMBER_SIZE					32
#define XN_LINK_MAX_BIST_NAME_LENGTH				32
#define XN_LINK_MAX_FILE_NAME_LENGTH				32
#define XN_LINK_MAX_VERSION_MODIFIER_LENGTH			16
#define XN_LINK_MAX_COMPONENT_NAME_LENGTH			32
#define XN_LINK_MAX_VERSION_LENGTH					32
#define XN_LINK_MAX_I2C_DEVICE_NAME_LENGTH			32
#define XN_LINK_MAX_LOG_FILE_NAME_LENGTH			32
#define XN_LINK_MAX_SENSOR_NAME_LENGTH				16
#define XN_LINK_MAX_TEMPERATURE_SENSORS				2

/* Interface ID's */
typedef enum XnLinkInterfaceID
{
	XN_LINK_INTERFACE_FW_MGMT					= 0x00,
	XN_LINK_INTERFACE_LINK						= 0x01,
	XN_LINK_INTERFACE_SYS_MGMT					= 0x02,
	XN_LINK_INTERFACE_DATA_STREAMING			= 0x03,
	XN_LINK_INTERFACE_RESERVED1					= 0x04,
	XN_LINK_INTERFACE_MAP_GENERATOR				= 0x05,
	XN_LINK_INTERFACE_STREAM_MGMT				= 0x06,
	XN_LINK_INTERFACE_PROPS						= 0x07,
	XN_LINK_INTERFACE_RESERVED2					= 0x08,
	XN_LINK_INTERFACE_HANDS_GENERATOR			= 0x09,
	XN_LINK_INTERFACE_S2D						= 0x0A,
	XN_LINK_INTERFACE_GESTURE_GENERATOR			= 0x0B,
	XN_LINK_INTERFACE_USER_GENERATOR			= 0x0C,
	XN_LINK_INTERFACE_DEPTH_GENERATOR			= 0x0D,
	XN_LINK_INTERFACE_MIRROR					= 0x0E,
	XN_LINK_INTERFACE_ALTERNATIVE_VIEW_POINT	= 0x0F,
	XN_LINK_INTERFACE_CROPPING					= 0x10,
	XN_LINK_INTERFACE_USER_POSITION				= 0x11,
	XN_LINK_INTERFACE_SKELETON					= 0x12,
	XN_LINK_INTERFACE_POSE_DETECTION			= 0x13,
	XN_LINK_INTERFACE_LOCK_AWARE				= 0x14,
	XN_LINK_INTERFACE_ERROR_STATE				= 0x15,
	XN_LINK_INTERFACE_FRAME_SYNC				= 0x16,
	XN_LINK_INTERFACE_DEVICE_IDENTIFICATION		= 0x17,
	XN_LINK_INTERFACE_BRIGHTNESS				= 0x18,
	XN_LINK_INTERFACE_CONTRAST					= 0x19,
	XN_LINK_INTERFACE_HUE						= 0x1A,
	XN_LINK_INTERFACE_SATURATION				= 0x1B,
	XN_LINK_INTERFACE_SHARPNESS					= 0x1C,
	XN_LINK_INTERFACE_GAMMA						= 0x1D,
	XN_LINK_INTERFACE_COLOR_TEMPERATURE			= 0x1E,
	XN_LINK_INTERFACE_BACKLIGHT_COMPENSATION	= 0x1F,
	XN_LINK_INTERFACE_GAIN						= 0x20,
	XN_LINK_INTERFACE_PAN						= 0x21,
	XN_LINK_INTERFACE_TILT						= 0x22,
	XN_LINK_INTERFACE_ROLL						= 0x23,
	XN_LINK_INTERFACE_ZOOM						= 0x24,
	XN_LINK_INTERFACE_EXPOSURE					= 0x25,
	XN_LINK_INTERFACE_IRIS						= 0x26,
	XN_LINK_INTERFACE_FOCUS						= 0x27,
	XN_LINK_INTERFACE_LOW_LIGHT_COMPENSATION	= 0x28,
	XN_LINK_INTERFACE_ANTI_FLICKER				= 0x29,
	XN_LINK_INTERFACE_HAND_TOUCHING_FOV_EDGE	= 0x2A,
	XN_LINK_INTERFACE_PROJECTOR_MGMT			= 0x2B,
	XN_LINK_INTERFACE_INVALID					= 0xFF, //Signifies an invalid interface ID
} XnLinkInterfaceID;

/* Message Types - Control messages */
typedef enum XnLinkMsgType
{
	XN_LINK_MSG_NONE							= 0x0000,

//XN_LINK_INTERFACE_FW_MGMT - Firmware management messages - group 0x00
	XN_LINK_MSG_RESERVED_1						= 0x0001,
	XN_LINK_MSG_UPLOAD_FILE         			= 0x0002,
	XN_LINK_MSG_RESERVED_3			            = 0x0003,
	XN_LINK_MSG_RESERVED_4						= 0x0004,
	XN_LINK_MSG_BEGIN_UPLOAD					= 0x0005,
	//Deprecated - XN_LINK_MSG_BEGIN_RECEIVE_DATA			= 0x0006,
	//Deprecated - XN_LINK_MSG_END_RECEIVE_DATA				= 0x0007,
	XN_LINK_MSG_END_UPLOAD						= 0x0008,
	XN_LINK_MSG_GET_FILE_LIST					= 0x0009,
	XN_LINK_MSG_DOWNLOAD_FILE					= 0x000A, // in: XnLinkDownloadFileParams, out: file data
    XN_LINK_MSG_FORMAT_ZONE                     = 0x000B,

//XN_LINK_INTERFACE_LINK - Link messages - group 0x01
	XN_LINK_MSG_CONTINUE_REPONSE				= 0x0101,

//XN_LINK_INTERFACE_SYS_MGMT - System management messages - group 0x02
	XN_LINK_MSG_SOFT_RESET						= 0x0201,
	XN_LINK_MSG_HARD_RESET						= 0x0202,
	XN_LINK_MSG_WRITE_I2C						= 0x0203,
	XN_LINK_MSG_READ_I2C						= 0x0204,
	XN_LINK_MSG_WRITE_AHB						= 0x0205,
	XN_LINK_MSG_READ_AHB						= 0x0206,
	XN_LINK_MSG_EXECUTE_BIST_TESTS				= 0x0207, // In: XnLinkExecuteBistParams Out: XnLinkExecuteBistResponse
	XN_LINK_MSG_GET_ACC_CURENT_PARAM            = 0x0208,
	XN_LINK_MSG_SET_PWM_DC                      = 0x0209,
	XN_LINK_MSG_START_USB_TEST					= 0x020A, 
	XN_LINK_MSG_STOP_USB_TEST					= 0x020B, 
	XN_LINK_MSG_START_LOG_FILE					= 0x020C,
	XN_LINK_MSG_STOP_LOG_FILE					= 0x020D,
	XN_LINK_MSG_READ_TEMPERATURE				= 0x020E,


//XN_LINK_INTERFACE_DATA_STREAMING - Data streaming messages - group 0x03
	XN_LINK_MSG_START_STREAMING					= 0x0300,
	XN_LINK_MSG_DATA							= 0x0301,	//Sent on data endpoint, not control
	XN_LINK_MSG_STOP_STREAMING					= 0x0302,
    XN_LINK_MSG_START_STREAMING_MULTI           = 0x0303,
    XN_LINK_MSG_STOP_STREAMING_MULTI            = 0x0304,

//XN_LINK_INTERFACE_RESERVED1

//XN_LINK_INTERFACE_MAP_GENERATOR - Map generator messages - group 0x05
	XN_LINK_MSG_GET_CAMERA_INTRINSICS 			= 0x0501, // Out: XnLinkCameraIntrinsics

//XN_LINK_INTERFACE_STREAM_MGMT - Stream management messages - group 0x06
	XN_LINK_MSG_ENUMERATE_STREAMS				= 0x0601,
	XN_LINK_MSG_CREATE_STREAM					= 0x0602,
	XN_LINK_MSG_DESTROY_STREAM					= 0x0603,

//XN_LINK_INTERFACE_PROPS - Set/Get property messages - group 0x07
	XN_LINK_MSG_GET_PROP						= 0x0701,
	XN_LINK_MSG_SET_PROP						= 0x0702,
	XN_LINK_MSG_SET_MULTI_PROPS					= 0x0703,
	XN_LINK_MSG_GET_DEBUG_DATA	                = 0x0704,

//XN_LINK_INTERFACE_HANDS_GENERATOR - HandGenerator messages - group 0x09
	XN_LINK_MSG_START_TARCKING_HAND				= 0x0901,
	XN_LINK_MSG_STOP_TARCKING_HAND				= 0x0902,
	XN_LINK_MSG_STOP_TARCKING_ALL_HANDS			= 0x0903,

//XN_LINK_INTERFACE_S2D - Shift to depth messages - group 0x0A
	XN_LINK_MSG_GET_S2D_CONFIG					= 0x0A01,

//XN_LINK_INTERFACE_GESTURE_GENERATOR - GestureGenerator messages - group 0x0B
	XN_LINK_MSG_ENUMERATE_AVAILABLE_GESTURES	= 0x0B01,
	XN_LINK_MSG_ENUMERATE_ACTIVE_GESTURES		= 0x0B02,
	XN_LINK_MSG_ACTIVATE_GESTURE				= 0x0B03,
	XN_LINK_MSG_DEACTIVATE_GESTURE				= 0x0B04,

//XN_LINK_INTERFACE_USER_GENERATOR - UserGenerator messages - group 0x0C

// XN_LINK_INTERFACE_MIRROR - Mirror messages - group 0x0E

// XN_LINK_INTERFACE_ALTERNATIVE_VIEW_POINT messages - group 0x0F

// XN_LINK_INTERFACE_CROPPING messages - group 0x10

// XN_LINK_INTERFACE_USER_POSITION messages - group 0x11

// XN_LINK_INTERFACE_SKELETON messages - group 0x12
	XN_LINK_MSG_SET_SKELETON_PROFILE			= 0x1201,
	XN_LINK_MSG_SET_SKELETON_JOINT_STATE		= 0x1202,
	XN_LINK_MSG_REQUEST_CALIBRATION				= 0x1203,
	XN_LINK_MSG_ABORT_CALIBRATION				= 0x1204,
	XN_LINK_MSG_SAVE_SKELETON_CALIBRATION_DATA	= 0x1205,
	XN_LINK_MSG_LOAD_SKELETON_CALIBRATION_DATA	= 0x1206,
	XN_LINK_MSG_CLEAR_SKELETON_CALIBRATION_DATA	= 0x1207,
	XN_LINK_MSG_IS_SKELETON_CALIBRATION_SLOT_FREE	= 0x1208,
	XN_LINK_MSG_GET_SKELETON_CALIBRATION_DATA	= 0x1209,
	XN_LINK_MSG_SET_SKELETON_CALIBRATION_DATA	= 0x120A,
	XN_LINK_MSG_START_SKELETON_TRACKING			= 0x120B,
	XN_LINK_MSG_STOP_SKELETON_TRACKING			= 0x120C,
	XN_LINK_MSG_RESET_SKELETON_TRACKING			= 0x120D,

// XN_LINK_INTERFACE_POSE_DETECTION messages - group 0x13
	XN_LINK_MSG_START_POSE_DETECTION			= 0x130E,
	XN_LINK_MSG_STOP_POSE_DETECTION				= 0X130F,

// XN_LINK_INTERFACE_LOCK_AWARE messages - group 0x14

// XN_LINK_INTERFACE_ERROR_STATE messages - group 0x15

// XN_LINK_INTERFACE_FRAME_SYNC messages - group 0x16

// XN_LINK_INTERFACE_DEVICE_IDENTIFICATION messages - group 0x17

// XN_LINK_INTERFACE_BRIGHTNESS messages - group 0x18

// XN_LINK_INTERFACE_CONTRAST messages - group 0x19

// XN_LINK_INTERFACE_HUE messages - group 0x1A

// XN_LINK_INTERFACE_SATURATION messages - group 0x1B

// XN_LINK_INTERFACE_SHARPNESS messages - group 0x1C

// XN_LINK_INTERFACE_GAMMA messages - group 0x1D

// XN_LINK_INTERFACE_COLOR_TEMPERATURE messages - group 0x1E

// XN_LINK_INTERFACE_BACKLIGHT_COMPENSATION messages - group 0x1F

// XN_LINK_INTERFACE_GAIN messages - group 0x20

// XN_LINK_INTERFACE_PAN messages - group 0x21

// XN_LINK_INTERFACE_TILT messages - group 0x22

// XN_LINK_INTERFACE_ROLL messages - group 0x23

// XN_LINK_INTERFACE_ZOOM messages - group 0x24

// XN_LINK_INTERFACE_EXPOSURE messages - group 0x25

// XN_LINK_INTERFACE_IRIS messages - group 0x26

// XN_LINK_INTERFACE_FOCUS messages - group 0x27

// XN_LINK_INTERFACE_LOW_LIGHT_COMPENSATION messages - group 0x28

// XN_LINK_INTERFACE_ANTI_FLICKER messages - group 0x29

// XN_LINK_INTERFACE_HAND_TOUCHING_FOV_EDGE messages - group 0x2A

// XN_LINK_INTERFACE_PROJECTOR_MGMT messages - group 0x2B
	
	XN_LINK_MSG_INVALID							= 0xFFFF,
} XnLinkMsgType;

/* Enumerations */
typedef enum XnLinkFragmentation
{
	XN_LINK_FRAG_MIDDLE							= 0x00,
	XN_LINK_FRAG_BEGIN							= 0x01,
	XN_LINK_FRAG_END							= 0x02,
	XN_LINK_FRAG_SINGLE							= 0x03,
} XnLinkFragmentation;

typedef enum XnLinkResponseCode
{
	XN_LINK_RESPONSE_OK							= 0,  //The command succeeded
	XN_LINK_RESPONSE_PENDING					= 1,  //The command is an async command, and the host should poll the device for completion.
	XN_LINK_RESPONSE_BAD_FILE_TYPE				= 2,  //The host requested to download a file type which does not exist.
	XN_LINK_RESPONSE_CMD_ERROR					= 3,  //General command error
	XN_LINK_RESPONSE_CMD_NOT_SUPPORTED			= 4,  //The host sent a command which is not supported.
	XN_LINK_RESPONSE_BAD_CMD_SIZE				= 5,  //The host has sent a command with the wrong size of parameters data.
	XN_LINK_RESPONSE_BAD_PARAMETERS				= 6,  //The host send some bad parameters. A list of the offsets of the bad parameters will be returned.
	XN_LINK_RESPONSE_CORRUPT_PACKET				= 7,  //The device has received a packet which does not conform to the protocol.
	XN_LINK_RESPONSE_RESERVED1					= 8,  
	XN_LINK_RESPONSE_RESERVED2					= 9, 
	XN_LINK_RESPONSE_RESERVED3					= 10,
	XN_LINK_RESPONSE_RESERVED4					= 11,
	XN_LINK_RESPONSE_RESERVED5					= 12,
	XN_LINK_RESPONSE_RESERVED6					= 13,
	XN_LINK_RESPONSE_FILE_CORRUPT				= 14, //The file being loaded is corrupt
	XN_LINK_RESPONSE_BAD_CRC					= 15, //Bad CRC
	XN_LINK_RESPONSE_INCORRECT_SIZE				= 16, //The received size is incorrect
	XN_LINK_RESPONSE_INPUT_BUFFER_OVERFLOW		= 17, //Input buffer overflow
} XnLinkResponseCode;

typedef enum XnLinkStreamType
{
	XN_LINK_STREAM_TYPE_NONE					= 0x0000,
	XN_LINK_STREAM_TYPE_COLOR					= 0x0001,
	XN_LINK_STREAM_TYPE_IR						= 0x0002,
	XN_LINK_STREAM_TYPE_SHIFTS					= 0x0003,
	XN_LINK_STREAM_TYPE_AUDIO					= 0x0004,
	XN_LINK_STREAM_TYPE_DY						= 0x0005,

	XN_LINK_STREAM_TYPE_LOG						= 0x0008,
	/////////////
	XN_LINK_STREAM_TYPE_USER					= 0x000A,
	XN_LINK_STREAM_TYPE_HANDS					= 0x000B,
	XN_LINK_STREAM_TYPE_GESTURES				= 0x000C,

	XN_LINK_STREAM_TYPE_INVALID					= 0xFFFF,
} XnLinkStreamType;

typedef enum
{
    XN_LINK_PIXEL_FORMAT_NONE					= 0x0000,
	XN_LINK_PIXEL_FORMAT_SHIFTS_9_3				= 0x0001,
	XN_LINK_PIXEL_FORMAT_GRAYSCALE16			= 0x0002,
	XN_LINK_PIXEL_FORMAT_YUV422					= 0x0003,
	XN_LINK_PIXEL_FORMAT_BAYER8					= 0x0004,
} XnLinkPixelFormat;

typedef enum
{
	XN_LINK_COMPRESSION_NONE					= 0x0000,
	XN_LINK_COMPRESSION_8Z						= 0x0001,
	XN_LINK_COMPRESSION_16Z						= 0x0002,
	XN_LINK_COMPRESSION_24Z						= 0x0003,
	XN_LINK_COMPRESSION_6_BIT_PACKED			= 0x0004,
	XN_LINK_COMPRESSION_10_BIT_PACKED			= 0x0005,
	XN_LINK_COMPRESSION_11_BIT_PACKED           = 0x0006,
	XN_LINK_COMPRESSION_12_BIT_PACKED			= 0x0007,
} XnLinkCompressionType;

typedef enum XnLinkStreamFragLevel
{
	XN_LINK_STREAM_FRAG_LEVEL_NONE				= 0,
	XN_LINK_STREAM_FRAG_LEVEL_FRAMES			= 1,
	XN_LINK_STREAM_FRAG_LEVEL_CONTINUOUS		= 2,
} XnLinkStreamFragLevel;

typedef enum XnLinkPropType
{
	XN_LINK_PROP_TYPE_NONE						= 0x0000,
	XN_LINK_PROP_TYPE_INT						= 0x0001,
	XN_LINK_PROP_TYPE_REAL						= 0x0002,
	XN_LINK_PROP_TYPE_STRING					= 0x0003,
	XN_LINK_PROP_TYPE_GENERAL					= 0x0004,
	XN_LINK_PROP_TYPE_MAX						= XN_LINK_PROP_TYPE_GENERAL,
} XnLinkPropType;

typedef enum XnLinkPropID
{
	XN_LINK_PROP_ID_NONE						= 0x0000,

	//FW management properties - group 0x00 (All of these are global, not related to specific stream)
	XN_LINK_PROP_ID_CONTROL_MAX_PACKET_SIZE		= 0x0001, //Int property (read only property)
	XN_LINK_PROP_ID_FW_VERSION					= 0x0002, //General property, holds XnLinkDetailedVersion (read only property)
	XN_LINK_PROP_ID_PROTOCOL_VERSION			= 0x0003, //General property, holds XnLinkLeanVersion (read only property)
	XN_LINK_PROP_ID_SUPPORTED_MSG_TYPES			= 0x0004, //General property, holds XnLinkIDSet (read only property)
	XN_LINK_PROP_ID_SUPPORTED_PROPS				= 0x0005, //General property, holds XnLinkIDSet (read only property)
    XN_LINK_PROP_ID_HW_VERSION                  = 0x0006, //Int property (read only property)
    XN_LINK_PROP_ID_SERIAL_NUMBER				= 0x0007, //General property, holds XnLinkSerialNumber (read only property)
    XN_LINK_PROP_ID_RESERVED					= 0x0008,
	XN_LINK_PROP_ID_COMPONENT_VERSIONS			= 0x0009, //General property, holds XnLinkComponentVersionsList (read only property)
	XN_LINK_PROP_ID_BOOT_STATUS					= 0x000A, //General property, holds XnLinkBootStatus (read only property)
	XN_LINK_PROP_ID_VDD_STATUS			    	= 0x000B,
	//System management properties - group 0x02 (All of these are global, not related to specific stream)
	XN_LINK_PROP_ID_SUPPORTED_BIST_TESTS		= 0x0201, //General property, holds XnLinkSupportedBistTests
	XN_LINK_PROP_ID_SUPPORTED_I2C_DEVICES       = 0x0202, //General property, holds XnLinkSupportedI2CDevices
	XN_LINK_PROP_ID_SUPPORTED_LOG_FILES         = 0x0203, //General property, holds XnLinkSupportedLogFiles

	//Map generator properties - group 0x05
	XN_LINK_PROP_ID_SUPPORTED_VIDEO_MODES		= 0x0501, //General property, holds XnLinkSupportedVideoModes
	XN_LINK_PROP_ID_VIDEO_MODE					= 0x0502, //General property, holds XnLinkVideoMode
	
	//Stream management properties - group 0x06
	XN_LINK_PROP_ID_STREAM_SUPPORTED_INTERFACES	= 0x0601, //General property, holds XnLinkBitSet with values from XnLinkInterfaceID (read only property)
	XN_LINK_PROP_ID_STREAM_FRAG_LEVEL			= 0x0602, //Int property, holds XnLinkStreamFragLevel

	//HandGenerator properties - group 0x09
	XN_LINK_PROP_ID_HAND_SMOOTHING				= 0x0901, //Real property

	// SKELETON properties - group 0x12
	XN_LINK_PROP_ID_SUPPORTED_SKELETON_JOINTS	= 0x1201, //General property, holds XnLinkBitSet of XnSkeletonJoint values
	XN_LINK_PROP_ID_SUPPORTED_SKELETON_PROFILES	= 0x1202, //General property, holds XnLinkBitSet of XnSkeletonProfile values
	XN_LINK_PROP_ID_NEEDED_CALIBRATION_POSE		= 0x1203, //Int property, holds XnLinkPoseType
	XN_LINK_PROP_ID_ACTIVE_JOINTS				= 0x1204, //General property, holds XnLinkBitSet of XnSkeletonJoint values
	XN_LINK_PROP_ID_SKELETON_SMOOTHING			= 0x1205, //Real
	
	// POSE_DETECTION properties - group 0x13
	XN_LINK_PROP_ID_SUPPORTED_POSES				= 0x1301, //Int property, holds a bit set of XnLinkPoseType

	// Mirror properties - group 0x0E
	XN_LINK_PROP_ID_MIRROR						= 0x0E01, //Int property, 0 = Mirror off, 1 = Mirror on
	
	// CROPPING properties - group 0x10
	XN_LINK_PROP_ID_CROPPING					= 0x1001, //General property, holds XnLinkCropping

	// GAIN properties - group 0x20
	XN_LINK_PROP_ID_GAIN						= 0x2001, //Int property

	// PROJECTOR_MGMT messages - group 0x2B
	XN_LINK_PROP_ID_PROJECTOR_PULSE				= 0x2B01, //General property, holds XnLinkProjectorPulse
	XN_LINK_PROP_ID_PROJECTOR_POWER				= 0x2B02, //Int property
	XN_LINK_PROP_ID_ACC_ENABLED                 = 0x2B03, //Int property ACC, 0 = disabled, 1 = enabled
	XN_LINK_PROP_ID_VDD_ENABLED					= 0x2B04, //Int property vdd sampling , 0 = disabled, 1 = enabled
	XN_LINK_PROP_ID_PROJECTOR_ENABLED			= 0x2B05, //Int property projector, 0 = OFF, 1 = ON
	XN_LINK_PROP_ID_PERIODIC_BIST_ENABLED		= 0x2B06, //Int property voltage sampling , 0 = disabled, 1 = enabled
	XN_LINK_PROP_ID_TEMPERATURE_LIST			= 0x2B07,
	//INVALID
	XN_LINK_PROP_ID_INVALID						= 0xFFFF, //Indicates invalid property ID
} XnLinkPropID;

typedef enum XnLinkFileFlags
{
	XN_LINK_FILE_FLAG_BAD_CRC					= 0x0001,
} XnLinkFileFlags;

typedef enum XnLinkGestureType
{
	XN_LINK_GESTURE_NONE						= 0x0000,
	XN_LINK_GESTURE_RAISE_HAND					= 0x0001,
	XN_LINK_GESTURE_WAVE						= 0x0002,
	XN_LINK_GESTURE_CLICK						= 0x0003,
	XN_LINK_GESTURE_MOVING_HAND					= 0x0004,
} XnLinkGestureType;

typedef enum XnLinkGestureEventType
{
	XN_LINK_GESTURE_EVENT_NONE					= 0x0000,
	XN_LINK_GESTURE_EVENT_RECOGNIZED			= 0x0001,
	XN_LINK_GESTURE_EVENT_PROGRESS				= 0x0002,
	XN_LINK_GESTURE_EVENT_STAGE_COMPLETE		= 0x0003,
	XN_LINK_GESTURE_EVENT_READY_FOR_NEXT_STAGE	= 0x0004,
} XnLinkGestureEventType;

typedef enum XnLinkUserElementType
{
	XN_LINK_USER_ELEMENT_INVALID				= 0x0000,
	XN_LINK_USER_ELEMENT_POSE_DETECTION			= 0x0001,
	XN_LINK_USER_ELEMENT_IN_POSE				= 0x0002,
	XN_LINK_USER_ELEMENT_CALIBRATION			= 0x0003,
	XN_LINK_USER_ELEMENT_TRACKING				= 0x0004,
} XnLinkUserElementType;

typedef enum XnLinkPoseType
{
	XN_LINK_POSE_TYPE_NONE						= 0,
	XN_LINK_POSE_TYPE_PSI						= 1 << 0,
} XnLinkPoseType;

typedef enum XnLinkUsersPixelBLOBFormats
{
	XN_LINK_USERS_PIXELS_UNFORMTED				= 0,
	XN_LINK_USERS_PIXELS_SEQUENCE_LIST			= 1, // Using XnMapSequenceListConverter
} XnLinkUsersPixelBLOBFormats;

typedef enum XnLinkUserStatus
{
	XN_LINK_USER_STATUS_NONE					= 0,
	XN_LINK_USER_STATUS_IN_POSE					= 1 << 0,
	XN_LINK_USER_STATUS_CALIBRATING				= 1 << 1,
	XN_LINK_USER_STATUS_CALIBRATED				= 1 << 2,
	XN_LINK_USER_STATUS_TRACKING				= 1 << 3,
	XN_LINK_USER_STATUS_OUT_OF_SCENE			= 1 << 4,
} XnLinkUserStatus;

typedef enum XnLinkIDSetFormat
{
	XN_LINK_ID_SET_FORMAT_NONE				= 0,
	XN_LINK_ID_SET_FORMAT_BITSET			= 1,
} XnLinkIDSetFormat;

//Log
typedef enum XnLinkLogCommand
{
	XN_LINK_LOG_COMMAND_OPEN     /*clear file when open*/= 0x00,
	XN_LINK_LOG_COMMAND_CLOSE                            = 0x01,
	XN_LINK_LOG_COMMAND_WRITE                            = 0x02,
	XN_LINK_LOG_COMMAND_OPEN_APPEND                      = 0x03,
}XnLinkLogCommand;

//Boot status
typedef enum XnLinkBootZone
{
    XN_LINK_BOOT_FACTORY_ZONE                   = 0x0000,
    XN_LINK_BOOT_UPDATE_ZONE                    = 0x0001,
} XnLinkBootZone;

typedef enum XnLinkBootErrorCode
{
    XN_LINK_BOOT_OK                             = 0x0000,
    XN_LINK_BOOT_BAD_CRC                        = 0x0001,
    XN_LINK_BOOT_UPLOAD_IN_PROGRESS             = 0x0002,
    XN_LINK_BOOT_FW_LOAD_FAILED                 = 0x0003,
} XnLinkBootErrorCode;

#endif // XNLINKDEFS_H
