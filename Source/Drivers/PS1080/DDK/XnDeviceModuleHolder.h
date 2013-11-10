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
#ifndef XNDEVICEMODULEHOLDER_H
#define XNDEVICEMODULEHOLDER_H

#include "XnActualPropertiesHash.h"
#include "XnDeviceModule.h"
#include <XnList.h>

class XnDeviceModuleHolder
{
public:
	/**
	* Creates a new module holder.
	*
	* @param	pModule			[in]	The actual module.
	* @param	bAllowNewProps	[in]	When TRUE, Init() method will create non-existing properties.
	*/
	XnDeviceModuleHolder(XnDeviceModule* pModule);
	virtual ~XnDeviceModuleHolder();

	virtual XnStatus Init(const XnActualPropertiesHash* pInitialValues);

	inline XnDeviceModule* GetModule() const { return m_pModule; }

protected:
	virtual XnStatus Free();

private:
	XnDeviceModule* m_pModule;
};

typedef xnl::List<XnDeviceModuleHolder*> XnDeviceModuleHolderList;


#endif // XNDEVICEMODULEHOLDER_H
