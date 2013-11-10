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
#ifndef XNDEVICEBASE_H
#define XNDEVICEBASE_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnStringsHash.h>
#include <XnDevice.h>
#include <DDK/XnDeviceModule.h>
#include "XnDeviceModuleHolder.h"
#include <XnEvent.h>
#include <DDK/XnDeviceStream.h>
#include <DDK/XnActualStringProperty.h>
#include <DDK/XnActualIntProperty.h>
#include <DDK/XnActualGeneralProperty.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define XN_MASK_DEVICE						"Device"
#define XN_DEVICE_BASE_MAX_STREAMS_COUNT	100

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

class XnDeviceBase
{
public:
	XnDeviceBase();
	virtual ~XnDeviceBase();

	//---------------------------------------------------------------------------
	// Properties Getters
	//---------------------------------------------------------------------------
	inline XnActualIntProperty& DeviceMirrorProperty() { return m_DeviceMirror; }

	//---------------------------------------------------------------------------
	// Getters
	//---------------------------------------------------------------------------
	inline XnBool GetDeviceMirror() const { return (XnBool)m_DeviceMirror.GetValue(); }

	inline XnDeviceModule* DeviceModule() { return m_pDevicePropertiesHolder->GetModule(); }
	inline XnDeviceModuleHolder* DeviceModuleHolder() { return m_pDevicePropertiesHolder; }

	//---------------------------------------------------------------------------
	// Setters
	//---------------------------------------------------------------------------
	virtual XnStatus SetMirror(XnBool bMirror);

	//---------------------------------------------------------------------------
	// IXnDevice Methods
	//---------------------------------------------------------------------------
	virtual XnStatus Init(const XnDeviceConfig* pDeviceConfig);
	virtual XnStatus Destroy();
	virtual XnStatus GetSupportedStreams(const XnChar** aStreamNames, XnUInt32* pnStreamNamesCount);
	virtual XnStatus CreateStream(const XnChar* StreamType, const XnChar* StreamName = NULL, const XnPropertySet* pInitialValues = NULL);
	virtual XnStatus DestroyStream(const XnChar* StreamName);
	virtual XnStatus OpenStream(const XnChar* StreamName);
	virtual XnStatus CloseStream(const XnChar* StreamName);
	virtual XnStatus GetStreamNames(const XnChar** pstrNames, XnUInt32* pnNamesCount);
	virtual XnStatus DoesModuleExist(const XnChar* ModuleName, XnBool* pbDoesExist);
	virtual XnStatus OpenAllStreams();
	virtual XnStatus CloseAllStreams();
	virtual XnStatus RegisterToNewStreamData(XnDeviceOnNewStreamDataEventHandler Handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual XnStatus UnregisterFromNewStreamData(XnCallbackHandle hCallback);
	virtual XnStatus DoesPropertyExist(const XnChar* ModuleName, XnUInt32 propertyId, XnBool* pbDoesExist);
	virtual XnStatus GetPropertyType(const XnChar* ModuleName, XnUInt32 propertyId, XnPropertyType* pnType);
	virtual XnStatus SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnUInt64 nValue);
	virtual XnStatus SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnDouble dValue);
	virtual XnStatus SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, const XnChar* csValue);
	virtual XnStatus SetProperty(const XnChar* ModuleName, XnUInt32 propertyId, const OniGeneralBuffer& Value);
	virtual XnStatus GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnUInt64* pnValue);
	virtual XnStatus GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnDouble* pdValue);
	virtual XnStatus GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, XnChar* csValue);
	virtual XnStatus GetProperty(const XnChar* ModuleName, XnUInt32 propertyId, const OniGeneralBuffer& pValue);
	virtual XnStatus LoadConfigFromFile(const XnChar* csINIFilePath, const XnChar* csSectionName);
	virtual XnStatus BatchConfig(const XnPropertySet* pChangeSet);
	virtual XnStatus GetAllProperties(XnPropertySet* pSet, XnBool bNoStreams = FALSE, const XnChar* strModule = NULL);
	virtual XnStatus RegisterToPropertyChange(const XnChar* Module, XnUInt32 propertyId, XnDeviceOnPropertyChangedEventHandler Handler, void* pCookie, XnCallbackHandle& hCallback);
	virtual XnStatus UnregisterFromPropertyChange(const XnChar* Module, XnUInt32 propertyId, XnCallbackHandle hCallback);

	typedef xnl::Event<XnNewStreamDataEventArgs> NewStreamDataEvent;
	NewStreamDataEvent::Interface& OnNewStreamDataEvent() { return m_OnNewStreamDataEvent; }

	/**
	* Finds a stream (a module which has the IS_STREAM property set to TRUE). 
	*/
	XnStatus FindStream(const XnChar* StreamName, XnDeviceStream** ppStream);

protected:
	virtual XnStatus InitImpl(const XnDeviceConfig* pDeviceConfig);
	virtual XnStatus CreateStreamImpl(const XnChar* strType, const XnChar* strName, const XnActualPropertiesHash* pInitialSet);

	virtual XnStatus CreateModule(const XnChar* strName, XnDeviceModuleHolder** ppModuleHolder);
	virtual XnStatus CreateDeviceModule(XnDeviceModuleHolder** ppModuleHolder);
	virtual void DestroyModule(XnDeviceModuleHolder* pModuleHolder);

	XnStatus CreateStreams(const XnPropertySet* pSet);

	/**
	* Adds a module to the device modules.
	*/
	XnStatus AddModule(XnDeviceModuleHolder* pModuleHolder);

	/**
	* Removes a module from the device modules.
	*/
	XnStatus RemoveModule(const XnChar* ModuleName);

	/**
	* Finds a module. 
	*/
	XnStatus FindModule(const XnChar* ModuleName, XnDeviceModule** ppModule);

	/**
	* Finds a module. 
	*/
	XnStatus FindModule(const XnChar* ModuleName, XnDeviceModuleHolder** ppModuleHolder);

	/**
	* Checks if a module is a stream.
	*/
	static XnBool IsStream(XnDeviceModule* pModule);

	/**
	* Finds a stream holder (a module which has the IS_STREAM property set to TRUE). 
	*/
	XnStatus FindStream(const XnChar* StreamName, XnDeviceModuleHolder** ppStreamHolder);

	/**
	* Adds a stream to the list of supported streams.
	*/
	XnStatus AddSupportedStream(const XnChar* StreamType);

	/**
	* Creates a stream.
	*
	* @param	StreamType	[in]	Type of the stream to create.
	* @param	StreamName	[in]	The name of the new stream.
	*/
	virtual XnStatus CreateStreamModule(const XnChar* StreamType, const XnChar* StreamName, XnDeviceModuleHolder** ppStreamHolder) = 0;

	virtual void DestroyStreamModule(XnDeviceModuleHolder* pStreamHolder) = 0;

	/**
	* Gets the required output size of a stream.
	*/
	XnStatus GetStreamRequiredDataSize(const XnChar* StreamName, XnUInt32* pnRequiredSize);

	/**
	* Gets the list of modules the device supports.
	*
	* @param	aModules	[out]	an array of modules.
	* @param	pnModules	[out]	The number of modules.
	*/
	XnStatus GetModulesList(XnDeviceModuleHolder** apModules, XnUInt32* pnCount);
	XnStatus GetModulesList(XnDeviceModuleHolderList& list);

	XnStatus GetStreamsList(XnDeviceModuleHolderList& list);

	/**
	* Raises the NewStreamData event.
	*
	* @param	StreamName	[in]	The name of the stream with new data.
	*/
	XnStatus RaiseNewStreamDataEvent(const XnChar* StreamName, OniFrame* pFrame);

	/** Gets called when a stream has new data. */
	virtual void OnNewStreamData(XnDeviceStream* pStream, OniFrame* pFrame);

	XnStatus ValidateOnlyModule(const XnPropertySet* pSet, const XnChar* StreamName);

private:
	void FreeModuleRegisteredProperties(const XnChar* strModule);

	static XnStatus XN_CALLBACK_TYPE PropertyValueChangedCallback(const XnProperty* pSender, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetMirrorCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);

	static void NewStreamDataCallback(XnDeviceStream* pSender, OniFrame* pFrame, void* pCookie);

	XnDeviceModuleHolder* m_pDevicePropertiesHolder;
	XnActualIntProperty m_DeviceMirror;

	static XnStatus XN_CALLBACK_TYPE StreamNewDataCallback(XnDeviceStream* pStream, void* pCookie);

	typedef XnStringsHashT<XnDeviceModuleHolder*> ModuleHoldersHash;
	ModuleHoldersHash m_Modules;

	XnStringsSet m_SupportedStreams;

	struct XnPropertyCallback
	{
		XnPropertyCallback(const XnChar* strModule, XnUInt32 propertyId, XnDeviceOnPropertyChangedEventHandler pHandler, void* pCookie);

		XnChar strModule[XN_DEVICE_MAX_STRING_LENGTH];
		XnUInt32 propertyId;
		void* pCookie;
		XnDeviceOnPropertyChangedEventHandler pHandler;
		XnCallbackHandle hCallback;
	};
	typedef xnl::List<XnPropertyCallback*> PropertiesCallbacks;
	PropertiesCallbacks m_PropertyCallbacks;

	NewStreamDataEvent m_OnNewStreamDataEvent;

	XnDumpFile* m_StreamsDataDump;

	XN_CRITICAL_SECTION_HANDLE m_hLock;
};

#endif // XNDEVICEBASE_H
