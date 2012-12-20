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
#include "XnActualRealProperty.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnActualRealProperty::XnActualRealProperty(XnUInt32 propertyId, const XnChar* strName, XnDouble dInitialValue /* = 0.0 */, const XnChar* strModule /* = "" */) :
	XnRealProperty(propertyId, strName, &m_dValue, strModule),
	m_dValue(dInitialValue)
{
	// set a callback for get operations
	UpdateGetCallback(GetCallback, this);
}

XnStatus XnActualRealProperty::SetCallback(XnActualRealProperty* pSender, XnDouble dValue, void* /*pCookie*/)
{
	return pSender->UnsafeUpdateValue(dValue);
}

XnStatus XN_CALLBACK_TYPE XnActualRealProperty::GetCallback(const XnActualRealProperty* pSender, XnDouble* pdValue, void* /*pCookie*/)
{
	*pdValue = pSender->GetValue();
	return XN_STATUS_OK;
}
