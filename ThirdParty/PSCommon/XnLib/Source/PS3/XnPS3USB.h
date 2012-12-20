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
#ifndef _XN_USBPS3_H_
#define _XN_USBPS3_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnOS.h>
#include <cell/usbd.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
//#define XN_USB_DEBUG 1

#define XN_USB_DEFAULT_EP_TIMEOUT 1000
#define XN_USB_READ_THREAD_KILL_TIMEOUT 10000
#define XN_USB_OPEN_TIMEOUT 5000
#define XN_USB_CONTROL_TIMEOUT 5000
#define XN_USB_READ_THREAD_PRIORITY 10
#define XN_USB_LIBUSBD_EVENT_PRIORITY 1
#define XN_USB_LIBUSBD_EVENT_USBD 2
#define XN_USB_LIBUSBD_EVENT_CALLBACK 3

#ifdef XN_USB_DEBUG
	#define XN_USB_DEBUG_PRINTF(DUMMYFIRST, ...) printf(DUMMYFIRST, ##__VA_ARGS__)
#else
	#define XN_USB_DEBUG_PRINTF(DUMMYFIRST, ...)
#endif

//---------------------------------------------------------------------------
// USBD Prototypes
//---------------------------------------------------------------------------
static int32_t XnUSBProbe(int32_t dev_id);
static int32_t XnUSBAttach(int32_t dev_id);
static int32_t XnUSBDetach(int32_t dev_id);

static CellUsbdLddOps XnUSBDOps = 
{
	"XnUSB",
	XnUSBProbe,
	XnUSBAttach,
	XnUSBDetach
};

//---------------------------------------------------------------------------
// Structures & Enums
//---------------------------------------------------------------------------
typedef struct XnUSBTransferContext
{
	XN_BOOL	bIsBusy;
	XN_EVENT_HANDLE BusyEvent;

	XN_INT32 nTransferBytes;

	XnStatus nRetVal;
	XN_INT32 nUSBResult;
} XnUSBTransferContext;

typedef struct XnUSBDeviceHandle
{
	XN_BOOL bValid;

	XN_INT32 nDevId;

	XN_INT8 nInterface;
	XN_INT8 nAltInterface;
	XnUSBDeviceSpeed nDevSpeed;

	XN_INT32 nCtrlPipe;

	XnUSBTransferContext ovlpIO;
} XnUSBDevHandle;

typedef struct XnUSBBuffersInfo
{
	XN_UCHAR* pBuffer;
	XN_UINT32 nBufferSize;
	XnUSBTransferContext ovlpIO;
	XN_BOOL bFailed;
} XnUSBBuffersInfo;

typedef struct XnUSBReadThreadData
{
	XN_BOOL bInUse;

	XN_USB_EP_HANDLE pEPHandle;
	XN_UINT32 nBufferSize;
	XN_UINT32 nNumBuffers;
	XN_UINT32 nTimeOut;
	XnUSBReadCallbackFunctionPtr pCallbackFunction;
	void* pCallbackData;

	XN_THREAD_HANDLE  hReadThread;
	XN_BOOL			  bKillReadThread;
	XnUSBBuffersInfo* pBuffersInfo;
} XnUSBReadThreadData;

typedef struct XnUSBEndPointHandle
{
	XN_BOOL bValid;

	XN_INT32 hEP;
	XnUSBDeviceHandle* pDevHandle;

	XnUSBTransferContext ovlpIO;
	XnUSBReadThreadData ThreadData;
	
	XnUSBEndPointType  nEPType;
	XnUSBDirectionType nEPDir;
} XnUSBEPHandle;

#endif //_XN_USBPS3_H_