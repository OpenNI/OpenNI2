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
#include "LinkOniIRStream.h"

#define LINK_MAX_IR_PIXEL_VALUE 4095

//---------------------------------------------------------------------------
// LinkOniIRStream class
//---------------------------------------------------------------------------

LinkOniIRStream::LinkOniIRStream(const char* configFile, xn::PrimeClient* pSensor, LinkOniDevice* pDevice) : 
	LinkOniMapStream(configFile, "IR", pSensor, ONI_SENSOR_IR, pDevice)
{
}

OniStatus LinkOniIRStream::getProperty(int propertyId, void* data, int* pDataSize)
{	
    switch (propertyId)
    {
        // int props
    case ONI_STREAM_PROPERTY_MAX_VALUE:
        {
            int value = LINK_MAX_IR_PIXEL_VALUE;
            ENSURE_PROP_SIZE(*pDataSize, int);
            ASSIGN_PROP_VALUE_INT(data, *pDataSize, value);
        }
        break;
    default:
        return LinkOniMapStream::getProperty(propertyId, data, pDataSize);
    }
        
    return ONI_STATUS_OK;
}
