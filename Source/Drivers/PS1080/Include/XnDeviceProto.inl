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
/**
* Gets the definition of a device.
* 
* @param	pDeviceDefinition	[out]	The returned device definition.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetDefinition, (XnDeviceDefinition* pDeviceDefinition))

/**
* This function returns a list of possible connection strings for the specified device. (For example, the list of attached sensors' serial numbers', in the case of a sensor device).
* 
* @param	aConnectionStrings		[in/out]	An array to be filled with connection strings.
* @param	pnCount					[in/out]	In: the size of the array. Out: the number of elements filled in the array.
*/
XN_DEVICE_INTERFACE_FUNCTION(Enumerate, (XnConnectionString* aConnectionStrings, XnUInt32* pnCount))

/**
* This function will create a device and return a handle to it.
* 
* @param	pDeviceHandle			[out]		The opened device handle. If the function fails, NULL is returned.
* @param	pDeviceConfig			[in]		The requested device configuration mode. Contains the mode (read/write) and the target connection string.
*/
XN_DEVICE_INTERFACE_FUNCTION(Create, (XnDeviceHandle* pDeviceHandle, const XnDeviceConfig* pDeviceConfig))

/**
* Destroys a previously created device.
* 
* @param	pDeviceHandle			[in/out]	The requested device handle.
*/
XN_DEVICE_INTERFACE_FUNCTION(Destroy, (XnDeviceHandle* pDeviceHandle))

/**
* Returns the types of the streams supported by this device.
* 
* @param	DeviceHandle		[in]		The requested device handle.
* @param	aStreamName			[in/out]	An array of stream names. Will be filled by the function.
* @param	pnStreamNamesCount	[in/out]	The size of the array. Upon successful return, will contain the number of elements written to the array.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetSupportedStreams,(const XnDeviceHandle DeviceHandle, const XnChar** aStreamName, XnUInt32* pnStreamNamesCount))

/**
* Creates a new stream in the device.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	StreamType		[in]	The type of the stream to create (one of the types returned by XnDeviceEnumerateStreams).
* @param	StreamName		[in]	A name for the new stream.
* @param	pInitialValues	[in]	[Optional] A set of initial values for properties.
*/
XN_DEVICE_INTERFACE_FUNCTION(CreateStream,(const XnDeviceHandle DeviceHandle, const XnChar* StreamType, const XnChar* StreamName, const XnPropertySet* pInitialValues))

/**
* Destroys a previously created stream.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	StreamName		[in]	The name of the stream to destroy.
*/
XN_DEVICE_INTERFACE_FUNCTION(DestroyStream,(const XnDeviceHandle DeviceHandle, const XnChar* StreamName))

/**
* Opens a stream for I/O operations.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	StreamName		[in]	The name of the stream to open.
*/
XN_DEVICE_INTERFACE_FUNCTION(OpenStream,(const XnDeviceHandle DeviceHandle, const XnChar* StreamName))

/**
* Closes an open stream.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	StreamName		[in]	The name of the stream to close.
*/
XN_DEVICE_INTERFACE_FUNCTION(CloseStream,(const XnDeviceHandle DeviceHandle, const XnChar* StreamName))

/**
* Opens all closed streams.
* 
* @param	DeviceHandle	[in]	The requested device handle.
*/
XN_DEVICE_INTERFACE_FUNCTION(OpenAllStreams,(const XnDeviceHandle DeviceHandle))

/**
* Closes all open streams.
* 
* @param	DeviceHandle	[in]	The requested device handle.
*/
XN_DEVICE_INTERFACE_FUNCTION(CloseAllStreams,(const XnDeviceHandle DeviceHandle))

/**
* Get a list of all the streams that exist in the device.
* 
* @param	DeviceHandle		[in]		The requested device handle.
* @param	pstrStreamNames		[in/out]	An array of stream names. Will be filled by the function.
* @param	pnArraySize			[in/out]	The size of the array. Upon successful return, will contain the number of elements written to the array.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetStreamNames,(const XnDeviceHandle DeviceHandle, const XnChar** pstrStreamNames, XnUInt32* pnArraySize))

/**
* Checks if a specific module exists in this device.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	The name of the module to look for.
* @param	pbDoesExist		[out]	TRUE if the module exists, FALSE otherwise.
*/
XN_DEVICE_INTERFACE_FUNCTION(DoesModuleExist,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, XnBool* pbDoesExist))

/**
* Registers to the event of streams change (stream created / destroyed)
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	Handler			[in]	A pointer to the function that will handle the event.
* @param	pCookie			[in]	User cookie that will be passed as an argument to the event handler.
* @param	hCallback		[out]	A handle for unregister.
*/
XN_DEVICE_INTERFACE_FUNCTION(RegisterToStreamsChange,(const XnDeviceHandle DeviceHandle, XnDeviceOnStreamsChangedEventHandler Handler, void* pCookie, XnCallbackHandle& hCallback))

/**
* Unregisters from the event of streams change (stream created / destroyed)
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	hCallback		[in]	The handle returned from RegisterToStreamsChange.
*/
XN_DEVICE_INTERFACE_FUNCTION(UnregisterFromStreamsChange,(const XnDeviceHandle DeviceHandle, XnCallbackHandle hCallback))

/**
* Creates a stream data object for the requested stream.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	StreamName		[in]	The requested stream.
* @param	ppStreamData	[out]	The created stream data object. 
*/
XN_DEVICE_INTERFACE_FUNCTION(CreateStreamData,(const XnDeviceHandle DeviceHandle, const XnChar* StreamName, XnStreamData** ppStreamOutput))

/**
* Destroys a stream output object that was previously created using CreateStreamData.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ppStreamData	[in]	The stream output object to destroy.
*/
XN_DEVICE_INTERFACE_FUNCTION(DestroyStreamData,(XnStreamData** ppStreamData))

/**
* Registers to the event of new data from a stream.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	Handler			[in]	A pointer to the function that will handle the event.
* @param	pCookie			[in]	User cookie that will be passed as an argument to the event handler.
* @param	hCallback		[out]	A handle for unregister.
*/
XN_DEVICE_INTERFACE_FUNCTION(RegisterToNewStreamData,(const XnDeviceHandle DeviceHandle, XnDeviceOnNewStreamDataEventHandler Handler, void* pCookie, XnCallbackHandle& hCallback))

/**
* Unregisters from the event of new data from a stream.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	hCallback		[in]	The handle returned from RegisterToNewStreamData.
*/
XN_DEVICE_INTERFACE_FUNCTION(UnregisterFromNewStreamData,(const XnDeviceHandle DeviceHandle, XnCallbackHandle hCallback))

/**
* Checks if new data is available from stream.
* 
* @param	DeviceHandle		[in]	The requested device handle.
* @param	StreamName			[in]	The name of the stream to check.
* @param	pbNewDataAvailable	[out]	TRUE if new data is available, FALSE otherwise.
*/
XN_DEVICE_INTERFACE_FUNCTION(IsNewDataAvailable,(const XnDeviceHandle DeviceHandle, const XnChar* StreamName, XnBool* pbNewDataAvailable, XnUInt64* pnTimestamp))

/**
* Waits for new data to be available from requested stream, and then return it.
* 
* @param	DeviceHandle	[in]		The requested device handle.
* @param	pStreamOutput	[in/out]	A stream output object. The function will use the stream output object to determine which stream to read.
*/
XN_DEVICE_INTERFACE_FUNCTION(ReadStream,(const XnDeviceHandle DeviceHandle, XnStreamData* pStreamOutput))

/**
* Waits for new data from the primary stream to be available, and then reads all requested streams.
* 
* @param	DeviceHandle		[in]		The requested device handle.
* @param	pStreamOutputSet	[in/out]	A set of stream output objects.
*/
XN_DEVICE_INTERFACE_FUNCTION(Read,(const XnDeviceHandle DeviceHandle, XnStreamDataSet* pStreamOutputSet))

/**
* Writes a single stream data to the device.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	pStreamOutput	[in]	A stream output object.
*/
XN_DEVICE_INTERFACE_FUNCTION(WriteStream,(const XnDeviceHandle DeviceHandle, XnStreamData* pStreamOutput))

/**
* Writes multiple streams to the device.
* 
* @param	DeviceHandle		[in]	The requested device handle.
* @param	pStreamOutputSet	[in]	A set of stream output objects.
*/
XN_DEVICE_INTERFACE_FUNCTION(Write,(const XnDeviceHandle DeviceHandle, XnStreamDataSet* pStreamOutputSet))

/**
* Gets current position of the device.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	pnTimestamp		[out]	Current device timestamp.
*/
XN_DEVICE_INTERFACE_FUNCTION(Tell,(const XnDeviceHandle DeviceHandle, XnUInt64* pnTimestamp))

/**
* Seeks the device to the requested position.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	nTimestamp		[in]	Requested device timestamp.
*/
XN_DEVICE_INTERFACE_FUNCTION(Seek,(const XnDeviceHandle DeviceHandle, XnUInt64 nTimestamp))

/**
* Gets current frame position of the device.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	pnFrameID		[out]	Current device frame.
*/
XN_DEVICE_INTERFACE_FUNCTION(TellFrame,(const XnDeviceHandle DeviceHandle, XnUInt32* pnFrameID))

/**
* Seeks the device to the requested frame position.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	nFrameID		[in]	Requested device frame.
*/
XN_DEVICE_INTERFACE_FUNCTION(SeekFrame,(const XnDeviceHandle DeviceHandle, XnUInt32 nFrameID))

/**
* Checks if a specific property exists in a module.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	pbDoesExist		[out]	TRUE if the property exists, FALSE otherwise.
*/
XN_DEVICE_INTERFACE_FUNCTION(DoesPropertyExist,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, XnBool* pbDoesExist))

/**
* Returns the type of a specific property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	pnType			[out]	Type of this property.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetPropertyType,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, XnPropertyType* pnType))

/**
* Sets the value of an int property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	nValue			[in]	New requested value.
*/
XN_DEVICE_INTERFACE_FUNCTION(SetIntProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, XnUInt64 nValue))

/**
* Sets the value of a real property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	dValue			[in]	New requested value.
*/
XN_DEVICE_INTERFACE_FUNCTION(SetRealProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, XnDouble dValue))

/**
* Sets the value of a string property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	csValue			[in]	New requested value.
*/
XN_DEVICE_INTERFACE_FUNCTION(SetStringProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, const XnChar* csValue))

/**
* Sets the value of a general property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	Value			[in]	New requested value.
*/
XN_DEVICE_INTERFACE_FUNCTION(SetGeneralProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, OniGeneralBuffer Value))

/**
* Gets the value of an int property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	pnValue			[out]	Current value.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetIntProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, XnUInt64* pnValue))

/**
* Gets the value of a real property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	pdValue			[out]	Current value.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetRealProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, XnDouble* pdValue))

/**
* Gets the value of a string property.
* 
* @param	DeviceHandle	[in]		The requested device handle.
* @param	ModuleName		[in]		Name of the module.
* @param	PropertyName	[in]		Name of the property to change.
* @param	csValue			[in/out]	Current value. The passed buffer should be of size XN_DEVICE_MAX_STRING_LENGTH.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetStringProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, XnChar* csValue))

/**
* Gets the value of a general property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	ModuleName		[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to change.
* @param	pValue			[out]	A buffer to fill.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetGeneralProperty,(const XnDeviceHandle DeviceHandle, const XnChar* ModuleName, const XnChar* PropertyName, const OniGeneralBuffer* pValue))

/**
* Loads configuration from INI file.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	csINIFilePath	[in]	A path to the INI file.
* @param	csSectionName	[in]	The name of the section containing configuration.
*/
XN_DEVICE_INTERFACE_FUNCTION(LoadConfigFromFile,(const XnDeviceHandle DeviceHandle, const XnChar* csINIFilePath, const XnChar* csSectionName))

/**
* Batch-Configures device. All the properties in the set will be set as a single transaction.
*
* @param	DeviceHandle	[in]	The requested device handle.
* @param	pChangeSet		[in]	A set of properties to be changed.
*/
XN_DEVICE_INTERFACE_FUNCTION(BatchConfig,(const XnDeviceHandle DeviceHandle, const XnPropertySet* pChangeSet))

/**
* Gets all the properties of a device.
* 
* @param	DeviceHandle	[in]		The requested device handle.
* @param	pPropertySet	[in]		A property set to be filled with all the properties.
* @param	bNoStreams		[in]		When TRUE, only modules will be returned.
* @param	strModule		[in]		If provided, only this module's properties will be returned.
*/
XN_DEVICE_INTERFACE_FUNCTION(GetAllProperties,(const XnDeviceHandle DeviceHandle, XnPropertySet* pPropertySet, XnBool bNoStreams, const XnChar* strModule))

/**
* Registers an event handler to the Property Changed event of a specific property.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	Module			[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to register to.
* @param	Handler			[in]	A pointer to the function that will handle the event.
* @param	pCookie			[in]	User cookie that will be passed as an argument to the event handler.
* @param	hCallback		[out]	A handle for unregister.
*/
XN_DEVICE_INTERFACE_FUNCTION(RegisterToPropertyChange,(const XnDeviceHandle DeviceHandle, const XnChar* Module, const XnChar* PropertyName, XnDeviceOnPropertyChangedEventHandler Handler, void* pCookie, XnCallbackHandle& hCallback))

/**
* Unregisters an event handler from the Property Changed event.
* 
* @param	DeviceHandle	[in]	The requested device handle.
* @param	Module			[in]	Name of the module.
* @param	PropertyName	[in]	Name of the property to register to.
* @param	hCallback		[in]	The handle returned from RegisterToNewStreamData.
*/
XN_DEVICE_INTERFACE_FUNCTION(UnregisterFromPropertyChange,(const XnDeviceHandle DeviceHandle, const XnChar* Module, const XnChar* PropertyName, XnCallbackHandle hCallback))

