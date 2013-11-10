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
#ifndef XNLINKSTATUSCODES_H
#define XNLINKSTATUSCODES_H

#include <XnStatus.h>
#include "XnLinkProtoLibDefs.h"

#define XN_ERROR_GROUP_LINKPROTOLIB 6000

#define XN_PS_STATUS_MESSAGE_MAP_START(module)								\
    XN_STATUS_MESSAGE_MAP_START_FROM(XN_ERROR_GROUP_PRIMESENSE, module)

#define XN_PS_STATUS_MESSAGE_MAP_END(module)								\
    XN_STATUS_MESSAGE_MAP_END_FROM(XN_ERROR_GROUP_PRIMESENSE, module)

XN_PS_STATUS_MESSAGE_MAP_START(XN_ERROR_GROUP_LINKPROTOLIB)

//31770
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_HEADER_SIZE, "Bad link layer header size")

//31771
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_MAGIC, "Bad link layer magic")

//31772
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_FRAGMENTATION, "Bad link layer fragmentation flags")

//31773
XN_STATUS_MESSAGE(XN_STATUS_LINK_PARTIAL_PACKET, "Received a partial link layer packet")

//31774
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_STREAM_ID, "Bad stream ID in link layer packet")

//31775
XN_STATUS_MESSAGE(XN_STATUS_LINK_PACKETS_LOST, "One or more Link layer packets were lost")

//31776
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESPONSE_MSG_TYPE_MISMATCH, "Response message type mismatch")

//31777
XN_STATUS_MESSAGE(XN_STATUS_LINK_MISSING_RESPONSE_INFO, "Response info missing in response packet")

//31778
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_PENDING, "Link response: Response pending")

//31779
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_BAD_FILE_TYPE, "Link response: Bad file type")

//31780
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_CMD_ERROR, "Link response: General command error")

//31781
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_CMD_NOT_SUPPORTED, "Link response: Command not supported") //FW replied that command is not supported

//31782
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_BAD_CMD_SIZE, "Link response: Bad command size")

//31783
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_BAD_PARAMETERS, "Link response: bad parameters")

//31784
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_CORRUPT_PACKET, "Link response: Corrupt packet")

//31785
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_CORRUPT_FILE, "Link response: File is corrupt")

//31786
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_BAD_CRC, "Link response: Bad CRC")

//31787
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_INCORRECT_SIZE, "Link response: Incorrect size")

//31788
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_INPUT_BUFFER_OVERFLOW, "Link response: Input buffer overflow")

//31789
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESP_UNKNOWN, "Link response: Unknown response code")

//31790
XN_STATUS_MESSAGE(XN_STATUS_LINK_INVALID_MAX_SHIFT, "Max shift value is too big")

//31791
XN_STATUS_MESSAGE(XN_STATUS_LINK_INVALID_MAX_DEPTH, "Max depth value is too big")

//31792
XN_STATUS_MESSAGE(XN_STATUS_LINK_MISSING_TIMESTAMP, "Missing timestamp in data unit")

//31793
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_RESPONSE_SIZE, "Response message has incorrect size")

//31794
XN_STATUS_MESSAGE(XN_STATUS_LINK_RESERVED5, "RESERVED5")

//31795
XN_STATUS_MESSAGE(XN_STATUS_LINK_UNKNOWN_GESTURE, "Unknown gesture")

//31796
XN_STATUS_MESSAGE(XN_STATUS_LINK_UNKNOWN_POSE, "Unknown pose")

//31796
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_PACKET_FORMAT, "Bad packet format")

//31797
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_PROP_TYPE, "Bad property type")

//31798
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_PROP_ID, "Bad property ID")

//31799
XN_STATUS_MESSAGE(XN_STATUS_LINK_CMD_NOT_SUPPORTED, "Command not supported") //Command not supported as indicated by supported msg types by FW

//31800
XN_STATUS_MESSAGE(XN_STATUS_LINK_PROP_NOT_SUPPORTED, "Property not supported") //Property not supported as indicated by supported properties by FW

//31801
XN_STATUS_MESSAGE(XN_STATUS_LINK_BAD_PROP_SIZE, "Bad property size")

XN_PS_STATUS_MESSAGE_MAP_END(XN_ERROR_GROUP_LINKPROTOLIB)

#endif // XNLINKSTATUSCODES_H
