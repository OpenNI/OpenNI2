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
#ifndef __XN_INT_PROPERTY_SYNCHRONIZER_H__
#define __XN_INT_PROPERTY_SYNCHRONIZER_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnList.h>
#include <DDK/XnIntProperty.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
typedef XnStatus (XN_CALLBACK_TYPE* XnIntPropertyConvertCallback)(XnUInt64 nSourceValue, XnUInt64* pnDestValue);

class XnIntSynchronizerCookie; // forward declaration

class XnIntPropertySynchronizer
{
public:
	XnIntPropertySynchronizer();
	~XnIntPropertySynchronizer();

	XnStatus RegisterSynchronization(XnIntProperty* pSource, XnIntProperty* pDestination, XnIntPropertyConvertCallback pConvertFunc = NULL);

private:
	typedef xnl::List<XnIntSynchronizerCookie*> CookiesList;
	CookiesList m_Cookies;
};

#endif //__XN_INT_PROPERTY_SYNCHRONIZER_H__
