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
#include "XnIntPropertySynchronizer.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnIntSynchronizerCookie
{
public:
	XnIntSynchronizerCookie(XnIntProperty* pSource, XnIntProperty* pDestination, XnIntPropertyConvertCallback pConvertFunc) :
	  pSource(pSource), pDestination(pDestination), pConvertFunc(pConvertFunc)
	{}

	XnIntProperty* pSource;
	XnIntProperty* pDestination;
	XnIntPropertyConvertCallback pConvertFunc;
	XnCallbackHandle hCallback;
};

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnIntPropertySynchronizer::XnIntPropertySynchronizer()
{
}

XnIntPropertySynchronizer::~XnIntPropertySynchronizer()
{
	for (CookiesList::Iterator it = m_Cookies.Begin(); it != m_Cookies.End(); ++it)
	{
		XnIntSynchronizerCookie* pSynchData = (XnIntSynchronizerCookie*)*it;
		pSynchData->pSource->OnChangeEvent().Unregister(pSynchData->hCallback);
		XN_DELETE(pSynchData);
	}
}

XnStatus XN_CALLBACK_TYPE IntPropertyValueChangedCallback(const XnProperty* pSender, void* pCookie)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// get the property current value
	XnIntProperty* pIntProp = (XnIntProperty*)pSender;

	XnUInt64 nNewValue;
	nRetVal = pIntProp->GetValue(&nNewValue);
	XN_IS_STATUS_OK(nRetVal);

	XnIntSynchronizerCookie* pSynchData = (XnIntSynchronizerCookie*)pCookie;

	XnUInt64 nDestValue;

	// convert the value if needed
	if (pSynchData->pConvertFunc != NULL)
	{
		nRetVal = pSynchData->pConvertFunc(nNewValue, &nDestValue);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		nDestValue = nNewValue;
	}

	// now set the new value
	nRetVal = pSynchData->pDestination->UnsafeUpdateValue(nDestValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnIntPropertySynchronizer::RegisterSynchronization(XnIntProperty* pSource, XnIntProperty* pDestination, XnIntPropertyConvertCallback pConvertFunc /* = NULL */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnIntSynchronizerCookie* pCookie;
	XN_VALIDATE_NEW(pCookie, XnIntSynchronizerCookie, pSource, pDestination, pConvertFunc);

	nRetVal = m_Cookies.AddFirst(pCookie);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(pCookie);
		return (nRetVal);
	}

	nRetVal = pSource->OnChangeEvent().Register(IntPropertyValueChangedCallback, pCookie, pCookie->hCallback);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(pCookie);
		m_Cookies.Remove(m_Cookies.Begin());
		return (nRetVal);
	}
	
	return (XN_STATUS_OK);
}

