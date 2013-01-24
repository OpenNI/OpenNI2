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
#ifndef _XN_PAIR_H_
#define _XN_PAIR_H_

namespace xnl
{

template <class T1, class T2>
struct Pair
{
	Pair() : first(T1()), second(T2()) {}
	Pair(T1 t1, T2 t2) : first(t1), second(t2) {}
	Pair(const Pair& other) : first(other.first), second(other.second) {}
	Pair& operator=(const Pair& other) {first = other.first; second = other.second; return *this;}
	XnBool operator==(const Pair& other) {return first == other.first && second == other.second;}
	XnBool operator!=(const Pair& other) {return !operator==(other);}

	T1 first;
	T2 second;
};

} // xnl

#endif // _XN_PAIR_H_