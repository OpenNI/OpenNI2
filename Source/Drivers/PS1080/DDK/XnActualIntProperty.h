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
#ifndef XNACTUALINTPROPERTY_H
#define XNACTUALINTPROPERTY_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnIntProperty.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

/**
* A property of type integer.
*/
class XnActualIntProperty : public XnIntProperty
{
public:
	XnActualIntProperty(XnUInt32 propertyId, const XnChar* strName, XnUInt64 nInitialValue = 0, const XnChar* strModule = "");

	inline XnUInt64 GetValue() const { return m_nValue; }

	typedef XnStatus (XN_CALLBACK_TYPE* SetFuncPtr)(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	typedef XnStatus (XN_CALLBACK_TYPE* GetFuncPtr)(const XnActualIntProperty* pSender, XnUInt64* pnValue, void* pCookie);

	inline void UpdateSetCallback(SetFuncPtr pFunc, void* pCookie)
	{
		XnIntProperty::UpdateSetCallback((XnIntProperty::SetFuncPtr)pFunc, pCookie);
	}

	inline void UpdateSetCallbackToDefault()
	{
		UpdateSetCallback(SetCallback, this);
	}

	inline void UpdateGetCallback(GetFuncPtr pFunc, void* pCookie)
	{
		XnIntProperty::UpdateGetCallback((XnIntProperty::GetFuncPtr)pFunc, pCookie);
	}

private:
	static XnStatus XN_CALLBACK_TYPE SetCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE GetCallback(const XnActualIntProperty* pSender, XnUInt64* pnValue, void* pCookie);

	XnUInt64 m_nValue;
};

#endif // XNACTUALINTPROPERTY_H
