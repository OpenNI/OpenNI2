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
#if 0 // Audio support
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDeviceSensorInit.h"
#include "XnSensorAudioStream.h"
#include "XnSensor.h"
#include "XnAudioProcessor.h"
#include <XnFormatsStatus.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_AUDIO_MAX_SAMPLE_RATE				48000
#define XN_AUDIO_MAX_NUMBER_OF_CHANNELS			2

#define XN_SENSOR_PROTOCOL_AUDIO_PACKET_SIZE_BULK		424
#define XN_SENSOR_PROTOCOL_AUDIO_PACKET_SIZE_ISO		180

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnSensorAudioStream::XnSensorAudioStream(const XnChar* strDeviceName, const XnChar* StreamName, XnSensorObjects* pObjects, XnBool bAllowOtherUsers) :
	XnAudioStream(StreamName, XN_AUDIO_MAX_NUMBER_OF_CHANNELS),
	m_Helper(pObjects),
	m_strDeviceName(strDeviceName),
	m_bAllowOtherUsers(bAllowOtherUsers),
	m_LeftChannelVolume(XN_STREAM_PROPERTY_LEFT_CHANNEL_VOLUME, "LeftChannelVolume", XN_AUDIO_STREAM_DEFAULT_VOLUME),
	m_RightChannelVolume(XN_STREAM_PROPERTY_RIGHT_CHANNEL_VOLUME, "RightChannelVolume", XN_AUDIO_STREAM_DEFAULT_VOLUME),
	m_ActualRead(XN_STREAM_PROPERTY_ACTUAL_READ_DATA, "ActualReadData", FALSE),
	m_nFrameID(0)
{
	m_buffer.pAudioBuffer = NULL;
	m_LeftChannelVolume.UpdateSetCallback(SetLeftChannelVolumeCallback, this);
	m_RightChannelVolume.UpdateSetCallback(SetRightChannelVolumeCallback, this);
	m_ActualRead.UpdateSetCallback(SetActualReadCallback, this);
}

XnStatus XnSensorAudioStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// init base
	nRetVal = XnAudioStream::Init();
	XN_IS_STATUS_OK(nRetVal);

	// init helper
	nRetVal = m_Helper.Init(this, this);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSCreateCriticalSection(&m_buffer.hLock);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = SetReadChunkSize(XN_AUDIO_STREAM_DEFAULT_CHUNK_SIZE);
	XN_IS_STATUS_OK(nRetVal);

	// add properties
	XN_VALIDATE_ADD_PROPERTIES(this, &m_LeftChannelVolume, &m_RightChannelVolume, &m_ActualRead);

	// check what's the firmware audio packet size
	if (m_Helper.GetPrivateData()->SensorHandle.MiscConnection.bIsISO)
		m_nOrigAudioPacketSize = XN_SENSOR_PROTOCOL_AUDIO_PACKET_SIZE_ISO;
	else
		m_nOrigAudioPacketSize = XN_SENSOR_PROTOCOL_AUDIO_PACKET_SIZE_BULK;

	// alloc buffer
	nRetVal = ReallocBuffer();
	XN_IS_STATUS_OK(nRetVal);

	m_buffer.pAudioCallback = NewDataCallback;
	m_buffer.pAudioCallbackCookie = this;

	// data processor
	nRetVal = m_Helper.RegisterDataProcessorProperty(NumberOfChannelsProperty());
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::Free()
{
	if (m_buffer.pAudioBuffer != NULL)
	{
		xnOSFreeAligned(m_buffer.pAudioBuffer);
		m_buffer.pAudioBuffer = NULL;
	}

	if (m_buffer.pAudioPacketsTimestamps != NULL)
	{
		xnOSFreeAligned(m_buffer.pAudioPacketsTimestamps);
		m_buffer.pAudioPacketsTimestamps = NULL;
	}

	m_Helper.Free();
	XnAudioStream::Free();

	// close critical sections
	if (m_buffer.hLock != NULL)
	{
		xnOSCloseCriticalSection(&m_buffer.hLock);
		m_buffer.hLock = NULL;
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::MapPropertiesToFirmware()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_Helper.MapFirmwareProperty(SampleRateProperty(), GetFirmwareParams()->m_AudioSampleRate, FALSE, ConvertSampleRateToFirmwareRate);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(NumberOfChannelsProperty(), GetFirmwareParams()->m_AudioStereo, FALSE, ConvertNumberOfChannelsToStereo);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(m_LeftChannelVolume, GetFirmwareParams()->m_AudioLeftChannelGain, TRUE);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.MapFirmwareProperty(m_RightChannelVolume, GetFirmwareParams()->m_AudioRightChannelGain, TRUE);
	XN_IS_STATUS_OK(nRetVal);;

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::ConfigureStreamImpl()
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnUSBShutdownReadThread(GetHelper()->GetPrivateData()->pSpecificMiscUsb->pUsbConnection->UsbEp);

	nRetVal = SetActualRead(TRUE);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Helper.ConfigureFirmware(SampleRateProperty());
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.ConfigureFirmware(NumberOfChannelsProperty());
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.ConfigureFirmware(m_LeftChannelVolume);
	XN_IS_STATUS_OK(nRetVal);;
	nRetVal = m_Helper.ConfigureFirmware(m_RightChannelVolume);
	XN_IS_STATUS_OK(nRetVal);;

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::SetActualRead(XnBool bRead)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	if ((XnBool)m_ActualRead.GetValue() != bRead)
	{
		if (bRead)
		{
			xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Creating USB audio read thread...");
			XnSpecificUsbDevice* pUSB = GetHelper()->GetPrivateData()->pSpecificMiscUsb;
			nRetVal = xnUSBInitReadThread(pUSB->pUsbConnection->UsbEp, pUSB->nChunkReadBytes, pUSB->nNumberOfBuffers, pUSB->nTimeout, XnDeviceSensorProtocolUsbEpCb, pUSB);
			XN_IS_STATUS_OK(nRetVal);
		}
		else
		{
			xnLogVerbose(XN_MASK_DEVICE_SENSOR, "Shutting down USB audio read thread...");
			xnUSBShutdownReadThread(GetHelper()->GetPrivateData()->pSpecificMiscUsb->pUsbConnection->UsbEp);
		}

		nRetVal = m_ActualRead.UnsafeUpdateValue(bRead);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::OpenStreamImpl()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = GetFirmwareParams()->m_Stream2Mode.SetValue(XN_AUDIO_STREAM_ON);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnAudioStream::Open();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::CloseStreamImpl()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = GetFirmwareParams()->m_Stream2Mode.SetValue(XN_AUDIO_STREAM_OFF);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = SetActualRead(FALSE);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnAudioStream::Close();
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::CreateDataProcessor(XnDataProcessor** ppProcessor)
{
	XnDataProcessor* pAudioProcessor;
	XN_VALIDATE_NEW_AND_INIT(pAudioProcessor, XnAudioProcessor, this, &m_Helper, &m_buffer, m_nOrigAudioPacketSize);

	*ppProcessor = pAudioProcessor;

	return XN_STATUS_OK;
}

XnStatus XnSensorAudioStream::SetOutputFormat(OniFormat nOutputFormat)
{
	XnStatus nRetVal = XN_STATUS_OK;

	switch (nOutputFormat)
	{
	case ONI_PIXEL_FORMAT_PCM:
		break;
	default:
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DEVICE_SENSOR, "Output format %d, isn't supported by sensor audio stream!", nOutputFormat);
	}

	nRetVal = XnAudioStream::SetOutputFormat(nOutputFormat);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::SetSampleRate(XnSampleRate nSampleRate)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_Helper.BeforeSettingFirmwareParam(SampleRateProperty(), (XnUInt16)nSampleRate);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnAudioStream::SetSampleRate(nSampleRate);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Helper.AfterSettingFirmwareParam(SampleRateProperty());
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::SetNumberOfChannels(XnUInt32 nNumberOfChannels)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_Helper.BeforeSettingFirmwareParam(NumberOfChannelsProperty(), (XnUInt16)nNumberOfChannels);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnAudioStream::SetNumberOfChannels(nNumberOfChannels);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = ReallocBuffer();
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_Helper.AfterSettingFirmwareParam(NumberOfChannelsProperty());
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::NewData()
{
	// TODO: fix this
/*	// check how many buffers we have
	XnInt32 nAvailbalePackets = m_buffer.nAudioWriteIndex - m_buffer.nAudioReadIndex;
	if (nAvailbalePackets < 0)
		nAvailbalePackets += m_buffer.nAudioBufferNumOfPackets;

	if ((XnUInt32)nAvailbalePackets * m_buffer.nAudioPacketSize >= GetReadChunkSize())
	{
		// update last write index (the last written byte)
		m_header.nWritePacketIndex = m_buffer.nAudioWriteIndex;
		// take first packet timestamp
		NewDataAvailable(m_buffer.pAudioPacketsTimestamps[m_buffer.nAudioReadIndex], 0);
	}
*/
	return XN_STATUS_OK;
}

XnStatus XnSensorAudioStream::ConvertNumberOfChannelsToStereo(XnUInt64 nSource, XnUInt64* pnDest)
{
	*pnDest = (nSource == 2);
	return XN_STATUS_OK;
}

XnStatus XnSensorAudioStream::ConvertStereoToNumberOfChannels(XnUInt64 nSource, XnUInt64* pnDest)
{
	*pnDest = nSource ? 2 : 1;
	return XN_STATUS_OK;
}

XnStatus XnSensorAudioStream::ConvertSampleRateToFirmwareRate(XnUInt64 nSource, XnUInt64* pnDest)
{
	switch (nSource)
	{
	case XN_SAMPLE_RATE_8K:
		*pnDest = A2D_SAMPLE_RATE_8KHZ;
		break;
	case XN_SAMPLE_RATE_11K:
		*pnDest = A2D_SAMPLE_RATE_11KHZ;
		break;
	case XN_SAMPLE_RATE_12K:
		*pnDest = A2D_SAMPLE_RATE_12KHZ;
		break;
	case XN_SAMPLE_RATE_16K:
		*pnDest = A2D_SAMPLE_RATE_16KHZ;
		break;
	case XN_SAMPLE_RATE_22K:
		*pnDest = A2D_SAMPLE_RATE_22KHZ;
		break;
	case XN_SAMPLE_RATE_24K:
		*pnDest = A2D_SAMPLE_RATE_24KHZ;
		break;
	case XN_SAMPLE_RATE_32K:
		*pnDest = A2D_SAMPLE_RATE_32KHZ;
		break;
	case XN_SAMPLE_RATE_44K:
		*pnDest = A2D_SAMPLE_RATE_44KHZ;
		break;
	case XN_SAMPLE_RATE_48K:
		*pnDest = A2D_SAMPLE_RATE_48KHZ;
		break;
	default:
		return XN_STATUS_DEVICE_UNSUPPORTED_MODE;
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::ConvertFirmwareRateToSampleRate(XnUInt64 nSource, XnUInt64* pnDest)
{
	switch (nSource)
	{
	case A2D_SAMPLE_RATE_8KHZ:
		*pnDest = XN_SAMPLE_RATE_8K;
		break;
	case A2D_SAMPLE_RATE_11KHZ:
		*pnDest = XN_SAMPLE_RATE_11K;
		break;
	case A2D_SAMPLE_RATE_12KHZ:
		*pnDest = XN_SAMPLE_RATE_12K;
		break;
	case A2D_SAMPLE_RATE_16KHZ:
		*pnDest = XN_SAMPLE_RATE_16K;
		break;
	case A2D_SAMPLE_RATE_22KHZ:
		*pnDest = XN_SAMPLE_RATE_22K;
		break;
	case A2D_SAMPLE_RATE_24KHZ:
		*pnDest = XN_SAMPLE_RATE_24K;
		break;
	case A2D_SAMPLE_RATE_32KHZ:
		*pnDest = XN_SAMPLE_RATE_32K;
		break;
	case A2D_SAMPLE_RATE_44KHZ:
		*pnDest = XN_SAMPLE_RATE_44K;
		break;
	case A2D_SAMPLE_RATE_48KHZ:
		*pnDest = XN_SAMPLE_RATE_48K;
		break;
	default:
		return XN_STATUS_DEVICE_UNSUPPORTED_MODE;
	}

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::SetLeftChannelVolume(XnUInt32 nVolume)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_Helper.SimpleSetFirmwareParam(m_LeftChannelVolume, (XnUInt16)nVolume);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::SetRightChannelVolume(XnUInt32 nVolume)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_Helper.SimpleSetFirmwareParam(m_RightChannelVolume, (XnUInt16)nVolume);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnSensorAudioStream::ReallocBuffer()
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (m_buffer.pAudioBuffer == NULL)
	{
		// we allocate enough for 5 seconds of audio
		XnUInt32 nSampleSize = 2 * 2; // 16-bit per channel (2 bytes) * max number of channels (2)
		XnUInt32 nSamples = 48000 * 5; // max sample rate * number of seconds

		XnUInt32 nMaxBufferSize = nSamples * nSampleSize;

		// find min packet size (so we'll have max packet count)
		XnUInt32 nMinPacketSize = XN_MIN(XN_SENSOR_PROTOCOL_AUDIO_PACKET_SIZE_BULK, XN_SENSOR_PROTOCOL_AUDIO_PACKET_SIZE_ISO);
		XnUInt32 nMaxPacketCount = nMaxBufferSize / nMinPacketSize - 1;

		nRetVal = RequiredSizeProperty().UnsafeUpdateValue(nMaxBufferSize);
		XN_IS_STATUS_OK(nRetVal);

		m_buffer.pAudioPacketsTimestamps = (XnUInt64*)xnOSMallocAligned(sizeof(XnUInt64) * nMaxPacketCount, XN_DEFAULT_MEM_ALIGN);
		m_buffer.pAudioBuffer = (XnUInt8*)xnOSMallocAligned(nMaxBufferSize, XN_DEFAULT_MEM_ALIGN);
		m_buffer.nAudioBufferSize = nMaxBufferSize;
	}

	// calculate current packet size
	m_buffer.nAudioPacketSize = m_nOrigAudioPacketSize;

	if (m_Helper.GetFirmwareVersion() >= XN_SENSOR_FW_VER_5_2 && GetNumberOfChannels() == 1)
	{
		m_buffer.nAudioPacketSize /= 2;
	}

	m_buffer.nAudioBufferNumOfPackets = m_buffer.nAudioBufferSize / m_buffer.nAudioPacketSize;
	m_buffer.nAudioBufferSize = m_buffer.nAudioBufferNumOfPackets * m_buffer.nAudioPacketSize;

	m_header.nPacketCount = m_buffer.nAudioBufferNumOfPackets;
	m_header.nPacketSize = m_buffer.nAudioPacketSize;

	// set read and write indices
	m_buffer.nAudioReadIndex = 0;
	m_buffer.nAudioWriteIndex = 0;

	return (XN_STATUS_OK);
}

XnStatus XN_CALLBACK_TYPE XnSensorAudioStream::SetLeftChannelVolumeCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensorAudioStream* pThis = (XnSensorAudioStream*)pCookie;
	return pThis->SetLeftChannelVolume((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensorAudioStream::SetRightChannelVolumeCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensorAudioStream* pThis = (XnSensorAudioStream*)pCookie;
	return pThis->SetRightChannelVolume((XnUInt32)nValue);
}

XnStatus XN_CALLBACK_TYPE XnSensorAudioStream::SetActualReadCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnSensorAudioStream* pThis = (XnSensorAudioStream*)pCookie;
	return pThis->SetActualRead(nValue == TRUE);
}

XnStatus XN_CALLBACK_TYPE XnSensorAudioStream::NewDataCallback(void* pCookie)
{
	XnSensorAudioStream* pThis = (XnSensorAudioStream*)pCookie;
	return pThis->NewData();
}

#endif // Audio support
