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
#include "XnDeviceModule.h"
#include <XnDDKStatus.h>
#include <XnMacros.h>
#include <XnLog.h>
#include "XnActualIntProperty.h"
#include "XnActualRealProperty.h"
#include "XnActualStringProperty.h"
#include "XnActualGeneralProperty.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnDeviceModule::XnDeviceModule(const XnChar* strName) :
	m_Lock(XN_MODULE_PROPERTY_LOCK, "Lock", FALSE, strName),
	m_hLockCS(NULL)
{
	strncpy(m_strName, strName, XN_DEVICE_MAX_STRING_LENGTH);
	m_Lock.UpdateSetCallback(SetLockStateCallback, this);
}

XnDeviceModule::~XnDeviceModule()
{
	Free();
}

XnStatus XnDeviceModule::Init()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = AddProperty(&m_Lock);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = xnOSCreateCriticalSection(&m_hLockCS);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::Free()
{
	xnOSCloseCriticalSection(&m_hLockCS);
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::AddProperty(XnProperty* pProperty)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// make sure another property with this name doesn't exist
	XnPropertiesHash::Iterator it = m_Properties.End();
	if (XN_STATUS_NO_MATCH != m_Properties.Find(pProperty->GetId(), it))
		return XN_STATUS_DEVICE_PROPERTY_ALREADY_EXISTS;

	nRetVal = m_Properties.Set(pProperty->GetId(), pProperty);
	XN_IS_STATUS_OK(nRetVal);

	pProperty->UpdateName(GetName(), pProperty->GetName());

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::AddProperties(XnProperty** apProperties, XnUInt32 nCount)
{
	XnStatus nRetVal = XN_STATUS_OK;

	for (XnUInt32 i = 0; i < nCount; ++i)
	{
		nRetVal = AddProperty(apProperties[i]);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::DoesPropertyExist(XnUInt32 propertyId, XnBool* pbDoesExist) const
{
	XnStatus nRetVal = XN_STATUS_OK;

	*pbDoesExist = FALSE;

	XnPropertiesHash::ConstIterator it = m_Properties.End();
	nRetVal = m_Properties.Find(propertyId, it);
	if (nRetVal != XN_STATUS_NO_MATCH && nRetVal != XN_STATUS_OK)
	{
		return (nRetVal);
	}

	*pbDoesExist = (nRetVal == XN_STATUS_OK);
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::GetPropertyType(XnUInt32 propertyId, XnPropertyType* pnType) const
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	*pnType = pProp->GetType();
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::GetPropertyImpl(XnUInt32 propertyId, XnPropertyType Type, XnProperty** ppProperty) const
{
	*ppProperty = NULL;

	XnProperty* pProperty;
	if (XN_STATUS_NO_MATCH == m_Properties.Get(propertyId, pProperty))
	{
		return XN_STATUS_DEVICE_PROPERTY_DONT_EXIST;
	}

	if (pProperty->GetType() != Type)
	{
		return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
	}

	*ppProperty = pProperty;
	return XN_STATUS_OK;
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnProperty **ppProperty) const
{
	XnProperty* pProperty;
	if (XN_STATUS_NO_MATCH == m_Properties.Get(propertyId, pProperty))
	{
		return XN_STATUS_DEVICE_PROPERTY_DONT_EXIST;
	}

	*ppProperty = pProperty;

	return XN_STATUS_OK;
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnUInt64* pnValue) const
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnIntProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->GetValue(pnValue);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnDouble* pdValue) const
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnRealProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->GetValue(pdValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnChar* csValue) const
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnStringProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->GetValue(csValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);

}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, const OniGeneralBuffer& gbValue) const
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnGeneralProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->GetValue(gbValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, void* data, int* pDataSize) const
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnPropertyType type;
	nRetVal = GetPropertyType(propertyId, &type);
	XN_IS_STATUS_OK(nRetVal);

	switch (type)
	{
	case XN_PROPERTY_TYPE_INTEGER:
		{
			XnUInt64 nValue;
			nRetVal = GetProperty(propertyId, &nValue);
			if (nRetVal != XN_STATUS_OK)
			{
				// XN_ASSERT(FALSE); // This could happen when the property was write only
				return XN_STATUS_ERROR;
			}

			if (*pDataSize == sizeof(XnUInt64))
			{
				*(XnUInt64*)data = nValue;
			}
			else if (*pDataSize == sizeof(XnUInt32))
			{
				*(XnUInt32*)data = (XnUInt32)nValue;
			}
			else if (*pDataSize == sizeof(XnUInt16))
			{
				*(XnUInt16*)data = (XnUInt16)nValue;
			}
			else if (*pDataSize == sizeof(XnUInt8))
			{
				*(XnUInt8*)data = (XnUInt8)nValue;
			}
			else
			{
				XN_ASSERT(FALSE);
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}
			break;
		}
	case XN_PROPERTY_TYPE_REAL:
		{
			XnDouble dValue;
			nRetVal = GetProperty(propertyId, &dValue);
			if (nRetVal != XN_STATUS_OK)
			{
				XN_ASSERT(FALSE);
				return XN_STATUS_ERROR;
			}

			if (*pDataSize == sizeof(XnDouble))
			{
				*(XnDouble*)data = dValue;
			}
			else if (*pDataSize == sizeof(XnFloat))
			{
				*(XnFloat*)data = (XnFloat)dValue;
			}
			else
			{
				XN_ASSERT(FALSE);
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}
			break;
		}
	case XN_PROPERTY_TYPE_STRING:
		{
			XnChar strValue[XN_DEVICE_MAX_STRING_LENGTH];
			nRetVal = GetProperty(propertyId, strValue);
			if (nRetVal != XN_STATUS_OK)
			{
				XN_ASSERT(FALSE);
				return XN_STATUS_ERROR;
			}

			nRetVal = xnOSStrCopy((XnChar*)data, strValue, *pDataSize);
			if (nRetVal != XN_STATUS_OK)
			{
				// wrong size?
				XN_ASSERT(FALSE);
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}
			break;
		}
	case XN_PROPERTY_TYPE_GENERAL:
		{
			OniGeneralBuffer buffer;
			buffer.data = data;
			buffer.dataSize = *pDataSize;
			nRetVal = GetProperty(propertyId, buffer);
			XN_IS_STATUS_OK(nRetVal);
			break;
		}
	default:
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	return XN_STATUS_OK;
}

XnStatus XnDeviceModule::SetProperty(XnUInt32 propertyId, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnIntProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->SetValue(nValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::SetProperty(XnUInt32 propertyId, XnDouble dValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnRealProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->SetValue(dValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::SetProperty(XnUInt32 propertyId, const XnChar* strValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnStringProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->SetValue(strValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::SetProperty(XnUInt32 propertyId, const OniGeneralBuffer& gbValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnGeneralProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->SetValue(gbValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::SetProperty(XnUInt32 propertyId, const void* data, int dataSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnPropertyType type;
	nRetVal = GetPropertyType(propertyId, &type);
	XN_IS_STATUS_OK(nRetVal);

	switch (type)
	{
	case XN_PROPERTY_TYPE_INTEGER:
		{
			XnUInt64 nValue;

			if (dataSize == sizeof(XnUInt64))
			{
				nValue = *(XnUInt64*)data;
			}
			else if (dataSize == sizeof(XnUInt32))
			{
				nValue = *(XnUInt32*)data;
			}
			else if (dataSize == sizeof(XnUInt16))
			{
				nValue = *(XnUInt16*)data;
			}
			else if (dataSize == sizeof(XnUInt8))
			{
				nValue = *(XnUInt8*)data;
			}
			else
			{
				XN_ASSERT(FALSE);
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}

			nRetVal = SetProperty(propertyId, nValue);
			XN_IS_STATUS_OK(nRetVal);

			break;
		}
	case XN_PROPERTY_TYPE_REAL:
		{
			XnDouble dValue;

			if (dataSize == sizeof(XnDouble))
			{
				dValue = *(XnDouble*)data;
			}
			else if (dataSize == sizeof(XnFloat))
			{
				dValue = *(XnFloat*)data;
			}
			else
			{
				XN_ASSERT(FALSE);
				return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
			}

			nRetVal = SetProperty(propertyId, dValue);
			XN_IS_STATUS_OK(nRetVal);
			break;
		}
	case XN_PROPERTY_TYPE_STRING:
		{
			nRetVal = SetProperty(propertyId, (const XnChar*)data);
			XN_IS_STATUS_OK(nRetVal);
			break;
		}
	case XN_PROPERTY_TYPE_GENERAL:
		{
			OniGeneralBuffer buffer;
			buffer.data = (void*)data;
			buffer.dataSize = dataSize;
			nRetVal = SetProperty(propertyId, buffer);
			XN_IS_STATUS_OK(nRetVal);
			break;
		}
	default:
		XN_ASSERT(FALSE);
		return XN_STATUS_ERROR;
	}

	return XN_STATUS_OK;
}

XnStatus XnDeviceModule::UnsafeUpdateProperty(XnUInt32 propertyId, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnIntProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->UnsafeUpdateValue(nValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::UnsafeUpdateProperty(XnUInt32 propertyId, XnDouble dValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnRealProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->UnsafeUpdateValue(dValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::UnsafeUpdateProperty(XnUInt32 propertyId, const XnChar* strValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnStringProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->UnsafeUpdateValue(strValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::UnsafeUpdateProperty(XnUInt32 propertyId, const OniGeneralBuffer& gbValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnGeneralProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->UnsafeUpdateValue(gbValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::RegisterForOnPropertyValueChanged(XnUInt32 propertyId, XnProperty::OnValueChangedHandler pFunc, void* pCookie, XnCallbackHandle& hCallback)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->OnChangeEvent().Register(pFunc, pCookie, hCallback);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::UnregisterFromOnPropertyValueChanged(XnUInt32 propertyId, XnCallbackHandle hCallback)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnProperty* pProp;
	nRetVal = GetProperty(propertyId, &pProp);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pProp->OnChangeEvent().Unregister(hCallback);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnIntProperty **ppIntProperty) const
{
	return GetPropertyImpl(propertyId, XN_PROPERTY_TYPE_INTEGER, (XnProperty**)ppIntProperty);
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnRealProperty **ppRealProperty) const
{
	return GetPropertyImpl(propertyId, XN_PROPERTY_TYPE_REAL, (XnProperty**)ppRealProperty);
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnStringProperty **ppStringProperty) const
{
	return GetPropertyImpl(propertyId, XN_PROPERTY_TYPE_STRING, (XnProperty**)ppStringProperty);
}

XnStatus XnDeviceModule::GetProperty(XnUInt32 propertyId, XnGeneralProperty **ppPtrProperty) const
{
	return GetPropertyImpl(propertyId, XN_PROPERTY_TYPE_GENERAL, (XnProperty**)ppPtrProperty);
}

XnStatus XnDeviceModule::GetAllProperties(XnPropertySet* pSet) const
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// add the module
	nRetVal = XnPropertySetAddModule(pSet, GetName());
	XN_IS_STATUS_OK(nRetVal);

	// now add all properties
	for (XnPropertiesHash::ConstIterator it = m_Properties.Begin(); it != m_Properties.End(); ++it)
	{
		XnProperty* pProperty = it->Value();

		if (pProperty->IsActual())
		{
			nRetVal = pProperty->AddToPropertySet(pSet);
			XN_IS_STATUS_OK(nRetVal);
		}
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::LoadConfigFromFile(const XnChar* csINIFilePath, const XnChar* strSectionName /* = NULL */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (strSectionName == NULL)
	{
		strSectionName = GetName();
	}

	xnLogVerbose(XN_MASK_DDK, "Configuring module '%s' from section '%s' in file '%s'...", GetName(), strSectionName, csINIFilePath);

	for (XnPropertiesHash::Iterator it = m_Properties.Begin(); it != m_Properties.End(); ++it)
	{
		XnProperty* pProp = it->Value();

		// only read writable properties
		if (!pProp->IsReadOnly())
		{
			nRetVal = pProp->ReadValueFromFile(csINIFilePath, strSectionName);
			XN_IS_STATUS_OK(nRetVal);
		}
	}

	xnLogInfo(XN_MASK_DDK, "Module '%s' configuration was loaded from file.", GetName());

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::BatchConfig(const XnActualPropertiesHash& props)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	for (XnActualPropertiesHash::ConstIterator it = props.Begin(); it != props.End(); ++it)
	{
		XnProperty* pRequestProp = it->Value();
		switch (pRequestProp->GetType())
		{
		case XN_PROPERTY_TYPE_INTEGER:
			{
				XnActualIntProperty* pProp = (XnActualIntProperty*)pRequestProp;
				nRetVal = SetProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_REAL:
			{
				XnActualRealProperty* pProp = (XnActualRealProperty*)pRequestProp;
				nRetVal = SetProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_STRING:
			{
				XnActualStringProperty* pProp = (XnActualStringProperty*)pRequestProp;
				nRetVal = SetProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_GENERAL:
			{
				XnActualGeneralProperty* pProp = (XnActualGeneralProperty*)pRequestProp;
				nRetVal = SetProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		default:
			XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DDK, "Unknown property type: %d\n", pRequestProp->GetType());
		} // type switch
	} // props loop
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::UnsafeBatchConfig(const XnActualPropertiesHash& props)
{
	XnStatus nRetVal = XN_STATUS_OK;

	for (XnActualPropertiesHash::ConstIterator it = props.Begin(); it != props.End(); ++it)
	{
		XnProperty* pRequestProp = it->Value();
		switch (pRequestProp->GetType())
		{
		case XN_PROPERTY_TYPE_INTEGER:
			{
				XnActualIntProperty* pProp = (XnActualIntProperty*)pRequestProp;
				nRetVal = UnsafeUpdateProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_REAL:
			{
				XnActualRealProperty* pProp = (XnActualRealProperty*)pRequestProp;
				nRetVal = UnsafeUpdateProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_STRING:
			{
				XnActualStringProperty* pProp = (XnActualStringProperty*)pRequestProp;
				nRetVal = UnsafeUpdateProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_GENERAL:
			{
				XnActualGeneralProperty* pProp = (XnActualGeneralProperty*)pRequestProp;
				nRetVal = UnsafeUpdateProperty(pProp->GetId(), pProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		default:
			XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DDK, "Unknown property type: %d\n", pRequestProp->GetType());
		} // type switch
	} // props loop

	return (XN_STATUS_OK);
}

XnStatus XnDeviceModule::SetLockState(XnBool bLocked)
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (bLocked && m_Lock.GetValue() == TRUE)
	{
		return XN_STATUS_NODE_IS_LOCKED;
	}

	xnOSEnterCriticalSection(&m_hLockCS);

	// check again
	if (bLocked && m_Lock.GetValue() == TRUE)
	{
		xnOSLeaveCriticalSection(&m_hLockCS);
		return XN_STATUS_NODE_IS_LOCKED;
	}

	nRetVal = m_Lock.UnsafeUpdateValue(bLocked);
	xnOSLeaveCriticalSection(&m_hLockCS);

	return (nRetVal);
}

XnStatus XnDeviceModule::SetLockStateCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnDeviceModule* pThis = (XnDeviceModule*)pCookie;
	return pThis->SetLockState(nValue != FALSE);
}
