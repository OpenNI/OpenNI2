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
#include "XnOniIRStream.h"

//---------------------------------------------------------------------------
// XnOniIRStream class
//---------------------------------------------------------------------------

XnOniIRStream::XnOniIRStream(XnSensor* pSensor, XnOniDevice* pDevice) : XnOniMapStream(pSensor, XN_STREAM_TYPE_IR, ONI_SENSOR_IR, pDevice)
{
}

OniStatus XnOniIRStream::getProperty(int propertyId, void* data, int* pDataSize)
{
    switch (propertyId)
    {
    case ONI_STREAM_PROPERTY_MAX_VALUE:
        if (*pDataSize != sizeof(int))
        {
            return ONI_STATUS_BAD_PARAMETER;
        }

        XnUInt64 nValue;
        m_pSensor->GetProperty(m_strType, XN_STREAM_PROPERTY_DEVICE_MAX_IR, &nValue);

        *(int*)data = (int)nValue;
        return ONI_STATUS_OK;
    default:
        return XnOniMapStream::getProperty(propertyId, data, pDataSize);
    }
}

OniBool XnOniIRStream::isPropertySupported(int propertyId)
{
    return (
        propertyId == ONI_STREAM_PROPERTY_MAX_VALUE ||
        XnOniMapStream::isPropertySupported(propertyId));
}
