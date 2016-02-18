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
#ifndef MWCLOSESTPOINT_H
#define MWCLOSESTPOINT_H

#include <OpenNI.h>

#ifdef _CLOSEST_POINT
#define MW_CP_API ONI_API_EXPORT
#else
#define MW_CP_API ONI_API_IMPORT
#endif


namespace openni
{
	class Device;
}

namespace closest_point
{

struct IntPoint3D
{
	int X;
	int Y;
	int Z;
};

struct ClosestPointInternal;

class MW_CP_API ClosestPoint
{
public:
	class Listener
	{
	public:
		virtual void readyForNextData(ClosestPoint*) = 0;
	};

	ClosestPoint(const char* uri = NULL);
	ClosestPoint(openni::Device* pDevice);
	~ClosestPoint();

	bool isValid() const;

	openni::Status setListener(Listener& listener);
	void resetListener();

	openni::Status getNextData(IntPoint3D& closestPoint, openni::VideoFrameRef& rawFrame);
private:
	void initialize();

	ClosestPointInternal* m_pInternal;
};

}






#endif // MWCLOSESTPOINT_H
