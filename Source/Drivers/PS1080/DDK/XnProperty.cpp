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
#include "XnProperty.h"
#include <XnDDKStatus.h>
#include <XnOS.h>
#include <XnCallback.h>
#include <XnLog.h>

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnProperty::XnProperty(XnPropertyType Type, void* pValueHolder, XnUInt32 propertyId, const XnChar* strName, const XnChar* strModule) :
	m_propertyId(propertyId),
	m_Type(Type),
	m_pSetCallback(NULL),
	m_pSetCallbackCookie(NULL),
	m_pGetCallback(NULL),
	m_pGetCallbackCookie(NULL),
	m_pValueHolder(pValueHolder),
	m_LogSeverity(XN_LOG_INFO),
	m_bAlwaysSet(FALSE)
{
	UpdateName(strModule, strName);
}

XnProperty::~XnProperty()
{
}

void XnProperty::UpdateName(const XnChar* strModule, const XnChar* strName)
{
	strncpy(m_strModule, strModule, XN_DEVICE_MAX_STRING_LENGTH);
	if (m_strName != strName) {
		strncpy(m_strName, strName, XN_DEVICE_MAX_STRING_LENGTH);
	}
}

XnStatus XnProperty::SetValue(const void* pValue)
{
	if (m_pSetCallback == NULL)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_PROPERTY_READ_ONLY, XN_MASK_DDK, "Property %s.%s is read only.", GetModule(), GetName());
	}

	if (m_LogSeverity != -1)
	{
		XnChar strValue[XN_DEVICE_MAX_STRING_LENGTH];
		if (ConvertValueToString(strValue, pValue))
		{
			xnLogWrite(XN_MASK_DDK, (XnLogSeverity)m_LogSeverity, __FILE__, __LINE__, "Setting %s.%s to %s...", GetModule(), GetName(), strValue);
		}
		else
		{
			xnLogWrite(XN_MASK_DDK, (XnLogSeverity)m_LogSeverity, __FILE__, __LINE__, "Setting %s.%s...", GetModule(), GetName());
		}
	}

	if (!m_bAlwaysSet && IsActual() && IsEqual(m_pValueHolder, pValue))
	{
		xnLogWrite(XN_MASK_DDK, (XnLogSeverity)m_LogSeverity, __FILE__, __LINE__, "%s.%s value did not change.", GetModule(), GetName());
	}
	else
	{
		XnStatus nRetVal = CallSetCallback(m_pSetCallback, pValue, m_pSetCallbackCookie);
		if (nRetVal != XN_STATUS_OK)
		{
			if (m_LogSeverity != -1)
			{
				xnLogWrite(XN_MASK_DDK, (XnLogSeverity)m_LogSeverity, __FILE__, __LINE__, "Failed setting %s.%s: %s", GetModule(), GetName(), xnGetStatusString(nRetVal));
			}
			return (nRetVal);
		}
		else
		{
			xnLogWrite(XN_MASK_DDK, (XnLogSeverity)m_LogSeverity, __FILE__, __LINE__, "%s.%s was successfully set.", GetModule(), GetName());
		}
	}

	return (XN_STATUS_OK);
}

XnStatus XnProperty::GetValue(void* pValue) const
{
	if (m_pGetCallback == NULL)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_PROPERTY_WRITE_ONLY, XN_MASK_DDK, "Property %s.%s is write only.", GetModule(), GetName());
	}

	return CallGetCallback(m_pGetCallback, pValue, m_pGetCallbackCookie);
}

XnStatus XnProperty::UnsafeUpdateValue(const void* pValue /* = NULL */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnBool bValueChanged = TRUE;

	if (IsActual())
	{
		if (IsEqual(m_pValueHolder, pValue))
		{
			bValueChanged = FALSE;
		}
		else
		{
			// update the value
			nRetVal = CopyValueImpl(m_pValueHolder, pValue);
			XN_IS_STATUS_OK(nRetVal);
		}
	}

	if (bValueChanged)
	{
		// print a message
		if (m_LogSeverity != -1)
		{
			XnChar strValue[XN_DEVICE_MAX_STRING_LENGTH];
			XnBool bValueString = FALSE;

			if (IsActual())
			{
				bValueString = ConvertValueToString(strValue, pValue);
			}

			xnLogWrite(XN_MASK_DDK, (XnLogSeverity)m_LogSeverity, __FILE__, __LINE__, "Property %s.%s was changed%s%s.", GetModule(), GetName(),
				bValueString ? " to " : "", bValueString ? strValue : "");
		}

		// raise the event
		nRetVal = m_OnChangeEvent.Raise(this);
		XN_IS_STATUS_OK(nRetVal);
	}

	return XN_STATUS_OK;
}

void XnProperty::UpdateSetCallback(SetFuncPtr pFunc, void* pCookie)
{
	m_pSetCallback = pFunc;
	m_pSetCallbackCookie = pCookie;
}

void XnProperty::UpdateGetCallback(GetFuncPtr pFunc, void* pCookie)
{
	m_pGetCallback = pFunc;
	m_pGetCallbackCookie = pCookie;
}

XnBool XnProperty::ConvertValueToString(XnChar* /*csValue*/, const void* /*pValue*/) const
{
	return FALSE;
}

XnStatus XnProperty::ChangeEvent::Raise(const XnProperty* pSender)
{
	XnStatus nRetVal = XN_STATUS_OK;
	xnl::AutoCSLocker locker(m_hLock);
	ApplyListChanges();

	for (CallbackPtrList::ConstIterator it = m_callbacks.Begin(); it != m_callbacks.End(); ++it)
	{
		Callback* pCallback = *it;
		nRetVal = pCallback->pFunc(pSender, pCallback->pCookie);
		if (nRetVal != XN_STATUS_OK)
		{
			break;
		}
	}

	ApplyListChanges();
	return (nRetVal);
}
