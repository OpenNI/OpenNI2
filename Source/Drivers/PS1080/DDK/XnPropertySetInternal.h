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
#ifndef __XN_PROPERTY_SET_INTERNAL_H__
#define __XN_PROPERTY_SET_INTERNAL_H__

#include <XnPropertySet.h>
#include "XnActualPropertiesHash.h"

typedef XnStringsHashT<XnActualPropertiesHash*> XnPropertySetDataInternal;

class XnPropertySetData;

struct XnPropertySet
{
	XnPropertySetData* pData;
};

class XnPropertySetData : public XnPropertySetDataInternal
{
public:
	~XnPropertySetData()
	{
		XnPropertySet set;
		set.pData = this;
		XnPropertySetClear(&set);
	}
};

#define _XN_PROPERTY_SET_NAME(name)	__ ## name ## _ ## Data

#define XN_PROPERTY_SET_CREATE_ON_STACK(name)		\
	XnPropertySetData _XN_PROPERTY_SET_NAME(name);	\
	XnPropertySet name;								\
	name.pData = &_XN_PROPERTY_SET_NAME(name);

XnStatus XnPropertySetDataAttachModule(XnPropertySetData* pSetData, const XnChar* strModuleName, XnActualPropertiesHash* pModule);
XnStatus XnPropertySetDataDetachModule(XnPropertySetData* pSetData, const XnChar* strModuleName, XnActualPropertiesHash** ppModule);
XnStatus XnPropertySetCloneModule(const XnPropertySet* pSource, XnPropertySet* pDest, const XnChar* strModule, const XnChar* strNewName);

#endif //__XN_PROPERTY_SET_INTERNAL_H__
