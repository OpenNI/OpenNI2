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
#include "XnDeviceModuleHolder.h"
#include "XnActualIntProperty.h"
#include "XnActualRealProperty.h"
#include "XnActualStringProperty.h"
#include "XnActualGeneralProperty.h"
#include <XnCore.h>

XnDeviceModuleHolder::XnDeviceModuleHolder(XnDeviceModule* pModule) :
	m_pModule(pModule)
{}

XnDeviceModuleHolder::~XnDeviceModuleHolder()
{
	XnDeviceModuleHolder::Free();
}

XnStatus XnDeviceModuleHolder::Init(const XnActualPropertiesHash* pInitialValues) 
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = m_pModule->Init();
	XN_IS_STATUS_OK(nRetVal);

	if (pInitialValues != NULL)
	{
		nRetVal = m_pModule->BatchConfig(*pInitialValues);
		XN_IS_STATUS_OK(nRetVal);
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModuleHolder::Free() 
{
	return XN_STATUS_OK;
}
