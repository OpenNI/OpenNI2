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
#ifndef _XN_LOCK_GUARD_H_
#define _XN_LOCK_GUARD_H_ 1

#include "XnMacros.h"

namespace xnl
{

/**
 *
 */
template<typename T>
class LockGuard
{
public:
    /**
     *
     */
    LockGuard(T& lockable);

    /**
     *
     */
    ~LockGuard();

private:
    XN_DISABLE_COPY_AND_ASSIGN(LockGuard)

    T& m_lockable;
};

template<typename T>
LockGuard<T>::LockGuard(T& lockable) : m_lockable(lockable)
{
    m_lockable.Lock();
}

template<typename T>
LockGuard<T>::~LockGuard()
{
    m_lockable.Unlock();
}

} // namespace xnl

#endif // _XN_LOCK_GUARD_H_
