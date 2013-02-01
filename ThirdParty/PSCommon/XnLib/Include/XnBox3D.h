/*****************************************************************************
*                                                                            *
*  PrimeSense PSCommon Library                                               *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PSCommon.                                            *
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
#ifndef _XN_BOX_3D_H_
#define _XN_BOX_3D_H_

#include "XnVector3D.h"
#include "XnMath.h"
namespace xnl
{

class Box3D
{
public:
	Box3D() : m_topRightFar(Vector3D::All(FLT_MIN)), m_bottomLeftNear(Vector3D::All(FLT_MAX)) {}
	Box3D(const Point3D& point1, const Point3D& point2)
	{
		EnlargeToInclude(point1);
		EnlargeToInclude(point2);
	}

	void Crop(Point3D& point)
	{
		point.x = Math::Crop(point.x, m_topRightFar.x, m_bottomLeftNear.x);
		point.y = Math::Crop(point.y, m_topRightFar.y, m_bottomLeftNear.y);
		point.z = Math::Crop(point.z, m_topRightFar.z, m_bottomLeftNear.z);
	}
	void EnlargeToInclude(const Point3D& point)
	{
		m_topRightFar.x = Math::Max(point.x, m_topRightFar.x);
		m_topRightFar.y = Math::Max(point.y, m_topRightFar.y);
		m_topRightFar.z = Math::Max(point.z, m_topRightFar.z);

		m_bottomLeftNear.x = Math::Min(point.x, m_bottomLeftNear.x);
		m_bottomLeftNear.y = Math::Min(point.y, m_bottomLeftNear.y);
		m_bottomLeftNear.z = Math::Min(point.z, m_bottomLeftNear.z);

// 		if (point.x > m_topRightFar.x) m_topRightFar.x = point.x;
// 		if (point.y > m_topRightFar.y) m_topRightFar.y = point.y;
// 		if (point.z > m_topRightFar.z) m_topRightFar.z = point.z;
// 		if (point.x < m_bottomLeftNear.x) m_bottomLeftNear.x = point.x;
// 		if (point.y < m_bottomLeftNear.y) m_bottomLeftNear.y = point.y;
// 		if (point.z < m_bottomLeftNear.z) m_bottomLeftNear.z = point.z;
	}
	bool IsIn(const Point3D& point)
	{
		return Math::IsBetween(point.x, m_topRightFar.x, m_bottomLeftNear.x) &&
			Math::IsBetween(point.y, m_topRightFar.y, m_bottomLeftNear.y) &&
			Math::IsBetween(point.z, m_topRightFar.z, m_bottomLeftNear.z);
	}

	Point3D m_topRightFar;
	Point3D m_bottomLeftNear;
};


} // xnl


#endif // _XN_BOX_3D_H_
