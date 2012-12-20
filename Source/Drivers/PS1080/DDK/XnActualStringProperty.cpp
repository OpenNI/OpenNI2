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
#include "XnActualStringProperty.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnActualStringProperty::XnActualStringProperty(XnUInt32 propertyId, const XnChar* strName, const XnChar* strInitialValue /* = "" */, const XnChar* strModule /* = "" */ ) :
	XnStringProperty(propertyId, strName, m_strValue, strModule)
{
	strncpy(m_strValue, strInitialValue, XN_DEVICE_MAX_STRING_LENGTH);
	// set a callback for get operations
	UpdateGetCallback(GetCallback, this);
}

XnStatus XnActualStringProperty::SetCallback(XnActualStringProperty* pSender, const XnChar* strValue, void* /*pCookie*/)
{
	return pSender->UnsafeUpdateValue(strValue);
}

XnStatus XnActualStringProperty::GetCallback(const XnActualStringProperty* pSender, XnChar* csValue, void* /*pCookie*/)
{
	strncpy(csValue, pSender->GetValue(), XN_DEVICE_MAX_STRING_LENGTH);
	return XN_STATUS_OK;
}
