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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnCore.h>
#include <XnOS.h>
#include <Core/XnCoreGlobals.h>
#include <XnLog.h>
#include <XnProfiling.h>

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
// Note: See the XnIOGlobals.h file for global variables description
XnBool g_bXnCoreWasInit = FALSE;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnStatus XnGeneralBufferCopy(OniGeneralBuffer* pDest, const OniGeneralBuffer* pSrc)
{
        XN_VALIDATE_INPUT_PTR(pDest);
        XN_VALIDATE_INPUT_PTR(pSrc);

        if (pSrc->dataSize > pDest->dataSize)
                return XN_STATUS_OUTPUT_BUFFER_OVERFLOW;

        xnOSMemCopy(pDest->data, pSrc->data, pSrc->dataSize);
        pDest->dataSize = pSrc->dataSize;
        return XN_STATUS_OK;
}

XnStatus XnGeneralBufferAlloc(OniGeneralBuffer* pDest, XnUInt32 nSize)
{
        XN_VALIDATE_INPUT_PTR(pDest);

        void* pData;
        pData = xnOSMalloc(nSize);
        XN_VALIDATE_ALLOC_PTR(pData);

        pDest->data = pData;
        pDest->dataSize = nSize;
        return XN_STATUS_OK;
}

XnStatus XnGeneralBufferRealloc(OniGeneralBuffer* pDest, XnUInt32 nSize)
{
        XN_VALIDATE_INPUT_PTR(pDest);

        void* pData;
        pData = xnOSRealloc(pDest, nSize);
        XN_VALIDATE_ALLOC_PTR(pData);

        pDest->data = pData;
        pDest->dataSize = nSize;
        return XN_STATUS_OK;
}

void XnGeneralBufferFree(OniGeneralBuffer* pDest)
{
        XN_FREE_AND_NULL(pDest->data);
        pDest->dataSize = 0;
}
