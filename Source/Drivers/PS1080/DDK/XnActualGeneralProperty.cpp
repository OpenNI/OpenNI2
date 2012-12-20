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
#include "XnActualGeneralProperty.h"
#include <XnCore.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnActualGeneralProperty::XnActualGeneralProperty(XnUInt32 propertyId, const XnChar* strName, void* pData, XnUInt32 nDataSize, ReadValueFromFileFuncPtr pReadFromFileFunc /* = NULL */, const XnChar* strModule /* = "" */) :
	XnGeneralProperty(propertyId, strName, &m_gbValue, pReadFromFileFunc, strModule),
	m_gbValue(XnGeneralBufferPack(pData, nDataSize)),
	m_bOwner(FALSE)
{
	// set a callback for get operations
	UpdateGetCallback(GetCallback, this);
}

XnActualGeneralProperty::XnActualGeneralProperty(XnUInt32 propertyId, const XnChar* strName, const OniGeneralBuffer& gbValue, ReadValueFromFileFuncPtr pReadFromFileFunc /* = NULL */, const XnChar* strModule /* = "" */) :
	XnGeneralProperty(propertyId, strName, &m_gbValue, pReadFromFileFunc, strModule),
	m_gbValue(gbValue),
	m_bOwner(FALSE)
{
	// set a callback for get operations
	UpdateGetCallback(GetCallback, this);
}

XnActualGeneralProperty::~XnActualGeneralProperty()
{
	if (m_bOwner)
	{
		XnGeneralBufferFree(&m_gbValue);
	}
}

void XnActualGeneralProperty::SetAsBufferOwner(XnBool bOwner)
{
	m_bOwner = bOwner;
}

XnStatus XnActualGeneralProperty::SetCallback(XnActualGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* /*pCookie*/)
{
	return pSender->UnsafeUpdateValue(gbValue);
}

XnStatus XnActualGeneralProperty::GetCallback(const XnActualGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* /*pCookie*/)
{
	if (gbValue.dataSize != pSender->GetValue().dataSize)
	{
		return XN_STATUS_DEVICE_PROPERTY_SIZE_DONT_MATCH;
	}

	xnOSMemCopy(gbValue.data, pSender->GetValue().data, pSender->GetValue().dataSize);
	return XN_STATUS_OK;
}

XnStatus XnActualGeneralProperty::AddToPropertySet(XnPropertySet* pSet)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = XnPropertySetAddGeneralProperty(pSet, GetModule(), GetId(), &m_gbValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}
