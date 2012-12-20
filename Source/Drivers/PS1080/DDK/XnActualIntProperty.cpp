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
#include "XnActualIntProperty.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnActualIntProperty::XnActualIntProperty(XnUInt32 propertyId, const XnChar* strName, XnUInt64 nInitialValue /* = 0 */, const XnChar* strModule /* = "" */) :
	XnIntProperty(propertyId, strName, &m_nValue, strModule),
	m_nValue(nInitialValue)
{
	// set a callback for get operations
	UpdateGetCallback(GetCallback, this);
}

XnStatus XN_CALLBACK_TYPE XnActualIntProperty::SetCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* /*pCookie*/)
{
	return pSender->UnsafeUpdateValue(nValue);
}

XnStatus XN_CALLBACK_TYPE XnActualIntProperty::GetCallback(const XnActualIntProperty* pSender, XnUInt64* pnValue, void* /*pCookie*/)
{
	*pnValue = pSender->GetValue();
	return XN_STATUS_OK;
}
