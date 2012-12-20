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
#ifndef _XN_LOCKABLE_H_
#define _XN_LOCKABLE_H_ 1

#include "XnMacros.h"

namespace xnl
{

/**
 *
 */
template<typename T>
class Lockable : public T
{
public:
    /**
     *
     */
    Lockable();

    /**
     *
     */
    ~Lockable();

    /** Forwarding constructor */
    /*@{*/
    template<typename P00>
    Lockable(P00 param00);
    template<typename P00, typename P01>
    Lockable(P00 param00, P01 param01);
    template<typename P00, typename P01, typename P02>
    Lockable(P00 param00, P01 param01, P02 param02);
    template<typename P00, typename P01, typename P02, typename P03>
    Lockable(P00 param00, P01 param01, P02 param02, P03 param03);
    template<typename P00, typename P01, typename P02, typename P03, typename P04>
    Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04);
    template<typename P00, typename P01, typename P02, typename P03, typename P04,
             typename P05>
    Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
             P05 param05);
    template<typename P00, typename P01, typename P02, typename P03, typename P04,
             typename P05, typename P06>
    Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
             P05 param05, P06 param06);
    template<typename P00, typename P01, typename P02, typename P03, typename P04,
             typename P05, typename P06, typename P07>
    Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
             P05 param05, P06 param06, P07 param07);
    template<typename P00, typename P01, typename P02, typename P03, typename P04,
             typename P05, typename P06, typename P07, typename P08>
    Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
             P05 param05, P06 param06, P07 param07, P08 param08);
    template<typename P00, typename P01, typename P02, typename P03, typename P04,
             typename P05, typename P06, typename P07, typename P08, typename P09>
    Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
             P05 param05, P06 param06, P07 param07, P08 param08, P09 param09);
    /*@}*/

    /**
     *
     */
    XnStatus Lock();

    /**
     *
     */
    XnStatus Unlock();

private:
    XN_DISABLE_COPY_AND_ASSIGN(Lockable)

    void Initialize();

    XN_CRITICAL_SECTION_HANDLE m_criticalSection;
};

template<typename T>
Lockable<T>::Lockable() 
        : m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
Lockable<T>::~Lockable()
{
	xnOSCloseCriticalSection(&m_criticalSection);
}

template<typename T>
template<typename P00>
Lockable<T>::Lockable(P00 param00)
        : T(param00),
          m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01>
Lockable<T>::Lockable(P00 param00, P01 param01) 
    : T(param00, param01),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02) 
    : T(param00, param01, param02),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02, typename P03>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02, P03 param03) 
    : T(param00, param02, param03),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02, typename P03, typename P04>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04) 
    : T(param00, param01, param02, param03, param04),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02, typename P03, typename P04,
         typename P05>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
                      P05 param05) 
    : T(param00, param01, param02, param03, param04, param05),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02, typename P03, typename P04,
         typename P05, typename P06>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
                      P05 param05, P06 param06) 
    : T(param00, param01, param02, param03, param04, param05, param06),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02, typename P03, typename P04,
         typename P05, typename P06, typename P07>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
                      P05 param05, P06 param06, P07 param07) 
    : T(param00, param01, param02, param03, param04, param05, param06, param07),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02, typename P03, typename P04,
         typename P05, typename P06, typename P07, typename P08>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
                      P05 param05, P06 param06, P07 param07, P08 param08) 
    : T(param00, param01, param02, param03, param04, param05, param06, param07,
        param08),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
template<typename P00, typename P01, typename P02, typename P03, typename P04,
         typename P05, typename P06, typename P07, typename P08, typename P09>
Lockable<T>::Lockable(P00 param00, P01 param01, P02 param02, P03 param03, P04 param04,
                      P05 param05, P06 param06, P07 param07, P08 param08, P09 param09) 
    : T(param00, param01, param02, param03, param04, param05, param06, param07,
        param08, param09),
      m_criticalSection(NULL)
{
    Initialize();
}

template<typename T>
XnStatus Lockable<T>::Lock()
{
    if (m_criticalSection != NULL)
    {
        xnOSEnterCriticalSection(&m_criticalSection);
    }
    return XN_STATUS_OK;
}

template<typename T>
XnStatus Lockable<T>::Unlock()
{
    if (m_criticalSection != NULL)
    {
        xnOSLeaveCriticalSection(&m_criticalSection);
    }
    return XN_STATUS_OK;
}

template<typename T>
void Lockable<T>::Initialize()
{
    XnStatus rc = xnOSCreateCriticalSection(&m_criticalSection);
    if (rc != XN_STATUS_OK)
    {
        m_criticalSection = NULL;
    }
}

} // namespace xnl

#endif // _XN_LOCKABLE_H_
