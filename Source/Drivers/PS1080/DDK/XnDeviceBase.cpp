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
#include "XnDeviceBase.h"
#include <XnOS.h>
#include <XnLog.h>
#include "XnIntProperty.h"
#include "XnRealProperty.h"
#include "XnStringProperty.h"
#include "XnGeneralProperty.h"
#include "XnPropertySetInternal.h"
#include <XnPsVersion.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_DUMP_STREAMS_DATA	"StreamsData"

//---------------------------------------------------------------------------
// Public Methods
//---------------------------------------------------------------------------
XnDeviceBase::XnDeviceBase() :
	m_pDevicePropertiesHolder(NULL),
	m_DeviceMirror(XN_MODULE_PROPERTY_MIRROR, "Mirror", TRUE),
	m_StreamsDataDump(NULL),
	m_hLock(NULL)
{
	// update set callbacks
	m_DeviceMirror.UpdateSetCallback(SetMirrorCallback, this);
}

XnDeviceBase::~XnDeviceBase()
{
}

XnStatus XnDeviceBase::Init(const XnDeviceConfig* pDeviceConfig)
{
	XnStatus nRetVal = XN_STATUS_OK;

	nRetVal = xnOSCreateCriticalSection(&m_hLock);
	XN_IS_STATUS_OK(nRetVal);

	// first init the device
	nRetVal = InitImpl(pDeviceConfig);
	XN_IS_STATUS_OK(nRetVal);

	// and now create streams
	if (pDeviceConfig->pInitialValues != NULL)
	{
		nRetVal = CreateStreams(pDeviceConfig->pInitialValues);
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::InitImpl(const XnDeviceConfig* pDeviceConfig)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pDeviceConfig);

	// create device module
	nRetVal = CreateDeviceModule(&m_pDevicePropertiesHolder);
	XN_IS_STATUS_OK(nRetVal);

	// check if we have initial values for device modules
	XnActualPropertiesHash* pDeviceModuleInitialProps = NULL;
	if (pDeviceConfig->pInitialValues != NULL)
	{
		pDeviceConfig->pInitialValues->pData->Get(XN_MODULE_NAME_DEVICE, pDeviceModuleInitialProps);
	}

	// init device module
	nRetVal = m_pDevicePropertiesHolder->Init(pDeviceModuleInitialProps);
	XN_IS_STATUS_OK(nRetVal);

	// add the device module
	nRetVal = AddModule(m_pDevicePropertiesHolder);
	XN_IS_STATUS_OK(nRetVal);

	// init dump
	m_StreamsDataDump = xnDumpFileOpen(XN_DUMP_STREAMS_DATA, "%s.csv", XN_DUMP_STREAMS_DATA);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::Destroy()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// free all modules
	while (m_Modules.Size() != 0)
	{
		XnDeviceModuleHolder* pModuleHolder = m_Modules.Begin()->Value();
		if (IsStream(pModuleHolder->GetModule()))
		{
			XnChar strName[XN_DEVICE_MAX_STRING_LENGTH];
			strcpy(strName, pModuleHolder->GetModule()->GetName());
			nRetVal = DestroyStream(strName);
			XN_IS_STATUS_OK(nRetVal);
		}
		else
		{
			// free memory of registered properties to this module
			FreeModuleRegisteredProperties(m_Modules.Begin()->Key());

			pModuleHolder->GetModule()->Free();
			DestroyModule(pModuleHolder);
			m_Modules.Remove(m_Modules.Begin());
		}
	}

	m_pDevicePropertiesHolder = NULL;

	m_Modules.Clear();

	// close dump
	xnDumpFileClose(m_StreamsDataDump);

	if (m_hLock != NULL)
	{
		xnOSCloseCriticalSection(&m_hLock);
		m_hLock = NULL;
	}

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::CreateModule(const XnChar* strName, XnDeviceModuleHolder** ppModuleHolder)
{
	XnDeviceModule* pModule;
	XnDeviceModuleHolder* pHolder;

	// create module
	XN_VALIDATE_NEW(pModule, XnDeviceModule, strName);

	// create holder
	pHolder = XN_NEW(XnDeviceModuleHolder, pModule);
	if (pHolder == NULL)
	{
		XN_DELETE(pModule);
		return XN_STATUS_ALLOC_FAILED;
	}

	*ppModuleHolder = pHolder;

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::CreateDeviceModule(XnDeviceModuleHolder** ppModuleHolder)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// create module
	nRetVal = CreateModule(XN_MODULE_NAME_DEVICE, ppModuleHolder);
	XN_IS_STATUS_OK(nRetVal);

	XnDeviceModule* pModule = (*ppModuleHolder)->GetModule();

	// add device properties
	XnProperty* pProps[] = { &m_DeviceMirror };

	nRetVal = pModule->AddProperties(pProps, sizeof(pProps)/sizeof(XnProperty*));
	if (nRetVal != XN_STATUS_OK)
	{
		DestroyModule(*ppModuleHolder);
		*ppModuleHolder = NULL;
		return (nRetVal);
	}

	return XN_STATUS_OK;
}

void XnDeviceBase::DestroyModule(XnDeviceModuleHolder* pModuleHolder)
{
	XN_DELETE(pModuleHolder->GetModule());
	XN_DELETE(pModuleHolder);
}

XnStatus XnDeviceBase::SetMirror(XnBool bMirror)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	// change all streams
	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		XnDeviceModuleHolder* pModuleHolder = it->Value();
		if (IsStream(pModuleHolder->GetModule()))
		{
			XnDeviceStream* pStream = (XnDeviceStream*)pModuleHolder->GetModule();
			nRetVal = pStream->SetMirror(bMirror);
			XN_IS_STATUS_OK(nRetVal);
		}
	}

	// and set property
	nRetVal = m_DeviceMirror.UnsafeUpdateValue(bMirror);
	XN_IS_STATUS_OK(nRetVal);
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::GetSupportedStreams(const XnChar** aStreamNames, XnUInt32* pnStreamNamesCount)
{
	XN_VALIDATE_OUTPUT_PTR(pnStreamNamesCount);
	// NOTE: we allow aStreamName to be NULL

	// first of all count streams
	XnUInt32 nStreamsCount = m_SupportedStreams.Size();

	// now check if we have enough room
	if (nStreamsCount > *pnStreamNamesCount)
	{
		*pnStreamNamesCount = nStreamsCount;
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	// now copy values
	nStreamsCount = 0;
	for (XnStringsSet::Iterator it = m_SupportedStreams.Begin(); it != m_SupportedStreams.End(); ++it)
	{
		aStreamNames[nStreamsCount] = it->Key();
		nStreamsCount++;
	}

	*pnStreamNamesCount = nStreamsCount;
	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::OpenStream(const XnChar* StreamName)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(StreamName);

	xnLogVerbose(XN_MASK_DDK, "Opening stream %s...", StreamName);

	// find this stream
	XnDeviceStream* pStream;
	nRetVal = FindStream(StreamName, &pStream);
	XN_IS_STATUS_OK(nRetVal);

	// open it
	nRetVal = pStream->Open();
	XN_IS_STATUS_OK(nRetVal);

	xnLogInfo(XN_MASK_DDK, "Stream %s is open.", StreamName);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::CloseStream(const XnChar* StreamName)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(StreamName);

	xnLogVerbose(XN_MASK_DDK, "Closing stream %s...", StreamName);

	// find this stream
	XnDeviceStream* pStream;
	nRetVal = FindStream(StreamName, &pStream);
	XN_IS_STATUS_OK(nRetVal);

	// close it
	nRetVal = pStream->Close();
	XN_IS_STATUS_OK(nRetVal);

	xnLogInfo(XN_MASK_DDK, "Stream %s is closed.", StreamName);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::OpenAllStreams()
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnLogVerbose(XN_MASK_DDK, "Opening all streams...");

	// go over modules list, and look for closed streams
	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		XnDeviceModuleHolder* pModuleHolder = it->Value();
		if (IsStream(pModuleHolder->GetModule()))
		{
			XnDeviceStream* pStream = (XnDeviceStream*)pModuleHolder->GetModule();
			if (!pStream->IsOpen())
			{
				nRetVal = pStream->Open();
				XN_IS_STATUS_OK(nRetVal);
			}
		}
	}

	xnLogInfo(XN_MASK_DDK, "All streams are open.");

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::CloseAllStreams()
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnLogVerbose(XN_MASK_DDK, "Closing all streams...");

	// go over modules list, and look for closed streams
	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		XnDeviceModuleHolder* pModuleHolder = it->Value();
		if (IsStream(pModuleHolder->GetModule()))
		{
			XnDeviceStream* pStream = (XnDeviceStream*)pModuleHolder->GetModule();
			if (pStream->IsOpen())
			{
				nRetVal = pStream->Close();
				XN_IS_STATUS_OK(nRetVal);
			}
		}
	}

	xnLogInfo(XN_MASK_DDK, "All streams are closed.");

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::GetStreamNames(const XnChar** pstrNames, XnUInt32* pnNamesCount)
{
	// first we need to count them
	XnUInt32 nCount = 0;

	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		XnDeviceModuleHolder* pModuleHolder = it->Value();
		if (IsStream(pModuleHolder->GetModule()))
		{
			nCount++;
		}
	}

	if (nCount > *pnNamesCount)
	{
		*pnNamesCount = nCount;
		return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;
	}

	// OK. we have enough space. Copy into it
	nCount = 0;
	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		XnDeviceModuleHolder* pModuleHolder = it->Value();
		if (IsStream(pModuleHolder->GetModule()))
		{
			pstrNames[nCount] = it->Key();
			nCount++;
		}
	}

	*pnNamesCount = nCount;

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::DoesModuleExist(const XnChar* ModuleName, XnBool* pbDoesExist)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(ModuleName);
	XN_VALIDATE_OUTPUT_PTR(pbDoesExist);

	*pbDoesExist = FALSE;

	XnDeviceModuleHolder* pModuleHolder;
	nRetVal = FindModule(ModuleName, &pModuleHolder);
	if (nRetVal == XN_STATUS_OK)
	{
		*pbDoesExist = TRUE;
	}
	else if (nRetVal != XN_STATUS_DEVICE_MODULE_NOT_FOUND)
	{
		return nRetVal;
	}

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::RegisterToNewStreamData(XnDeviceOnNewStreamDataEventHandler Handler, void* pCookie, XnCallbackHandle& hCallback)
{
	XN_VALIDATE_INPUT_PTR(Handler);

	return m_OnNewStreamDataEvent.Register(Handler, pCookie, hCallback);
}

XnStatus XnDeviceBase::UnregisterFromNewStreamData(XnCallbackHandle hCallback)
{
	XN_VALIDATE_INPUT_PTR(hCallback);

	return m_OnNewStreamDataEvent.Unregister(hCallback);
}

XnStatus XnDeviceBase::DoesPropertyExist(const XnChar* ModuleName, XnUInt32 propertyId, XnBool* pbDoesExist)
{
	XnStatus nRetVal = XN_STATUS_OK;

	*pbDoesExist = FALSE;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	if (nRetVal == XN_STATUS_DEVICE_MODULE_NOT_FOUND)
	{
		return XN_STATUS_OK;
	}
	else
	{
		XN_IS_STATUS_OK(nRetVal);
	}

	nRetVal = pModule->DoesPropertyExist(propertyId, pbDoesExist);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::GetPropertyType(const XnChar* ModuleName, XnUInt32 propertyId, XnPropertyType* pnType)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->GetPropertyType(propertyId, pnType);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->SetProperty(propertyId, nValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnDouble dValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->SetProperty(propertyId, dValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, const XnChar* csValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->SetProperty(propertyId, csValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, const OniGeneralBuffer& gbValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->SetProperty(propertyId, gbValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnUInt64* pnValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->GetProperty(propertyId, pnValue);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnDouble* pdValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->GetProperty(propertyId, pdValue);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnChar* csValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->GetProperty(propertyId, csValue);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, const OniGeneralBuffer& gbValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnDeviceModule* pModule;
	nRetVal = FindModule(ModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = pModule->GetProperty(propertyId, gbValue);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::LoadConfigFromFile(const XnChar* csINIFilePath, const XnChar* csSectionName)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	nRetVal = DeviceModule()->LoadConfigFromFile(csINIFilePath, csSectionName);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::BatchConfig(const XnPropertySet* pChangeSet)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XN_VALIDATE_INPUT_PTR(pChangeSet);

	for (XnPropertySetData::ConstIterator itModule = pChangeSet->pData->Begin(); itModule != pChangeSet->pData->End(); ++itModule)
	{
		// find this module
		XnDeviceModule* pModule = NULL;
		nRetVal = FindModule(itModule->Key(), &pModule);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = pModule->BatchConfig(*itModule->Value());
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::GetAllProperties(XnPropertySet* pSet, XnBool bNoStreams /* = FALSE */, const XnChar* strModule /* = NULL */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);

	// clear the set
	nRetVal = XnPropertySetClear(pSet);
	XN_IS_STATUS_OK(nRetVal);

	if (strModule != NULL)
	{
		XnDeviceModule* pModule;
		nRetVal = FindModule(strModule, &pModule);
		XN_IS_STATUS_OK(nRetVal);

		nRetVal = pModule->GetAllProperties(pSet);
		XN_IS_STATUS_OK(nRetVal);
	}
	else
	{
		// enumerate over modules
		for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
		{
			XnDeviceModuleHolder* pModuleHolder = it->Value();

			if (bNoStreams && IsStream(pModuleHolder->GetModule()))
				continue;

			nRetVal = pModuleHolder->GetModule()->GetAllProperties(pSet);
			XN_IS_STATUS_OK(nRetVal);
		}
	}

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::RegisterToPropertyChange(const XnChar* Module, XnUInt32 propertyId, XnDeviceOnPropertyChangedEventHandler Handler, void* pCookie, XnCallbackHandle& hCallback)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnDeviceModule* pModule;
	nRetVal = FindModule(Module, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	XnPropertyCallback* pRealCookie = NULL;
	XN_VALIDATE_NEW(pRealCookie, XnPropertyCallback, Module, propertyId, Handler, pCookie);

	// register
	nRetVal = pModule->RegisterForOnPropertyValueChanged(propertyId, PropertyValueChangedCallback, pRealCookie, pRealCookie->hCallback);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(pRealCookie);
		return (nRetVal);
	}

	m_PropertyCallbacks.AddLast(pRealCookie);

	hCallback = (XnCallbackHandle) pRealCookie;
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::UnregisterFromPropertyChange(const XnChar* Module, XnUInt32 propertyId, XnCallbackHandle hCallback)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XN_VALIDATE_INPUT_PTR(Module);
	XN_VALIDATE_INPUT_PTR(hCallback);

	XnPropertyCallback* pRealCookie = (XnPropertyCallback*)hCallback;

	XnDeviceModule* pModule;
	nRetVal = FindModule(Module, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	// first unregister it from property
	nRetVal = pModule->UnregisterFromOnPropertyValueChanged(propertyId, pRealCookie->hCallback);
	XN_IS_STATUS_OK(nRetVal);

	PropertiesCallbacks::Iterator it = m_PropertyCallbacks.Find(pRealCookie);
	if (it != m_PropertyCallbacks.End())
	{
		m_PropertyCallbacks.Remove(it);
	}

	// now free the memory
	XN_DELETE(pRealCookie);

	return (XN_STATUS_OK);
}

//---------------------------------------------------------------------------
// Protected Helper Methods
//---------------------------------------------------------------------------

XnStatus XnDeviceBase::AddModule(XnDeviceModuleHolder* pModuleHolder)
{
	XnDeviceModule* pModule = pModuleHolder->GetModule();

	// make sure module doesn't exist yet
	if (m_Modules.Find(pModule->GetName()) != m_Modules.End())
	{
		xnLogError(XN_MASK_DEVICE, "A module with the name %s already exists!", pModule->GetName());
		return XN_STATUS_ERROR;
	}

	// add it to the list
	XnStatus nRetVal = m_Modules.Set(pModule->GetName(), pModuleHolder);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::RemoveModule(const XnChar* ModuleName)
{
	// remove it
	XnStatus nRetVal = m_Modules.Remove(ModuleName);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::FindModule(const XnChar* ModuleName, XnDeviceModule** ppModule)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnDeviceModuleHolder* pHolder;
	nRetVal = FindModule(ModuleName, &pHolder);
	XN_IS_STATUS_OK(nRetVal);

	*ppModule = pHolder->GetModule();
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::FindModule(const XnChar* ModuleName, XnDeviceModuleHolder** ppModuleHolder)
{
	XnStatus nRetVal = XN_STATUS_OK;
	ModuleHoldersHash::Iterator it = m_Modules.Find(ModuleName);
	if (it == m_Modules.End())
	{
		return (XN_STATUS_DEVICE_MODULE_NOT_FOUND);
	}
	XN_IS_STATUS_OK(nRetVal);

	*ppModuleHolder = it->Value();

	return XN_STATUS_OK;
}

XnBool XnDeviceBase::IsStream(XnDeviceModule* pModule)
{
	XnProperty* pProperty;
	XnStatus nRetVal = pModule->GetProperty(XN_STREAM_PROPERTY_IS_STREAM, &pProperty);
	if (nRetVal != XN_STATUS_OK)
		return FALSE;

	if (pProperty->GetType() != XN_PROPERTY_TYPE_INTEGER)
		return FALSE;

	XnIntProperty* pIntProperty = (XnIntProperty*)pProperty;

	XnUInt64 nValue;
	nRetVal = pIntProperty->GetValue(&nValue);
	if (nRetVal != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_DDK, "Failed getting the value of the IsStream property: %s", xnGetStatusString(nRetVal));
		return FALSE;
	}

	return (XnBool)nValue;
}

XnStatus XnDeviceBase::FindStream(const XnChar* StreamName, XnDeviceStream** ppStream)
{
	// find the module
	XnDeviceModuleHolder* pStreamHolder = NULL;
	XnStatus nRetVal = FindStream(StreamName, &pStreamHolder);
	XN_IS_STATUS_OK(nRetVal);

	*ppStream = (XnDeviceStream*)pStreamHolder->GetModule();

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::FindStream(const XnChar* StreamName, XnDeviceModuleHolder** ppStreamHolder)
{
	// find the module
	XnDeviceModuleHolder* pModuleHolder = NULL;
	XnStatus nRetVal = FindModule(StreamName, &pModuleHolder);
	XN_IS_STATUS_OK(nRetVal);

	// check if this is a stream
	if (!IsStream(pModuleHolder->GetModule()))
		return XN_STATUS_MODULE_IS_NOT_STREAM;

	*ppStreamHolder = pModuleHolder;

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::AddSupportedStream(const XnChar* StreamType)
{
	// make sure stream doesn't exist yet
	XnStringsSet::Iterator it = m_SupportedStreams.End();
	if (XN_STATUS_OK == m_SupportedStreams.Find(StreamType, it))
	{
		xnLogError(XN_MASK_DEVICE, "A stream with the name %s already exists!", StreamType);
		return XN_STATUS_STREAM_ALREADY_EXISTS;
	}

	// add it to the list
	XnStatus nRetVal = m_SupportedStreams.Set(StreamType);
	XN_IS_STATUS_OK(nRetVal);

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::GetStreamRequiredDataSize(const XnChar* StreamName, XnUInt32* pnRequiredSize)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// find stream
	XnDeviceStream* pStream;
	nRetVal = FindStream(StreamName, &pStream);
	XN_IS_STATUS_OK(nRetVal);

	*pnRequiredSize = pStream->GetRequiredDataSize();

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::CreateStreams(const XnPropertySet* pSet)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	for (XnPropertySetData::ConstIterator it = pSet->pData->Begin(); it != pSet->pData->End(); ++it)
	{
		// check if this module is a stream
		XnActualPropertiesHash* pModule = it->Value();

		XnActualPropertiesHash::ConstIterator itProp = pModule->End();
		if (XN_STATUS_OK == pModule->Find(XN_STREAM_PROPERTY_TYPE, itProp))
		{
			// create a copy of the properties
			XnActualPropertiesHash streamProps(it->Key());
			nRetVal = streamProps.CopyFrom(*pModule);
			XN_IS_STATUS_OK(nRetVal);

			// remove the type property
			nRetVal = streamProps.Remove(XN_STREAM_PROPERTY_TYPE);
			XN_IS_STATUS_OK(nRetVal);

			// and create the stream
			XnActualStringProperty* pActualProp = (XnActualStringProperty*)itProp->Value();
			nRetVal = CreateStreamImpl(pActualProp->GetValue(), it->Key(), &streamProps);
			XN_IS_STATUS_OK(nRetVal);
		}
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::ValidateOnlyModule(const XnPropertySet* pSet, const XnChar* StreamName)
{
	XnPropertySetData::ConstIterator it = pSet->pData->Begin();
	if (it == pSet->pData->End())
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DDK, "Property set did not contain any stream!");
	}

	if (strcmp(it->Key(), StreamName) != 0)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DDK, "Property set module name does not match stream name!");
	}

	if (++it != pSet->pData->End())
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_DEVICE_BAD_PARAM, XN_MASK_DDK, "Property set contains more than one module!");
	}
	
	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::CreateStream(const XnChar* StreamType, const XnChar* StreamName /* = NULL */, const XnPropertySet* pInitialValues /* = NULL */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	// check for name
	if (StreamName == NULL)
		StreamName = StreamType;

	XnActualPropertiesHash* pInitialValuesHash = NULL;

	if (pInitialValues != NULL)
	{
		// validate property set
		nRetVal = ValidateOnlyModule(pInitialValues, StreamName);
		XN_IS_STATUS_OK(nRetVal);

		pInitialValuesHash = pInitialValues->pData->Begin()->Value();
	}

	nRetVal = CreateStreamImpl(StreamType, StreamName, pInitialValuesHash);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::CreateStreamImpl(const XnChar* strType, const XnChar* strName, const XnActualPropertiesHash* pInitialSet)
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnLogInfo(XN_MASK_DDK, "Creating stream '%s' of type '%s'...", strName, strType);

	xnl::AutoCSLocker lock(m_hLock);

	XnDeviceModule* pModule;
	if (FindModule(strName, &pModule) == XN_STATUS_OK)
	{
		// already exists. check sharing mode (when shared, we allow "creating" the same stream)
		if (!IsStream(pModule) ||
			strcmp(strType, ((XnDeviceStream*)pModule)->GetType()) != 0)
		{
			XN_LOG_WARNING_RETURN(XN_STATUS_STREAM_ALREADY_EXISTS, XN_MASK_DDK, "A stream with this name already exists!");
		}

		// OK, we'll allow this. Just set new configuration
		if (pInitialSet != NULL)
		{
			nRetVal = pModule->BatchConfig(*pInitialSet);
			XN_IS_STATUS_OK(nRetVal);
		}

		((XnDeviceStream*)pModule)->AddRef();
	}
	else
	{
		// create stream
		XnDeviceModuleHolder* pNewStreamHolder = NULL;

		nRetVal = CreateStreamModule(strType, strName, &pNewStreamHolder);
		XN_IS_STATUS_OK(nRetVal);

		XnDeviceStream* pNewStream = (XnDeviceStream*)(pNewStreamHolder->GetModule());
		if (pNewStream == NULL)
		{
			DestroyStreamModule(pNewStreamHolder);
			XN_LOG_ERROR_RETURN(XN_STATUS_ERROR, XN_MASK_DDK, "Internal Error: Invalid new stream!");
		}

		// initialize the stream
		xnLogVerbose(XN_MASK_DDK, "Initializing stream '%s'...", strName);

		nRetVal = pNewStreamHolder->Init(pInitialSet);
		if (nRetVal != XN_STATUS_OK)
		{
			DestroyStreamModule(pNewStreamHolder);
			return (nRetVal);
		}

		// set it's mirror value (if not requested otherwise)
		XnBool bSetMirror = TRUE;

		if (pInitialSet != NULL)
		{
			XnActualPropertiesHash::ConstIterator it = pInitialSet->End();
			if (XN_STATUS_OK == pInitialSet->Find(XN_MODULE_PROPERTY_MIRROR, it))
			{
				bSetMirror = FALSE;
			}
		}

		if (bSetMirror)
		{
			nRetVal = pNewStream->SetMirror((XnBool)m_DeviceMirror.GetValue());
			if (nRetVal != XN_STATUS_OK)
			{
				DestroyStreamModule(pNewStreamHolder);
				return (nRetVal);
			}
		}

		// add it to the list of existing modules
		nRetVal = AddModule(pNewStreamHolder);
		if (nRetVal != XN_STATUS_OK)
		{
			DestroyStreamModule(pNewStreamHolder);
			return (nRetVal);
		}

		xnLogInfo(XN_MASK_DDK, "Stream '%s' was initialized.", strName);

		pNewStream->SetNewDataCallback(NewStreamDataCallback, this);

		xnLogInfo(XN_MASK_DDK, "'%s' stream was created.", strName);
	}

	return (XN_STATUS_OK);
}

void XnDeviceBase::FreeModuleRegisteredProperties(const XnChar* strModule)
{
	// free memory of registered properties to this stream
	PropertiesCallbacks::Iterator it = m_PropertyCallbacks.Begin();
	while (it != m_PropertyCallbacks.End())
	{
		PropertiesCallbacks::Iterator cur = it;
		it++;

		XnPropertyCallback* pRealCallback = *cur;
		if (strcmp(pRealCallback->strModule, strModule) == 0)
		{
			m_PropertyCallbacks.Remove(cur);
			XN_DELETE(pRealCallback);
		}
	}
}

XnStatus XnDeviceBase::DestroyStream(const XnChar* StreamName)
{
	XnStatus nRetVal = XN_STATUS_OK;

	xnLogInfo(XN_MASK_DDK, "Destroying stream '%s'...", StreamName);

	// keep the stream name (we now delete the module, so the name will be lost)
	XnChar strStreamName[XN_DEVICE_MAX_STRING_LENGTH];
	strncpy(strStreamName, StreamName, XN_DEVICE_MAX_STRING_LENGTH);

	xnl::AutoCSLocker lock(m_hLock);

	// Find the stream
	XnDeviceModuleHolder* pStreamHolder;
	nRetVal = FindStream(strStreamName, &pStreamHolder);
	XN_IS_STATUS_OK(nRetVal);

	XnDeviceStream* pStream = (XnDeviceStream*)pStreamHolder->GetModule();
	XnUInt32 nRefCount = pStream->DecRef();
	if (0 == nRefCount)
	{
		// remove it from map
		nRetVal = RemoveModule(strStreamName);
		XN_IS_STATUS_OK(nRetVal);

		// and free it's memory
		DestroyStreamModule(pStreamHolder);

		// free memory of registered properties to this stream
		FreeModuleRegisteredProperties(StreamName);

		xnLogVerbose(XN_MASK_DDK, "'%s' stream destroyed.", strStreamName);
	}
	else
	{
		xnLogVerbose(XN_MASK_DDK, "'%s' stream now has %d references.", strStreamName, nRefCount);
	}

	return XN_STATUS_OK;
}

XnStatus XnDeviceBase::GetModulesList(XnDeviceModuleHolder** apModules, XnUInt32* pnCount)
{
	XnUInt32 nCount = 0;

	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		apModules[nCount] = it->Value();
		nCount++;
	}

	*pnCount = nCount;

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::GetModulesList(XnDeviceModuleHolderList& list)
{
	list.Clear();

	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		list.AddLast(it->Value());
	}

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::GetStreamsList(XnDeviceModuleHolderList& list)
{
	list.Clear();

	for (ModuleHoldersHash::Iterator it = m_Modules.Begin(); it != m_Modules.End(); ++it)
	{
		XnDeviceModuleHolder* pModuleHolder = it->Value();
		if (IsStream(pModuleHolder->GetModule()))
		{
			list.AddLast(pModuleHolder);
		}
	}

	return (XN_STATUS_OK);
}

XnStatus XnDeviceBase::RaiseNewStreamDataEvent(const XnChar* StreamName, OniFrame* pFrame)
{
	XnNewStreamDataEventArgs eventArgs;
	eventArgs.strStreamName = StreamName;
	eventArgs.pFrame = pFrame;

	m_OnNewStreamDataEvent.Raise(eventArgs);

	return XN_STATUS_OK;
}

void XnDeviceBase::OnNewStreamData(XnDeviceStream* pStream, OniFrame* pFrame)
{
	XnUInt64 nNow;
	xnOSGetHighResTimeStamp(&nNow);
	xnDumpFileWriteString(m_StreamsDataDump, "%llu,%s,%llu,%u\n", nNow, pStream->GetName(), pFrame->timestamp, pFrame->frameIndex);

	RaiseNewStreamDataEvent(pStream->GetName(), pFrame);
}

void XnDeviceBase::NewStreamDataCallback(XnDeviceStream* pSender, OniFrame* pFrame, void* pCookie)
{
	XnDeviceBase* pThis = (XnDeviceBase*)pCookie;
	pThis->OnNewStreamData(pSender, pFrame);
}

XnStatus XN_CALLBACK_TYPE XnDeviceBase::PropertyValueChangedCallback(const XnProperty* /*pSender*/, void* pCookie)
{
	XnPropertyCallback* pUserCallback = (XnPropertyCallback*)pCookie;

	// TODO: consider catching exceptions (if user does some stupid things)
	pUserCallback->pHandler(pUserCallback->strModule, pUserCallback->propertyId, pUserCallback->pCookie);

	return XN_STATUS_OK;
}

XnStatus XN_CALLBACK_TYPE XnDeviceBase::SetMirrorCallback(XnActualIntProperty* /*pSender*/, XnUInt64 nValue, void* pCookie)
{
	XnDeviceBase* pThis = (XnDeviceBase*)pCookie;
	return pThis->SetMirror((XnBool)nValue);
}

XnDeviceBase::XnPropertyCallback::XnPropertyCallback(const XnChar* strModule, XnUInt32 propertyId, XnDeviceOnPropertyChangedEventHandler pHandler, void* pCookie) :
	propertyId(propertyId),
	pCookie(pCookie),
	pHandler(pHandler)
{
	strcpy(this->strModule, strModule);
}
