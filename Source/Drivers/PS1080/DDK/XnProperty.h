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
#ifndef __XN_PROPERTY_H__
#define __XN_PROPERTY_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnDevice.h>
#include <XnList.h>
#include <XnStringsHash.h>
#include <XnLog.h>
#include <XnEvent.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

/**
* A holder for a property (a name and value pair). Note that this class should be inherited, and
* can not be used directly.
*/
class XnProperty
{
public:
	typedef XnStatus (XN_CALLBACK_TYPE* OnValueChangedHandler)(const XnProperty* pSender, void* pCookie);
	typedef xnl::EventInterface<OnValueChangedHandler> ChangeEventInterface;

	/**
	* Creates a new property. 
	*
	* @param	Type			[in]	Type of the property.
	* @param	pValueHolder	[in]	A pointer to the holder of the value.
	* @param	strName			[in]	Name of the property.
	* @param	strModule		[in]	Name of the module holding this property.
	*/
	XnProperty(XnPropertyType Type, void* pValueHolder, XnUInt32 propertyId, const XnChar* strName, const XnChar* strModule);
	virtual ~XnProperty();

	inline XnUInt32 GetId() const { return m_propertyId; }
	inline const XnChar* GetName() const { return m_strName; }
	inline const XnChar* GetModule() const { return m_strModule; }
	inline XnBool IsActual() const { return (m_pValueHolder != NULL); }
	inline XnBool IsReadOnly() const { return (m_pGetCallback == NULL); }
	inline XnPropertyType GetType() const { return m_Type; }

	inline ChangeEventInterface& OnChangeEvent() { return m_OnChangeEvent; }

	/** Updates property name. */
	void UpdateName(const XnChar* strModule, const XnChar* strName);

	/** Updates the value of the property according to an INI file. */
	virtual XnStatus ReadValueFromFile(const XnChar* csINIFile, const XnChar* csSection) = 0;

	/** Adds this property to the property set. */
	virtual XnStatus AddToPropertySet(XnPropertySet* pSet) = 0;

	/** Sets the log severity under which changes to the property are printed. */
	inline void SetLogSeverity(XnInt32 nSeverity) { m_LogSeverity = nSeverity; }

	/** When TRUE, the property will always call the set callback, even if value hasn't changed. */
	inline void SetAlwaysSet(XnBool bAlwaysSet) { m_bAlwaysSet = bAlwaysSet; }

protected:

	typedef XnStatus (XN_CALLBACK_TYPE* SetFuncPtr)(XnProperty* pSender, const void* pValue, void* pCookie);
	typedef XnStatus (XN_CALLBACK_TYPE* GetFuncPtr)(const XnProperty* pSender, void* pValue, void* pCookie);

	/** Sets the property value. */
	XnStatus SetValue(const void* pValue);

	/** Gets the property value. */
	XnStatus GetValue(void* pValue) const;

	/** Updates the value of the property without any checks. */
	XnStatus UnsafeUpdateValue(const void* pValue = NULL);

	/** Updates the set callback. */
	void UpdateSetCallback(SetFuncPtr pFunc, void* pCookie);

	/** Updates the get callback. */
	void UpdateGetCallback(GetFuncPtr pFunc, void* pCookie);

	virtual XnStatus CopyValueImpl(void* pDest, const void* pSource) const = 0;
	virtual XnBool IsEqual(const void* pValue1, const void* pValue2) const = 0;
	virtual XnStatus CallSetCallback(SetFuncPtr pFunc, const void* pValue, void* pCookie) = 0;
	virtual XnStatus CallGetCallback(GetFuncPtr pFunc, void* pValue, void* pCookie) const = 0;
	virtual XnBool ConvertValueToString(XnChar* csValue, const void* pValue) const;

	inline void* Value() const { return m_pValueHolder; }

private:
	class ChangeEvent : public xnl::EventInterface<OnValueChangedHandler>
	{
	public:
		XnStatus Raise(const XnProperty* pSender);
	};

	XnChar m_strModule[XN_DEVICE_MAX_STRING_LENGTH]; // module name
	XnChar m_strName[XN_DEVICE_MAX_STRING_LENGTH]; // property name
	XnUInt32 m_propertyId;
	XnPropertyType m_Type; // property type

	// Set callback
	SetFuncPtr m_pSetCallback; 
	void* m_pSetCallbackCookie;

	// Get callback
	GetFuncPtr m_pGetCallback;
	void* m_pGetCallbackCookie;

	void* m_pValueHolder; // a pointer to the storage of the property

	ChangeEvent m_OnChangeEvent;

	XnInt32 m_LogSeverity;
	XnBool m_bAlwaysSet;
};

/** A property list */
typedef xnl::List<XnProperty*> XnPropertiesList;

/** A hash table, mapping property name to the property */
typedef xnl::Hash<XnUInt32, XnProperty*> XnPropertiesHash;

#endif //__XN_PROPERTY_H__
