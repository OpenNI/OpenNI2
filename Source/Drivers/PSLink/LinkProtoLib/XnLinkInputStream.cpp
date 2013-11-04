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
#include "XnLinkInputStream.h"
#include "XnLinkControlEndpoint.h"

//Link msg parsers
#include "XnLinkMsgParser.h"

namespace xn
{

LinkInputStream::LinkInputStream()
{
    Reset();
}

LinkInputStream::~LinkInputStream()
{
    Shutdown();
}

void LinkInputStream::Shutdown()
{
}

XnStatus LinkInputStream::Init(LinkControlEndpoint* pLinkControlEndpoint, 
                               XnStreamType streamType,
                               XnUInt16 nStreamID, 
                               IConnection* pConnection)
{
    XnStatus nRetVal = XN_STATUS_OK;
    XN_VALIDATE_INPUT_PTR(pLinkControlEndpoint);
    XN_VALIDATE_INPUT_PTR(pConnection);
    if (!pLinkControlEndpoint->IsConnected())
    {
        xnLogError(XN_MASK_LINK, "Link control endpoint is not connected");
        XN_ASSERT(FALSE);
        return XN_STATUS_ERROR;
    }

    m_pLinkControlEndpoint = pLinkControlEndpoint;
    m_streamType = streamType;
    m_nStreamID = nStreamID;
    m_pConnection = pConnection;
	m_streamingRefcount = 0;

    /***** Get all stream properties *****/
	nRetVal = m_pLinkControlEndpoint->GetSupportedInterfaces(nStreamID, m_supportedInterfaces);
	XN_IS_STATUS_OK_LOG_ERROR("Get stream supported interfaces", nRetVal);

	if (IsInterfaceSupported(XN_LINK_INTERFACE_MIRROR))
	{
		nRetVal = m_pLinkControlEndpoint->GetMirror(nStreamID, m_bMirror);
		XN_IS_STATUS_OK_LOG_ERROR("Get mirror", nRetVal);
	}

	switch (m_streamType)
	{
	case XN_LINK_STREAM_TYPE_SHIFTS:
		m_outputFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		break;
	case XN_LINK_STREAM_TYPE_COLOR:
		m_outputFormat = ONI_PIXEL_FORMAT_YUV422;
		break;
	case XN_LINK_STREAM_TYPE_IR:
		m_outputFormat = ONI_PIXEL_FORMAT_GRAY16;
		break;
	default:
		m_outputFormat = XN_FORMAT_PASS_THROUGH_UNPACK;
		break;
	}

    return XN_STATUS_OK;
}

void LinkInputStream::Reset()
{
    m_pLinkControlEndpoint = NULL;
    m_pConnection = NULL;
    m_nStreamID = XN_LINK_STREAM_ID_INVALID;

    m_bMirror = FALSE;
    m_streamType = XN_LINK_STREAM_TYPE_NONE;
	m_outputFormat = XN_FORMAT_PASS_THROUGH_UNPACK;
}

XnStatus LinkInputStream::Start()
{
	if(++m_streamingRefcount == 1)
	{
		return StartImpl();
	}
	return XN_STATUS_OK;
}

XnStatus LinkInputStream::Stop()
{
	if(--m_streamingRefcount == 0)
	{
		return StopImpl();
	}
	return XN_STATUS_OK;
}

XnBool LinkInputStream::IsStreaming() const
{
	return m_bStreaming;
}

XnUInt16 LinkInputStream::GetStreamID() const
{
    return m_nStreamID;
}


XnBool LinkInputStream::IsInterfaceSupported(XnUInt8 nInterfaceID) const
{
    return m_supportedInterfaces.IsSet(nInterfaceID);
}

XnBool LinkInputStream::GetMirror() const
{
    return m_bMirror;
}

XnStatus LinkInputStream::SetMirror(XnBool bMirror)
{
    XnStatus nRetVal = m_pLinkControlEndpoint->SetMirror(m_nStreamID, bMirror);
    XN_IS_STATUS_OK_LOG_ERROR("Set mirror", nRetVal);
    m_bMirror = bMirror;
    return XN_STATUS_OK;
}

LinkMsgParser* LinkInputStream::CreateLinkMsgParser()
{
	if (m_outputFormat  == XN_FORMAT_PASS_THROUGH_RAW)
	{
		return XN_NEW(LinkMsgParser);
	} else {
		xnLogError(XN_MASK_LINK, "Unknown output format: %d", m_outputFormat);
		XN_ASSERT(FALSE);
		return NULL;
	}
}

XnBool LinkInputStream::IsOutputFormatSupported(OniPixelFormat format) const
{
	return format == XN_FORMAT_PASS_THROUGH_RAW;
}

OniPixelFormat LinkInputStream::GetOutputFormat() const
{
	return m_outputFormat;
}

XnStatus LinkInputStream::SetOutputFormat(OniPixelFormat format)
{
	if (!IsOutputFormatSupported(format))
	{
		XN_ASSERT(FALSE);
		return XN_STATUS_BAD_PARAM;
	}

	if (m_bStreaming)
	{
		xnLogWarning(XN_MASK_LINK, "Can't change output format while streaming!");
		return XN_STATUS_INVALID_OPERATION;
	}

	m_outputFormat = format;

	return XN_STATUS_OK;
}

XnStatus LinkInputStream::SetGain(XnUInt16 gain)
{
	return m_pLinkControlEndpoint->SetGain(m_nStreamID, gain);
}

XnStatus LinkInputStream::GetGain(XnUInt16& gain)
{
	return m_pLinkControlEndpoint->GetGain(m_nStreamID, gain);
}

}
