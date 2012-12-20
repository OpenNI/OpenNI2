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
#ifndef __XN_REAL_PROPERTY_H__
#define __XN_REAL_PROPERTY_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnProperty.h>

//---------------------------------------------------------------------------
// Class
//---------------------------------------------------------------------------

/**
* A property of type integer.
*/
class XnRealProperty : public XnProperty
{
public:
	XnRealProperty(XnUInt32 propertyId, const XnChar* strName, XnDouble* pValueHolder, const XnChar* strModule = "");

	typedef XnStatus (XN_CALLBACK_TYPE* SetFuncPtr)(XnRealProperty* pSender, XnDouble dValue, void* pCookie);
	typedef XnStatus (XN_CALLBACK_TYPE* GetFuncPtr)(const XnRealProperty* pSender, XnDouble* pdValue, void* pCookie);

	inline XnStatus SetValue(XnDouble dValue)
	{
		return XnProperty::SetValue(&dValue);
	}

	inline XnStatus GetValue(XnDouble* pdValue) const
	{
		XN_VALIDATE_OUTPUT_PTR(pdValue);
		return XnProperty::GetValue(pdValue);
	}

	inline XnStatus UnsafeUpdateValue(XnDouble dValue)
	{
		return XnProperty::UnsafeUpdateValue(&dValue);
	}

	inline void UpdateSetCallback(SetFuncPtr pFunc, void* pCookie)
	{
		XnProperty::UpdateSetCallback((XnProperty::SetFuncPtr)pFunc, pCookie);
	}

	inline void UpdateGetCallback(GetFuncPtr pFunc, void* pCookie)
	{
		XnProperty::UpdateGetCallback((XnProperty::GetFuncPtr)pFunc, pCookie);
	}

	virtual XnStatus ReadValueFromFile(const XnChar* csINIFile, const XnChar* csSection);

	XnStatus AddToPropertySet(XnPropertySet* pSet);

protected:
	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	virtual XnStatus CopyValueImpl(void* pDest, const void* pSource) const;
	virtual XnBool IsEqual(const void* pValue1, const void* pValue2) const;
	virtual XnStatus CallSetCallback(XnProperty::SetFuncPtr pFunc, const void* pValue, void* pCookie);
	virtual XnStatus CallGetCallback(XnProperty::GetFuncPtr pFunc, void* pValue, void* pCookie) const;
	virtual XnBool ConvertValueToString(XnChar* csValue, const void* pValue) const;
};

#endif //__XN_REAL_PROPERTY_H__
