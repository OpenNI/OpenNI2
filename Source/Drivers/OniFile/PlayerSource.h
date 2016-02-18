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
/// @file
/// Contains the declaration of Device class that implements a virtual OpenNI
/// device, capable of reading data from a *.ONI file.

#ifndef PLAYERSOURCE_H
#define PLAYERSOURCE_H

#include "PlayerProperties.h"
#include "OniCProperties.h"
#include "XnEvent.h"
#include "XnString.h"

enum
{
	ONI_STREAM_PROPERTY_BYTES_PER_PIXEL = ONI_STREAM_PROPERTY_PRIVATE_BASE, //int
};

namespace oni_file {

class Codec;

/// Represents a source in a played .ONI file.
class PlayerSource
{
public:
	// New data event.
	typedef struct 
	{
		XnUInt64 nTimeStamp;
		XnUInt32 nFrameId;
		void* pData;
		XnUInt32 nSize;
	} NewDataEventArgs;
	typedef xnl::Event<NewDataEventArgs> NewDataEvent;
	typedef void (ONI_CALLBACK_TYPE* NewDataCallback)(const NewDataEventArgs& newDataEventArgs, void* pCookie);

public:

	/// Constructor.
	PlayerSource(const XnChar* strNodeName, OniSensorType sensorType);

	/// Destructor.
	virtual ~PlayerSource();

	/// Return the source info associated with the source.
	OniSensorInfo* GetInfo();

	/// Return the node name for the source.
	const XnChar* GetNodeName();

	/// Get property.
	virtual OniStatus GetProperty(int propertyId, void* data, int* pDataSize);

	/// Set property.
	virtual OniStatus SetProperty(int propertyId, const void* data, int dataSize);

	// Process new data.
	void ProcessNewData(XnUInt64 nTimeStamp, XnUInt32 nFrameId, void* pData, XnUInt32 nSize);

	// Register for new data event.
	OniStatus RegisterNewDataEvent(NewDataCallback callback, void* pCookie, OniCallbackHandle& handle);

	// Unregister from new data event.
	void UnregisterNewDataEvent(OniCallbackHandle handle);

	void SetRequiredFrameSize(int requiredFrameSize) { m_requiredFrameSize = requiredFrameSize; }
	int GetRequiredFrameSize() const { return m_requiredFrameSize; }

	PlayerProperties::PropertiesHash::ConstIterator Begin() {return m_properties.Begin();}
	PlayerProperties::PropertiesHash::ConstIterator End() {return m_properties.End();}
protected:
	XN_DISABLE_COPY_AND_ASSIGN(PlayerSource);

	// Name of the node.
	const xnl::String m_nodeName;

	// Source information.
	OniSensorInfo m_sourceInfo;

	// Properties.
	PlayerProperties m_properties;

	// Data ready event.
	NewDataEvent m_newDataEvent;

	int m_requiredFrameSize;

	xnl::CriticalSection m_cs;
};

} // namespace oni_files_player

#endif // PLAYERSOURCE_H
