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
#include "XnLinkProtoUtils.h"
#include "XnLinkProto.h"
#include "XnLinkDefs.h"
#include "XnLinkStatusCodes.h"
#include <XnBitSet.h>
#include <XnOS.h>
#include <XnLog.h>
#include <XnArray.h>

XnStatus xn::LinkPacketHeader::Validate(XnUInt32 nBytesToRead) const
{
	//First of all validate minimum header size
	if (nBytesToRead < sizeof(LinkPacketHeader))
	{
		xnLogError(XN_MASK_LINK, "Not enough data left to read - got only %u bytes, but link packet header is %u bytes", nBytesToRead, sizeof(LinkPacketHeader));
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_HEADER_SIZE;
	}

	//Validate Magic
	if (!IsMagicValid())
	{
		XnChar strData[256] = "";
		for (XnUInt32 i = 0; i < XN_MIN(nBytesToRead, 10); ++i)
		{
			XnChar s[10];
			sprintf(s, "%02X ", ((XnUInt8*)this)[i]);
			xnOSStrAppend(strData, s, sizeof(strData));
		}
		xnLogError(XN_MASK_LINK, "Got bad packet magic. size: %u. Beginning of packet data was: %s", nBytesToRead, strData);
//		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_MAGIC;
	}

	//Validate packet size
	if (nBytesToRead < GetSize())
	{
		xnLogError(XN_MASK_LINK, "Got partial packet - only %u bytes remaining", nBytesToRead);
//		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_PARTIAL_PACKET;
	}

	//We passed all validations :)
	return XN_STATUS_OK;
}

XnStatus xnLinkResponseCodeToStatus(XnUInt16 nResponseCode)
{
	switch (nResponseCode)
	{
		case XN_LINK_RESPONSE_OK:
			return XN_STATUS_OK;
		case XN_LINK_RESPONSE_PENDING:
			return XN_STATUS_LINK_RESP_PENDING;
		case XN_LINK_RESPONSE_BAD_FILE_TYPE:
			return XN_STATUS_LINK_RESP_BAD_FILE_TYPE;
		case XN_LINK_RESPONSE_CMD_ERROR:
			return XN_STATUS_LINK_RESP_CMD_ERROR;
		case XN_LINK_RESPONSE_CMD_NOT_SUPPORTED:
			return XN_STATUS_LINK_RESP_CMD_NOT_SUPPORTED;
		case XN_LINK_RESPONSE_BAD_CMD_SIZE:
			return XN_STATUS_LINK_RESP_BAD_CMD_SIZE;
		case XN_LINK_RESPONSE_BAD_PARAMETERS:
			return XN_STATUS_LINK_RESP_BAD_PARAMETERS;
		case XN_LINK_RESPONSE_CORRUPT_PACKET:
			return XN_STATUS_LINK_RESP_CORRUPT_PACKET;
		case XN_LINK_RESPONSE_FILE_CORRUPT:
			return XN_STATUS_LINK_RESP_CORRUPT_FILE;
		case XN_LINK_RESPONSE_BAD_CRC:
			return XN_STATUS_LINK_RESP_BAD_CRC;
		case XN_LINK_RESPONSE_INCORRECT_SIZE:
			return XN_STATUS_LINK_RESP_INCORRECT_SIZE;
		case XN_LINK_RESPONSE_INPUT_BUFFER_OVERFLOW:
			return XN_STATUS_LINK_RESP_INPUT_BUFFER_OVERFLOW;
		default:
			return XN_STATUS_LINK_RESP_UNKNOWN;
	}
}

const XnChar* xnLinkResponseCodeToStr(XnUInt16 nResponseCode)
{
	return xnGetStatusString(xnLinkResponseCodeToStatus(nResponseCode));
}

const XnChar* xnFragmentationFlagsToStr(XnLinkFragmentation fragmentation)
{
	switch (fragmentation)
	{
		case XN_LINK_FRAG_MIDDLE:
			return "MIDDLE";
		case XN_LINK_FRAG_BEGIN:
			return "BEGIN";
		case XN_LINK_FRAG_END:
			return "END";
		case XN_LINK_FRAG_SINGLE:
			return "SINGLE";
		default:
			XN_ASSERT(FALSE);
			return NULL;
	}
}

const XnChar* xnLinkStreamTypeToString(XnStreamType streamType)
{
	switch (streamType)
	{
	case XN_LINK_STREAM_TYPE_COLOR:
		return "Image";
	case XN_LINK_STREAM_TYPE_IR:
		return "IR";
	case XN_LINK_STREAM_TYPE_SHIFTS:
		return "Depth";
	case XN_LINK_STREAM_TYPE_AUDIO:
		return "Audio";
	case XN_LINK_STREAM_TYPE_LOG:
		return "Log";
	case XN_LINK_STREAM_TYPE_USER:
		return "User";
	case XN_LINK_STREAM_TYPE_HANDS:
		return "Hands";
	case XN_LINK_STREAM_TYPE_GESTURES:
		return "Gestures";
	case XN_LINK_STREAM_TYPE_DY:
		return "DY";
	default:
		return "Unknown";
	}
}

XnStreamType xnLinkStreamTypeFromString(const XnChar* strType)
{
	if (xnOSStrCaseCmp(strType, "Image") == 0)
	{
		return XN_LINK_STREAM_TYPE_COLOR;
	}
	else if (xnOSStrCaseCmp(strType, "IR") == 0)
	{
		return XN_LINK_STREAM_TYPE_IR;
	}
	else if (xnOSStrCaseCmp(strType, "Depth") == 0)
	{
		return XN_LINK_STREAM_TYPE_SHIFTS;
	}
	else if (xnOSStrCaseCmp(strType, "Audio") == 0)
	{
		return XN_LINK_STREAM_TYPE_AUDIO;
	}
	else if (xnOSStrCaseCmp(strType, "Log") == 0)
	{
		return XN_LINK_STREAM_TYPE_LOG;
	}
	else if (xnOSStrCaseCmp(strType, "User") == 0)
	{
		return XN_LINK_STREAM_TYPE_USER;
	}
	else if (xnOSStrCaseCmp(strType, "Hands") == 0)
	{
		return XN_LINK_STREAM_TYPE_HANDS;
	}
	else if (xnOSStrCaseCmp(strType, "Gestures") == 0)
	{
		return XN_LINK_STREAM_TYPE_GESTURES;
	}
	else if (xnOSStrCaseCmp(strType, "DY") == 0)
	{
		return XN_LINK_STREAM_TYPE_DY;
	}
	else
	{
		return XN_LINK_STREAM_TYPE_INVALID;
	}
}

#define XN_GESTURE_NAME_RAISE_HAND "RaiseHand"
#define XN_GESTURE_NAME_WAVE "Wave"
#define XN_GESTURE_NAME_CLICK "Click"
#define XN_GESTURE_NAME_MOVING_HAND "MovingHand"

const XnChar* xnLinkGestureTypeToName(XnUInt32 gestureType)
{
	switch (gestureType)
	{
	case XN_LINK_GESTURE_RAISE_HAND:
		return XN_GESTURE_NAME_RAISE_HAND;
	case XN_LINK_GESTURE_WAVE:
		return XN_GESTURE_NAME_WAVE;
	case XN_LINK_GESTURE_CLICK:
		return XN_GESTURE_NAME_CLICK;
	case XN_LINK_GESTURE_MOVING_HAND:
		return XN_GESTURE_NAME_MOVING_HAND;
	default:
		xnLogError(XN_MASK_LINK, "Unknown gesture: %d", gestureType);
		XN_ASSERT(FALSE);
		return NULL;
	}
}

XnUInt32 xnLinkGestureNameToType(const XnChar* strGesture)
{
	if (strcmp(strGesture, XN_GESTURE_NAME_RAISE_HAND) == 0)
		return XN_LINK_GESTURE_RAISE_HAND;
	else if (strcmp(strGesture, XN_GESTURE_NAME_WAVE) == 0)
		return XN_LINK_GESTURE_WAVE;
	else if (strcmp(strGesture, XN_GESTURE_NAME_CLICK) == 0)
		return XN_LINK_GESTURE_CLICK;
	else if (strcmp(strGesture, XN_GESTURE_NAME_MOVING_HAND) == 0)
		return XN_LINK_GESTURE_MOVING_HAND;

	xnLogError(XN_MASK_LINK, "Unknown gesture: %s", strGesture);
	XN_ASSERT(FALSE); 
	return XN_PREPARE_VAR32_IN_BUFFER(XN_LINK_GESTURE_NONE);
}

#define XN_POSE_NAME_PSI "Psi"

const XnChar* xnLinkPoseTypeToName(XnUInt32 poseType)
{
	switch (poseType)
	{
	case XN_LINK_POSE_TYPE_PSI:
		return XN_POSE_NAME_PSI;
	case XN_LINK_POSE_TYPE_NONE:
		return NULL;
	default:
		xnLogError(XN_MASK_LINK, "Unknown pose: %d", poseType);
		XN_ASSERT(FALSE);
		return NULL;
	}
}

XnUInt32 xnLinkPoseNameToType(const XnChar* strPose)
{
	if (strPose == NULL)
		return XN_LINK_POSE_TYPE_NONE;
	else if (strcmp(strPose, XN_POSE_NAME_PSI) == 0)
		return XN_LINK_POSE_TYPE_PSI;

	xnLogError(XN_MASK_LINK, "Unknown pose: %s", strPose);
	XN_ASSERT(FALSE); 
	return XN_LINK_GESTURE_NONE;
}

XnStatus xnLinkPosesToNames(XnUInt32 nPoses, xnl::Array<const XnChar*>& aPosesNames)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	aPosesNames.Clear();

	XnUInt32 shifted = 0;
	while (nPoses != 0)
	{
		if ((nPoses & 0x01) != 0)
		{
			const XnChar* strPose = xnLinkPoseTypeToName(1 << shifted);
			if (strPose == NULL)
			{
				return XN_STATUS_LINK_UNKNOWN_POSE;
			}

			nRetVal = aPosesNames.AddLast(strPose);
			XN_IS_STATUS_OK(nRetVal);
		}

		shifted++;
		nPoses >>= 1;
	}

	return (XN_STATUS_OK);
}

xnl::Point3D xnLinkPoint3DToPoint3D(const XnLinkPoint3D& point)
{
	xnl::Point3D result;
	result.x = XN_PREPARE_VAR_FLOAT_IN_BUFFER(point.m_fX);
	result.y = XN_PREPARE_VAR_FLOAT_IN_BUFFER(point.m_fY);
	result.z = XN_PREPARE_VAR_FLOAT_IN_BUFFER(point.m_fZ);
	return result;
}

XnLinkPoint3D XnPoint3DToLinkPoint3D(const xnl::Point3D& point)
{
	XnLinkPoint3D result;
	result.m_fX = XN_PREPARE_VAR_FLOAT_IN_BUFFER(point.x);
	result.m_fY = XN_PREPARE_VAR_FLOAT_IN_BUFFER(point.y);
	result.m_fZ = XN_PREPARE_VAR_FLOAT_IN_BUFFER(point.z);
	return result;
}

XnLinkBoundingBox3D xnBoundingBox3DToLinkBoundingBox3D(const xnl::Box3D& box)
{
	XnLinkBoundingBox3D result;
	result.leftBottomNear = XnPoint3DToLinkPoint3D(box.m_bottomLeftNear);
	result.rightTopFar = XnPoint3DToLinkPoint3D(box.m_topRightFar);
	return result;
}

xnl::Box3D xnLinkBoundingBox3DToBoundingBox3D(const XnLinkBoundingBox3D& box)
{
	xnl::Box3D result;

	result.m_bottomLeftNear = xnLinkPoint3DToPoint3D(box.leftBottomNear);
	result.m_topRightFar = xnLinkPoint3DToPoint3D(box.rightTopFar);

	return result;
}

XnStatus xnLinkGetStreamDumpName(XnUInt16 nStreamID, XnChar* strDumpName, XnUInt32 nDumpNameSize)
{
	XnUInt32 nCharsWritten = 0;
	return xnOSStrFormat(strDumpName, nDumpNameSize, &nCharsWritten, "Stream.%05u.In.raw", nStreamID);
}

XnStatus xnLinkGetEPDumpName(XnUInt16 nEPID, XnChar* strDumpName, XnUInt32 nDumpNameSize)
{
    XnUInt32 nCharsWritten = 0;
    return xnOSStrFormat(strDumpName, nDumpNameSize, &nCharsWritten, "EP.%05u.In", nEPID);
}

XnStatus xnLinkParseIDSet(xnl::Array<xnl::BitSet>& idSet, const void* pLinkIDSet, XnUInt32 nSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	const XnUInt8* pSource = reinterpret_cast<const XnUInt8*>(pLinkIDSet);
	const XnUInt8* pNextSource = NULL;
	const XnUInt8* pSourceEnd = pSource + nSize;
	const XnLinkIDSetHeader* pHeader = reinterpret_cast<const XnLinkIDSetHeader*>(pSource);
	const XnLinkIDSetGroup* pLinkIDBitSet = NULL;
	XnUInt8 nGroupID = 0;
	XnUInt32 nGroupSize = 0;

	if (nSize < sizeof(*pHeader))
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_INPUT_BUFFER_OVERFLOW;
	}

	if (pHeader->m_nFormat != XN_PREPARE_VAR32_IN_BUFFER(XN_LINK_ID_SET_FORMAT_BITSET))
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	pSource += sizeof(*pHeader);

	while (pSource < pSourceEnd)
	{
		pLinkIDBitSet = reinterpret_cast<const XnLinkIDSetGroup*>(pSource);
		nGroupID = pLinkIDBitSet->m_header.m_nGroupID;
		nRetVal = idSet.SetMinSize(nGroupID + 1);
		XN_IS_STATUS_OK(nRetVal);
		nGroupSize = pLinkIDBitSet->m_header.m_nSize;
		pNextSource = pSource + nGroupSize;
		if (pNextSource > pSourceEnd)
		{
			XN_ASSERT(FALSE);
			return XN_STATUS_INPUT_BUFFER_OVERFLOW;
		}
		nRetVal = idSet[nGroupID].SetData(pLinkIDBitSet->m_idsBitmap, nGroupSize - sizeof(pLinkIDBitSet->m_header));
		XN_IS_STATUS_OK(nRetVal);
		pSource = pNextSource;
	}

	return XN_STATUS_OK;
}

XnStatus xnLinkEncodeIDSet(void* pIDSet, XnUInt32 *pnEncodedSize, const XnUInt16* pIDs, XnUInt32 nNumIDs)
{
	XnUInt8 nGroupID = 0xFF;
	XnUInt8 nNewGroupID = 0xFF;
	const XnUInt16* pMsgType = pIDs;
	const XnUInt16* pMsgTypesEnd = pIDs + nNumIDs;
	XnUInt32 nMaxEncodedSize = *pnEncodedSize;
	XnUInt32 nMsgTypeLow = 0;
	XnUInt32 nByteIndex = 0;
	XnLinkIDSetHeader* pHeader = (XnLinkIDSetHeader*)pIDSet;
	XnLinkIDSetGroup* pIDSetGroup = (XnLinkIDSetGroup*)((XnUInt8*)pIDSet + sizeof(*pHeader));
	XnUInt8* pIDSetEnd = (XnUInt8*)pIDSet + nMaxEncodedSize;
	XnUInt8 nGroupSize = 0;
	XnUInt16 nNumGroups = 0;

	if (nMaxEncodedSize < sizeof(*pHeader))
	{
		//Not enough room to encode header
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	//First reset entire output buffer
	memset(pIDSet, 0, nMaxEncodedSize);

	pHeader->m_nFormat = XN_PREPARE_VAR16_IN_BUFFER(XN_LINK_ID_SET_FORMAT_BITSET);

	while (pMsgType < pMsgTypesEnd)
	{
		nNewGroupID = (*pMsgType >> 8);
		//We already know the group number, now take only the low 8 bits of msg type
		nMsgTypeLow = (*pMsgType & 0xFF);
		nByteIndex = (nMsgTypeLow >> 3);

		if (nNewGroupID != nGroupID)
		{
			//Encountered a new group ID
			nNumGroups++;
			//Advance group pointer to next group
			pIDSetGroup = (XnLinkIDSetGroup*)((XnUInt8*)pIDSetGroup + pIDSetGroup->m_header.m_nSize);
			if (((XnUInt8*)pIDSetGroup + sizeof(pIDSetGroup->m_header)) > pIDSetEnd)
			{
				//Not enough room in output buffer to encode this group's header
				return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
			}
			pIDSetGroup->m_header.m_nGroupID = nNewGroupID;
			pIDSetGroup->m_header.m_nSize = sizeof(pIDSetGroup->m_header); //Size starts with size of header
			nGroupID = nNewGroupID;
		}

		if (((XnUInt8*)pIDSetGroup + sizeof(pIDSetGroup->m_header) + nByteIndex) > pIDSetEnd)
		{
			//Not enough room to add a bit for this msg type
			return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
		}

		nGroupSize = (sizeof(pIDSetGroup->m_header) + (XnUInt8)nByteIndex + 1);
		if (nGroupSize > pIDSetGroup->m_header.m_nSize)
		{
			//Update size of this group
			pIDSetGroup->m_header.m_nSize = nGroupSize;
		}

		//We OR the value in the array with a mask that contains a 1 bit only for this msg type.
		pIDSetGroup->m_idsBitmap[nByteIndex] |= (1 << ((~nMsgTypeLow) & 0x07));
		pMsgType++;
	}

	pHeader->m_nNumGroups = XN_PREPARE_VAR16_IN_BUFFER(nNumGroups);
	*pnEncodedSize = XnUInt32((XnUInt8*)pIDSetGroup + pIDSetGroup->m_header.m_nSize - (XnUInt8*)pIDSet);

	return XN_STATUS_OK; //Success
}

XnStatus xnLinkParseFrameSyncStreamIDs(xnl::Array<XnUInt16>& frameSyncStreamIDs, const void* pFrameSyncStreamIDs, XnUInt32 nBufferSize)
{
	XnStatus nRetVal = XN_STATUS_OK;
	const XnLinkFrameSyncStreamIDs* pLinkFrameSyncStreamIDs = reinterpret_cast<const XnLinkFrameSyncStreamIDs*>(pFrameSyncStreamIDs);
	XnUInt16 nNumStreamIDs = XN_PREPARE_VAR16_IN_BUFFER(pLinkFrameSyncStreamIDs->m_nNumStreamIDs);
	if (nBufferSize < sizeof(pLinkFrameSyncStreamIDs->m_nNumStreamIDs) + (sizeof(pLinkFrameSyncStreamIDs->m_anStreamIDs[0]) * nNumStreamIDs))
	{
		return XN_STATUS_INPUT_BUFFER_OVERFLOW;
	}

	nRetVal = frameSyncStreamIDs.SetSize(nNumStreamIDs);
	XN_IS_STATUS_OK(nRetVal);
	for (XnUInt16 i = 0; i < nNumStreamIDs; i++)
	{
		frameSyncStreamIDs[i] = XN_PREPARE_VAR16_IN_BUFFER(pLinkFrameSyncStreamIDs->m_anStreamIDs[i]);
	}

	return XN_STATUS_OK;
}

XnStatus xnLinkEncodeFrameSyncStreamIDs(void* pFrameSyncStreamIDs, XnUInt32& nBufferSize, const xnl::Array<XnUInt16>& frameSyncStreamIDs)
{
	XnLinkFrameSyncStreamIDs* pLinkFrameSyncStreamIDs = reinterpret_cast<XnLinkFrameSyncStreamIDs*>(pFrameSyncStreamIDs);
	
	if (nBufferSize < (sizeof(pLinkFrameSyncStreamIDs->m_anStreamIDs) + (sizeof(pLinkFrameSyncStreamIDs->m_anStreamIDs[0]) * frameSyncStreamIDs.GetSize())))
	{
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;		
	}

	pLinkFrameSyncStreamIDs->m_nNumStreamIDs = XN_PREPARE_VAR16_IN_BUFFER(XnUInt16(frameSyncStreamIDs.GetSize()));

	for (XnUInt32 i = 0; i < frameSyncStreamIDs.GetSize(); i++)
	{
		pLinkFrameSyncStreamIDs->m_anStreamIDs[i] = XN_PREPARE_VAR16_IN_BUFFER(frameSyncStreamIDs[i]);
	}
	
	return XN_STATUS_OK;
}

XnStatus xnLinkParseComponentVersionsList(xnl::Array<XnComponentVersion>& componentVersions, const XnLinkComponentVersionsList* pLinkList, XnUInt32 nBufferSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (nBufferSize < sizeof(pLinkList->m_nCount))
	{
		xnLogError(XN_MASK_LINK, "Components versions list size should be at least %u bytes, but got %u bytes.", 
			sizeof(pLinkList->m_nCount), nBufferSize);
		return XN_STATUS_LINK_BAD_PROP_SIZE;
	}

	XnUInt32 nCount = XN_PREPARE_VAR32_IN_BUFFER(pLinkList->m_nCount);
	
	XnUInt32 nExpectedSize = 
		sizeof(pLinkList->m_nCount) + 
		(sizeof(pLinkList->m_components[0]) * nCount);

	if (nBufferSize != nExpectedSize)
	{
		xnLogError(XN_MASK_LINK, "Got bad size of 'components versions list' property: %u instead of %u", nBufferSize, nExpectedSize);
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_PROP_SIZE;
	}

	nRetVal = componentVersions.SetSize(nCount);
	XN_IS_STATUS_OK_LOG_ERROR("Set size of output supported map output modes array", nRetVal);
	for (XnUInt32 i = 0; i < nCount; i++)
	{
		nRetVal = xnOSStrCopy(componentVersions[i].m_strName, pLinkList->m_components[i].m_strName, sizeof(componentVersions[i].m_strName));
		XN_IS_STATUS_OK(nRetVal);
		nRetVal = xnOSStrCopy(componentVersions[i].m_strVersion, pLinkList->m_components[i].m_strVersion, sizeof(componentVersions[i].m_strVersion));
		XN_IS_STATUS_OK(nRetVal);
	}

	return XN_STATUS_OK;
}

/*static struct 
{
	const XnChar* m_strCapabilityName;
	XnUInt8 m_nInterfaceID;
} CAP_NAME_TO_LINK_INTERFACE_ID[] = {
	{XN_CAPABILITY_MIRROR,					XN_LINK_INTERFACE_MIRROR},
	{XN_CAPABILITY_ALTERNATIVE_VIEW_POINT,	XN_LINK_INTERFACE_ALTERNATIVE_VIEW_POINT},
	{XN_CAPABILITY_CROPPING,				XN_LINK_INTERFACE_CROPPING},
	{XN_CAPABILITY_USER_POSITION,			XN_LINK_INTERFACE_USER_POSITION},
	{XN_CAPABILITY_SKELETON,				XN_LINK_INTERFACE_SKELETON},
	{XN_CAPABILITY_POSE_DETECTION, 			XN_LINK_INTERFACE_POSE_DETECTION},
	{XN_CAPABILITY_LOCK_AWARE, 				XN_LINK_INTERFACE_LOCK_AWARE},
	{XN_CAPABILITY_ERROR_STATE, 			XN_LINK_INTERFACE_ERROR_STATE},
	{XN_CAPABILITY_FRAME_SYNC, 				XN_LINK_INTERFACE_FRAME_SYNC},
	{XN_CAPABILITY_DEVICE_IDENTIFICATION, 	XN_LINK_INTERFACE_DEVICE_IDENTIFICATION},
	{XN_CAPABILITY_BRIGHTNESS, 				XN_LINK_INTERFACE_BRIGHTNESS},
	{XN_CAPABILITY_CONTRAST, 				XN_LINK_INTERFACE_CONTRAST},
	{XN_CAPABILITY_HUE, 					XN_LINK_INTERFACE_HUE},
	{XN_CAPABILITY_SATURATION, 				XN_LINK_INTERFACE_SATURATION},
	{XN_CAPABILITY_SHARPNESS, 				XN_LINK_INTERFACE_SHARPNESS},
	{XN_CAPABILITY_GAMMA, 					XN_LINK_INTERFACE_GAMMA},
	{XN_CAPABILITY_COLOR_TEMPERATURE, 		XN_LINK_INTERFACE_COLOR_TEMPERATURE},
	{XN_CAPABILITY_BACKLIGHT_COMPENSATION, 	XN_LINK_INTERFACE_BACKLIGHT_COMPENSATION},
	{XN_CAPABILITY_GAIN, 					XN_LINK_INTERFACE_GAIN},
	{XN_CAPABILITY_PAN, 					XN_LINK_INTERFACE_PAN},
	{XN_CAPABILITY_TILT, 					XN_LINK_INTERFACE_TILT},
	{XN_CAPABILITY_ROLL, 					XN_LINK_INTERFACE_ROLL},
	{XN_CAPABILITY_ZOOM, 					XN_LINK_INTERFACE_ZOOM},
	{XN_CAPABILITY_EXPOSURE, 				XN_LINK_INTERFACE_EXPOSURE},
	{XN_CAPABILITY_IRIS, 					XN_LINK_INTERFACE_IRIS},
	{XN_CAPABILITY_FOCUS, 					XN_LINK_INTERFACE_FOCUS},
	{XN_CAPABILITY_LOW_LIGHT_COMPENSATION, 	XN_LINK_INTERFACE_LOW_LIGHT_COMPENSATION},
	{XN_CAPABILITY_ANTI_FLICKER, 			XN_LINK_INTERFACE_ANTI_FLICKER},
	{XN_CAPABILITY_HAND_TOUCHING_FOV_EDGE, 	XN_LINK_INTERFACE_HAND_TOUCHING_FOV_EDGE},
};

XnUInt8 xnLinkNICapabilityToInterfaceID(const XnChar* strCapabilityName)
{
	for (XnUInt32 i = 0; i < sizeof(CAP_NAME_TO_LINK_INTERFACE_ID) / sizeof(CAP_NAME_TO_LINK_INTERFACE_ID[0]); i++)
	{
		if (xnOSStrCmp(strCapabilityName, CAP_NAME_TO_LINK_INTERFACE_ID[i].m_strCapabilityName) == 0)
		{
			return CAP_NAME_TO_LINK_INTERFACE_ID[i].m_nInterfaceID;
		}
	}

	return XN_LINK_INTERFACE_INVALID;
}

const XnChar* xnLinkInterfaceIDToNICapability(XnUInt8 nInterfaceID)
{
	for (XnUInt32 i = 0; i < sizeof(CAP_NAME_TO_LINK_INTERFACE_ID) / sizeof(CAP_NAME_TO_LINK_INTERFACE_ID[0]); i++)
	{
		if (nInterfaceID == CAP_NAME_TO_LINK_INTERFACE_ID[i].m_nInterfaceID)
		{
			return CAP_NAME_TO_LINK_INTERFACE_ID[i].m_strCapabilityName;
		}
	}

	return NULL;
}
*/

const XnChar* xnLinkPropTypeToStr(XnLinkPropType propType)
{
	static const XnChar* PROP_TYPE_STRS[] = 
	{
		"None",		//0x0000
		"Int",		//0x0001
		"Real",		//0x0002
		"String",	//0x0003
		"General",	//0x0004
	};

	return ((size_t)propType < sizeof(PROP_TYPE_STRS) / sizeof(PROP_TYPE_STRS[0])) ? PROP_TYPE_STRS[propType] : "Unknown";
}

/*XnProductionNodeType xnLinkStreamTypeToNINodeType(XnLinkStreamType streamType)
{
	switch (streamType)
	{
		case XN_LINK_STREAM_TYPE_COLOR:
			return XN_NODE_TYPE_IMAGE;
		case XN_LINK_STREAM_TYPE_IR:
			return XN_NODE_TYPE_IR;
		case XN_LINK_STREAM_TYPE_SHIFTS:
			return XN_NODE_TYPE_DEPTH;
		case XN_LINK_STREAM_TYPE_AUDIO:
			return XN_NODE_TYPE_AUDIO;
		case XN_LINK_STREAM_TYPE_USER:
			return XN_NODE_TYPE_USER;
		case XN_LINK_STREAM_TYPE_HANDS:
			return XN_NODE_TYPE_HANDS;
		case XN_LINK_STREAM_TYPE_GESTURES:
			return XN_NODE_TYPE_GESTURE;
		case XN_LINK_STREAM_TYPE_NONE:
		case XN_LINK_STREAM_TYPE_INVALID:
		default:
			return XN_NODE_TYPE_INVALID;
	}
}

XnLinkStreamType xnLinkNINodeTypeToStreamType(XnProductionNodeType nodeType)
{
	switch (nodeType)
	{
		case XN_NODE_TYPE_DEPTH:
			return XN_LINK_STREAM_TYPE_SHIFTS;
		case XN_NODE_TYPE_IMAGE:
			return XN_LINK_STREAM_TYPE_COLOR;
		case XN_NODE_TYPE_AUDIO:
			return XN_LINK_STREAM_TYPE_AUDIO;
		case XN_NODE_TYPE_IR:
			return XN_LINK_STREAM_TYPE_IR;
		case XN_NODE_TYPE_USER:
			return XN_LINK_STREAM_TYPE_USER;
		case XN_NODE_TYPE_HANDS:
			return XN_LINK_STREAM_TYPE_HANDS;		
		case XN_NODE_TYPE_GESTURE:
			return XN_LINK_STREAM_TYPE_GESTURES;
		case XN_NODE_TYPE_INVALID:
		default:
			return XN_LINK_STREAM_TYPE_INVALID;
	}
}*/

void xnLinkParseDetailedVersion(XnLinkDetailedVersion& version, const XnLinkDetailedVersion& linkVersion)
{
	version.m_nMajor = linkVersion.m_nMajor;
    version.m_nMinor = linkVersion.m_nMinor;
    version.m_nMaintenance = XN_PREPARE_VAR16_IN_BUFFER(linkVersion.m_nMaintenance);
    version.m_nBuild = XN_PREPARE_VAR32_IN_BUFFER(linkVersion.m_nBuild);
	xnOSMemCopy(version.m_strModifier, linkVersion.m_strModifier, sizeof(version.m_strModifier));
}

void xnLinkParseLeanVersion(XnLeanVersion& version, const XnLinkLeanVersion& linkVersion)
{
	version.m_nMajor = linkVersion.m_nMajor;
	version.m_nMinor = linkVersion.m_nMinor;
}

void xnEncodeLeanVersion(XnLinkLeanVersion& linkVersion, const XnLeanVersion& version)
{
	linkVersion.m_nMajor = version.m_nMajor;
	linkVersion.m_nMinor = version.m_nMinor;
}

void xnLinkParseVideoMode(XnFwStreamVideoMode& videoMode, const XnLinkVideoMode& linkVideoMode)
{
	videoMode.m_nXRes = XN_PREPARE_VAR16_IN_BUFFER(linkVideoMode.m_nXRes);
	videoMode.m_nYRes = XN_PREPARE_VAR16_IN_BUFFER(linkVideoMode.m_nYRes);
	videoMode.m_nFPS = XN_PREPARE_VAR16_IN_BUFFER(linkVideoMode.m_nFPS);
	videoMode.m_nPixelFormat = (XnFwPixelFormat)XN_PREPARE_VAR16_IN_BUFFER(linkVideoMode.m_nPixelFormat);
	videoMode.m_nCompression = (XnFwCompressionType)XN_PREPARE_VAR16_IN_BUFFER(linkVideoMode.m_nCompression);
}

void xnLinkEncodeVideoMode(XnLinkVideoMode& linkVideoMode, const XnFwStreamVideoMode& videoMode)
{
	linkVideoMode.m_nXRes = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)videoMode.m_nXRes);
	linkVideoMode.m_nYRes = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)videoMode.m_nYRes);
	linkVideoMode.m_nFPS = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)videoMode.m_nFPS);
	linkVideoMode.m_nPixelFormat = (XnUInt8)videoMode.m_nPixelFormat;
	linkVideoMode.m_nCompression = (XnUInt8)videoMode.m_nCompression;
}

XnStatus xnLinkParseSupportedVideoModes(xnl::Array<XnFwStreamVideoMode>& aModes, 
                                                const XnLinkSupportedVideoModes* pLinkSupportedModes, 
                                                XnUInt32 nBufferSize)
{
    XnStatus nRetVal = XN_STATUS_OK;
    XnUInt32 nModes = 0;
    XnUInt32 nExpectedSize = 0;

    XN_VALIDATE_INPUT_PTR(pLinkSupportedModes);

    if (nBufferSize < sizeof(pLinkSupportedModes->m_nNumModes))
    {
        xnLogError(XN_MASK_LINK, "Size of link video modes was only %u bytes, must be at least %u.", nBufferSize, 
            sizeof(pLinkSupportedModes->m_nNumModes));
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_SIZE;
    }

    nModes = XN_PREPARE_VAR32_IN_BUFFER(pLinkSupportedModes->m_nNumModes);
    nExpectedSize = (sizeof(pLinkSupportedModes->m_nNumModes) + 
        (sizeof(pLinkSupportedModes->m_supportedVideoModes[0]) * nModes));
    if (nBufferSize != nExpectedSize)
    {
        xnLogError(XN_MASK_LINK, "Got bad size of 'supported video modes' property: %u instead of %u", nBufferSize, nExpectedSize);
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_RESPONSE_SIZE;
    }

    nRetVal = aModes.SetSize(nModes);
    XN_IS_STATUS_OK_LOG_ERROR("Set size of output supported map output modes array", nRetVal);
    for (XnUInt32 i = 0; i < nModes; i++)
    {
        xnLinkParseVideoMode(aModes[i], pLinkSupportedModes->m_supportedVideoModes[i]);
    }

    return XN_STATUS_OK;
}

XnStatus xnLinkParseBitSet(xnl::BitSet& bitSet, const XnLinkBitSet* pLinkBitSet, XnUInt32 nBufferSize)
{
    XnStatus nRetVal = XN_STATUS_OK;
    XnUInt32 nExpectedSize = 0;
    XnUInt32 nBitSetSize = 0;
    XN_VALIDATE_INPUT_PTR(pLinkBitSet);

    if (nBufferSize < sizeof(pLinkBitSet->m_nSize))
    {
        xnLogError(XN_MASK_LINK, "Size of link bit set was only %u bytes, must be at least %u", nBufferSize,
            sizeof(pLinkBitSet->m_nSize));
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_SIZE;        
    }
    
    nBitSetSize = XN_PREPARE_VAR32_IN_BUFFER(pLinkBitSet->m_nSize);
    nExpectedSize = sizeof(pLinkBitSet->m_aData[0]) * nBitSetSize;
    if (nExpectedSize != nBitSetSize)
    {
        xnLogError(XN_MASK_LINK, "Expected size of bitset to be %u bytes, but got %u", nExpectedSize, nBitSetSize);
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_SIZE;
    }

    nRetVal = bitSet.SetData(pLinkBitSet->m_aData, nBitSetSize);
    XN_IS_STATUS_OK_LOG_ERROR("Set data bytes in bit set", nRetVal);
    return XN_STATUS_OK;
}

XnStatus xnLinkEncodeBitSet(XnLinkBitSet& linkBitSet, XnUInt32& nBufferSize, const xnl::BitSet& bitSet)
{
	XnUInt32 nBytes   = (bitSet.GetSize() >> 3);
	XnUInt32 nPartial = (bitSet.GetSize()%4)?1:0;
	XnUInt32 nDataSize = (nBytes + nPartial)<<2;

	XnUInt32 nRequiredBufferSize = nDataSize + sizeof(linkBitSet.m_nSize);
	if (nBufferSize < nRequiredBufferSize)
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;        
	}

	linkBitSet.m_nSize = XN_PREPARE_VAR32_IN_BUFFER(nDataSize);
	xnOSMemCopy(linkBitSet.m_aData, bitSet.GetData(), nDataSize);
	nBufferSize = nRequiredBufferSize;
	
	return (XN_STATUS_OK);
}

void xnLinkParseShiftToDepthConfig(XnShiftToDepthConfig& shiftToDepthConfig, const XnLinkShiftToDepthConfig& linkShiftToDepthConfig)
{
    shiftToDepthConfig.nZeroPlaneDistance = XN_PREPARE_VAR16_IN_BUFFER(linkShiftToDepthConfig.nZeroPlaneDistance);
	shiftToDepthConfig.fZeroPlanePixelSize = XN_PREPARE_VAR_FLOAT_IN_BUFFER(linkShiftToDepthConfig.fZeroPlanePixelSize);
    shiftToDepthConfig.fEmitterDCmosDistance = XN_PREPARE_VAR_FLOAT_IN_BUFFER(linkShiftToDepthConfig.fEmitterDCmosDistance);
    shiftToDepthConfig.nDeviceMaxShiftValue = XN_PREPARE_VAR32_IN_BUFFER(linkShiftToDepthConfig.nDeviceMaxShiftValue);
    shiftToDepthConfig.nDeviceMaxDepthValue = XN_PREPARE_VAR32_IN_BUFFER(linkShiftToDepthConfig.nDeviceMaxDepthValue);
    shiftToDepthConfig.nConstShift = XN_PREPARE_VAR32_IN_BUFFER(linkShiftToDepthConfig.nConstShift);
    shiftToDepthConfig.nPixelSizeFactor = XN_PREPARE_VAR32_IN_BUFFER(linkShiftToDepthConfig.nPixelSizeFactor);
    shiftToDepthConfig.nParamCoeff = XN_PREPARE_VAR32_IN_BUFFER(linkShiftToDepthConfig.nParamCoeff);
    shiftToDepthConfig.nShiftScale = XN_PREPARE_VAR32_IN_BUFFER(linkShiftToDepthConfig.nShiftScale);
    shiftToDepthConfig.nDepthMinCutOff = XN_PREPARE_VAR16_IN_BUFFER(linkShiftToDepthConfig.nDepthMinCutOff);
    shiftToDepthConfig.nDepthMaxCutOff = XN_PREPARE_VAR16_IN_BUFFER(linkShiftToDepthConfig.nDepthMaxCutOff);
	shiftToDepthConfig.dDepthScale = 1.0; // not controlled by FW
}

void xnLinkParseCropping(OniCropping& cropping, const XnLinkCropping& linkCropping)
{
    cropping.enabled = linkCropping.m_bEnabled;
	cropping.originX = XN_PREPARE_VAR16_IN_BUFFER(linkCropping.m_nXOffset);
	cropping.originY = XN_PREPARE_VAR16_IN_BUFFER(linkCropping.m_nYOffset);
    cropping.width   = XN_PREPARE_VAR16_IN_BUFFER(linkCropping.m_nXSize);
    cropping.height  = XN_PREPARE_VAR16_IN_BUFFER(linkCropping.m_nYSize);
}

void xnLinkEncodeCropping(XnLinkCropping& linkCropping, const OniCropping& cropping)
{
    linkCropping.m_bEnabled = XnUInt8(cropping.enabled);
    linkCropping.m_nReserved1 = 0;
    linkCropping.m_nReserved2 = 0;
    linkCropping.m_nReserved3 = 0;
	linkCropping.m_nXOffset   = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)cropping.originX);
	linkCropping.m_nYOffset   = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)cropping.originY);
    linkCropping.m_nXSize     = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)cropping.width);
    linkCropping.m_nYSize     = XN_PREPARE_VAR16_IN_BUFFER((XnUInt16)cropping.height);
}

const XnChar* xnLinkGetPropName(XnLinkPropID propID)
{
    /* To build this switch, paste the contents of the XnLinkPropID enum here, and then use 
       visual studio's search & replace with regex, and replace this:
            {XN_LINK_PROP_ID_{:i}}:b*=.*
       Into this:
            case \1:\n\t\t\t\treturn "\2";
       
       Or just code it by hand if it relaxes you.
    */

    switch (propID)
    {
        case XN_LINK_PROP_ID_NONE:
            return "NONE";
        case XN_LINK_PROP_ID_CONTROL_MAX_PACKET_SIZE:
            return "CONTROL_MAX_PACKET_SIZE";
        case XN_LINK_PROP_ID_FW_VERSION:
            return "FW_VERSION";
        case XN_LINK_PROP_ID_PROTOCOL_VERSION:
			return "PROTOCOL_VERSION";
        case XN_LINK_PROP_ID_SUPPORTED_MSG_TYPES:
    		return "SUPPORTED_MSG_TYPES";
        case XN_LINK_PROP_ID_SUPPORTED_PROPS:
			return "SUPPORTED_PROPS";
        case XN_LINK_PROP_ID_HW_VERSION:
			return "HW_VERSION";
        case XN_LINK_PROP_ID_SERIAL_NUMBER:
			return "SERIAL_NUMBER";
		case XN_LINK_PROP_ID_SUPPORTED_BIST_TESTS:
			return "SUPPORTED_BIST_TESTS";
        case XN_LINK_PROP_ID_SUPPORTED_VIDEO_MODES:
			return "SUPPORTED_VIDEO_MODES";
        case XN_LINK_PROP_ID_VIDEO_MODE:
			return "VIDEO_MODE";
        case XN_LINK_PROP_ID_STREAM_SUPPORTED_INTERFACES:
			return "STREAM_SUPPORTED_INTERFACES";
        case XN_LINK_PROP_ID_STREAM_FRAG_LEVEL:
			return "STREAM_FRAG_LEVEL";
        case XN_LINK_PROP_ID_HAND_SMOOTHING:
			return "HAND_SMOOTHING";
        case XN_LINK_PROP_ID_SUPPORTED_SKELETON_JOINTS:
			return "SUPPORTED_SKELETON_JOINTS";
        case XN_LINK_PROP_ID_SUPPORTED_SKELETON_PROFILES:
			return "SUPPORTED_SKELETON_PROFILES";
        case XN_LINK_PROP_ID_NEEDED_CALIBRATION_POSE:
			return "NEEDED_CALIBRATION_POSE";
        case XN_LINK_PROP_ID_ACTIVE_JOINTS:
			return "ACTIVE_JOINTS";
        case XN_LINK_PROP_ID_SKELETON_SMOOTHING:
			return "SKELETON_SMOOTHING";
        case XN_LINK_PROP_ID_SUPPORTED_POSES:
			return "SUPPORTED_POSES";
        case XN_LINK_PROP_ID_MIRROR:
			return "MIRROR";
        case XN_LINK_PROP_ID_CROPPING:
			return "CROPPING";
        case XN_LINK_PROP_ID_INVALID:
			return "INVALID";
        default:
            return "UNKNOWN";
    }
}



XnStatus xnLinkValidateGeneralProp(XnLinkPropType propType, XnUInt32 nValueSize, XnUInt32 nMinSize)
{
    if (propType != XN_LINK_PROP_TYPE_GENERAL)
    {
        xnLogError(XN_MASK_LINK, "Property type should be %s, but got type %s", xnLinkPropTypeToStr(XN_LINK_PROP_TYPE_GENERAL), xnLinkPropTypeToStr(propType));
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_TYPE;
    }

    if (nValueSize < nMinSize)
    {
        xnLogError(XN_MASK_LINK, "Property value size should be at least %u bytes, but got %u bytes.", 
            nMinSize, nValueSize);
        return XN_STATUS_LINK_BAD_PROP_SIZE;
    }

    return XN_STATUS_OK;
}

XnStatus xnLinkParseLeanVersionProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, XnLeanVersion& leanVersion)
{
	XnStatus nRetVal = xnLinkValidateGeneralProp(propType, nValueSize, sizeof(XnLinkLeanVersion));
	XN_IS_STATUS_OK_LOG_ERROR("Validate version property", nRetVal);
	xnLinkParseLeanVersion(leanVersion, *reinterpret_cast<const XnLinkLeanVersion*>(pValue));
	return XN_STATUS_OK;
}

XnStatus xnLinkParseIDSetProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::Array<xnl::BitSet>& idSet)
{
    XnStatus nRetVal = xnLinkValidateGeneralProp(propType, nValueSize, sizeof(XnLinkIDSetHeader));
    XN_IS_STATUS_OK_LOG_ERROR("Validate id set property", nRetVal);
    nRetVal = xnLinkParseIDSet(idSet, pValue, nValueSize);
    XN_IS_STATUS_OK_LOG_ERROR("Parse id set", nRetVal);
    return XN_STATUS_OK;
}

XnStatus xnLinkParseBitSetProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::BitSet& bitSet)
{
    XnStatus nRetVal = xnLinkValidateGeneralProp(propType, nValueSize, sizeof(XnUInt32)); //min size of of m_nSize
    XN_IS_STATUS_OK_LOG_ERROR("Validate id set property", nRetVal);
    nRetVal = xnLinkParseBitSet(bitSet, reinterpret_cast<const XnLinkBitSet*>(pValue), nValueSize);
    XN_IS_STATUS_OK_LOG_ERROR("Parse bit set", nRetVal);
    return XN_STATUS_OK;
}

XnStatus xnLinkParseFrameSyncStreamIDsProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::Array<XnUInt16>& streamIDs)
{
    XnStatus nRetVal = xnLinkValidateGeneralProp(propType, nValueSize, sizeof(XnUInt16)); //Min size is of m_nNumStreamIDs
    XN_IS_STATUS_OK_LOG_ERROR("Validate frame sync stream IDs property", nRetVal);
    nRetVal = xnLinkParseFrameSyncStreamIDs(streamIDs, pValue, nValueSize);
    XN_IS_STATUS_OK_LOG_ERROR("Parse frame sync stream IDs", nRetVal);
    return XN_STATUS_OK;
}

XnStatus xnLinkParseComponentVersionsListProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::Array<XnComponentVersion>& componentVersions)
{
	const XnLinkComponentVersionsList* pLinkList = reinterpret_cast<const XnLinkComponentVersionsList*>(pValue);
	XnStatus nRetVal = xnLinkValidateGeneralProp(propType, nValueSize, sizeof(pLinkList->m_nCount)); //Min size is the count field
	XN_IS_STATUS_OK_LOG_ERROR("Validate components versions list property", nRetVal);
	nRetVal = xnLinkParseComponentVersionsList(componentVersions, pLinkList, nValueSize);
	XN_IS_STATUS_OK_LOG_ERROR("Parse frame sync stream IDs", nRetVal);
	return XN_STATUS_OK;
}

XnUInt32 xnLinkGetPixelSizeByStreamType(XnLinkStreamType streamType)
{
    switch (streamType)
    {
    case XN_LINK_STREAM_TYPE_SHIFTS:
        return sizeof(OniDepthPixel);
    case XN_LINK_STREAM_TYPE_IR:
        return sizeof(OniGrayscale16Pixel);
    case XN_LINK_STREAM_TYPE_COLOR:
        return sizeof(OniYUV422DoublePixel)/2; //TODO: different pixel formats
	case XN_LINK_STREAM_TYPE_DY:
		return sizeof(XnUInt16);
    default:
        xnLogError(XN_MASK_LINK, "Bad stream type: %u", streamType);
        XN_ASSERT(FALSE);
        return 0;
    }
}
XnStatus xnLinkReadDebugData(XnCommandDebugData& commandDebugData, XnLinkDebugDataResponse* pDebugDataResponse)
{
    XnStatus nRetVal = XN_STATUS_OK;

    if(commandDebugData.dataSize < pDebugDataResponse->m_header.m_nValueSize)
    {
        xnLogError(XN_MASK_LINK, "Size of retrieved data was larger than requested: %u bytes, must be at least %u.", pDebugDataResponse->m_header.m_nValueSize, 
            commandDebugData.dataSize);
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_SIZE;
    }
    commandDebugData.dataSize = pDebugDataResponse->m_header.m_nValueSize; //if the sized received is smaller than expected
    for(int i = 0; i < commandDebugData.dataSize; i++)
    {
        commandDebugData.data[i] = pDebugDataResponse->m_data[i];
    }
    return nRetVal;
}

XnStatus xnLinkParseSupportedI2CDevices(const XnLinkSupportedI2CDevices* pDevicesList, XnUInt32 nBufferSize, xnl::Array<XnLinkI2CDevice>& supportedDevices)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 nDevices = 0;
	XnUInt32 nExpectedSize = 0;

	XN_VALIDATE_INPUT_PTR(pDevicesList);

	if (nBufferSize < sizeof(pDevicesList->m_nCount))
	{
		xnLogError(XN_MASK_LINK, "Size of link supported devices list was only %u bytes, must be at least %u.", nBufferSize, 
			sizeof(pDevicesList->m_nCount));
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_PROP_SIZE;
	}

	nDevices = XN_PREPARE_VAR32_IN_BUFFER(pDevicesList->m_nCount);
	nExpectedSize = (sizeof(pDevicesList->m_nCount) + 
		(sizeof(pDevicesList->m_aI2CDevices[0]) * nDevices));
	if (nBufferSize != nExpectedSize)
	{
		xnLogError(XN_MASK_LINK, "Got bad size of 'supported devices list' property: %u instead of %u", nBufferSize, nExpectedSize);
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_RESPONSE_SIZE;
	}

	nRetVal = supportedDevices.SetSize(nDevices);
	XN_IS_STATUS_OK_LOG_ERROR("Set size of output supported device array", nRetVal);

	for (XnUInt32 i = 0; i < nDevices; i++)
	{
		supportedDevices[i].m_nID = XN_PREPARE_VAR32_IN_BUFFER(pDevicesList->m_aI2CDevices[i].m_nID);
		nRetVal = xnOSStrCopy(supportedDevices[i].m_strName, pDevicesList->m_aI2CDevices[i].m_strName, sizeof(supportedDevices[i].m_strName));
		XN_IS_STATUS_OK_LOG_ERROR("Copy I2C device name", nRetVal);
        supportedDevices[i].m_nMasterID = pDevicesList->m_aI2CDevices[i].m_nMasterID;
        supportedDevices[i].m_nSlaveID = pDevicesList->m_aI2CDevices[i].m_nSlaveID;
	}

	return XN_STATUS_OK;
}

XnStatus xnLinkParseSupportedLogFiles(const XnLinkSupportedLogFiles* pFilesList, XnUInt32 nBufferSize, xnl::Array<XnLinkLogFile>& supportedFiles)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 nLogFiles = 0;
	XnUInt32 nExpectedSize = 0;

	XN_VALIDATE_INPUT_PTR(pFilesList);

	if (nBufferSize < sizeof(pFilesList->m_nCount))
	{
		xnLogError(XN_MASK_LINK, "Size of link supported files list was only %u bytes, must be at least %u.", nBufferSize, 
			sizeof(pFilesList->m_nCount));
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_PROP_SIZE;
	}

	nLogFiles = XN_PREPARE_VAR32_IN_BUFFER(pFilesList->m_nCount);
	nExpectedSize = (sizeof(pFilesList->m_nCount) + 
		(sizeof(pFilesList->m_aLogFiles[0]) * nLogFiles));
	if (nBufferSize != nExpectedSize)
	{
		xnLogError(XN_MASK_LINK, "Got bad size of 'supported log files list' property: %u instead of %u", nBufferSize, nExpectedSize);
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_RESPONSE_SIZE;
	}

	nRetVal = supportedFiles.SetSize(nLogFiles);
	XN_IS_STATUS_OK_LOG_ERROR("Set size of output supported log files array", nRetVal);

	for (XnUInt32 i = 0; i < nLogFiles; i++)
	{
		supportedFiles[i].m_nID = XN_PREPARE_VAR32_IN_BUFFER(pFilesList->m_aLogFiles[i].m_nID);
		nRetVal = xnOSStrCopy(supportedFiles[i].m_strName, pFilesList->m_aLogFiles[i].m_strName, sizeof(supportedFiles[i].m_strName));
		XN_IS_STATUS_OK_LOG_ERROR("Copy log file name", nRetVal);
	}

	return XN_STATUS_OK;
}

XnStatus xnLinkParseSupportedBistTests(const XnLinkSupportedBistTests* pSupportedTests, XnUInt32 nBufferSize, xnl::Array<XnBistInfo>& supportedTests)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 nTests = 0;
	XnUInt32 nExpectedSize = 0;

	XN_VALIDATE_INPUT_PTR(pSupportedTests);

	if (nBufferSize < sizeof(pSupportedTests->m_nCount))
	{
		xnLogError(XN_MASK_LINK, "Size of link supported BIST tests was only %u bytes, must be at least %u.", nBufferSize, 
			sizeof(pSupportedTests->m_nCount));
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_PROP_SIZE;
	}

	nTests = XN_PREPARE_VAR32_IN_BUFFER(pSupportedTests->m_nCount);
	nExpectedSize = (sizeof(pSupportedTests->m_nCount) + 
		(sizeof(pSupportedTests->m_aTests[0]) * nTests));
	if (nBufferSize != nExpectedSize)
	{
		xnLogError(XN_MASK_LINK, "Got bad size of 'supported BIST tests' property: %u instead of %u", nBufferSize, nExpectedSize);
		XN_ASSERT(FALSE);
		return XN_STATUS_LINK_BAD_RESPONSE_SIZE;
	}

	nRetVal = supportedTests.SetSize(nTests);
	XN_IS_STATUS_OK_LOG_ERROR("Set size of output supported BIST tests array", nRetVal);

	for (XnUInt32 i = 0; i < nTests; i++)
	{
		supportedTests[i].id = XN_PREPARE_VAR32_IN_BUFFER(pSupportedTests->m_aTests[i].m_nID);
		nRetVal = xnOSStrCopy(supportedTests[i].name, pSupportedTests->m_aTests[i].m_strName, sizeof(supportedTests[i].name));
		XN_IS_STATUS_OK_LOG_ERROR("Copy BIST test name", nRetVal);
	}

	return XN_STATUS_OK;
}

XnStatus xnLinkParseSupportedTempList(const XnLinkTemperatureSensorsList* pSupportedList, XnUInt32 nBufferSize, xnl::Array<XnTempInfo>& supportedTempList)
{
    XnStatus nRetVal = XN_STATUS_OK;
    XnUInt32 nTemp = 0;
    XnUInt32 nExpectedSize = 0;

    XN_VALIDATE_INPUT_PTR(pSupportedList);

    if (nBufferSize < sizeof(pSupportedList->m_nCount))
    {
        xnLogError(XN_MASK_LINK, "Size of link supported Temperature list was only %u bytes, must be at least %u.", nBufferSize, 
            sizeof(pSupportedList->m_nCount));
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_SIZE;
    }

    nTemp = XN_PREPARE_VAR32_IN_BUFFER(pSupportedList->m_nCount);
    nExpectedSize = (sizeof(pSupportedList->m_nCount) + 
        (sizeof(pSupportedList->m_aSensors[0]) * nTemp));
    if (nBufferSize != nExpectedSize)
    {
        xnLogError(XN_MASK_LINK, "Got bad size of 'supported Temperature list' property: %u instead of %u", nBufferSize, nExpectedSize);
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_RESPONSE_SIZE;
    }

    nRetVal = supportedTempList.SetSize(nTemp);
    XN_IS_STATUS_OK_LOG_ERROR("Set size of output supported Temperature list array", nRetVal);

    for (XnUInt32 i = 0; i < nTemp; i++)
    {
        supportedTempList[i].id = XN_PREPARE_VAR32_IN_BUFFER(pSupportedList->m_aSensors[i].m_nID);
        nRetVal = xnOSStrCopy(supportedTempList[i].name, (const XnChar*) pSupportedList->m_aSensors[i].m_strName, sizeof(supportedTempList[i].name));
        XN_IS_STATUS_OK_LOG_ERROR("Copy Temperature list name", nRetVal);
    }

    return XN_STATUS_OK;
}

XnStatus xnLinkParseGetTemperature(const XnLinkTemperatureResponse* tempResponse, XnUInt32 nBufferSize, XnCommandTemperatureResponse& tempData)
{
    XnStatus nRetVal = XN_STATUS_OK;
    XnUInt32 nExpectedSize = 0;

    XN_VALIDATE_INPUT_PTR(tempResponse);

    if (nBufferSize < sizeof(tempData.temperature))
    {
        xnLogError(XN_MASK_LINK, "Size of link Get Temperature was only %u bytes, must be at least %u.", nBufferSize, 
            sizeof(tempData.temperature));
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_SIZE;
    }

    nExpectedSize = (sizeof(tempResponse));
    if (nBufferSize != nExpectedSize)
    {
        xnLogError(XN_MASK_LINK, "Got bad size of 'Temperature struct' property: %u instead of %u", nBufferSize, nExpectedSize);
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_RESPONSE_SIZE;
    }
    tempData.id = XN_PREPARE_VAR32_IN_BUFFER(tempResponse->m_nID);
    tempData.temperature = XN_PREPARE_VAR_FLOAT_IN_BUFFER(tempResponse->value);
    XN_IS_STATUS_OK_LOG_ERROR("Copy Temperature value", nRetVal);

    return XN_STATUS_OK;
}
const XnChar* xnLinkPixelFormatToName(XnFwPixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case XN_FW_PIXEL_FORMAT_SHIFTS_9_3:
		return "Shifts9.3";
	case XN_FW_PIXEL_FORMAT_GRAYSCALE16:
		return "Grayscale16";
	case XN_FW_PIXEL_FORMAT_YUV422:
		return "YUV422";
	case XN_FW_PIXEL_FORMAT_BAYER8:
		return "BAYER8";
	default:
		XN_ASSERT(FALSE);
		return "UNKNOWN";
	}
}

XnFwPixelFormat xnLinkPixelFormatFromName(const XnChar* name)
{
	if (xnOSStrCmp(name, "Shifts9.3") == 0)
		return XN_FW_PIXEL_FORMAT_SHIFTS_9_3;
	else if (xnOSStrCmp(name, "Grayscale16") == 0)
		return XN_FW_PIXEL_FORMAT_GRAYSCALE16;
	else if (xnOSStrCmp(name, "YUV422") == 0)
		return XN_FW_PIXEL_FORMAT_YUV422;
	else if (xnOSStrCmp(name, "BAYER8") == 0)
		return XN_FW_PIXEL_FORMAT_BAYER8;
	else
	{
		XN_ASSERT(FALSE);
		return (XnFwPixelFormat)(-1);
	}
}

const XnChar* xnLinkCompressionToName(XnFwCompressionType compression)
{
	switch (compression)
	{
	case XN_FW_COMPRESSION_NONE:
		return "None";
	case XN_FW_COMPRESSION_8Z:
		return "8z";
	case XN_FW_COMPRESSION_16Z:
		return "16z";
	case XN_FW_COMPRESSION_24Z:
		return "24z";
	case XN_FW_COMPRESSION_6_BIT_PACKED:
		return "6bit";
	case XN_FW_COMPRESSION_10_BIT_PACKED:
		return "10bit";
	case XN_FW_COMPRESSION_11_BIT_PACKED:
		return "11bit";
	case XN_FW_COMPRESSION_12_BIT_PACKED:
		return "12bit";
	default:
		XN_ASSERT(FALSE);
		return "UNKNOWN";
	}
}

XnFwCompressionType xnLinkCompressionFromName(const XnChar* name)
{
	if (xnOSStrCmp(name, "None") == 0)
		return XN_FW_COMPRESSION_NONE;
	else if (xnOSStrCmp(name, "8z") == 0)
		return XN_FW_COMPRESSION_8Z;
	else if (xnOSStrCmp(name, "16z") == 0)
		return XN_FW_COMPRESSION_16Z;
	else if (xnOSStrCmp(name, "24z") == 0)
		return XN_FW_COMPRESSION_24Z;
	else if (xnOSStrCmp(name, "6bit") == 0)
		return XN_FW_COMPRESSION_6_BIT_PACKED;
	else if (xnOSStrCmp(name, "10bit") == 0)
		return XN_FW_COMPRESSION_10_BIT_PACKED;
	else if (xnOSStrCmp(name, "11bit") == 0)
		return XN_FW_COMPRESSION_11_BIT_PACKED;
	else if (xnOSStrCmp(name, "12bit") == 0)
		return XN_FW_COMPRESSION_12_BIT_PACKED;
	else
	{
		XN_ASSERT(FALSE);
		return (XnFwCompressionType)-1;
	}
}

void xnLinkVideoModeToString(XnFwStreamVideoMode videoMode, XnChar* buffer, XnUInt32 bufferSize)
{
	XnUInt32 charsWritten = 0;
	xnOSStrFormat(buffer, bufferSize, &charsWritten, "%ux%u@%u (%s, %s)", 
		videoMode.m_nXRes, videoMode.m_nYRes, videoMode.m_nFPS, 
		xnLinkPixelFormatToName(videoMode.m_nPixelFormat),
		xnLinkCompressionToName(videoMode.m_nCompression));
}

void xnLinkParseBootStatus(XnBootStatus& bootStatus, const XnLinkBootStatus& linkBootStatus)
{
	bootStatus.errorCode = (XnBootErrorCode)XN_PREPARE_VAR32_IN_BUFFER(linkBootStatus.m_nErrorCode);
	bootStatus.zone = (XnFileZone)XN_PREPARE_VAR32_IN_BUFFER(linkBootStatus.m_nZone);
}
