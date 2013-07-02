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
#include "XnOniStream.h"
#include "DDK/XnFrameStream.h"

//---------------------------------------------------------------------------
// XnOniStream class
//---------------------------------------------------------------------------
XnOniStream::XnOniStream(XnSensor* pSensor, const XnChar* strName, OniSensorType sensorType, XnOniDevice* pDevice) : 
	m_sensorType(sensorType),
	m_pSensor(pSensor),
	m_strType(strName),
	m_pDevice(pDevice),
	m_started(FALSE)
{
}

XnOniStream::~XnOniStream()
{
	destroy();
}

XnStatus XnOniStream::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = m_pSensor->CreateStream(m_strType, m_strType, NULL);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = m_pSensor->RegisterToNewStreamData(OnNewStreamDataEventHandler, this, m_hNewDataCallback);
	XN_IS_STATUS_OK(nRetVal);
	
	nRetVal = m_pSensor->GetStream(m_strType, &m_pDeviceStream);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

void XnOniStream::destroy()
{
	stop();
	m_pSensor->UnregisterFromNewStreamData(m_hNewDataCallback);
	m_pSensor->DestroyStream(m_strType);
}

void XnOniStream::setServices(oni::driver::StreamServices* pStreamServices)
{
	oni::driver::StreamBase::setServices(pStreamServices);
	m_pDeviceStream->SetServices(*pStreamServices);
}

OniStatus XnOniStream::start()
{
	XnStatus nRetVal = ONI_STATUS_OK;

	if (!m_started)
	{
		// first lock the stream (so that IsOpen() and Open() will be an atomic action)
		xnl::AutoCSLocker locker(*m_pDeviceStream->GetOpenLock());

		if (m_pDeviceStream->IsOpen())
		{
			m_pDeviceStream->OpenAddRef();
		}
		else
		{
			nRetVal = m_pDeviceStream->Open();
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
		}

		m_started = TRUE;
	}

	return ONI_STATUS_OK;
}

void XnOniStream::stop()
{
	if (m_started)
	{
		m_started = FALSE;

		// first lock the stream (so that OpenDecRef() and Close() will be an atomic action)
		xnl::AutoCSLocker locker(*m_pDeviceStream->GetOpenLock());

		if (0 == m_pDeviceStream->OpenDecRef())
		{
			m_pDeviceStream->Close();
		}
	}
}

OniStatus XnOniStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	XnStatus nRetVal = m_pDeviceStream->GetProperty(propertyId, data, pDataSize);
	if (nRetVal != XN_STATUS_OK)
	{
		return ONI_STATUS_BAD_PARAMETER;
	}
	return ONI_STATUS_OK;
}

OniStatus XnOniStream::setProperty(int propertyId, const void* data, int dataSize)
{
	xnl::AutoCSLocker locker(*m_pDeviceStream->GetOpenLock());

	// if this stream is open, and not just by me (multiple depth streams for example), don't allow any changes
	XnUInt32 myOpenRefCount = m_started ? 1 : 0;
	if (m_pDeviceStream->GetOpenRefCount() > myOpenRefCount)	
	{
		return ONI_STATUS_OUT_OF_FLOW;
	}
	else
	{
		XnStatus nRetVal = SetPropertyImpl(propertyId, data, dataSize);
		switch(nRetVal)
		{
		case XN_STATUS_OK:
			return ONI_STATUS_OK;
		
		case XN_STATUS_DEVICE_PROPERTY_BAD_TYPE:
			return ONI_STATUS_BAD_PARAMETER;
		
		case XN_STATUS_BAD_PARAM:
		case XN_STATUS_DEVICE_BAD_PARAM:
		default:
			return ONI_STATUS_NOT_SUPPORTED;
		}
	}
}

XnStatus XnOniStream::SetPropertyImpl(int propertyId, const void* data, int dataSize)
{
	return m_pDeviceStream->SetProperty(propertyId, data, dataSize);
}

OniBool XnOniStream::isPropertySupported(int propertyId)
{
	XnBool exists;
	m_pDeviceStream->DoesPropertyExist(propertyId, &exists);
	return (exists == TRUE);
}

void XN_CALLBACK_TYPE XnOniStream::OnNewStreamDataEventHandler(const XnNewStreamDataEventArgs& args, void* pCookie)
{
	XnOniStream* pThis = (XnOniStream*)pCookie;
	if (pThis->m_started && strcmp(args.strStreamName, pThis->m_strType) == 0)
	{
		pThis->raiseNewFrame(args.pFrame);
	}
}

int XnOniStream::getRequiredFrameSize()
{
	return m_pDeviceStream->GetRequiredDataSize();
}

