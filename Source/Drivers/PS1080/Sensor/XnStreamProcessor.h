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
#ifndef __XN_STREAM_PROCESSOR_H__
#define __XN_STREAM_PROCESSOR_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDataProcessor.h"
#include "XnSensorStreamHelper.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

/**
* Base class for streams data processors
*/
class XnStreamProcessor : public XnDataProcessor
{
public:
	XnStreamProcessor(XnDeviceStream* pStream, XnSensorStreamHelper* pHelper);

//---------------------------------------------------------------------------
// Utility Functions
//---------------------------------------------------------------------------
protected:
	XnDeviceStream* GetStream() { return m_pStream; }
	XnSensorStreamHelper* GetStreamHelper() { return m_pHelper; }

//---------------------------------------------------------------------------
// Members
//---------------------------------------------------------------------------
private:
	XnDeviceStream* m_pStream;
	XnSensorStreamHelper* m_pHelper;
};

#endif //__XN_STREAM_PROCESSOR_H__
