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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnPixelStream.h"
#include <XnLog.h>
#include <Formats/XnFormats.h>
#include <XnOS.h>
#include <XnCore.h>

//---------------------------------------------------------------------------
// XnPixelStream
//---------------------------------------------------------------------------
XnPixelStream::XnPixelStream(const XnChar* csType, const XnChar* csName, XnBool bAllowCustomResolutions) :
	XnFrameStream(csType, csName),
	m_IsPixelStream(XN_STREAM_PROPERTY_IS_PIXEL_BASED, "IsPixelBased", TRUE),
	m_Resolution(XN_STREAM_PROPERTY_RESOLUTION, "Resolution", XN_RESOLUTION_VGA),
	m_XRes(XN_STREAM_PROPERTY_X_RES, "XRes", XN_VGA_X_RES),
	m_YRes(XN_STREAM_PROPERTY_Y_RES, "YRes", XN_VGA_Y_RES),
	m_BytesPerPixel(XN_STREAM_PROPERTY_BYTES_PER_PIXEL, "BytesPerPixel"),
	m_Cropping(XN_STREAM_PROPERTY_CROPPING, "Cropping", &m_CroppingData, sizeof(OniCropping), ReadCroppingFromFileCallback),
	m_SupportedModesCount(XN_STREAM_PROPERTY_SUPPORT_MODES_COUNT, "SupportedModesCount", 0),
	m_SupportedModes(XN_STREAM_PROPERTY_SUPPORT_MODES, "SupportedModes"),
	m_supportedModesData(30),
	m_bAllowCustomResolutions(bAllowCustomResolutions)
{
	xnOSMemSet(&m_CroppingData, 0, sizeof(OniCropping));
	m_SupportedModes.UpdateGetCallback(GetSupportedModesCallback, this);
}

XnStatus XnPixelStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// init base
	nRetVal = XnFrameStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	// update set callbacks
	m_Resolution.UpdateSetCallback(SetResolutionCallback, this);
	m_XRes.UpdateSetCallback(SetXResCallback, this);
	m_YRes.UpdateSetCallback(SetYResCallback, this);
	m_Cropping.UpdateSetCallback(SetCroppingCallback, this);

	// add properties
	XN_VALIDATE_ADD_PROPERTIES(this, &m_IsPixelStream, &m_Resolution, &m_XRes, &m_YRes, 
		&m_BytesPerPixel, &m_Cropping, &m_SupportedModesCount, &m_SupportedModes);

	// register required size properties
	nRetVal = RegisterRequiredSizeProperty(&m_XRes);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = RegisterRequiredSizeProperty(&m_YRes);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = RegisterRequiredSizeProperty(&m_BytesPerPixel);
	XN_IS_STATUS_OK(nRetVal);

	// register for important properties
	XnCallbackHandle hDummyCallback;
	nRetVal = m_Resolution.OnChangeEvent().Register(ResolutionValueChangedCallback, this, hDummyCallback);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = OutputFormatProperty().OnChangeEvent().Register(OutputFormatValueChangedCallback, this, hDummyCallback);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_XRes.OnChangeEvent().Register(FixCroppingCallback, this, hDummyCallback);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_YRes.OnChangeEvent().Register(FixCroppingCallback, this, hDummyCallback);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::AddSupportedModes(XnCmosPreset* aPresets, XnUInt32 nCount)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_supportedModesData.AddLast(aPresets, nCount);
	XN_IS_STATUS_OK(nRetVal);

	// update our general property
	XnUInt32 nAllPresetsCount = m_supportedModesData.GetSize();

	nRetVal = m_SupportedModesCount.UnsafeUpdateValue(nAllPresetsCount);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::ValidateSupportedMode(const XnCmosPreset& preset)
{
	for (XnUInt32 i = 0; i < m_supportedModesData.GetSize(); ++i)
	{
		if (preset.nFormat == m_supportedModesData[i].nFormat &&
			preset.nResolution == m_supportedModesData[i].nResolution && 
			preset.nFPS == m_supportedModesData[i].nFPS)
		{
			return (XN_STATUS_OK);
		}
	}

	XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DDK, "Mode is not supported (format: %d, resolution: %d, FPS: %d)!", preset.nFormat, preset.nResolution, preset.nFPS);
}

XnStatus XnPixelStream::GetSupportedModes(XnCmosPreset* aPresets, XnUInt32& nCount)
{
	if (nCount < m_supportedModesData.GetSize())
	{
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	xnOSMemCopy(aPresets, m_supportedModesData.GetData(), m_supportedModesData.GetSize() * sizeof(XnCmosPreset));
	return XN_STATUS_OK;
}

XnStatus XnPixelStream::SetResolution(XnResolutions nResolution)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_Resolution.UnsafeUpdateValue(nResolution);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::SetXRes(XnUInt32 nXRes)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnResolutions res = XnDDKGetResolutionFromXY(nXRes, GetYRes());

	// set resolution (this will also set X and Y resolution)
	nRetVal = SetResolution(res);
	XN_IS_STATUS_OK(nRetVal);

	if (res == XN_RESOLUTION_CUSTOM)
	{
		// update X res ourselves
		nRetVal = m_XRes.UnsafeUpdateValue(nXRes);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::SetYRes(XnUInt32 nYRes)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnResolutions res = XnDDKGetResolutionFromXY(GetXRes(), nYRes);

	// set resolution (this will also set X and Y resolution)
	nRetVal = SetResolution(res);
	XN_IS_STATUS_OK(nRetVal);

	if (res == XN_RESOLUTION_CUSTOM)
	{
		// update Y res ourselves
		nRetVal = m_YRes.UnsafeUpdateValue(nYRes);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::SetCropping(const OniCropping* pCropping)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = ValidateCropping(pCropping);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Cropping.UnsafeUpdateValue(XN_PACK_GENERAL_BUFFER(*(OniCropping*)pCropping));
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::ValidateCropping(const OniCropping* pCropping)
{
	if (pCropping->enabled)
	{
		if (pCropping->originX > (int)GetXRes() ||
			XnUInt32(pCropping->originX + pCropping->width) > GetXRes() ||
			pCropping->originY > (int)GetYRes() ||
			XnUInt32(pCropping->originY + pCropping->height) > GetYRes())
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DDK, "Cropping values do not match stream resolution!");
		}

		if (pCropping->width == 0 || pCropping->height == 0)
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DDK, "Cannot set a cropping window of zero size!");
		}
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::OnResolutionChanged()
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnResolutions res = (XnResolutions)m_Resolution.GetValue();
	if (res != XN_RESOLUTION_CUSTOM)
	{
		// update XRes and YRes accordingly
		XnUInt32 nXRes;
		XnUInt32 nYRes;
		if (!XnDDKGetXYFromResolution(res, &nXRes, &nYRes))
		{
			XN_ASSERT(FALSE);
		}

		nRetVal = m_XRes.UnsafeUpdateValue(nXRes);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = m_YRes.UnsafeUpdateValue(nYRes);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::OnOutputFormatChanged()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// update the bytes-per-pixel value
	XnUInt32 nBytesPerPixel;

	switch (GetOutputFormat())
	{
	case ONI_PIXEL_FORMAT_SHIFT_9_2:
		nBytesPerPixel = sizeof(XnUInt16);
		break;
	case ONI_PIXEL_FORMAT_DEPTH_1_MM:
	case ONI_PIXEL_FORMAT_DEPTH_100_UM:
		nBytesPerPixel = sizeof(OniDepthPixel);
		break;
	case ONI_PIXEL_FORMAT_GRAY8:
		nBytesPerPixel = sizeof(XnUInt8);
		break;
	case ONI_PIXEL_FORMAT_GRAY16:
		nBytesPerPixel = sizeof(XnUInt16);
		break;
	case ONI_PIXEL_FORMAT_YUV422:
	case ONI_PIXEL_FORMAT_YUYV:
		// YUV422 is actually 4 bytes for every 2 pixels
		nBytesPerPixel = sizeof(XnUChar) * 2;
		break;
	case ONI_PIXEL_FORMAT_RGB888:
		nBytesPerPixel = sizeof(XnUChar) * 3;
		break;
	case ONI_PIXEL_FORMAT_JPEG:
		// size is unknown.
		nBytesPerPixel = 1;
		break;
	default:
		return (XN_STATUS_DEVICE_BAD_PARAM);
	}

	nRetVal = m_BytesPerPixel.UnsafeUpdateValue(nBytesPerPixel);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::FixCropping()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	OniCropping cropping = *GetCropping();
	if (cropping.originX > (int)GetXRes() || 
		cropping.originY > (int)GetYRes() ||
		(cropping.originX + cropping.width) > (int)GetXRes() ||
		(cropping.originY + cropping.height) > (int)GetYRes())
	{
		// disable it
		cropping.enabled = FALSE;
		nRetVal = SetCropping(&cropping);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnPixelStream::CalcRequiredSize(XnUInt32* pnRequiredSize) const
{
	*pnRequiredSize = GetXRes() * GetYRes() * GetBytesPerPixel();
	return XN_STATUS_OK;
}


void XnPixelStream::NewDataAvailable(OniFrame* pFrame)
{
	// crop
	xnOSEnterCriticalSection(GetLock());
	OniCropping cropping = *GetCropping();
	xnOSLeaveCriticalSection(GetLock());

	if (cropping.enabled)
	{
		XnStatus nRetVal = CropImpl(pFrame, &cropping);
		if (nRetVal != XN_STATUS_OK)
		{
			xnLogWarning(XN_MASK_DDK, "Failed to crop! Frame will be dropped");
			return;
		}
	}

	XnFrameStream::NewDataAvailable(pFrame);
}

XnStatus XnPixelStream::Mirror(OniFrame* pFrame) const
{
	return XnFormatsMirrorPixelData(GetOutputFormat(), (XnUChar*)pFrame->data, pFrame->dataSize, pFrame->width);
}

XnStatus XnPixelStream::CropImpl(OniFrame* pFrame, const OniCropping* pCropping)
{
	XnUChar* pPixelData = (XnUChar*)pFrame->data;
	XnUInt32 nCurDataSize = 0;

	for (XnUInt32 y = pCropping->originY; y < XnUInt32(pCropping->originY + pCropping->height); ++y)
	{
		XnUChar* pOrigLine = &pPixelData[y * GetXRes() * GetBytesPerPixel()];

		// move line
		xnOSMemCopy(pPixelData + nCurDataSize, pOrigLine + pCropping->originX * GetBytesPerPixel(), pCropping->width * GetBytesPerPixel());
		nCurDataSize += pCropping->width * GetBytesPerPixel();
	}

	// update size
	pFrame->dataSize = nCurDataSize;

	return XN_STATUS_OK;
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::ResolutionValueChangedCallback(const XnProperty* /*pSenser*/, void* pCookie)
{
	XnPixelStream* pStream = (XnPixelStream*)pCookie;
	return pStream->OnResolutionChanged();
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::OutputFormatValueChangedCallback(const XnProperty* /*pSenser*/, void* pCookie)
{
	XnPixelStream* pStream = (XnPixelStream*)pCookie;
	return pStream->OnOutputFormatChanged();
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::FixCroppingCallback(const XnProperty* /*pSenser*/, void* pCookie)
{
	XnPixelStream* pStream = (XnPixelStream*)pCookie;
	return pStream->FixCropping();
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::SetResolutionCallback(XnActualIntProperty* /*pSenser*/, XnUInt64 nValue, void* pCookie)
{
	XnPixelStream* pStream = (XnPixelStream*)pCookie;
	return pStream->SetResolution((XnResolutions)nValue);
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::SetXResCallback(XnActualIntProperty* /*pSenser*/, XnUInt64 nValue, void* pCookie)
{
	XnPixelStream* pStream = (XnPixelStream*)pCookie;
	return pStream->SetXRes((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::SetYResCallback(XnActualIntProperty* /*pSenser*/, XnUInt64 nValue, void* pCookie)
{
	XnPixelStream* pStream = (XnPixelStream*)pCookie;
	return pStream->SetYRes((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::SetCroppingCallback(XnActualGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XnPixelStream* pStream = (XnPixelStream*)pCookie;
	if (gbValue.dataSize != sizeof(OniCropping))
	{
		return XN_STATUS_DEVICE_PROPERTY_SIZE_DONT_MATCH;
	}

	return pStream->SetCropping((OniCropping*)gbValue.data);
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::ReadCroppingFromFileCallback(XnGeneralProperty* pSender, const XnChar* csINIFile, const XnChar* csSection)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// read section name
	XnChar csCroppingSection[XN_FILE_MAX_PATH];
	sprintf(csCroppingSection, "%s.Cropping", csSection);

	// read cropping values
	XnInt32 nOffsetX;
	XnInt32 nOffsetY;
	XnInt32 nSizeX;
	XnInt32 nSizeY;
	XnInt32 bEnabled;

	// only if all values are here
	if (XN_STATUS_OK == xnOSReadIntFromINI(csINIFile, csCroppingSection, "OffsetX", &nOffsetX) &&
		XN_STATUS_OK == xnOSReadIntFromINI(csINIFile, csCroppingSection, "OffsetY", &nOffsetY) &&
		XN_STATUS_OK == xnOSReadIntFromINI(csINIFile, csCroppingSection, "SizeX", &nSizeX) &&
		XN_STATUS_OK == xnOSReadIntFromINI(csINIFile, csCroppingSection, "SizeY", &nSizeY) &&
		XN_STATUS_OK == xnOSReadIntFromINI(csINIFile, csCroppingSection, "Enabled", &bEnabled))
	{
		OniCropping Cropping;
		Cropping.originX = (XnUInt16)nOffsetX;
		Cropping.originY = (XnUInt16)nOffsetY;
		Cropping.width = (XnUInt16)nSizeX;
		Cropping.height = (XnUInt16)nSizeY;
		Cropping.enabled = bEnabled;

		// set value
		nRetVal = pSender->SetValue(XN_PACK_GENERAL_BUFFER(Cropping));
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XN_CALLBACK_TYPE XnPixelStream::GetSupportedModesCallback(const XnGeneralProperty* /*pSender*/, const OniGeneralBuffer& gbValue, void* pCookie)
{
	XnPixelStream* pThis = (XnPixelStream*)pCookie;
	if ((gbValue.dataSize % sizeof(XnCmosPreset)) != 0)
	{
		return XN_STATUS_INVALID_BUFFER_SIZE;
	}

	XnUInt32 nCount = gbValue.dataSize / sizeof(XnCmosPreset);
	if (pThis->m_SupportedModesCount.GetValue() != nCount)
	{
		return XN_STATUS_INVALID_BUFFER_SIZE;
	}

	return pThis->GetSupportedModes((XnCmosPreset*)gbValue.data, nCount);
}

//---------------------------------------------------------------------------
// XnResolutionProperty
//---------------------------------------------------------------------------
XnPixelStream::XnResolutionProperty::XnResolutionProperty(XnUInt32 propertyId, const XnChar* strName, XnUInt64 nInitialValue /* = 0 */, const XnChar* strModule /* = "" */) :
	XnActualIntProperty(propertyId, strName, nInitialValue, strModule)
{
}

XnBool XnPixelStream::XnResolutionProperty::ConvertValueToString(XnChar* csValue, const void* pValue) const
{
	XnUInt64 nValue = *(XnUInt64*)pValue;
	strcpy(csValue, XnDDKGetResolutionName((XnResolutions)nValue));
	return TRUE;
}
