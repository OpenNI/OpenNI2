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
#ifndef PLAYERPROPERTIES_H
#define PLAYERPROPERTIES_H

#include <XnHash.h>
#include <XnOSCpp.h>
#include "Driver/OniDriverTypes.h"

namespace oni_file {

class PlayerProperties
{
public:

	~PlayerProperties()
	{
		while (m_properties.Begin() != m_properties.End())
		{
			PropertiesHash::Iterator iter = m_properties.Begin();
			OniGeneralBuffer* pBuffer = (*iter).second;
			DestroyBuffer(pBuffer);
			m_properties.Remove(iter);
		}
	}

	OniBool Exists(int propertyId)
	{
		xnl::AutoCSLocker lock(m_cs);
		if (m_properties.Find(propertyId) != m_properties.End())
		{
			return TRUE;
		}
		return FALSE;
	}

	OniStatus GetProperty(int propertyId, void* data, int* pDataSize) const
	{
		xnl::AutoCSLocker lock(m_cs);

		// Get the property and make sure it exists.
		PropertiesHash::ConstIterator iter = m_properties.Find(propertyId);
		if (iter == m_properties.End())
		{
			return ONI_STATUS_ERROR;
		}

		// Copy the property.
		const OniGeneralBuffer* pBuffer = (*iter).second;
		*pDataSize = (*pDataSize < pBuffer->dataSize) ? *pDataSize : pBuffer->dataSize;
		memcpy(data, pBuffer->data, *pDataSize);

		return ONI_STATUS_OK;
	}

	OniStatus SetProperty(int propertyId, const void* data, int dataSize)
	{
		xnl::AutoCSLocker lock(m_cs);

		// Check if property exists.
		PropertiesHash::Iterator iter = m_properties.Find(propertyId);
		if (iter != m_properties.End())
		{
			DestroyBuffer((*iter).second);
			m_properties.Remove(iter);
		}

		// Copy the property.
		OniGeneralBuffer* pBuffer = CreateBuffer(data, dataSize);
		m_properties[propertyId] = pBuffer;

		return ONI_STATUS_OK;
	}

	typedef xnl::Hash<int, OniGeneralBuffer*> PropertiesHash;
	PropertiesHash::ConstIterator Begin() const {return m_properties.Begin();}
	PropertiesHash::ConstIterator End() const {return m_properties.End();}
private:

	OniGeneralBuffer* CreateBuffer(const void* data, int dataSize)
	{
		OniGeneralBuffer* pBuffer;

		pBuffer = XN_NEW(OniGeneralBuffer);
		if (pBuffer == NULL)
		{
			return NULL;
		}
		pBuffer->data = XN_NEW_ARR(unsigned char, dataSize);
		if (pBuffer->data == NULL)
		{
			XN_DELETE(pBuffer);
			return NULL;
		}
		memcpy(pBuffer->data, data, dataSize);
		pBuffer->dataSize = dataSize;

		return pBuffer;
	}

	void DestroyBuffer(OniGeneralBuffer* pBuffer)
	{
		XN_DELETE_ARR((unsigned char*)pBuffer->data);
		XN_DELETE(pBuffer);
	}

	xnl::CriticalSection m_cs;
	PropertiesHash m_properties;
};

} // namespace oni_files_player

#endif // PLAYERPROPERTIES_H
