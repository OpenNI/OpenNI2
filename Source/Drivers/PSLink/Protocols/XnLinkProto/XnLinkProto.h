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
#ifndef XNLINKPROTO_H
#define XNLINKPROTO_H

#include <XnPlatform.h>
#include "XnLinkDefs.h"

#if XN_PLATFORM != XN_PLATFORM_ARC
#pragma pack (push, 1)
#endif


//-----------------------------------------------------------------------
// Packet Structure
//-----------------------------------------------------------------------
#if XN_PLATFORM_IS_LITTLE_ENDIAN
typedef struct XnLinkPacketHeader
{
	XnUInt16 m_nMagic;
	XnUInt16 m_nSize;
	XnUInt16 m_nMsgType;
	XnUInt16 m_nCID;
	XnUInt16 m_nPacketID;
	XnUInt16 m_nStreamID : 14;
	XnUInt16 m_nFragmentation : 2; //The two most significant bits of these 16 bits are the fragmentation
} XnLinkPacketHeader;
#else
typedef struct XnLinkPacketHeader
{
	XnUInt16 m_nMagic;
	XnUInt16 m_nSize;
	XnUInt16 m_nMsgType;
	XnUInt16 m_nCID;
	XnUInt16 m_nPacketID;
	XnUInt16 m_nFragmentation : 2; //The two most significant bits of these 16 bits are the fragmentation
	XnUInt16 m_nStreamID : 14;
} XnLinkPacketHeader;

#endif

typedef struct XnLinkPacket
{
	XnLinkPacketHeader m_packetHeader;
	XnUInt8 m_data[1];
} XnLinkPacket;

typedef struct XnLinkDataHeader
{
	XnUInt32 m_nTimestampLo;
	XnUInt32 m_nTimestampHi;
} XnLinkDataHeader;


//-----------------------------------------------------------------------
// Log Header Structure
//-----------------------------------------------------------------------
typedef struct XnLinkLogParam
{
	XnUInt8 m_ID;       // 0 for normal log, 1,2... for other logType
	XnUInt8 command;    // from XnLinkLogCommand.
						// 0- write data to file  with m_ID,
						// 1- open the file with m_ID and Name logFileName
						// 2- for close file with m_ID and Name logFileName
	XnUInt16 size;		//size of all the message (included the Header and data)
} XnLinkLogParam;


typedef struct XnLinkLogFileParam
{
	XnUInt8 logFileName[XN_LINK_MAX_LOG_FILE_NAME_LENGTH];
}XnLinkLogFileParam;

//-----------------------------------------------------------------------
// Data Elements
//-----------------------------------------------------------------------
typedef struct XnLinkVideoMode
{
	XnUInt16 m_nXRes;	
	XnUInt16 m_nYRes;	
	XnUInt16 m_nFPS;
	XnUInt8 m_nPixelFormat; // from XnLinkPixelFormat
	XnUInt8 m_nCompression; // from XnLinkCompressionType
} XnLinkVideoMode;

typedef struct 
{
	XnUInt32 m_nNumModes;
	XnLinkVideoMode m_supportedVideoModes[1];
} XnLinkSupportedVideoModes;

typedef struct XnLinkShiftToDepthConfig
{
	/** The zero plane distance in depth units. */
	XnUInt16 nZeroPlaneDistance;
	XnUInt16 m_nReserved;
	/** The zero plane pixel size */
	XnFloat fZeroPlanePixelSize;
	/** The distance between the emitter and the Depth Cmos */
	XnFloat fEmitterDCmosDistance;
	/** The maximum possible shift value from this device. */
	XnUInt32 nDeviceMaxShiftValue;
	/** The maximum possible depth from this device (as opposed to a cut-off). */
	XnUInt32 nDeviceMaxDepthValue;

	XnUInt32 nConstShift;
	XnUInt32 nPixelSizeFactor;
	XnUInt32 nParamCoeff;
	XnUInt32 nShiftScale;
	XnUInt16 nDepthMinCutOff;
	XnUInt16 nDepthMaxCutOff;

} XnLinkShiftToDepthConfig;

typedef struct XnLinkSerialNumber
{
	XnChar m_strSerialNumber[XN_LINK_SERIAL_NUMBER_SIZE];
} XnLinkSerialNumber;

typedef struct XnLinkPoint3D
{
	XnFloat m_fX;
	XnFloat m_fY;
	XnFloat m_fZ;
} XnLinkPoint3D;

typedef struct XnLinkBoundingBox3D
{
	XnLinkPoint3D leftBottomNear;
	XnLinkPoint3D rightTopFar;
} XnLinkBoundingBox3D;

typedef struct XnLinkBitSet
{
	XnUInt32 m_nSize; //Size in bytes of encoded data
	XnUInt8 m_aData[1];
} XnLinkBitSet;

typedef struct XnLinkStreamInfo
{
	XnUInt32 m_nStreamType;
	XnChar m_strCreationInfo[XN_LINK_MAX_CREATION_INFO_LENGTH];
	//XnUInt16 m_nStreamID;
	//XnUInt16 m_nReserved;
} XnLinkStreamInfo;

typedef struct XnLinkHandData
{
	XnUInt32 m_nHandID;
	XnLinkPoint3D m_position;
	XnUInt32 m_touchingFOVEdge; // XnDirection values, ILLEGAL means not touching FOV edge
} XnLinkHandData;

typedef struct XnLinkHandsData
{
	XnUInt32 m_nHandsCount;
	XnLinkHandData m_aHands[1];
} XnLinkHandsData;

typedef struct XnLinkGestureRecognizedEventArgs
{
	XnLinkPoint3D m_IDPosition;
	XnLinkPoint3D m_EndPosition;
} XnLinkGestureRecognizedEventArgs;

typedef struct XnLinkGestureProgressEventArgs
{
	XnLinkPoint3D m_position;
	XnFloat m_fProgress;
} XnLinkGestureProgressEventArgs;

typedef struct XnLinkGestureIntermediateStageEventArgs
{
	XnLinkPoint3D m_position;
} XnLinkGestureIntermediateStageEventArgs;

typedef struct XnLinkGestureEventHeader
{
	XnUInt32 m_nGesture;// Taken from XnLinkGestureType
	XnUInt32 m_nGestureEventType;//Taken from XnLinkGestureEventType
} XnLinkGestureEventHeader;

typedef struct XnLinkGestureDataHeader
{
	XnUInt32 m_nEventsCount;
} XnLinkGestureDataHeader;

typedef struct XnLinkUserPoseDetectionElement
{
	XnUInt32 m_nPoseID; // Flags taken from XnLinkPoseType
	XnUInt32 m_nDetectionStatus; // XnPoseDetectionStatus
} XnLinkUserPoseDetectionElement;

typedef struct XnLinkUserInPoseElement
{
	XnUInt32 m_nPoseID; // Flags taken from XnLinkPoseType
} XnLinkUserPoseElement;

typedef struct XnLinkUserCalibrationElement
{
	XnUInt32 m_nCalibrationStatus; // taken from XnCalibrationStatus
} XnLinkUserCalibrationElement;

typedef struct XnLinkUserJointData
{
	XnUInt32 m_nJointID;
	XnLinkPoint3D m_position;
	XnFloat m_fPositionConfidence;
	XnFloat m_afOrientation[9];
	XnFloat m_fOrientationConfidence;
} XnLinkUserJointData;

typedef struct XnLinkUserTrackingElement
{
	XnLinkUserJointData m_aJoints[1];
} XnLinkUserTrackingElement;

typedef struct XnLinkUserDataElementHeader
{
	XnUInt32 m_nElementType;
	XnUInt32 m_nElementSize;
} XnLinkUserDataElementHeader;

typedef struct XnLinkUserDataElement
{
	XnLinkUserDataElementHeader m_header;
	XnUInt8 m_elementData[1];
} XnLinkUserDataElement;

typedef struct XnLinkUserDataHeader
{
	XnUInt32 m_nSize;
	XnUInt32 m_nUserID;
	XnUInt32 m_nUserStatus; // Flags taken from XnLinkUserStatus
	XnLinkPoint3D m_centerOfMass;
	XnUInt32 m_nDataElements;
} XnLinkUserDataHeader;

typedef struct XnLinkUserData
{
	XnLinkUserDataHeader m_header;
	XnLinkUserDataElement m_aElements[1];
} XnLinkUserData;

typedef struct XnLinkUserFrameHeader
{
	XnUInt32	m_nUsersCount;

	XnUInt32	m_nUsersPixelBLOBFormat;
	XnUInt32	m_nUsersPixelBLOBSize;

	XnUInt16	m_nUsersPixelXRes;
	XnUInt16	m_nUsersPixelYRes;
} XnLinkUserFrameHeader;

typedef struct XnLinkUserFrame
{
	XnLinkUserFrameHeader m_header;

	// Using 2 unfixed sized fields, Commented out to avoid direct usage
	//XnUChar			m_UsersPixelsBLOB[1];
	//XnLinkUserData	m_aUsers[1];
} XnLinkUserFrame;

typedef struct XnLinkEESectionHeader
{
	XnUInt32 m_nMagic; //Should be "LOAD"
	XnUInt32 m_nSize;
} XnLinkEESectionHeader;

typedef struct XnLinkIDSetHeader
{
	XnUInt16 m_nFormat;						//Values come from XnLinkIDSetFormat. Currently must be XN_LINK_IDS_LIST_FORMAT_BITSET.
	XnUInt16 m_nNumGroups;
} XnLinkIDSetHeader;

typedef struct XnLinkIDSetGroupHeader
{
	XnUInt8 m_nGroupID;
	XnUInt8 m_nSize;
} XnLinkIDSetGroupHeader;

typedef struct XnLinkIDSetGroup
{
	XnLinkIDSetGroupHeader m_header;
	XnUInt8 m_idsBitmap[1]; //Contains a bit of 1 for every id in the set, 0 for id not in the set
} XnLinkIDSetGroup;

typedef struct XnLinkCropping
{
	/** TRUE if cropping is turned on, FALSE otherwise. */
	XnUInt8 m_bEnabled;

	XnUInt8 m_nReserved1;
	XnUInt8 m_nReserved2;
	XnUInt8 m_nReserved3;

	/** Offset in the X-axis, in pixels. */
	XnUInt16 m_nXOffset;
	/** Offset in the Y-axis, in pixels. */
	XnUInt16 m_nYOffset;
	/** Number of pixels in the X-axis. */
	XnUInt16 m_nXSize;
	/** Number of pixels in the Y-axis. */
	XnUInt16 m_nYSize;
} XnLinkCropping;

typedef struct XnLinkStreamIDsList
{
    XnUInt16 m_nNumStreamIDs;
    XnUInt16 m_anStreamIDs[1];
} XnLinkStreamIDsList;

typedef struct XnLinkStreamIDsList XnLinkFrameSyncStreamIDs;

typedef struct XnLinkPropValHeader
{
	XnUInt16 m_nPropType;	//Values come from XnLinkPropType
	XnUInt16 m_nPropID;		//Values come from XnLinkInternalPropID
	XnUInt32 m_nValueSize;
} XnLinkPropValHeader;

typedef struct XnLinkPropVal
{
	XnLinkPropValHeader m_header;
	XnUInt8 m_value[1];
} XnLinkPropVal;

typedef struct XnLinkPropSet
{
    XnUInt32 m_nNumProps;
    //Followed by a number of XnLinkPropVal's
    XnUInt8 m_aData[1];
} XnLinkPropSet;

typedef struct XnLinkBistTest
{
	XnUInt32 m_nID;
	XnChar m_strName[XN_LINK_MAX_BIST_NAME_LENGTH];
} XnLinkBistTest;

typedef struct XnLinkSupportedBistTests
{
	XnUInt32 m_nCount;
	XnLinkBistTest m_aTests[1];
} XnLinkSupportedBistTests;

typedef struct XnLinkUploadFileHeader
{
	XnBool m_bOverrideFactorySettings;
} XnLinkUploadFileHeader;

typedef struct XnLinkLeanVersion
{
	XnUInt8 m_nMajor;
	XnUInt8 m_nMinor;
	XnUInt16 m_nReserved;
} XnLinkLeanVersion;

typedef struct XnLinkDetailedVersion
{
	XnUInt8 m_nMajor;
	XnUInt8 m_nMinor;
	XnUInt16 m_nMaintenance;
	XnUInt32 m_nBuild;
	XnChar m_strModifier[XN_LINK_MAX_VERSION_MODIFIER_LENGTH];
} XnLinkDetailedVersion;

typedef struct XnLinkFileVersion
{
	XnUInt8 m_nMajor;
	XnUInt8 m_nMinor;
	XnUInt8 m_nMaintenance;
	XnUInt8 m_nBuild;
} XnLinkFileVersion;

typedef struct XnLinkFileEntry
{
	XnChar m_strName[XN_LINK_MAX_FILE_NAME_LENGTH];
	XnLinkFileVersion m_nVersion;
	XnUInt32 m_nAddress;
	XnUInt32 m_nSize;
	XnUInt16 m_nCRC;
	XnUInt16 m_nZone;
	XnUInt8 m_nFlags; // bitmap of values from XnLinkFileFlags
	XnUInt8 m_nReserved1;
	XnUInt8 m_nReserved2;
	XnUInt8 m_nReserved3;
} XnLinkFileEntry;

typedef struct XnLinkComponentVersion
{
	XnChar m_strName[XN_LINK_MAX_COMPONENT_NAME_LENGTH];
	XnChar m_strVersion[XN_LINK_MAX_VERSION_LENGTH];
} XnLinkComponentVersion;

typedef struct XnLinkComponentVersionsList
{
	XnUInt32 m_nCount;
	XnLinkComponentVersion m_components[1];
} XnLinkComponentVersionsList;


typedef struct XnLinkAccCurentParam
{
	XnFloat m_nTemperature;
	XnUInt32 m_nLutTabLine;
	XnUInt16 m_nValueDC;
	XnUInt16 m_nValueDac;
	XnUInt16 m_nVoltage1;
	XnUInt16 m_nVoltage2;
} XnLinkAccCurentParam;

typedef struct XnLinkDCParam
{
	XnUInt32 m_nDCvalue;
} XnLinkDCParam ;

typedef struct XnLinkCameraIntrinsics
{
	XnUInt16 m_nOpticalCenterX;
	XnUInt16 m_nOpticalCenterY;
	XnFloat m_fEffectiveFocalLengthInPixels;
} XnLinkCameraIntrinsics;

typedef struct XnLinkI2CDevice
{
	XnUInt8 m_nMasterID;
	XnUInt8 m_nSlaveID;
	XnUInt16 m_nReserved;
	XnUInt32 m_nID;
	XnChar m_strName[XN_LINK_MAX_I2C_DEVICE_NAME_LENGTH];
} XnLinkI2CDevice;

typedef struct XnLinkSupportedI2CDevices
{
	XnUInt32 m_nCount;
	XnLinkI2CDevice m_aI2CDevices[1];
} XnLinkSupportedI2CDevices;

typedef struct XnLinkLogFile
{
	XnUInt8 m_nID;
	XnChar m_strName[XN_LINK_MAX_LOG_FILE_NAME_LENGTH];
} XnLinkLogFile;

typedef struct XnLinkSupportedLogFiles
{
	XnUInt32 m_nCount;
	XnLinkLogFile m_aLogFiles[1];
} XnLinkSupportedLogFiles;

typedef struct XnLinkProjectorPulse
{
	XnUInt16 m_bEnabled;
	XnUInt16 m_nReserved;
	XnFloat  m_nDelay; // Delay between frame start and the start of pulse, in milliseconds
	XnFloat  m_nWidth; // Pulse width, in milliseconds
	XnFloat  m_nCycle; // in pulse mode: number of frames to skip between projector pulses, from pulse start to next pulse start. in PWM : Cycle time
} XnLinkProjectorPulse;

typedef struct XnLinkTemperatureSensor
{
	XnUInt32 m_nID;
	XnUInt8 m_strName[XN_LINK_MAX_SENSOR_NAME_LENGTH];
} XnLinkTemperatureSensor;

typedef struct XnLinkTemperatureSensorsList{
 	XnUInt32 m_nCount;
 	XnLinkTemperatureSensor m_aSensors[XN_LINK_MAX_TEMPERATURE_SENSORS];
}XnLinkTemperatureSensorsList;
//-----------------------------------------------------------------------
// Command Parameters 
//-----------------------------------------------------------------------
typedef struct XnLinkDownloadFileParams
{
	XnUInt16 m_nZone;
	XnUInt16 m_nReserved1;
	XnChar m_strName[XN_LINK_MAX_FILE_NAME_LENGTH];
} XnLinkDownloadFileParams;

typedef struct XnLinkContinueReponseParams
{
	XnUInt16 m_nOriginalMsgType;
} XnLinkContinueReponseParams;

typedef struct XnLinkWriteI2CParams
{
	XnUInt8 m_nDeviceID;
	XnUInt8 m_nAddressSize;
	XnUInt8 m_nValueSize;
	XnUInt8 m_nReserved;
    XnUInt32 m_nAddress;
    XnUInt32 m_nValue;
    XnUInt32 m_nMask;
} XnLinkWriteI2CParams;

typedef struct XnLinkReadI2CParams
{
	XnUInt8 m_nDeviceID;
	XnUInt8 m_nAddressSize;
	XnUInt8 m_nValueSize;
	XnUInt8 m_nReserved;
    XnUInt32 m_nAddress;
} XnLinkReadI2CParams;

typedef struct XnLinkWriteAHBParams
{
	XnUInt32 m_nAddress;
	XnUInt32 m_nValue;
	XnUInt8 m_nBitOffset; //Offset in bits of value to write within address
	XnUInt8 m_nBitWidth; //Width in bits of value to write
	XnUInt16 m_nReserved;
} XnLinkWriteAHBParams;

typedef struct XnLinkReadAHBParams
{
	XnUInt32 m_nAddress;
	XnUInt8 m_nBitOffset; //Offset in bits of value to read within address
	XnUInt8 m_nBitWidth; //Width in bits of value to read
	XnUInt16 m_nReserved;
} XnLinkReadAHBParams;

typedef struct XnLinkStreamIDsList XnLinkStartStreamingMultiParams;
typedef struct XnLinkStreamIDsList XnLinkStopStreamingMultiParams;

typedef struct XnLinkSetVideoModeParams
{
	XnLinkVideoMode m_videoMode;
} XnLinkSetVideoModeParams;

//typedef struct XnLinkEnumerateStreamsParams
//{
//	XnUInt32 m_nStreamType;
//} XnLinkEnumerateStreamsParams;

typedef struct XnLinkCreateStreamParams
{
	XnUInt32 m_nStreamType;
	XnChar m_strCreationInfo[XN_LINK_MAX_CREATION_INFO_LENGTH];
} XnLinkCreateStreamParams;

typedef XnLinkPropVal XnLinkSetPropParams;

typedef struct XnLinkGetPropParams
{
	XnUInt16 m_nPropType;	//Values come from XnLinkPropType
	XnUInt16 m_nPropID;		//Values come from XnLinkInternalPropID
} XnLinkGetPropParams;

typedef struct XnLinkSetMultiPropsParams
{
	XnUInt32 m_nNumProps;
	//Followed by a number of XnLinkSetPropParams
	XnUInt8 m_aData[1];
} XnLinkSetMultiPropsParams;

typedef struct XnLinkStartTrackingHandParams
{
	XnLinkPoint3D m_ptPosition;
} XnLinkStartTrackingHandParams;

typedef struct XnLinkStopTrackingHandParams
{
	XnUInt32 m_nUserID;
} XnLinkStopTrackingHandParams;

typedef struct XnLinkAddGestureParams
{
	XnUInt32 m_nGestureType; //Values come from XnLinkGestureType
	XnLinkBoundingBox3D m_boundingBox;
} XnLinkAddGestureParams;

typedef struct XnLinkRemoveGestureParams
{
	XnUInt32 m_nGestureType; //Values come from XnLinkGestureType
} XnLinkRemoveGestureParams;

typedef struct XnLinkExecuteBistParams
{
	XnUInt32 m_nID;
} XnLinkExecuteBistParams;

typedef struct XnLinkFormatZoneParams
{
    XnUInt32 m_nZone; //0 or 1
} XnLinkFormatZoneParams;

typedef struct XnLinkLogOpenCloseParams
{
	XnUInt8 m_nID;
} XnLinkLogOpenCloseParams;

typedef struct XnLinkGetTemperatureParams
{
	XnUInt32 m_nID;
} XnLinkGetTemperatureParams;

typedef struct XnLinkGetDebugDataParams
{
	XnUInt32 m_nID;
} XnLinkGetDebugDataParams;

//-----------------------------------------------------------------------
// Command Response Structures 
//-----------------------------------------------------------------------
typedef struct XnLinkTemperatureResponse
{
	XnUInt32 m_nID;
	XnFloat value;
} XnLinkTemperatureResponse;

typedef struct XnLinkResponseInfo
{
	XnUInt16 m_nResponseCode;
	XnUInt16 m_nReserverd;
} XnLinkResponseInfo;

typedef struct XnLinkResponseHeader
{
	XnLinkPacketHeader m_header;
	XnLinkResponseInfo m_responseInfo;
} XnLinkResponseHeader;

typedef struct XnLinkResponsePacket
{
	XnLinkResponseHeader m_responseHeader;
	XnUInt8 m_data[1];
} XnLinkResponsePacket;

typedef struct XnLinkReadI2CResponse
{
	XnUInt32 m_nValue;
} XnLinkReadI2CResponse;

typedef struct XnLinkReadAHBResponse
{
	XnUInt32 m_nValue;
} XnLinkReadAHBResponse;

typedef XnLinkSupportedVideoModes XnLinkGetSupportedVideoModesResponse;

typedef struct XnLinkGetVideoModeResponse
{
	XnLinkVideoMode m_videoMode;
} XnLinkGetVideoModeResponse;

typedef struct XnLinkEnumerateStreamsResponse
{
	XnUInt32 m_nNumStreams;
	XnLinkStreamInfo m_streamInfos[1];
} XnLinkEnumerateStreamsResponse;

typedef struct XnLinkCreateStreamResponse
{
	XnUInt16 m_nStreamID;
	XnUInt16 m_nEndpointID;
} XnLinkCreateStreamResponse;

typedef struct XnLinkPropVal XnLinkGetPropResponse;

typedef struct XnLinkPropSet XnLinkGetAllPropsResponse;

typedef struct XnLinkGetShiftToDepthConfigResponse
{
	XnLinkShiftToDepthConfig m_config;
} XnLinkGetShiftToDepthConfigResponse;

typedef struct XnLinkEnumerateAvailableGesturesResponse
{
	XnUInt32 m_nGestures;
	XnUInt32 m_nProgressSupported;
	XnUInt32 m_nCurrentlyActive;
} XnLinkEnumerateAvailableGesturesResponse;

typedef struct XnLinkEnumerateActiveGesturesResponse
{
	XnUInt32 m_nGestures;
} XnLinkEnumerateActiveGesturesResponse;

typedef struct XnLinkSetSkeletonProfileParams
{
	XnUInt32 m_nProfile;
} XnLinkSetSkeletonProfileParams;

typedef struct XnLinkSetSkeletonJointStateParams
{
	XnUInt16 m_nJoint;
	XnUInt16 m_nState;
} XnLinkSetSkeletonJointStateParams;

typedef struct XnLinkRequestSkeletonCalibrationParams
{
	XnUInt32 m_nUserID;
	XnUInt32 m_bForce;
} XnLinkRequestSkeletonCalibrationParams;

typedef struct XnLinkAbortSkeletonCalibrationParams
{
	XnUInt32 m_nUserID;
} XnLinkAbortSkeletonCalibrationParams;

typedef struct XnLinkStartSkeletonTrackingParams
{
	XnUInt32 m_nUserID;
} XnLinkStartSkeletonTrackingParams;

typedef struct XnLinkStopSkeletonTrackingParams
{
	XnUInt32 m_nUserID;
} XnLinkStopSkeletonTrackingParams;

typedef struct XnLinkResetSkeletonTrackingParams
{
	XnUInt32 m_nUserID;
} XnLinkResetSkeletonTrackingParams;

typedef struct XnLinkStartPoseDetectionParams
{
	XnUInt32 m_nUserID;
	XnUInt32 m_nPose;
} XnLinkStartPoseDetectionParams;

typedef struct XnLinkStopPoseDetectionParams
{
	XnUInt32 m_nUserID;
} XnLinkStopPoseDetectionParams;

typedef struct XnLinkSaveSkeletonCalibrationDataParams
{
	XnUInt32 m_nUserID;
	XnUInt32 m_nSlot;
} XnLinkSaveSkeletonCalibrationDataParams;

typedef struct XnLinkLoadSkeletonCalibrationDataParams
{
	XnUInt32 m_nUserID;
	XnUInt32 m_nSlot;
} XnLinkLoadSkeletonCalibrationDataParams;

typedef struct XnLinkIsSkeletonCalibrationSlotTakenParams
{
	XnUInt32 m_nSlot;
} XnLinkIsSkeletonCalibrationSlotTakenParams;

typedef struct XnLinkIsSkeletonCalibrationSlotTakenResponse
{
	XnUInt32 m_nTaken;
} XnLinkIsSkeletonCalibrationSlotTakenResponse;

typedef struct XnLinkClearSkeletonCalibrationSlotParams
{
	XnUInt32 m_nSlot;
} XnLinkClearSkeletonCalibrationSlotParams;

typedef struct XnLinkGetSkeletonCalibrationDataParams
{
	XnUInt32 m_nUserID;
} XnLinkGetSkeletonCalibrationDataParams;

typedef struct XnLinkSetSkeletonCalibrationDataParamsHeader
{
	XnUInt32 m_nUserID;
	XnUInt32 m_nDataSize;
} XnLinkSetSkeletonCalibrationDataParamsHeader;

typedef struct XnLinkSetSkeletonCalibrationDataParams
{
	XnLinkSetSkeletonCalibrationDataParamsHeader m_header;
	XnUInt8 m_aData[1];
} XnLinkSetSkeletonCalibrationDataParams;

typedef struct XnLinkSetLogMaskSeverityParams
{
	XnChar m_strMask[XN_LINK_MAX_LOG_MASK_LENGTH];
	XnUInt32 m_nMinSeverity; // values from XnLogSeverity
} XnLinkSetLogMaskSeverityParams;

typedef struct XnLinkGetLogMaskSeverityParams
{
	XnChar m_strMask[XN_LINK_MAX_LOG_MASK_LENGTH];
} XnLinkGetLogMaskSeverityParams;

typedef struct XnLinkGetLogMaskSeverityResponse
{
	XnUInt32 m_nMinSeverity; // values from XnLogSeverity
} XnLinkGetLogMaskSeverityResponse;

typedef struct XnLinkExecuteBistResponse // Entire data in this struct is system- and test-specific
{
	XnUInt32 m_nErrorCode; // 0 for success
	XnUInt32 m_nExtraDataSize;
	XnUChar m_ExtraData[1];
} XnLinkExecuteBistResponse;

typedef struct XnLinkGetFileListResponse
{
	XnUInt32 m_nCount;
	XnLinkFileEntry m_aFileEntries[1];
} XnLinkGetFileListResponse;

typedef struct XnLinkDebugDataResponseHeader
{
    XnUInt16 m_nDataID;		//Values come from XnLinkInternalPropID
    XnUInt16 m_nValueSize;
} XnLinkDebugDataResponseHeader;

typedef struct XnLinkDebugDataResponse
{
    XnLinkDebugDataResponseHeader m_header;
    XnUInt8 m_data[1];
} XnLinkDebugDataResponse;

typedef struct XnLinkBootStatus
{
    XnUInt8  m_nZone;		    //Values come from XnLinkBootZone
    XnUInt8  m_nErrorCode;	    //Values come from XnLinkBootErrorCode
    XnUInt16 m_nReserved;
} XnLinkBootStatus;

//-----------------------------------------------------------------------
// Device Notifications
//-----------------------------------------------------------------------

#if XN_PLATFORM != XN_PLATFORM_ARC
#pragma pack (pop)
#endif

#endif // XNLINKPROTO_H
