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
#ifndef __XN_GENERAL_PROPERTY_H__
#define __XN_GENERAL_PROPERTY_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnProperty.h>
#include <Driver/OniDriverTypes.h>

//---------------------------------------------------------------------------
// Class
//---------------------------------------------------------------------------

/**
* A property of type general.
*/
class XnGeneralProperty : public XnProperty
{
public:
	typedef XnStatus (XN_CALLBACK_TYPE* ReadValueFromFileFuncPtr)(XnGeneralProperty* pSender, const XnChar* csINIFile, const XnChar* csSection);

	XnGeneralProperty(XnUInt32 propertyId, const XnChar* strName, OniGeneralBuffer* pValueHolder = NULL, ReadValueFromFileFuncPtr pReadFromFileFunc = NULL, const XnChar* strModule = "");

	typedef XnStatus (XN_CALLBACK_TYPE* SetFuncPtr)(XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	typedef XnStatus (XN_CALLBACK_TYPE* GetFuncPtr)(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);

	inline XnStatus SetValue(const OniGeneralBuffer& gbValue)
	{
		return XnProperty::SetValue(&gbValue);
	}

	inline XnStatus GetValue(const OniGeneralBuffer& gbValue) const
	{
		return XnProperty::GetValue((void*)&gbValue);
	}

	inline XnStatus UnsafeUpdateValue(const OniGeneralBuffer& gbValue)
	{
		return XnProperty::UnsafeUpdateValue(&gbValue);
	}

	inline void UpdateSetCallback(SetFuncPtr pFunc, void* pCookie)
	{
		XnProperty::UpdateSetCallback((XnProperty::SetFuncPtr)pFunc, pCookie);
	}

	inline void UpdateGetCallback(GetFuncPtr pFunc, void* pCookie)
	{
		XnProperty::UpdateGetCallback((XnProperty::GetFuncPtr)pFunc, pCookie);
	}

	XnStatus AddToPropertySet(XnPropertySet* pSet);

protected:
	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	virtual XnStatus CopyValueImpl(void* pDest, const void* pSource) const;
	virtual XnBool IsEqual(const void* pValue1, const void* pValue2) const;
	virtual XnStatus CallSetCallback(XnProperty::SetFuncPtr pFunc, const void* pValue, void* pCookie);
	virtual XnStatus CallGetCallback(XnProperty::GetFuncPtr pFunc, void* pValue, void* pCookie) const;
	virtual XnStatus ReadValueFromFile(const XnChar* csINIFile, const XnChar* csSection);

private:
	ReadValueFromFileFuncPtr m_pReadFromFileFunc;
};

#endif //__XN_GENERAL_PROPERTY_H__