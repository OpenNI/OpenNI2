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
#ifndef ONIDEBUG_H
#define ONIDEBUG_H

namespace oni
{
namespace implementation
{
enum DebugObjectType
{
	DEBUG_OBJECT_POINT,
	DEBUG_OBJECT_LINE,
	DEBUG_OBJECT_CIRCLE
};

class DebugObject
{
public:
	DebugObject(DebugObjectType type) : m_type(type)
	{
		m_color.r = m_color.g = m_color.b = 255;
	}

	void SetColor(OniRGBPixel color)
	{
		m_color = color;
	}
	void SetColor(OniUInt8 r, OniUInt8 g, OniUInt8 b)
	{
		m_color.r = r;
		m_color.g = g;
		m_color.b = b;
	}

	void GetColor(OniRGBPixel& color) const
	{
		color = m_color;
	}

	DebugObjectType GetType() const
	{
		return m_type;
	}
protected:
	OniRGBPixel m_color;
	DebugObjectType m_type;
};

class DebugPoint : public DebugObject
{
public:
	DebugPoint(pscommon::Point3D point, DebugObjectType type = DEBUG_OBJECT_POINT) :
	  DebugObject(type),
		m_point(point)
	{}
	DebugPoint(OniUInt16 x, OniUInt16 y, DebugObjectType type = DEBUG_OBJECT_POINT) :
	  DebugObject(type)
	  {
		  m_point.x = x;
		  m_point.y = y;
	  }

	pscommon::Point3D GetPoint() const {return m_point;}
protected:
	pscommon::Point3D m_point;
};

class DebugLine : public DebugPoint
{
public:
	DebugLine(pscommon::Point3D point1, pscommon::Point3D point2) :
	  DebugPoint(point1, DEBUG_OBJECT_LINE)
	  {
		  m_point2 = point2;
	  }

	  pscommon::Point3D GetPoint2() const {return m_point2;}
protected:
	pscommon::Point3D m_point2;
};

class DebugCircle : public DebugPoint
{
public:
	DebugCircle(pscommon::Point3D center, OniFloat radius) :
	  DebugPoint(center, DEBUG_OBJECT_CIRCLE),
		  m_radius(radius)
	  {}
	  OniFloat GetRadius() const {return m_radius;}
protected:
	OniFloat m_radius;
};

class Debug
{
public:
	class TimedDebugObject
	{
	public:
		TimedDebugObject(DebugObject* pObject, OniUInt32 ttl) :
		  m_pObject(pObject), m_ttl(ttl)
		  {}
		void Tick()
		{
			if (m_ttl != 0) --m_ttl;
		}
		OniUInt32 GetTTL() const {return m_ttl;}
		DebugObject* GetObject() const {return m_pObject;}
	protected:
		DebugObject* m_pObject;
		OniUInt32 m_ttl;
	};

	typedef pscommon::List<pscommon::SmartPointer<TimedDebugObject> > DebugObjectList;

	Debug()
	{}

	void Tick()
	{
		for (pscommon::Hash<OniUInt16, DebugObjectList>::Iterator iter = m_objectsByDepth.Begin(); iter != m_objectsByDepth.End(); ++iter)
		{
			for (DebugObjectList::Iterator objectIter = (*iter).Value().Begin(); objectIter != (*iter).Value().End(); ++objectIter)
			{
				(*objectIter)->Tick();
				if ((*objectIter)->GetTTL() == 0)
				{
					DebugObjectList::Iterator toErase = objectIter;
					--objectIter;
					(*iter).Value().Remove(toErase);
				}
			}
		}
	}
	void AddDebugObject(DebugObject* pObject, OniUInt16 layer, OniUInt16 ttl)
	{
		m_objectsByDepth[layer].AddLast(new TimedDebugObject(pObject, ttl));
	}

private:
	pscommon::Hash<OniUInt16, DebugObjectList> m_objectsByDepth;
};
}
}






#endif // ONIDEBUG_H
