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
#include "XnVector3D.h"

namespace xnl
{

Vector3D Vector3D::Zero() {return Vector3D(0,0,0);}
Vector3D Vector3D::All(XnFloat f) {return Vector3D(f,f,f);}

Vector3D Vector3D::Up() {return Vector3D(0,1,0);}
Vector3D Vector3D::Down() {return Vector3D(0,-1,0);}
Vector3D Vector3D::Right() {return Vector3D(1,0,0);}
Vector3D Vector3D::Left() {return Vector3D(-1,0,0);}


XnFloat Vector3D::GetTolerance() const {return m_tolerance;}
void Vector3D::SetTolerance(XnFloat tolerance) {m_tolerance = tolerance;}

}  // XnLib
