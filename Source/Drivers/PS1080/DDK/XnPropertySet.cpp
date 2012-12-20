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
#include "XnPropertySetInternal.h"
#include "XnActualIntProperty.h"
#include "XnActualRealProperty.h"
#include "XnActualStringProperty.h"
#include "XnActualGeneralProperty.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
struct XnPropertySetModuleEnumerator
{
	XnBool bFirst;
	XnPropertySetData* pModules;
	XnPropertySetData::ConstIterator it;
};

struct XnPropertySetEnumerator
{
	XnBool bFirst;
	XnPropertySetData* pModules;
	XnPropertySetData::ConstIterator itModule;
	XnChar strModule[XN_DEVICE_MAX_STRING_LENGTH];
	XnActualPropertiesHash::ConstIterator itProp;
};

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnStatus XnPropertySetCreate(XnPropertySet** ppSet)
{
	XN_VALIDATE_OUTPUT_PTR(ppSet);

	XnPropertySet* pSet;
	XN_VALIDATE_ALLOC(pSet, XnPropertySet);

	pSet->pData = XN_NEW(XnPropertySetData);
	if (pSet->pData == NULL)
	{
		xnOSFree(pSet);
		return XN_STATUS_ALLOC_FAILED;
	}

	*ppSet = pSet;

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetDestroy(XnPropertySet** ppSet)
{
	XN_VALIDATE_INPUT_PTR(ppSet);
	XN_VALIDATE_INPUT_PTR(*ppSet);

	XnPropertySet* pSet = (*ppSet);

	if (pSet->pData != NULL)
	{
		XnPropertySetClear(pSet);
		XN_DELETE(pSet->pData);
	}

	xnOSFree(pSet);

	*ppSet = NULL;

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetClear(XnPropertySet* pSet)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);

	while (!pSet->pData->IsEmpty())
	{
		nRetVal = XnPropertySetRemoveModule(pSet, pSet->pData->Begin()->Key());
		XN_IS_STATUS_OK(nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetAddModule(XnPropertySet* pSet, const XnChar* strModuleName)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModuleName);

	XnActualPropertiesHash* pModule = NULL;

	// make sure this module doesn't already exists
	nRetVal = pSet->pData->Get(strModuleName, pModule);
	if (XN_STATUS_OK == nRetVal)
	{
		return XN_STATUS_DEVICE_MODULE_ALREADY_EXISTS;
	}

	// doesn't exist. create a new one
	XN_VALIDATE_NEW(pModule, XnActualPropertiesHash, strModuleName);

	nRetVal = XnPropertySetDataAttachModule(pSet->pData, strModuleName, pModule);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_DELETE(pModule);
		return (nRetVal);
	}

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetRemoveModule(XnPropertySet* pSet, const XnChar* strModuleName)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModuleName);

	XnActualPropertiesHash* pModule = NULL;
	nRetVal = XnPropertySetDataDetachModule(pSet->pData, strModuleName, &pModule);
	XN_IS_STATUS_OK(nRetVal);

	// now free its memory
	XN_DELETE(pModule);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetAddIntProperty(XnPropertySet* pSet, const XnChar* strModuleName, XnUInt32 propertyId, XnUInt64 nValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModuleName);

	// get module
	XnActualPropertiesHash* pModule = NULL;
	nRetVal = pSet->pData->Get(strModuleName, pModule);
	XN_IS_STATUS_OK(nRetVal);

	// add property
	nRetVal = pModule->Add(propertyId, "", nValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetAddRealProperty(XnPropertySet* pSet, const XnChar* strModuleName, XnUInt32 propertyId, XnDouble dValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModuleName);

	// get module
	XnActualPropertiesHash* pModule = NULL;
	nRetVal = pSet->pData->Get(strModuleName, pModule);
	XN_IS_STATUS_OK(nRetVal);

	// add property
	nRetVal = pModule->Add(propertyId, "", dValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetAddStringProperty(XnPropertySet* pSet, const XnChar* strModuleName, XnUInt32 propertyId, const XnChar* strValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModuleName);
	XN_VALIDATE_INPUT_PTR(strValue);

	// get module
	XnActualPropertiesHash* pModule = NULL;
	nRetVal = pSet->pData->Get(strModuleName, pModule);
	XN_IS_STATUS_OK(nRetVal);

	// add property
	nRetVal = pModule->Add(propertyId, "", strValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetAddGeneralProperty(XnPropertySet* pSet, const XnChar* strModuleName, XnUInt32 propertyId, const OniGeneralBuffer* pgbValue)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModuleName);
	XN_VALIDATE_INPUT_PTR(pgbValue);

	// get module
	XnActualPropertiesHash* pModule = NULL;
	nRetVal = pSet->pData->Get(strModuleName, pModule);
	XN_IS_STATUS_OK(nRetVal);

	// add property
	nRetVal = pModule->Add(propertyId, "", *pgbValue);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetRemoveProperty(XnPropertySet* pSet, const XnChar* strModuleName, XnUInt32 propertyId)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModuleName);

	// get module
	XnActualPropertiesHash* pModule = NULL;
	nRetVal = pSet->pData->Get(strModuleName, pModule);
	XN_IS_STATUS_OK(nRetVal);

	// remove the property
	nRetVal = pModule->Remove(propertyId);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetGetModuleEnumerator(const XnPropertySet* pSet, XnPropertySetModuleEnumerator** ppEnumerator)
{
	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_OUTPUT_PTR(ppEnumerator);

	XnPropertySetModuleEnumerator* pEnumer;
	XN_VALIDATE_NEW(pEnumer, XnPropertySetModuleEnumerator);

	pEnumer->bFirst = TRUE;
	pEnumer->it = pSet->pData->End();
	pEnumer->pModules = pSet->pData;

	*ppEnumerator = pEnumer;

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetModuleEnumeratorFree(XnPropertySetModuleEnumerator** ppEnumer)
{
	XN_VALIDATE_INPUT_PTR(ppEnumer);
	XN_VALIDATE_INPUT_PTR(*ppEnumer);

	XN_DELETE(*ppEnumer);
	*ppEnumer = NULL;

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetModuleEnumeratorMoveNext(XnPropertySetModuleEnumerator* pEnumerator, XnBool* pbEnd)
{
	XN_VALIDATE_INPUT_PTR(pEnumerator);
	XN_VALIDATE_OUTPUT_PTR(pbEnd);

	if (pEnumerator->bFirst)
	{
		pEnumerator->it = pEnumerator->pModules->Begin();
		pEnumerator->bFirst = FALSE;
	}
	else if (pEnumerator->it == pEnumerator->pModules->End())
	{
		return XN_STATUS_ILLEGAL_POSITION;
	}
	else
	{
		pEnumerator->it++;
	}

	*pbEnd = (pEnumerator->it == pEnumerator->pModules->End());

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetModuleEnumeratorGetCurrent(const XnPropertySetModuleEnumerator* pEnumer, const XnChar** pstrModuleName)
{
	XN_VALIDATE_INPUT_PTR(pEnumer);
	XN_VALIDATE_OUTPUT_PTR(pstrModuleName);

	if (pEnumer->it == pEnumer->pModules->End())
	{
		return XN_STATUS_ILLEGAL_POSITION;
	}

	*pstrModuleName = pEnumer->it->Key();

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetGetEnumerator(const XnPropertySet* pSet, XnPropertySetEnumerator** ppEnumerator, const XnChar* strModule /* = NULL */)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_OUTPUT_PTR(ppEnumerator);

	if (strModule != NULL)
	{
		// make sure module exists
		XnPropertySetData::ConstIterator it = pSet->pData->End();
		nRetVal = pSet->pData->Find(strModule, it);
		XN_IS_STATUS_OK(nRetVal);
	}

	XnPropertySetEnumerator* pEnumer;
	XN_VALIDATE_NEW(pEnumer, XnPropertySetEnumerator)

		pEnumer->bFirst = TRUE;
	pEnumer->pModules = pSet->pData;
	if (strModule != NULL)
	{
		strncpy(pEnumer->strModule, strModule, XN_DEVICE_MAX_STRING_LENGTH);
	}
	else
	{
		pEnumer->strModule[0] = '\0';
	}

	*ppEnumerator = pEnumer;

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetFindProperty(const XnPropertySet* pSet, const XnChar* strModule, XnUInt32 propertyId, XnPropertySetEnumerator** ppEnumerator)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSet);
	XN_VALIDATE_INPUT_PTR(strModule);
	XN_VALIDATE_OUTPUT_PTR(ppEnumerator);

	// find module
	XnPropertySetData::Iterator itModule = pSet->pData->End();
	nRetVal = pSet->pData->Find(strModule, itModule);
	XN_IS_STATUS_OK(nRetVal);

	XnActualPropertiesHash* pModule = itModule->Value();

	// find property
	XnActualPropertiesHash::Iterator itProp = pModule->End();
	nRetVal = pModule->Find(propertyId, itProp);
	XN_IS_STATUS_OK(nRetVal);

	// create enumerator
	XnPropertySetEnumerator* pEnumer;
	XN_VALIDATE_NEW(pEnumer, XnPropertySetEnumerator);

	pEnumer->itModule = itModule;
	pEnumer->itProp = itProp;
	pEnumer->pModules = pSet->pData;
	pEnumer->strModule[0] = '\0';
	pEnumer->bFirst = FALSE;

	*ppEnumerator = pEnumer;

	return XN_STATUS_OK;
}

XnStatus XnPropertySetEnumeratorFree(XnPropertySetEnumerator** ppEnumerator)
{
	XN_VALIDATE_INPUT_PTR(ppEnumerator);
	XN_VALIDATE_INPUT_PTR(*ppEnumerator);

	XN_DELETE(*ppEnumerator);
	*ppEnumerator = NULL;

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetEnumeratorMoveNext(XnPropertySetEnumerator* pEnumerator, XnBool* pbEnd)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pEnumerator);
	XN_VALIDATE_OUTPUT_PTR(pbEnd);

	*pbEnd = TRUE;

	if (pEnumerator->strModule[0] != '\0') // single module
	{
		if (pEnumerator->bFirst)
		{
			pEnumerator->bFirst = FALSE;

			// find this module
			nRetVal = pEnumerator->pModules->Find(pEnumerator->strModule, pEnumerator->itModule);
			if (nRetVal == XN_STATUS_NO_MATCH)
			{
				pEnumerator->itModule = pEnumerator->pModules->End();
			}
			XN_IS_STATUS_OK(nRetVal);

			pEnumerator->itProp = pEnumerator->itModule->Value()->Begin();
		}
		else if (pEnumerator->itProp == pEnumerator->itModule->Value()->End())
		{
			return XN_STATUS_ILLEGAL_POSITION;
		}
		else
		{
			// advance prop iterator
			++pEnumerator->itProp;
		}

		*pbEnd = (pEnumerator->itProp == pEnumerator->itModule->Value()->End());
	}
	else // all modules
	{
		if (pEnumerator->bFirst)
		{
			pEnumerator->bFirst = FALSE;

			// search for the first modules that has properties
			pEnumerator->itModule = pEnumerator->pModules->Begin();
			while (pEnumerator->itModule != pEnumerator->pModules->End() && pEnumerator->itModule->Value()->IsEmpty())
			{
				pEnumerator->itModule++;
			}

			// if we found one, take it's first property
			if (pEnumerator->itModule != pEnumerator->pModules->End())
			{
				pEnumerator->itProp = pEnumerator->itModule->Value()->Begin();
				*pbEnd = FALSE;
			}
			else
			{
				*pbEnd = TRUE;
			}
		}
		else if (pEnumerator->itModule == pEnumerator->pModules->End())
		{
			return XN_STATUS_ILLEGAL_POSITION;
		}
		else
		{
			// move to next one
			++pEnumerator->itProp;

			// check if we reached end of module
			if (pEnumerator->itProp == pEnumerator->itModule->Value()->End())
			{
				// move to next module with properties
				do
				{
					pEnumerator->itModule++;
				}
				while (pEnumerator->itModule != pEnumerator->pModules->End() && pEnumerator->itModule->Value()->IsEmpty());

				// if we found one, take it's first property
				if (pEnumerator->itModule != pEnumerator->pModules->End())
				{
					pEnumerator->itProp = pEnumerator->itModule->Value()->Begin();
					*pbEnd = FALSE;
				}
				else
				{
					*pbEnd = TRUE;
				}
			}
			else
			{
				*pbEnd = FALSE;
			}
		}
	}

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetEnumeratorGetCurrentPropertyInfo(const XnPropertySetEnumerator* pEnumerator, XnPropertyType* pnType, const XnChar** pstrModule, const XnChar** pstrProp)
{
	XN_VALIDATE_INPUT_PTR(pEnumerator);
	XN_VALIDATE_OUTPUT_PTR(pnType);
	XN_VALIDATE_OUTPUT_PTR(pstrModule);
	XN_VALIDATE_OUTPUT_PTR(pstrProp);

	XnProperty* pProp = pEnumerator->itProp->Value();
	*pnType = pProp->GetType();
	*pstrModule = pProp->GetModule();
	*pstrProp = pProp->GetName();

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetEnumeratorGetIntValue(const XnPropertySetEnumerator* pEnumerator, XnUInt64* pnValue)
{
	XN_VALIDATE_INPUT_PTR(pEnumerator);
	XN_VALIDATE_OUTPUT_PTR(pnValue);

	XnProperty* pPropBase = pEnumerator->itProp->Value();
	if (pPropBase->GetType() != XN_PROPERTY_TYPE_INTEGER)
	{
		return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
	}

	XnActualIntProperty* pProp = (XnActualIntProperty*)pPropBase;
	*pnValue = pProp->GetValue();

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetEnumeratorGetRealValue(const XnPropertySetEnumerator* pEnumerator, XnDouble* pdValue)
{
	XN_VALIDATE_INPUT_PTR(pEnumerator);
	XN_VALIDATE_OUTPUT_PTR(pdValue);

	XnProperty* pPropBase = pEnumerator->itProp->Value();
	if (pPropBase->GetType() != XN_PROPERTY_TYPE_REAL)
	{
		return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
	}

	XnActualRealProperty* pProp = (XnActualRealProperty*)pPropBase;
	*pdValue = pProp->GetValue();

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetEnumeratorGetStringValue(const XnPropertySetEnumerator* pEnumerator, const XnChar** pstrValue)
{
	XN_VALIDATE_INPUT_PTR(pEnumerator);
	XN_VALIDATE_OUTPUT_PTR(pstrValue);

	XnProperty* pPropBase = pEnumerator->itProp->Value();
	if (pPropBase->GetType() != XN_PROPERTY_TYPE_STRING)
	{
		return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
	}

	XnActualStringProperty* pProp = (XnActualStringProperty*)pPropBase;
	*pstrValue = pProp->GetValue();

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetEnumeratorGetGeneralValue(const XnPropertySetEnumerator* pEnumerator, OniGeneralBuffer* pgbValue)
{
	XN_VALIDATE_INPUT_PTR(pEnumerator);
	XN_VALIDATE_OUTPUT_PTR(pgbValue);

	XnProperty* pPropBase = pEnumerator->itProp->Value();
	if (pPropBase->GetType() != XN_PROPERTY_TYPE_GENERAL)
	{
		return XN_STATUS_DEVICE_PROPERTY_BAD_TYPE;
	}

	XnActualGeneralProperty* pProp = (XnActualGeneralProperty*)pPropBase;
	*pgbValue = pProp->GetValue();

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetDataAttachModule(XnPropertySetData* pSetData, const XnChar* strModuleName, XnActualPropertiesHash* pModule)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSetData);
	XN_VALIDATE_INPUT_PTR(strModuleName);
	XN_VALIDATE_INPUT_PTR(pModule);

	nRetVal = pSetData->Set(strModuleName, pModule);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetDataDetachModule(XnPropertySetData* pSetData, const XnChar* strModuleName, XnActualPropertiesHash** ppModule)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XN_VALIDATE_INPUT_PTR(pSetData);
	XN_VALIDATE_INPUT_PTR(strModuleName);
	XN_VALIDATE_OUTPUT_PTR(ppModule);

	// remove it
	XnPropertySetDataInternal::Iterator it = pSetData->Find(strModuleName);
	if (it == pSetData->End())
	{
		return XN_STATUS_NO_MATCH;
	}

	*ppModule = it->Value();

	nRetVal = pSetData->Remove(strModuleName);
	XN_IS_STATUS_OK(nRetVal);

	return (XN_STATUS_OK);
}

XnStatus XnPropertySetCloneModule(const XnPropertySet* pSource, XnPropertySet* pDest, const XnChar* strModule, const XnChar* strNewName)
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnActualPropertiesHash* pModuleProps = NULL;
	nRetVal = pSource->pData->Get(strModule, pModuleProps);
	XN_IS_STATUS_OK(nRetVal);

	nRetVal = XnPropertySetAddModule(pDest, strNewName);
	XN_IS_STATUS_OK(nRetVal);

	for (XnActualPropertiesHash::ConstIterator it = pModuleProps->Begin(); it != pModuleProps->End(); ++it)
	{
		XnProperty* pProp = it->Value();
		switch (pProp->GetType())
		{
		case XN_PROPERTY_TYPE_INTEGER:
			{
				XnActualIntProperty* pIntProp = (XnActualIntProperty*)pProp;
				nRetVal = XnPropertySetAddIntProperty(pDest, strNewName, pIntProp->GetId(), pIntProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_REAL:
			{
				XnActualRealProperty* pRealProp = (XnActualRealProperty*)pProp;
				nRetVal = XnPropertySetAddRealProperty(pDest, strNewName, pRealProp->GetId(), pRealProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_STRING:
			{
				XnActualStringProperty* pStrProp = (XnActualStringProperty*)pProp;
				nRetVal = XnPropertySetAddStringProperty(pDest, strNewName, pStrProp->GetId(), pStrProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		case XN_PROPERTY_TYPE_GENERAL:
			{
				XnActualGeneralProperty* pGenProp = (XnActualGeneralProperty*)pProp;
				nRetVal = XnPropertySetAddGeneralProperty(pDest, strNewName, pGenProp->GetId(), &pGenProp->GetValue());
				XN_IS_STATUS_OK(nRetVal);
				break;
			}
		default:
			XN_LOG_WARNING_RETURN(XN_STATUS_ERROR, XN_MASK_DDK, "Unknown property type: %d", pProp->GetType());
		}
	}

	return (XN_STATUS_OK);
}
