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
#include "XnDeviceSensorInit.h"
#include "XnSensorIRStream.h"
#include "XnIRProcessor.h"
#include <XnOS.h>
#include "XnCmosInfo.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_IR_MAX_BUFFER_SIZE					(XN_SXGA_X_RES * XN_SXGA_Y_RES * sizeof(XnRGB24Pixel))

//---------------------------------------------------------------------------
// XnSensorIRStream class
//---------------------------------------------------------------------------
XnSensorIRStream::XnSensorIRStream(const XnChar* StreamName, XnSensorObjects* pObjects) : 
	XnIRStream(StreamName, FALSE),
	m_InputFormat(XN_STREAM_PROPERTY_INPUT_FORMAT, "InputFormat", 0),
	m_CroppingMode(XN_STREAM_PROPERTY_CROPPING_MODE, "CroppingMode", XN_CROPPING_MODE_NORMAL),
	m_Helper(pObjects),
	m_FirmwareCropSizeX(0, "FirmwareCropSizeX", 0, StreamName),
	m_FirmwareCropSizeY(0, "FirmwareCropSizeY", 0, StreamName),
	m_FirmwareCropOffsetX(0, "FirmwareCropOffsetX", 0, StreamName),
	m_FirmwareCropOffsetY(0, "FirmwareCropOffsetY", 0, StreamName),
	m_FirmwareCropMode(0, "FirmwareCropMode", XN_FIRMWARE_CROPPING_MODE_DISABLED, StreamName),
	m_ActualRead(XN_STREAM_PROPERTY_ACTUAL_READ_DATA, "ActualReadData", FALSE)
{
	m_ActualRead.UpdateSetCallback(SetActualReadCallback, this);
	m_CroppingMode.UpdateSetCallback(SetCroppingModeCallback, this);
}

XnStatus XnSensorIRStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

// 	nRetVal = SetBufferPool(&m_BufferPool);
// 	XN_IS_STATUS_OK(nRetVal);

	// init base
	nRetVal = XnIRStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	// add properties
	XN_VALIDATE_ADD_PROPERTIES(this, &m_InputFormat, &m_ActualRead, &m_CroppingMode);

	// set base properties default values
	nRetVal = ResolutionProperty().UnsafeUpdateValue(XN_IR_STREAM_DEFAULT_RESOLUTION);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = FPSProperty().UnsafeUpdateValue(XN_IR_STREAM_DEFAULT_FPS);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = OutputFormatProperty().UnsafeUpdateValue(XN_IR_STREAM_DEFAULT_OUTPUT_FORMAT);
	XN_IS_STATUS_OK(nRetVal);

	// init helper
	nRetVal = m_Helper.Init(this, this);
	XN_IS_STATUS_OK(nRetVal);

	// register supported modes
	XnCmosPreset* pSupportedModes = m_Helper.GetPrivateData()->FWInfo.irModes.GetData();
	XnUInt32 nSupportedModes = m_Helper.GetPrivateData()->FWInfo.irModes.GetSize();
	nRetVal = AddSupportedModes(pSupportedModes, nSupportedModes);
	XN_IS_STATUS_OK(nRetVal);

	// data processor
	nRetVal = m_Helper.RegisterDataProcessorProperty(ResolutionProperty());
	XN_IS_STATUS_OK(nRetVal);

	// register for mirror
	XnCallbackHandle hCallbackDummy;
	nRetVal = IsMirroredProperty().OnChangeEvent().Register(IsMirroredChangedCallback, this, hCallbackDummy);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::Free()
{
	m_Helper.Free();
	XnIRStream::Free();
	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::MapPropertiesToFirmware()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_Helper.MapFirmwareProperty(ResolutionProperty(), GetFirmwareParams()->m_IRResolution, FALSE);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(FPSProperty(), GetFirmwareParams()->m_IRFPS, FALSE);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(m_FirmwareCropSizeX, GetFirmwareParams()->m_IRCropSizeX, TRUE);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(m_FirmwareCropSizeY, GetFirmwareParams()->m_IRCropSizeY, TRUE);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(m_FirmwareCropOffsetX, GetFirmwareParams()->m_IRCropOffsetX, TRUE);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(m_FirmwareCropOffsetY, GetFirmwareParams()->m_IRCropOffsetY, TRUE);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(m_FirmwareCropMode, GetFirmwareParams()->m_IRCropMode, TRUE);
	XN_IS_STATUS_OK(nRetVal);;

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::ConfigureStreamImpl()
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnUSBShutdownReadThread(GetHelper()->GetPrivateData()->pSpecificImageUsb->pUsbConnection->UsbEp);

	nRetVal = SetActualRead(TRUE);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Helper.ConfigureFirmware(ResolutionProperty());
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.ConfigureFirmware(FPSProperty());
	XN_IS_STATUS_OK(nRetVal);;

	// IR mirror is always off in firmware
	nRetVal = GetFirmwareParams()->m_IRMirror.SetValue(FALSE);
	XN_IS_STATUS_OK(nRetVal);

	// CMOS
	if (GetResolution() != XN_RESOLUTION_SXGA)
	{
		nRetVal = m_Helper.GetCmosInfo()->SetCmosConfig(XN_CMOS_TYPE_DEPTH, GetResolution(), GetFPS());
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::SetActualRead(XnBool bRead)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if ((XnBool)m_ActualRead.GetValue() != bRead)
	{
		if (bRead)
		{
			xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Creating USB IR read thread...");
			XnSpecificUsbDevice* pUSB = GetHelper()->GetPrivateData()->pSpecificImageUsb;
			nRetVal = xnUSBInitReadThread(pUSB->pUsbConnection->UsbEp, pUSB->nChunkReadBytes, pUSB->nNumberOfBuffers, pUSB->nTimeout, XnDeviceSensorProtocolUsbEpCb, pUSB);
			XN_IS_STATUS_OK(nRetVal);
		}
		else
		{
			xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Shutting down IR image read thread...");
			xnUSBShutdownReadThread(GetHelper()->GetPrivateData()->pSpecificImageUsb->pUsbConnection->UsbEp);
		}

		nRetVal = m_ActualRead.UnsafeUpdateValue(bRead);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::OpenStreamImpl()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = GetFirmwareParams()->m_Stream0Mode.SetValue(XN_VIDEO_STREAM_IR);
	XN_IS_STATUS_OK(nRetVal);

	// Cropping
	if (m_FirmwareCropMode.GetValue() != XN_FIRMWARE_CROPPING_MODE_DISABLED)
	{
		nRetVal = m_Helper.ConfigureFirmware(m_FirmwareCropSizeX);
		XN_IS_STATUS_OK(nRetVal);;
		nRetVal = m_Helper.ConfigureFirmware(m_FirmwareCropSizeY);
		XN_IS_STATUS_OK(nRetVal);;
		nRetVal = m_Helper.ConfigureFirmware(m_FirmwareCropOffsetX);
		XN_IS_STATUS_OK(nRetVal);;
		nRetVal = m_Helper.ConfigureFirmware(m_FirmwareCropOffsetY);
		XN_IS_STATUS_OK(nRetVal);;
	}
	nRetVal = m_Helper.ConfigureFirmware(m_FirmwareCropMode);
	XN_IS_STATUS_OK(nRetVal);;

	nRetVal = FixFirmwareBug();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnIRStream::Open();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::FixFirmwareBug()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// Firmware bug ugly workaround: in v5.1, IR 1.3 would not turn off decimation, so image is
	// corrupted. we need to turn it off ourselves. The problem is that the firmware does not 
	// even provide a way to do so, so we need to directly change the register...
	// the bug only happens when cropping is off
	if (m_Helper.GetFirmware()->GetInfo()->nFWVer == XN_SENSOR_FW_VER_5_1 && GetResolution() == XN_RESOLUTION_SXGA && !GetCropping()->enabled)
	{
		nRetVal = XnHostProtocolWriteAHB(m_Helper.GetPrivateData(), 0x2a003c00, 0x3ff0000, 0xffffffff);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::CloseStreamImpl()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = GetFirmwareParams()->m_Stream0Mode.SetValue(XN_VIDEO_STREAM_OFF);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = SetActualRead(FALSE);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnIRStream::Close();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::SetOutputFormat(OniPixelFormat nOutputFormat)
{
	XnStatus nRetVal = XN_STATUS_OK;

	switch (nOutputFormat)
	{
	case ONI_PIXEL_FORMAT_RGB888:
	case ONI_PIXEL_FORMAT_GRAY16:
		break;
	default:
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Unsupported IR output format: %d", nOutputFormat);
	}

	nRetVal = m_Helper.BeforeSettingDataProcessorProperty();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnIRStream::SetOutputFormat(nOutputFormat);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Helper.AfterSettingDataProcessorProperty();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::SetFPS(XnUInt32 nFPS)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_Helper.BeforeSettingFirmwareParam(FPSProperty(), (XnUInt16)nFPS);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnIRStream::SetFPS(nFPS);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Helper.AfterSettingFirmwareParam(FPSProperty());
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::SetResolution(XnResolutions nResolution)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_Helper.BeforeSettingFirmwareParam(ResolutionProperty(), (XnUInt16)nResolution);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnIRStream::SetResolution(nResolution);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Helper.AfterSettingFirmwareParam(ResolutionProperty());
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::SetCroppingImpl(const OniCropping* pCropping, XnCroppingMode mode)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnFirmwareCroppingMode firmwareMode = m_Helper.GetFirmwareCroppingMode(mode, pCropping->enabled);

	nRetVal = ValidateCropping(pCropping);
	XN_IS_STATUS_OK(nRetVal);

	xnOSEnterCriticalSection(GetLock());

	if (m_Helper.GetFirmwareVersion() > XN_SENSOR_FW_VER_3_0)
	{
		nRetVal = m_Helper.StartFirmwareTransaction();
		if (nRetVal != XN_STATUS_OK)
		{
			xnOSLeaveCriticalSection(GetLock());
			return (nRetVal);
		}

		// mirror is done by software (meaning AFTER cropping, which is bad). So we need to flip the cropping area
		// to match requested area.
		XnUInt16 nXOffset = (XnUInt16) pCropping->originX;
		if (IsMirrored())
		{
			nXOffset = (XnUInt16)(GetXRes() - pCropping->originX - pCropping->width);
		}

		if (pCropping->enabled)
		{
			nRetVal = m_Helper.SimpleSetFirmwareParam(m_FirmwareCropSizeX,       (XnUInt16) pCropping->width);

			if (nRetVal == XN_STATUS_OK)
				nRetVal = m_Helper.SimpleSetFirmwareParam(m_FirmwareCropSizeY,   (XnUInt16) pCropping->height);

			if (nRetVal == XN_STATUS_OK)
				nRetVal = m_Helper.SimpleSetFirmwareParam(m_FirmwareCropOffsetX, (XnUInt16) nXOffset);

			if (nRetVal == XN_STATUS_OK)
				nRetVal = m_Helper.SimpleSetFirmwareParam(m_FirmwareCropOffsetY, (XnUInt16) pCropping->originY);
		}

		if (nRetVal == XN_STATUS_OK)
		{
			nRetVal = m_Helper.SimpleSetFirmwareParam(m_FirmwareCropMode, (XnUInt16)firmwareMode);
		}

		if (nRetVal != XN_STATUS_OK)
		{
			m_Helper.RollbackFirmwareTransaction();
			m_Helper.UpdateFromFirmware(m_FirmwareCropMode);
			m_Helper.UpdateFromFirmware(m_FirmwareCropOffsetX);
			m_Helper.UpdateFromFirmware(m_FirmwareCropOffsetY);
			m_Helper.UpdateFromFirmware(m_FirmwareCropSizeX);
			m_Helper.UpdateFromFirmware(m_FirmwareCropSizeY);
			xnOSLeaveCriticalSection(GetLock());
			return (nRetVal);
		}

		nRetVal = m_Helper.CommitFirmwareTransactionAsBatch();
		if (nRetVal != XN_STATUS_OK)
		{
			m_Helper.UpdateFromFirmware(m_FirmwareCropMode);
			m_Helper.UpdateFromFirmware(m_FirmwareCropOffsetX);
			m_Helper.UpdateFromFirmware(m_FirmwareCropOffsetY);
			m_Helper.UpdateFromFirmware(m_FirmwareCropSizeX);
			m_Helper.UpdateFromFirmware(m_FirmwareCropSizeY);
			xnOSLeaveCriticalSection(GetLock());
			return (nRetVal);
		}
	}

	nRetVal = m_CroppingMode.UnsafeUpdateValue(mode);
	XN_ASSERT(nRetVal == XN_STATUS_OK);

	nRetVal = XnIRStream::SetCropping(pCropping);
	if (nRetVal == XN_STATUS_OK)
	{
		nRetVal = FixFirmwareBug();
	}

	xnOSLeaveCriticalSection(GetLock());
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::SetCropping(const OniCropping* pCropping)
{
	return SetCroppingImpl(pCropping, (XnCroppingMode)m_CroppingMode.GetValue());
}

XnStatus XnSensorIRStream::SetCroppingMode(XnCroppingMode mode)
{
	switch (mode)
	{
	case XN_CROPPING_MODE_NORMAL:
	case XN_CROPPING_MODE_INCREASED_FPS:
	case XN_CROPPING_MODE_SOFTWARE_ONLY:
		break;
	default:
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Bad cropping mode: %u", mode);
	}

	return SetCroppingImpl(GetCropping(), mode);
}

XnStatus XnSensorIRStream::CalcRequiredSize(XnUInt32* pnRequiredSize) const
{
	// in IR, in all resolutions except SXGA, we get additional 8 lines
	XnUInt32 nYRes = GetYRes();
	if (GetResolution() != XN_RESOLUTION_SXGA)
	{
		nYRes += 8;
	}

	*pnRequiredSize = GetXRes() * nYRes * GetBytesPerPixel();
	return XN_STATUS_OK;
}

XnStatus XnSensorIRStream::CropImpl(OniFrame* pFrame, const OniCropping* pCropping)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// if firmware cropping is disabled, crop
	if (m_FirmwareCropMode.GetValue() == XN_FIRMWARE_CROPPING_MODE_DISABLED)
	{
		nRetVal = XnIRStream::CropImpl(pFrame, pCropping);
		XN_IS_STATUS_OK(nRetVal);
	}
	else if (IsMirrored())
	{
		// mirror is done in software and cropping in chip, so we crop the other side (see SetCroppingImpl()).
		pFrame->cropOriginX = GetXRes() - pFrame->cropOriginX - pFrame->width;
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::CreateDataProcessor(XnDataProcessor** ppProcessor)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnFrameBufferManager* pBufferManager;
	nRetVal = StartBufferManager(&pBufferManager);
	XN_IS_STATUS_OK(nRetVal);

	XnDataProcessor* pNew;
	XN_VALIDATE_NEW_AND_INIT(pNew, XnIRProcessor, this, &m_Helper, pBufferManager);

	*ppProcessor = pNew;

	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::OnIsMirroredChanged()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// if cropping is on, we need to flip it
	OniCropping cropping = *GetCropping();
	if (cropping.enabled)
	{
		nRetVal = SetCropping(&cropping);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorIRStream::IsMirroredChangedCallback(const XnProperty* /*pSender*/, void* pCookie)
{
	XnSensorIRStream* pThis = (XnSensorIRStream*)pCookie;
	return pThis->OnIsMirroredChanged();
}

XnStatus XN_CALLBACK_TYPE XnSensorIRStream::SetActualReadCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensorIRStream* pThis = (XnSensorIRStream*)pCookie;
	return pThis->SetActualRead(nValue == TRUE);
}

XnStatus XN_CALLBACK_TYPE XnSensorIRStream::SetCroppingModeCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensorIRStream* pStream = (XnSensorIRStream*)pCookie;
	return pStream->SetCroppingMode((XnCroppingMode)nValue);
}

