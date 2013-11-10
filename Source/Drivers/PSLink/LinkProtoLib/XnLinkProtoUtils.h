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
#ifndef XNLINKPROTOUTILS_H
#define XNLINKPROTOUTILS_H

#include "XnLinkDefs.h"
#include "XnLinkProto.h"
#include "XnLinkStatusCodes.h"
#include "XnShiftToDepth.h"
#include <XnStatus.h>
#include <XnLog.h>
#include <XnDataStructures.h>
#include <XnArray.h>
#include <XnBitSet.h>
#include <PSLink.h>

#define XN_MASK_LINK "xnLink"

template <typename T>
class XnArray;

namespace xn
{

class LinkPacketHeader : private XnLinkPacketHeader
{
public:
	XnStatus Validate(XnUInt32 nBytesToRead) const;
	XnBool IsMagicValid() const { return (m_nMagic == XN_LINK_MAGIC); }
	XnUInt16 GetSize() const { return m_nSize; }
	XnUInt16 GetDataSize() const { return (m_nSize - sizeof(XnLinkPacketHeader)); }
	XnUInt16 GetMsgType() const { return m_nMsgType; }
	XnLinkFragmentation GetFragmentationFlags() const { return XnLinkFragmentation(m_nFragmentation); }
	XnUInt16 GetStreamID() const { return m_nStreamID; }
	XnUInt16 GetPacketID() const { return m_nPacketID; }
	XnUInt16 GetCID() const { return m_nCID; }
	const XnUInt8* GetPacketData() const {return (reinterpret_cast<const XnUInt8*>(this) + sizeof(XnLinkPacketHeader)); }
	XnUInt8* GetPacketData() {return (reinterpret_cast<XnUInt8*>(this) + sizeof(XnLinkPacketHeader)); }

	void SetMagic() { m_nMagic = XN_LINK_MAGIC; }
	void SetSize(XnUInt16 nSize) { m_nSize = nSize; }
	void SetMsgType(XnUInt16 nMsgType) { m_nMsgType = nMsgType; }
	void SetFragmentationFlags(XnLinkFragmentation flags) { m_nFragmentation = flags; }
	void SetStreamID(XnUInt16 nStreamID) { m_nStreamID = nStreamID; }
	void SetPacketID(XnUInt16 nPacketID) { m_nPacketID = nPacketID; }
	void SetCID(XnUInt16 nCID) { m_nCID = nCID; }
};

}

XnStatus xnLinkResponseCodeToStatus(XnUInt16 nResponseCode);
const XnChar* xnLinkResponseCodeToStr(XnUInt16 nResponseCode);
const XnChar* xnFragmentationFlagsToStr(XnLinkFragmentation fragmentation);

const XnChar* xnLinkStreamTypeToString(XnStreamType streamType);
XnStreamType xnLinkStreamTypeFromString(const XnChar* strType);

const XnChar* xnLinkGestureTypeToName(XnUInt32 gestureType);
XnUInt32 xnLinkGestureNameToType(const XnChar* strGesture);

const XnChar* xnLinkPixelFormatToName(XnFwPixelFormat pixelFormat);
XnFwPixelFormat xnLinkPixelFormatFromName(const XnChar* name);
const XnChar* xnLinkCompressionToName(XnFwCompressionType compression);
XnFwCompressionType xnLinkCompressionFromName(const XnChar* name);

const XnChar* xnLinkPoseTypeToName(XnUInt32 poseType);
XnUInt32 xnLinkPoseNameToType(const XnChar* strPose);
XnStatus xnLinkPosesToNames(XnUInt32 nPoses, xnl::Array<const XnChar*>& aPosesNames);

xnl::Point3D xnLinkPoint3DToPoint3D(const XnLinkPoint3D& point);
XnLinkPoint3D XnPoint3DToLinkPoint3D(const xnl::Point3D& point);

XnLinkBoundingBox3D xnBoundingBox3DToLinkBoundingBox3D(const xnl::Box3D& box);
xnl::Box3D xnLinkBoundingBox3DToBoundingBox3D(const XnLinkBoundingBox3D& box);

XnStatus xnLinkGetStreamDumpName(XnUInt16 nStreamID, XnChar* strDumpName, XnUInt32 nDumpNameSize);
XnStatus xnLinkGetEPDumpName(XnUInt16 nEPID, XnChar* strDumpName, XnUInt32 nDumpNameSize);

XnStatus xnLinkParseIDSet(xnl::Array<xnl::BitSet>& idSet, const void* pIDSet, XnUInt32 nSize);

/*pnEncodedSize is max size on input, actual size on output. pIDs is an array of uint16 values that must be grouped by interface ID.*/
XnStatus xnLinkEncodeIDSet(void* pIDSet, XnUInt32 *pnEncodedSize, const XnUInt16* pIDs, XnUInt32 nNumIDs);

XnStatus xnLinkParseFrameSyncStreamIDs(xnl::Array<XnUInt16>& frameSyncStreamIDs, const void* pFrameSyncStreamIDs, XnUInt32 nBufferSize);
//nBufferSize is max size on input, actual size on output
XnStatus xnLinkEncodeFrameSyncStreamIDs(void* pFrameSyncStreamIDs, XnUInt32& nBufferSize, const xnl::Array<XnUInt16>& frameSyncStreamIDs);
XnStatus xnLinkParseComponentVersionsList(xnl::Array<XnComponentVersion>& componentVersions, const XnLinkComponentVersionsList* pLinkList, XnUInt32 nBufferSize);

/*
XnUInt8 xnLinkNICapabilityToInterfaceID(const XnChar* strCapabilityName);
const XnChar* xnLinkInterfaceIDToNICapability(XnUInt8 nInterfaceID);
XnProductionNodeType xnLinkStreamTypeToNINodeType(XnLinkStreamType streamType);
XnLinkStreamType xnLinkNINodeTypeToStreamType(XnProductionNodeType  nodeType);
*/
void xnLinkParseVideoMode(XnFwStreamVideoMode& videoMode, const XnLinkVideoMode& linkVideoMode);
void xnLinkEncodeVideoMode(XnLinkVideoMode& linkVideoMode, const XnFwStreamVideoMode& videoMode);

const XnChar* xnLinkPropTypeToStr(XnLinkPropType propType);

void xnLinkParseDetailedVersion(XnLinkDetailedVersion& version, const XnLinkDetailedVersion& linkVersion);

void xnLinkParseLeanVersion(XnLeanVersion& version, const XnLinkLeanVersion& linkVersion);
void xnEncodeLeanVersion(XnLinkLeanVersion& linkVersion, const XnLeanVersion& version);

/* nNumModes is max number of modes on input, actual number on output. */
XnStatus xnLinkParseSupportedVideoModes(xnl::Array<XnFwStreamVideoMode>& aModes, 
                                                const XnLinkSupportedVideoModes* pLinkSupportedModes,
                                                XnUInt32 nBufferSize);
XnStatus xnLinkParseBitSet(xnl::BitSet& bitSet, const XnLinkBitSet* pBitSet, XnUInt32 nBufferSize);
XnStatus xnLinkEncodeBitSet(XnLinkBitSet& linkBitSet, XnUInt32& nBufferSize, const xnl::BitSet& bitSet);

void xnLinkParseShiftToDepthConfig(XnShiftToDepthConfig& shiftToDepthConfig, const XnLinkShiftToDepthConfig& linkShiftToDepthConfig);

void xnLinkParseCropping(OniCropping& cropping, const XnLinkCropping& linkCropping);
void xnLinkEncodeCropping(XnLinkCropping& linkCropping, const OniCropping& cropping);

const XnChar* xnLinkGetPropName(XnLinkPropID propID);

XnStatus xnLinkValidateGeneralProp(XnLinkPropType propType, XnUInt32 nValueSize, XnUInt32 nMinSize);

template <typename T>
XnStatus xnLinkParseIntProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, T& nParsedVal)
{
    if (nValueSize < sizeof(T))
    {
        xnLogError(XN_MASK_LINK, "Property value size should be at least %u bytes, but got only %u bytes.", 
            sizeof(T), nValueSize);
        return XN_STATUS_LINK_BAD_PROP_SIZE;
    }

    if (propType != XN_LINK_PROP_TYPE_INT)
    {
        xnLogError(XN_MASK_LINK, "Property type should be %s, but got type %s", 
            xnLinkPropTypeToStr(XN_LINK_PROP_TYPE_INT), 
            xnLinkPropTypeToStr(propType));
        XN_ASSERT(FALSE);
        return XN_STATUS_LINK_BAD_PROP_TYPE;
    }

    nParsedVal = static_cast<T>(XN_PREPARE_VAR64_IN_BUFFER(*reinterpret_cast<const XnUInt64*>(pValue)));
    return XN_STATUS_OK;
}

XnStatus xnLinkParseLeanVersionProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, XnLeanVersion& leanVersion);
XnStatus xnLinkParseIDSetProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::Array<xnl::BitSet>& idSet);
XnStatus xnLinkParseBitSetProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::BitSet& bitSet);
XnStatus xnLinkParseFrameSyncStreamIDsProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::Array<XnUInt16>& streamIDs);
XnStatus xnLinkParseComponentVersionsListProp(XnLinkPropType propType, const void* pValue, XnUInt32 nValueSize, xnl::Array<XnComponentVersion>& componentVersions);

XnStatus xnLinkParseSupportedBistTests(const XnLinkSupportedBistTests* pSupportedTests, XnUInt32 nBufferSize, xnl::Array<XnBistInfo>& supportedTests);
XnStatus xnLinkParseSupportedTempList(const XnLinkTemperatureSensorsList* pSupportedList, XnUInt32 nBufferSize, xnl::Array<XnTempInfo>& supportedTempList);
XnStatus xnLinkParseGetTemperature(const XnLinkTemperatureResponse* tempResponse, XnUInt32 nBufferSize, XnCommandTemperatureResponse& tempData);
XnStatus xnLinkReadDebugData(XnCommandDebugData& commandDebugData, XnLinkDebugDataResponse* pDebugDataResponse);
XnStatus xnLinkParseSupportedI2CDevices(const XnLinkSupportedI2CDevices* pSupportedTests, XnUInt32 nBufferSize, xnl::Array<XnLinkI2CDevice>& supportedDevices);
XnStatus xnLinkParseSupportedLogFiles(const XnLinkSupportedLogFiles* pFilesList, XnUInt32 nBufferSize, xnl::Array<XnLinkLogFile>& supportedFiles);

void xnLinkParseBootStatus(XnBootStatus& bootStatus, const XnLinkBootStatus& linkBootStatus);

XnUInt32 xnLinkGetPixelSizeByStreamType(XnLinkStreamType streamType);

void xnLinkVideoModeToString(XnFwStreamVideoMode videoMode, XnChar* buffer, XnUInt32 bufferSize);
#endif // XNLINKPROTOUTILS_H
