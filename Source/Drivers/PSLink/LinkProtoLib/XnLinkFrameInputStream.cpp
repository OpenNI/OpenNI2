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
#include "XnLinkFrameInputStream.h"
#include "XnLinkProtoUtils.h"
#include "XnLinkControlEndpoint.h"
#include <XnLog.h>
#include <XnDump.h>

#include "XnLink12BitS2DParser.h"
#include "XnLink6BitParser.h"
#include "XnLinkPacked10BitParser.h"
#include "XnLinkUnpackedS2DParser.h"
#include "XnLink11BitS2DParser.h"
#include "XnLink16zParser.h"
#include "XnLink24zYuv422Parser.h"
#include "XnLinkYuv422ToRgb888Parser.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define XN_MASK_INPUT_STREAM "xnInputStream"

namespace xn
{

LinkFrameInputStream::LinkFrameInputStream()
{
	m_bInitialized = FALSE;
	m_defaultServices.setStream(this);
	m_pServices = &m_defaultServices;
    m_bStreaming = FALSE;
	m_pCurrFrame = NULL;
	m_nDumpFrameID = 0;

	m_frameIndex = 0;

	m_nBufferSize = 0;
	m_hCriticalSection = NULL;
	m_pDumpFile = NULL;
	m_currentFrameCorrupt = FALSE;
	xnOSCreateCriticalSection(&m_hCriticalSection);

	xnOSMemSet(&m_shiftToDepthConfig, 0, sizeof(m_shiftToDepthConfig));
	xnOSMemSet(&m_shiftToDepthTables, 0, sizeof(m_shiftToDepthTables));
}

LinkFrameInputStream::~LinkFrameInputStream()
{
    Shutdown();
	xnOSCloseCriticalSection(&m_hCriticalSection);
}

XnStatus LinkFrameInputStream::Init(LinkControlEndpoint* pLinkControlEndpoint,
                                    XnStreamType streamType,
									XnUInt16 nStreamID, 
                                    IConnection* pConnection)
{
    XnStatus nRetVal = XN_STATUS_OK;
	if (m_hCriticalSection == NULL)
	{
		xnLogError(XN_MASK_INPUT_STREAM, "Cannot initialize - critical section was not created successfully");
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	xnl::AutoCSLocker csLock(m_hCriticalSection);

	if (m_bInitialized)
	{
		//We shutdown first so we can re-initialize.
		Shutdown();
	}

    nRetVal = LinkInputStream::Init(pLinkControlEndpoint, streamType, nStreamID, pConnection);
    XN_IS_STATUS_OK_LOG_ERROR("Init base link input stream", nRetVal);
    //Now we have all the stream properties
    m_nStreamID = nStreamID;

	nRetVal = pLinkControlEndpoint->GetSupportedVideoModes(nStreamID, m_supportedVideoModes);
	XN_IS_STATUS_OK_LOG_ERROR("Get supported video modes", nRetVal);

	nRetVal = pLinkControlEndpoint->GetVideoMode(nStreamID, m_videoMode);
	XN_IS_STATUS_OK_LOG_ERROR("Get video mode", nRetVal);

	if (IsInterfaceSupported(XN_LINK_INTERFACE_CROPPING))
	{
		nRetVal = pLinkControlEndpoint->GetCropping(nStreamID, m_cropping);
		XN_IS_STATUS_OK_LOG_ERROR("Get cropping", nRetVal);
	}

	nRetVal = UpdateCameraIntrinsics();
	XN_IS_STATUS_OK_LOG_ERROR("Update Camera Intrinsics", nRetVal);

	// if needed, build shift-to-depth tables
	if (streamType == XN_LINK_STREAM_TYPE_SHIFTS)
	{
		nRetVal = pLinkControlEndpoint->GetShiftToDepthConfig(nStreamID, m_shiftToDepthConfig);
		XN_IS_STATUS_OK_LOG_ERROR("Get S2D config", nRetVal);

		// construct tables
		nRetVal = XnShiftToDepthInit(&m_shiftToDepthTables, &m_shiftToDepthConfig);
		XN_IS_STATUS_OK_LOG_ERROR("Init shift to depth tables", nRetVal);
	}

	nRetVal = xnLinkGetStreamDumpName(m_nStreamID, m_strDumpName, sizeof(m_strDumpName));
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogWarning("Failed to get stream dump name: %s", xnGetStatusString(nRetVal));
		XN_ASSERT(FALSE);
	}

	if (m_hCriticalSection == NULL)
	{
		nRetVal = xnOSCreateCriticalSection(&m_hCriticalSection);
		XN_IS_STATUS_OK_LOG_ERROR("Create critical section", nRetVal);
	}

	m_nDumpFrameID = 1;
	m_bInitialized = TRUE;

	return XN_STATUS_OK;
}

void LinkFrameInputStream::Reset()
{
	xnOSMemSet(&m_videoMode, 0, sizeof(m_videoMode));
	xnOSMemSet(&m_cropping, 0, sizeof(m_cropping));
	LinkInputStream::Reset();
}

XnBool LinkFrameInputStream::IsInitialized() const
{
	return m_bInitialized;
}

void LinkFrameInputStream::Shutdown()
{
	if (!m_bInitialized)
		return;

	xnOSEnterCriticalSection(&m_hCriticalSection);
    Stop();

	if (m_pCurrFrame != NULL)
	{
		m_pServices->releaseFrame(m_pCurrFrame);
		m_pCurrFrame = NULL;
	}

	XnShiftToDepthFree(&m_shiftToDepthTables);

	xnDumpFileClose(m_pDumpFile);
    LinkInputStream::Shutdown();
    m_bInitialized = FALSE;
	xnOSLeaveCriticalSection(&m_hCriticalSection);
}

XnStatus LinkFrameInputStream::HandlePacket(const LinkPacketHeader& origHeader, const XnUInt8* pData, XnBool& bPacketLoss)
{
	XnStatus nRetVal = XN_STATUS_OK;
	xnl::AutoCSLocker csLock(m_hCriticalSection);
	if (!m_bInitialized)
	{
		return XN_STATUS_NOT_INIT;
	}

	LinkPacketHeader header = origHeader;

    if (header.GetFragmentationFlags() & XN_LINK_FRAG_BEGIN)
    {
		bPacketLoss = FALSE;

        xnDumpFileClose(m_pDumpFile); //In case we didn't get an END packet before
        m_pDumpFile = xnDumpFileOpen(m_strDumpName, "%s.%05u.raw", m_strDumpName, m_nDumpFrameID++);
		m_currentFrameCorrupt = FALSE;

		// acquire a frame
		if (m_pCurrFrame == NULL)
		{
			m_pCurrFrame = m_pServices->acquireFrame();
			if (m_pCurrFrame == NULL)
			{
				xnLogError(XN_MASK_LINK, "Failed to acquire frame. Stream can't function!");
				return XN_STATUS_ALLOC_FAILED;
			}
		}

		// take timestamp
		if (header.GetDataSize() < sizeof(XnUInt64))
		{
			m_currentFrameCorrupt = TRUE;
			xnLogWarning(XN_MASK_LINK, "Got a BEGIN packet with no timestamp!");
			XN_ASSERT(FALSE);
			return XN_STATUS_LINK_MISSING_TIMESTAMP;
		}
		m_pCurrFrame->timestamp = *(XnUInt64*)pData;
		pData += sizeof(XnUInt64);
		header.SetSize(header.GetSize() - sizeof(XnUInt64));

        // TEMP: inject the host's timestamp. Firmware can't produce timestamps yet
		XnUInt64 nTimestamp;
        nRetVal = xnOSGetHighResTimeStamp(&nTimestamp);
        if (nRetVal != XN_STATUS_OK)
        {
            xnLogWarning(XN_MASK_LINK, "Failed to get timestamp from os: %s", xnGetStatusString(nRetVal));
            XN_ASSERT(FALSE);
        }
		m_pCurrFrame->timestamp = nTimestamp;

		// begin parsing frame
		nRetVal = m_pLinkMsgParser->BeginParsing(m_pCurrFrame->data, m_nBufferSize);
		XN_IS_STATUS_OK_LOG_ERROR("Begin parsing link frame msg", nRetVal);
	}
	else if (bPacketLoss)
	{
		m_currentFrameCorrupt = TRUE;
	}

	if (!m_currentFrameCorrupt)
	{
		XnUInt32 nPrevSize = m_pLinkMsgParser->GetParsedSize();
		nRetVal = m_pLinkMsgParser->ParsePacket(header, pData);
		if (nRetVal != XN_STATUS_OK)
		{
			m_currentFrameCorrupt = TRUE;
			if (nRetVal != XN_STATUS_OK)
			{
				return nRetVal;
			}
		}

		//Write new data to dump (if it's on)
		xnDumpFileWriteBuffer(m_pDumpFile, 
			reinterpret_cast<const XnUInt8*>(m_pLinkMsgParser->GetParsedData()) + nPrevSize, 
			m_pLinkMsgParser->GetParsedSize() - nPrevSize);
	}

	if (header.GetFragmentationFlags() & XN_LINK_FRAG_END)
	{
		/* Yay, we now have a full message. */
		xnDumpFileClose(m_pDumpFile);

		if (m_pLinkMsgParser->GetParsedSize() != CalcExpectedSize())
		{
			m_currentFrameCorrupt = TRUE;
			xnLogWarning(XN_MASK_LINK, "Received bad frame. Expected Size: %u, Actual Size: %u", CalcExpectedSize(), m_pLinkMsgParser->GetParsedSize());
		}

		if (!m_currentFrameCorrupt)
		{
			//Save actual size of data in working buffer info
			m_pCurrFrame->dataSize = m_pLinkMsgParser->GetParsedSize();
			m_pCurrFrame->frameIndex            = ++m_frameIndex;
			m_pCurrFrame->croppingEnabled       = m_cropping.enabled;
			if (m_cropping.enabled)
			{
				m_pCurrFrame->width             = m_cropping.width;
				m_pCurrFrame->height            = m_cropping.height;
				m_pCurrFrame->cropOriginX       = m_cropping.originX;
				m_pCurrFrame->cropOriginY       = m_cropping.originY;
			} else {
				m_pCurrFrame->width             = m_videoMode.m_nXRes;
				m_pCurrFrame->height            = m_videoMode.m_nYRes;
				m_pCurrFrame->cropOriginX       = 0;
				m_pCurrFrame->cropOriginY       = 0;
			}
			m_pCurrFrame->stride                = m_pCurrFrame->width * GetOutputBytesPerPixel();
			m_pCurrFrame->videoMode.fps         = m_videoMode.m_nFPS;
			m_pCurrFrame->videoMode.pixelFormat = m_outputFormat;
			m_pCurrFrame->videoMode.resolutionX = m_videoMode.m_nXRes;
			m_pCurrFrame->videoMode.resolutionY = m_videoMode.m_nYRes;

			switch (m_streamType)
			{
			case XN_LINK_STREAM_TYPE_SHIFTS:	m_pCurrFrame->sensorType = ONI_SENSOR_DEPTH; break;
			case XN_LINK_STREAM_TYPE_COLOR:		m_pCurrFrame->sensorType = ONI_SENSOR_COLOR; break;
			case XN_LINK_STREAM_TYPE_IR:		m_pCurrFrame->sensorType = ONI_SENSOR_IR;    break;
			}

			NewFrameEventArgs args;
			args.pFrame = m_pCurrFrame;
			nRetVal = m_newFrameEvent.Raise(args);
			m_pServices->releaseFrame(m_pCurrFrame);
			m_pCurrFrame = NULL;
			XN_IS_STATUS_OK_LOG_ERROR("Raise new frame event", nRetVal);
		}
	}

	return XN_STATUS_OK;
}

XnStatus LinkFrameInputStream::StartImpl()
{
    XnStatus nRetVal = XN_STATUS_OK;

    if (m_bStreaming)
    {
        //Already streaming
        return XN_STATUS_OK;
    }

    //Allocate buffers
    m_nBufferSize = CalcBufferSize();
    if (m_nBufferSize == 0)
    {
        xnLogError(XN_MASK_LINK, "Failed to calculate buffer size for stream of type %u", m_streamType);
        XN_ASSERT(FALSE);
        return XN_STATUS_ERROR;
    }
    xnLogVerbose(XN_MASK_LINK, "Stream %u calculated buffer size: %u", m_nStreamID, m_nBufferSize);

    //Prepare parser
	m_pLinkMsgParser = CreateLinkMsgParser();
	XN_VALIDATE_ALLOC_PTR(m_pLinkMsgParser);
    nRetVal = m_pLinkMsgParser->Init();
    XN_IS_STATUS_OK_LOG_ERROR("Init link msg parser", nRetVal);
    //TODO: Delete LinkMsgParser and buffers on each of these errors
    
    //We must set the streaming flag first cuz the data handler checks it
    m_bStreaming = TRUE;
    //Connect to input connection
    nRetVal = m_pConnection->Connect();
    if (nRetVal != XN_STATUS_OK)
    {
        m_bStreaming = FALSE;
        xnLogError(XN_MASK_LINK, "Failed to connect stream's input connection: %s", xnGetStatusString(nRetVal));
        XN_ASSERT(FALSE);
        return nRetVal;       
    }

    //Now send command to device
    nRetVal = m_pLinkControlEndpoint->StartStreaming(m_nStreamID);
    XN_IS_STATUS_OK_LOG_ERROR("Connect stream's input connection", nRetVal);
    if (nRetVal != XN_STATUS_OK)
    {
        m_bStreaming = FALSE;
        xnLogError(XN_MASK_LINK, "Failed to start streaming: %s", xnGetStatusString(nRetVal));
        XN_ASSERT(FALSE);
        return nRetVal;       
    }

    return XN_STATUS_OK;
}

XnStatus LinkFrameInputStream::StopImpl()
{
    XnStatus nRetVal = XN_STATUS_OK;
    if (!m_bStreaming)
    {
        return XN_STATUS_OK;
    }

    m_pLinkControlEndpoint->StopStreaming(m_nStreamID);
    XN_IS_STATUS_OK_LOG_ERROR("Stop streaming", nRetVal);
    m_pConnection->Disconnect();

    if (m_pLinkMsgParser != NULL)
    {
        m_pLinkMsgParser->Shutdown();
        XN_DELETE(m_pLinkMsgParser);
        m_pLinkMsgParser = NULL;
    }

    //Free curr buffer if we still hold it
	if (m_pCurrFrame != NULL)
	{
		m_pServices->releaseFrame(m_pCurrFrame);
		m_pCurrFrame = NULL;
	}
        
    m_bStreaming = FALSE;

    return XN_STATUS_OK;
}

void LinkFrameInputStream::SetDumpName(const XnChar* /*strDumpName*/)
{
    //Not implemented for frame input stream
    XN_ASSERT(FALSE);
}

void LinkFrameInputStream::SetDumpOn(XnBool bDumpOn)
{
    XnStatus nRetVal = XN_STATUS_OK;
    (void)nRetVal;

    nRetVal = xnDumpSetMaskState(m_strDumpName, bDumpOn);
    if (nRetVal != XN_STATUS_OK)
    {
        xnLogWarning(XN_MASK_INPUT_STREAM, "Failed to set dump state: %s", xnGetStatusString(nRetVal));
        XN_ASSERT(FALSE);
    }
}

void LinkFrameInputStream::Swap(XnUInt32& nVal1, XnUInt32& nVal2)
{
	XnUInt32 nTemp = nVal1;
	nVal1 = nVal2;
	nVal2 = nTemp;
}

XnUInt32 LinkFrameInputStream::GetOutputBytesPerPixel() const
{
	if (m_outputFormat == XN_FORMAT_PASS_THROUGH_RAW    ||
        m_outputFormat == XN_FORMAT_PASS_THROUGH_UNPACK   )
	{
		return xnLinkGetPixelSizeByStreamType(XnLinkStreamType(m_streamType));
	}
	switch (m_outputFormat)
	{
	case ONI_PIXEL_FORMAT_DEPTH_1_MM:
	case ONI_PIXEL_FORMAT_DEPTH_100_UM:
		return sizeof(OniDepthPixel);
	case ONI_PIXEL_FORMAT_YUV422:
		return sizeof(OniYUV422DoublePixel)/2;
	case ONI_PIXEL_FORMAT_RGB888:
		return sizeof(OniRGB888Pixel);
	case ONI_PIXEL_FORMAT_GRAY16:
		return sizeof(OniGrayscale16Pixel);
	default:
		XN_ASSERT(FALSE);
		xnLogError(XN_MASK_LINK, "Unknown output format!");
		return 0;
	}
}

XnUInt32 LinkFrameInputStream::CalcBufferSize() const
{
    XnUInt32 nPixelSize = 0;

	if (IsInterfaceSupported(XN_LINK_INTERFACE_MAP_GENERATOR))
	{
		//Stream is map based - calculate size according to current resolution.
		nPixelSize = GetOutputBytesPerPixel();
		if (nPixelSize == 0)
		{
			//This means we got a bad stream type
			return 0;
		}

		//64 bits for timestamp, then space for data according to resolution and pixel width
		return (nPixelSize * m_videoMode.m_nXRes * m_videoMode.m_nYRes);
	}
	else
	{
		// TEMP: replace this with a value taken from firmware
		return (10 * 1024);
	}
}

XnUInt32 LinkFrameInputStream::CalcExpectedSize() const
{
	XnUInt32 nPixelSize = 0;

	if (IsInterfaceSupported(XN_LINK_INTERFACE_MAP_GENERATOR))
	{
		//Stream is map based - calculate size according to current resolution.
		nPixelSize = GetOutputBytesPerPixel();
		if (nPixelSize == 0)
		{
			//This means we got a bad stream type
			return 0;
		}

		if (m_cropping.enabled)
		{
			return (nPixelSize * m_cropping.width * m_cropping.height);
		}
		else
		{
			return (nPixelSize * m_videoMode.m_nXRes * m_videoMode.m_nYRes);
		}
	}
	else
	{
		return (0);
	}
}

XnBool LinkFrameInputStream::IsOutputFormatSupported(OniPixelFormat format) const
{
	if (format == XN_FORMAT_PASS_THROUGH_RAW   ||
        format == XN_FORMAT_PASS_THROUGH_UNPACK  )
	{
		return TRUE;
	}
	switch (format)
	{
	case ONI_PIXEL_FORMAT_DEPTH_1_MM:
	case ONI_PIXEL_FORMAT_DEPTH_100_UM:
		return (m_streamType == XN_LINK_STREAM_TYPE_SHIFTS);
	case ONI_PIXEL_FORMAT_GRAY16:
		return (m_streamType == XN_LINK_STREAM_TYPE_IR) && (m_videoMode.m_nPixelFormat == XN_FW_PIXEL_FORMAT_GRAYSCALE16);
	default:
		return LinkInputStream::IsOutputFormatSupported(format);
	}
}

const xnl::Array<XnFwStreamVideoMode>& LinkFrameInputStream::GetSupportedVideoModes() const
{
	return m_supportedVideoModes;
}

const XnFwStreamVideoMode& LinkFrameInputStream::GetVideoMode() const
{
	return m_videoMode;
}

XnStatus LinkFrameInputStream::SetVideoMode(const XnFwStreamVideoMode& videoMode)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnBool bModeSupported = FALSE;

	XnChar strVideoMode[100];
	xnLinkVideoModeToString(videoMode, strVideoMode, sizeof(strVideoMode));

	xnLogVerbose(XN_MASK_LINK, "Stream %u - Setting video mode %s...", m_nStreamID, strVideoMode);

	for (XnUInt32 i = 0; i < m_supportedVideoModes.GetSize() && !bModeSupported; i++)
	{
		if (xnOSMemCmp(&videoMode, &m_supportedVideoModes[i], sizeof(videoMode)) == 0)
		{
			//Found our supported mode
			bModeSupported = TRUE;
		}
	}

	if (!bModeSupported)
	{
		xnLogError(XN_MASK_LINK, "Tried to set unsupported mode: %s", strVideoMode);
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}

	nRetVal = m_pLinkControlEndpoint->SetVideoMode(m_nStreamID, videoMode);
	XN_IS_STATUS_OK_LOG_ERROR("Set map output mode", nRetVal);

	m_videoMode = videoMode;

	nRetVal = UpdateCameraIntrinsics();
	XN_IS_STATUS_OK_LOG_ERROR("Update Camera Intrinsics", nRetVal);
	
	// if needed, build shift-to-depth tables
	if (m_streamType == XN_LINK_STREAM_TYPE_SHIFTS)
	{
		nRetVal = m_pLinkControlEndpoint->GetShiftToDepthConfig(m_nStreamID, m_shiftToDepthConfig);
		
		if (m_outputFormat == ONI_PIXEL_FORMAT_DEPTH_100_UM)
		{
			m_shiftToDepthConfig.nDeviceMaxDepthValue = XN_MIN(m_shiftToDepthConfig.nDeviceMaxDepthValue * 10, XN_MAX_UINT16);
			m_shiftToDepthConfig.nDepthMaxCutOff = XN_MIN(m_shiftToDepthConfig.nDepthMaxCutOff * 10, XN_MAX_UINT16);
			m_shiftToDepthConfig.dDepthScale = 10.0;

			nRetVal = XnShiftToDepthInit(&m_shiftToDepthTables, &m_shiftToDepthConfig);
			XN_IS_STATUS_OK_LOG_ERROR("Init shift to depth tables", nRetVal);
		}

		XN_IS_STATUS_OK(nRetVal);

		// construct tables
		nRetVal = XnShiftToDepthUpdate(&m_shiftToDepthTables, &m_shiftToDepthConfig);
		XN_IS_STATUS_OK_LOG_ERROR("update shift to depth tables", nRetVal);
	}

	return XN_STATUS_OK;
}

const XnShiftToDepthConfig& LinkFrameInputStream::GetShiftToDepthConfig() const
{
	return m_shiftToDepthConfig;
}

XnStatus LinkFrameInputStream::GetShiftToDepthTables(const XnShiftToDepthTables*& pTables) const
{
	if (!m_shiftToDepthTables.bIsInitialized)
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	pTables = &m_shiftToDepthTables;

	return XN_STATUS_OK;
}

const OniCropping& LinkFrameInputStream::GetCropping() const
{
	return m_cropping;
}

XnStatus LinkFrameInputStream::SetCropping(OniCropping cropping)
{
	// validate
	if (cropping.enabled)
	{
		if ((XnUInt32(cropping.originX + cropping.width)  > m_videoMode.m_nXRes) || 
			(XnUInt32(cropping.originY + cropping.height) > m_videoMode.m_nYRes))
		{
			xnLogWarning(XN_MASK_LINK, "cropping window is out of full resolution");
			return XN_STATUS_BAD_PARAM;
		}
	}

	XnStatus nRetVal = m_pLinkControlEndpoint->SetCropping(m_nStreamID, cropping);
	XN_IS_STATUS_OK_LOG_ERROR("Set cropping", nRetVal);
	m_cropping = cropping;
	return XN_STATUS_OK;
}

LinkMsgParser* LinkFrameInputStream::CreateLinkMsgParser()
{
	OniPixelFormat outputFormat = m_outputFormat;
	XnFwPixelFormat pixelFormat = m_videoMode.m_nPixelFormat;
	XnFwCompressionType compression = m_videoMode.m_nCompression;

	// TODO: validate this is a depth stream if format requires S2D

	if (outputFormat == XN_FORMAT_PASS_THROUGH_RAW)
	{
		return XN_NEW(LinkMsgParser);
	} else if (outputFormat == XN_FORMAT_PASS_THROUGH_UNPACK)
	{
		switch (compression)
		{
		case XN_FW_COMPRESSION_NONE:
			return XN_NEW(LinkMsgParser);
		case XN_FW_COMPRESSION_6_BIT_PACKED:
			return XN_NEW(Link6BitParser);
		case XN_FW_COMPRESSION_10_BIT_PACKED:
			return XN_NEW(LinkPacked10BitParser);
		case XN_FW_COMPRESSION_16Z:
			return XN_NEW(Link16zParser<false>, m_shiftToDepthTables);
		case XN_FW_COMPRESSION_24Z:
			return XN_NEW(Link24zYuv422Parser, m_videoMode.m_nXRes, m_videoMode.m_nYRes, FALSE);
		default:
			xnLogError(XN_MASK_LINK, "Unknown compression for pass-through: %d", compression);
			XN_ASSERT(FALSE);
			return NULL;
		}
	}
	switch (outputFormat)
	{
	case ONI_PIXEL_FORMAT_DEPTH_100_UM:
	case ONI_PIXEL_FORMAT_DEPTH_1_MM:
		{
			if (pixelFormat != XN_FW_PIXEL_FORMAT_SHIFTS_9_3)
			{
				xnLogError(XN_MASK_LINK, "Cannot convert from pixel format %d to depth!", pixelFormat);
				XN_ASSERT(pixelFormat == XN_FW_PIXEL_FORMAT_SHIFTS_9_3);
				return NULL;
			}

			switch (compression)
			{
			case XN_FW_COMPRESSION_NONE:
				return XN_NEW(LinkUnpackedS2DParser, m_shiftToDepthTables);
			case XN_FW_COMPRESSION_11_BIT_PACKED:
				return XN_NEW(Link11BitS2DParser, m_shiftToDepthTables);
			case XN_FW_COMPRESSION_12_BIT_PACKED:
				return XN_NEW(Link12BitS2DParser, m_shiftToDepthTables);
			case XN_FW_COMPRESSION_16Z:
				return XN_NEW(Link16zParser<true>, m_shiftToDepthTables);
			default:
				xnLogError(XN_MASK_LINK, "Unknown compression for shifts: %d", compression);
				XN_ASSERT(FALSE);
				return NULL;
			}
		}
	case ONI_PIXEL_FORMAT_YUV422:
		{
			if (pixelFormat != XN_FW_PIXEL_FORMAT_YUV422)
			{
				xnLogError(XN_MASK_LINK, "Cannot convert from pixel format %d to YUV422!", pixelFormat);
				XN_ASSERT(pixelFormat == XN_FW_PIXEL_FORMAT_YUV422);
				return NULL;
			}

			switch (compression)
			{
			case XN_FW_COMPRESSION_NONE:
				return XN_NEW(LinkMsgParser);
			case XN_FW_COMPRESSION_24Z:
				return XN_NEW(Link24zYuv422Parser, m_videoMode.m_nXRes, m_videoMode.m_nYRes, FALSE);
			default:
				xnLogError(XN_MASK_LINK, "Unknown compression YUV422: %d", compression);
				XN_ASSERT(FALSE);
				return NULL;
			}
		}
	case ONI_PIXEL_FORMAT_RGB888:
		{
			if (pixelFormat == XN_FW_PIXEL_FORMAT_YUV422)
			{
				switch (compression)
				{
				case XN_FW_COMPRESSION_NONE:
					return XN_NEW(LinkYuv422ToRgb888Parser);
				case XN_FW_COMPRESSION_24Z:
					return XN_NEW(Link24zYuv422Parser, m_videoMode.m_nXRes, m_videoMode.m_nYRes, TRUE);
				default:
					xnLogError(XN_MASK_LINK, "Unknown compression YUV422: %d", compression);
					XN_ASSERT(FALSE);
					return NULL;
				}
			}
			else if (pixelFormat == XN_FW_PIXEL_FORMAT_BAYER8)
			{
				xnLogError(XN_MASK_LINK, "Bayer to RGB888 conversion is not supported yet");
				XN_ASSERT(FALSE);
				return NULL;
			}
		}
	case ONI_PIXEL_FORMAT_GRAY16:
		switch (compression)
		{
		case XN_FW_COMPRESSION_NONE:
			return XN_NEW(LinkMsgParser);
		case XN_FW_COMPRESSION_10_BIT_PACKED:
			return XN_NEW(LinkPacked10BitParser);
		default:
			xnLogError(XN_MASK_LINK, "Unknown compression for grey16: %d", compression);
			XN_ASSERT(FALSE);
			return NULL;
		}
	default:
		xnLogError(XN_MASK_LINK, "Unknown output format: %d", outputFormat);
		XN_ASSERT(FALSE);
		return NULL;
	}
}

XnStatus LinkFrameInputStream::UpdateCameraIntrinsics()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_pLinkControlEndpoint->GetCameraIntrinsics(m_nStreamID, m_cameraIntrinsics);
	XN_IS_STATUS_OK_LOG_ERROR("Get Camera Intrinsics", nRetVal);

	m_fHFOV = (XnFloat)(2 * atan(m_videoMode.m_nXRes / 2. / m_cameraIntrinsics.m_fEffectiveFocalLengthInPixels));
	m_fVFOV = (XnFloat)(2 * atan(m_videoMode.m_nYRes / 2. / m_cameraIntrinsics.m_fEffectiveFocalLengthInPixels));

	xnLogVerbose(XN_MASK_LINK, "Stream %u intrinsics - EFL: %.2f, Optic Center: (%u,%u), Field-of-View: %.1fx%.1f",
		m_nStreamID,
		m_cameraIntrinsics.m_fEffectiveFocalLengthInPixels,
		m_cameraIntrinsics.m_nOpticalCenterX,
		m_cameraIntrinsics.m_nOpticalCenterY,
		m_fHFOV*180/M_PI, 
		m_fVFOV*180/M_PI);

	return (XN_STATUS_OK);
}


LinkFrameInputStream::DefaultStreamServices::DefaultStreamServices()
{
	OniStreamServices::getDefaultRequiredFrameSize = getDefaultRequiredFrameSizeCallback;
	OniStreamServices::acquireFrame = acquireFrameCallback;
	OniStreamServices::addFrameRef = addFrameRefCallback;
	OniStreamServices::releaseFrame = releaseFrameCallback;
}

void LinkFrameInputStream::DefaultStreamServices::setStream(LinkFrameInputStream* pStream)
{
	OniStreamServices::streamServices = pStream;
}

int ONI_CALLBACK_TYPE LinkFrameInputStream::DefaultStreamServices::getDefaultRequiredFrameSizeCallback(void* streamServices)
{
	LinkFrameInputStream* pThis = (LinkFrameInputStream*)streamServices;
	return pThis->GetRequiredFrameSize();
}

OniFrame* ONI_CALLBACK_TYPE LinkFrameInputStream::DefaultStreamServices::acquireFrameCallback(void* streamServices)
{
	LinkFrameInputStream* pThis = (LinkFrameInputStream*)streamServices;
	LinkOniFrame* pFrame = XN_NEW(LinkOniFrame);
	if (pFrame == NULL)
	{
		return NULL;
	}

	pFrame->refCount = 1;
	pFrame->dataSize = pThis->GetRequiredFrameSize();
	pFrame->data = xnOSMallocAligned(pFrame->dataSize, XN_DEFAULT_MEM_ALIGN);
	if (pFrame->data == NULL)
	{
		XN_DELETE(pFrame);
		return NULL;
	}

	return pFrame;
}

void ONI_CALLBACK_TYPE LinkFrameInputStream::DefaultStreamServices::addFrameRefCallback(void* /*streamServices*/, OniFrame* pFrame)
{
	LinkOniFrame* pLinkFrame = (LinkOniFrame*)pFrame;
	++pLinkFrame->refCount;
}

void ONI_CALLBACK_TYPE LinkFrameInputStream::DefaultStreamServices::releaseFrameCallback(void* /*streamServices*/, OniFrame* pFrame)
{
	LinkOniFrame* pLinkFrame = (LinkOniFrame*)pFrame;
	if (--pLinkFrame->refCount == 0)
	{
		xnOSFreeAligned(pLinkFrame->data);
		XN_DELETE(pLinkFrame);
	}
}

}

