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
#ifndef XNACTUALSTRINGPROPERTY_H
#define XNACTUALSTRINGPROPERTY_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnStringProperty.h>

//---------------------------------------------------------------------------
// Class
//---------------------------------------------------------------------------

/**
* A property of type general.
*/
class XnActualStringProperty : public XnStringProperty
{
public:
	XnActualStringProperty(XnUInt32 propertyId, const XnChar* strName, const XnChar* strInitialValue = "", const XnChar* strModule = "");

	inline const XnChar* GetValue() const { return m_strValue; }

	typedef XnStatus (XN_CALLBACK_TYPE* SetFuncPtr)(XnActualStringProperty* pSender, const XnChar* strValue, void* pCookie);
	typedef XnStatus (XN_CALLBACK_TYPE* GetFuncPtr)(const XnActualStringProperty* pSender, XnChar* csValue, void* pCookie);

	inline void UpdateSetCallback(SetFuncPtr pFunc, void* pCookie)
	{
		XnProperty::UpdateSetCallback((XnProperty::SetFuncPtr)pFunc, pCookie);
	}

	inline void UpdateSetCallbackToDefault()
	{
		UpdateSetCallback(SetCallback, this);
	}

	inline void UpdateGetCallback(GetFuncPtr pFunc, void* pCookie)
	{
		XnProperty::UpdateGetCallback((XnProperty::GetFuncPtr)pFunc, pCookie);
	}

private:
	static XnStatus XN_CALLBACK_TYPE SetCallback(XnActualStringProperty* pSender, const XnChar* strValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetCallback(const XnActualStringProperty* pSender, XnChar* csValue, void* pCookie);

	XnChar m_strValue[XN_DEVICE_MAX_STRING_LENGTH];
};

#endif // XNACTUALSTRINGPROPERTY_H
