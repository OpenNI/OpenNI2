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
#pragma once

#include <XnPlatform.h>

#define XN_REAL XnDouble

/**
* This function takes a set of points, and performs linear fitting to find the best function producing those
* points using the Linear Least Squares method.
* The found function is of the form:
* z = Ax + By + C
* The output of this function is the coefficients A, B and C.
* 
* @param	x		[in]	The array of the x-values of all the points.
* @param	y		[in]	The array of the y-values of all the points.
* @param	z		[in]	The array of the z-values of all the points.
* @param	nVals	[in]	The number of points (hence, the size of the three arrays: x, y and z).
* @param	pA		[out]	The coefficient of x in the function.
* @param	pB		[out]	The coefficient of y in the function.
* @param	pC		[out]	The free coefficient in the function.
*/
void DoLinearFitting(XN_REAL x[], XN_REAL y[], XN_REAL z[], XnUInt32 nVals, XN_REAL* pA, XN_REAL* pB, XN_REAL* pC);
