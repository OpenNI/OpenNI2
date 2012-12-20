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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnUSB.h>
#include "../XnUSBInternal.h"
#include "XnUSBPS3.h"

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
XnUSBDevHandle* g_XnUSBDevHandle = NULL;

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------
#define XN_IS_USB_ENDPOINT_DIRECTION_OUT(addr) (!((addr) & USB_ENDPOINT_DIRECTION_BITS))
#define XN_IS_USB_ENDPOINT_DIRECTION_IN(addr) ((addr) & USB_ENDPOINT_DIRECTION_BITS)
#define XN_USB_GET_EP_TYPE_FROM_ATTR(attr) ((attr) & USB_ENDPOINT_TRANSFER_TYPE_BITS)

//---------------------------------------------------------------------------
// USBD Callbacks
//---------------------------------------------------------------------------
static int32_t XnUSBProbe(int32_t dev_id)
{
	XN_USB_DEBUG_PRINTF("XnUSBProbe: DeviceID=%d\n", dev_id);

	// Local function variables
	UsbDeviceDescriptor* USBDevDesc = NULL;

	// Get the device descriptor
	USBDevDesc = (UsbDeviceDescriptor*)cellUsbdScanStaticDescriptor(dev_id, NULL, USB_DESCRIPTOR_TYPE_DEVICE);
	if (USBDevDesc == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBProbe: cellUsbdScanStaticDescriptor failed to read the device descriptor!\n");
		return CELL_USBD_PROBE_FAILED;
	}

	// Success! This is the device we want.
	XN_USB_DEBUG_PRINTF("XnUSBProbe: Success! VID=0x%02X, PID=0x%02X\n", XnOSEndianSwapUINT16(USBDevDesc->idVendor), XnOSEndianSwapUINT16(USBDevDesc->idProduct));

	// All is good
	return (CELL_USBD_PROBE_SUCCEEDED);
}

static int32_t XnUSBAttach(int32_t dev_id)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;

	XN_USB_DEBUG_PRINTF("XnUSBAttach: DeviceID=%d\n", dev_id);

	// Make sure we have a valid global device handle
	if (g_XnUSBDevHandle == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBAttach: Global device handle is NULL!\n");
		return (CELL_USBD_ATTACH_FAILED);
	}

	// Update the selected device id in the global handle
	g_XnUSBDevHandle->nDevId = dev_id;

	// Signal the callback complete
	nRetVal = XnOSSetEvent(g_XnUSBDevHandle->ovlpIO.BusyEvent);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBAttach: Failed to set the busy event! (%s)", XnGetStatusString(nRetVal));
		return (CELL_USBD_ATTACH_FAILED);
	}

	// Success!
	XN_USB_DEBUG_PRINTF("XnUSBAttach: Success! Attached to DeviceID=%d\n", dev_id);

	// All is good...
	return (CELL_USBD_ATTACH_SUCCEEDED);
}

static int32_t XnUSBDetach(int32_t dev_id)
{
	XN_USB_DEBUG_PRINTF("XnUSBDetach: DeviceID=%d\n", dev_id);

	// Make sure we have a valid global device handle
	if (g_XnUSBDevHandle == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBDetach: Global device handle is NULL!\n");
		return (CELL_USBD_DETACH_FAILED);
	}

	// Success!
	XN_USB_DEBUG_PRINTF("XnUSBDetach: Detached from DeviceID=%d\n", dev_id);

	// All is good...
	return (CELL_USBD_DETACH_SUCCEEDED);
}

//---------------------------------------------------------------------------
// XnUSB Callbacks
//---------------------------------------------------------------------------
static void XnUSBTransferDoneCB(int32_t result, int32_t count, void *arg)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;
	XnUSBTransferContext* pUSBTransferContext = (XnUSBTransferContext*)arg;

	// Make sure that we have a valid transfer context
	if (pUSBTransferContext == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBTransferDoneCB: Invalid transfer context!\n");
		return;
	}
	
	// Update the result and number of returned bytes
	pUSBTransferContext->nUSBResult = result;
	pUSBTransferContext->nTransferBytes = count;

	// Check if the transaction was successful...
	if (result != HC_CC_NOERR)
	{
		// Nope, it wasn't.
		switch (result)
		{
			case EHCI_CC_MISSMF:
				pUSBTransferContext->nRetVal = XN_STATUS_USB_TRANSFER_MICRO_FRAME_ERROR;
				break;
			case EHCI_CC_XACT:
				pUSBTransferContext->nRetVal = XN_STATUS_USB_TRANSFER_TIMEOUT;
				break;
			case EHCI_CC_BABBLE:
			case EHCI_CC_DATABUF:
				pUSBTransferContext->nRetVal = XN_STATUS_USB_TOO_MUCH_DATA;
				break;
			case EHCI_CC_HALTED:
				pUSBTransferContext->nRetVal = XN_STATUS_USB_TRANSFER_STALL;
				break;
			default:
				pUSBTransferContext->nRetVal = XN_STATUS_USB_TRANSFER_UNKNOWN_ERROR;
		}

		XN_USB_DEBUG_PRINTF("XnUSBTransferDoneCB: USB transfer failure! (0x%x)\n", result);
	}
	else
	{
		// All's good...
		pUSBTransferContext->nRetVal = XN_STATUS_OK;

		XN_USB_DEBUG_PRINTF("XnUSBTransferDoneCB: %d bytes were successfully transfered\n", count);
	}

	// Signal the callback complete
	nRetVal = XnOSSetEvent(pUSBTransferContext->BusyEvent);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBTransferDoneCB: Failed to set the busy event! (%s)\n", XnGetStatusString(nRetVal));
	}
}

//---------------------------------------------------------------------------
// Helper Functions
//---------------------------------------------------------------------------
XnStatus XnUSBSetBusy(XnUSBTransferContext* pUSBTransferContext)
{
	// Make sure that the USB is not already busy...
	if (pUSBTransferContext->bIsBusy == TRUE)
	{
		return (XN_STATUS_USB_IS_BUSY);
	}

	// Set the USB state to busy
	pUSBTransferContext->bIsBusy = TRUE;

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnUSBClearBusy(XnUSBTransferContext* pUSBTransferContext)
{
	// Set the USB state to not busy
	pUSBTransferContext->bIsBusy = FALSE;

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnUSBWaitWithTimeout(XnUSBTransferContext* pUSBTransferContext, XN_UINT32 nTimeoutMillisecond)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;

	// Make sure that the USB is busy
	if (pUSBTransferContext->bIsBusy != TRUE)
	{
		return (XN_STATUS_USB_NOT_BUSY);
	}

	// Wait for the busy event to be signaled
	nRetVal = XnOSWaitEvent(pUSBTransferContext->BusyEvent, nTimeoutMillisecond);
	if (nRetVal != XN_STATUS_OK)
	{
		if (nRetVal == XN_STATUS_OS_EVENT_TIMEOUT)
		{
			return (XN_STATUS_USB_TRANSFER_TIMEOUT);
		}

		return(nRetVal);
	}

	// All is good...
	return (XN_STATUS_OK);
}

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnStatus XnUSBPlatformSpecificInit()
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;

	// Load USBD
	nCellRetVal = cellSysmoduleLoadModule(CELL_SYSMODULE_USBD);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBPlatformSpecificInit: cellSysmoduleLoadModule failed to load the USBD! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_LOAD_FAILED);
	}

	// Init USBD
	nCellRetVal = cellUsbdInit();
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBPlatformSpecificInit: cellUsbdInit failed to init USBD! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_INIT_FAILED);
	}

	nCellRetVal = cellUsbdSetThreadPriority2(XN_USB_LIBUSBD_EVENT_PRIORITY, XN_USB_LIBUSBD_EVENT_USBD, XN_USB_LIBUSBD_EVENT_CALLBACK);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBPlatformSpecificInit: cellUsbdSetThreadPriority2 failed to set USBD thread priorities! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_INIT_FAILED);
	}

	// All is good...
	return (XN_STATUS_OK);
}

XnStatus XnUSBPlatformSpecificShutdown()
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;

	// Shutdown USBD
	nCellRetVal = cellUsbdEnd();
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBPlatformSpecificShutdown: cellUsbdEnd failed to end the USBD! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_SHUTDOWN_FAILED);
	}

	// Unload USBD
	nCellRetVal = cellSysmoduleUnloadModule(CELL_SYSMODULE_USBD);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBPlatformSpecificShutdown: cellSysmoduleUnloadModule failed to unload the USBD! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_FREE_FAILED);
	}

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBIsDevicePresent(XN_UINT16 nVendorID, XN_UINT16 nProductID, void* pExtraParam, XN_BOOL* pbDevicePresent)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnUSBOpenDevice(XN_UINT16 nVendorID, XN_UINT16 nProductID, void* pExtraParam, XN_USB_DEV_HANDLE* pDevHandlePtr)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;
	XN_INT32 nCellRetVal = 0;
	XN_USB_DEV_HANDLE pDevHandle = NULL;
	UsbConfigurationDescriptor* pUsbConfigurationDescriptor = NULL;
	UsbInterfaceDescriptor* pUsbInterfaceDescriptor = NULL;

	// Make sure XnUSB was init
	XN_VALIDATE_USB_INIT();

	// Check input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pDevHandlePtr);

	// Make sure no device is already open
	if (g_XnUSBDevHandle != NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: A device is already opened!\n");
		return (XN_STATUS_USB_ALREADY_OPEN);
	}

	// Allocate the device handle
	XN_VALIDATE_ALIGNED_CALLOC(*pDevHandlePtr, XnUSBDeviceHandle, 1, XN_DEFAULT_MEM_ALIGN);

	// Init the device handle
	pDevHandle = *pDevHandlePtr;
	pDevHandle->bValid = FALSE;
	pDevHandle->nDevId = -1;
	pDevHandle->nInterface = 0;
	pDevHandle->nAltInterface = 0;
	pDevHandle->nCtrlPipe = -1;
	pDevHandle->nDevSpeed = XN_USB_DEVICE_HIGH_SPEED;

	// Set the global device handle to point to the new device handle
	g_XnUSBDevHandle = pDevHandle;

	// Create the busy event
	nRetVal = XnOSCreateEvent(&pDevHandle->ovlpIO.BusyEvent);
	XN_IS_STATUS_OK(nRetVal);

	// Register the device VID/PID into USBD
	XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Registering callbacks in USBD...\n");

	nRetVal = XnUSBSetBusy(&pDevHandle->ovlpIO);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: failed to set the busy state!\n");
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (nRetVal);
	}

	nCellRetVal = cellUsbdRegisterExtraLdd(&XnUSBDOps, nVendorID, nProductID);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: cellUsbdRegisterExtraLdd failed to register USBD LDDs! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (XN_STATUS_USB_REGISTER_FAILED);
	}

	// Wait for the open event to happen...
	XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Waiting for the USDB attach event...\n");

	nRetVal = XnUSBWaitWithTimeout(&pDevHandle->ovlpIO, XN_USB_OPEN_TIMEOUT);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Device not found!\n");
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (XN_STATUS_USB_DEVICE_NOT_FOUND);
	}
	else
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Device found! DeviceID=%d\n", pDevHandle->nDevId);
	}

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pDevHandle->ovlpIO);
	if (nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Failed to clear the busy state!\n");
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (nRetVal);
	}

	// Get the device speed
	XN_UINT8 nDeviceSpeed = 0;
	nCellRetVal = cellUsbdGetDeviceSpeed(pDevHandle->nDevId, &nDeviceSpeed);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Failed to get the device speed! (%Xh)\n", nCellRetVal);
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (XN_STATUS_USB_GET_SPEED_FAILED);
	}

	if (nDeviceSpeed == CELL_USBD_DEVICE_SPEED_LS)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Device speed is Low-Speed (1.5Mbps).\n");
		pDevHandle->nDevSpeed = XN_USB_DEVICE_LOW_SPEED;
	}
	else if (nDeviceSpeed == CELL_USBD_DEVICE_SPEED_FS)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Device speed is Full-Speed (12Mbps).\n");
		pDevHandle->nDevSpeed = XN_USB_DEVICE_FULL_SPEED;
	}
	else if (nDeviceSpeed == CELL_USBD_DEVICE_SPEED_HS)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Device speed is High-Speed (480Mbps).\n");
		pDevHandle->nDevSpeed = XN_USB_DEVICE_HIGH_SPEED;
	}
	else
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Unknown device speed!\n");
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (XN_STATUS_USB_UNKNOWN_DEVICE_SPEED);
	}	

	// Open the control endpoint
	pDevHandle->nCtrlPipe = cellUsbdOpenPipe(pDevHandle->nDevId, NULL);
	if (pDevHandle->nCtrlPipe < 0)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: cellUsbdOpenPipe failed to open the control pipe! (%Xh)\n", pDevHandle->nCtrlPipe);
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (XN_STATUS_USB_ENDPOINT_NOT_FOUND);	
	}
	else
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: Control endpoint is open! EP=%d\n", pDevHandle->nCtrlPipe);
	}

	// Set the default device config 
	pUsbConfigurationDescriptor = (UsbConfigurationDescriptor*)cellUsbdScanStaticDescriptor(pDevHandle->nDevId, NULL, USB_DESCRIPTOR_TYPE_CONFIGURATION);
	if (pUsbConfigurationDescriptor == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: cellUsbdScanStaticDescriptor failed to get the device configuration descriptor!\n");
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (XN_STATUS_USB_CONFIG_QUERY_FAILED);
	}

	nRetVal = XnUSBSetConfig(pDevHandle, pUsbConfigurationDescriptor->bConfigurationValue);
	if (nRetVal != XN_STATUS_OK)
	{
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (nRetVal);	
	}

	// Set the default device interface
	pUsbInterfaceDescriptor = (UsbInterfaceDescriptor*)pUsbConfigurationDescriptor;
	if ((pUsbInterfaceDescriptor = (UsbInterfaceDescriptor*)cellUsbdScanStaticDescriptor(pDevHandle->nDevId, pUsbInterfaceDescriptor, USB_DESCRIPTOR_TYPE_INTERFACE)) == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenDevice: cellUsbdScanStaticDescriptor failed to get the device interface descriptor!\n");
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (XN_STATUS_USB_CONFIG_QUERY_FAILED);
	}

	nRetVal = XnUSBSetInterface(pDevHandle, pUsbInterfaceDescriptor->bInterfaceNumber, pUsbInterfaceDescriptor->bAlternateSetting);
	if (nRetVal != XN_STATUS_OK)
	{
		XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
		XN_ALIGNED_FREE_AND_NULL(pDevHandle);
		return (nRetVal);	
	}

	// Mark the handle as valid
	pDevHandle->bValid = TRUE;

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBCloseDevice(XN_USB_DEV_HANDLE pDevHandle)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;
	XN_INT32 nCellRetVal = 0;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PDEV_HANDLE(pDevHandle);

	// Close the endpoint handle
	XN_USB_DEBUG_PRINTF("XnUSBCloseDevice: Closing DeviceID=%d...\n", pDevHandle->nDevId);

	// Close the control endpoint
	nCellRetVal = cellUsbdClosePipe(pDevHandle->nCtrlPipe);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBCloseDevice: cellUsbdClosePipe failed to close the control endpoint! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_CLOSE_ENDPOINT_FAILED);
	}

	// Detach the device from USBD...
	nRetVal = XnUSBDetach(g_XnUSBDevHandle->nDevId);
	XN_IS_STATUS_OK(nRetVal);

	// Unregister the LDDs from USBD
	nCellRetVal = cellUsbdUnregisterExtraLdd(&XnUSBDOps);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBCloseDevice: cellUsbdUnregisterExtraLdd failed to unregister USBD LDDs! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_UNREGISTER_FAILED);
	}

	// Close the busy event
	nRetVal = XnOSCloseEvent(&pDevHandle->ovlpIO.BusyEvent);
	XN_IS_STATUS_OK(nRetVal);

	// Free the device handle
	XN_ALIGNED_FREE_AND_NULL(pDevHandle);

	// Clear the global device handle
	g_XnUSBDevHandle = NULL;

	// All is good...
	XN_USB_DEBUG_PRINTF("XnUSBCloseDevice: Success!\n");

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBGetDeviceSpeed(XN_USB_DEV_HANDLE pDevHandle, XnUSBDeviceSpeed* pDevSpeed)
{
	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PDEV_HANDLE(pDevHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pDevSpeed);

	// Update the device speed result
	*pDevSpeed = pDevHandle->nDevSpeed;

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBSetConfig(XN_USB_DEV_HANDLE pDevHandle, XN_UINT8 nConfig)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate USB init
	XN_VALIDATE_USB_INIT();

	XN_USB_DEBUG_PRINTF("XnUSBSetConfig: Setting the device config interface to %d...\n", nConfig);

	// Set busy
	nRetVal = XnUSBSetBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pDevHandle->ovlpIO.nTransferBytes = 0;
	pDevHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Change the config interface via a control transfer
	nCellRetVal = cellUsbdSetConfiguration(pDevHandle->nCtrlPipe, nConfig, XnUSBTransferDoneCB, &pDevHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSetConfig: cellUsbdSetInterface failed to set the config interface! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_SET_CONFIG_FAILED);
	}

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pDevHandle->ovlpIO, XN_USB_CONTROL_TIMEOUT);
	if (nRetVal != XN_STATUS_OK)
	{
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pDevHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSetConfig: USB Control transfer failed! (%s) (0x%x)\n", XnGetStatusString(pDevHandle->ovlpIO.nRetVal), pDevHandle->ovlpIO.nUSBResult);
		nRetVal = pDevHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// We are not expecting any data to return
	if (pDevHandle->ovlpIO.nTransferBytes != 0)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSetConfig: USB Control returned with unexpected data! (%d)\n", pDevHandle->ovlpIO.nTransferBytes);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_GOT_UNEXPECTED_BYTES);
	}

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	XN_USB_DEBUG_PRINTF("XnUSBSetConfig: Device config interface is now set to %d\n", nConfig);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBGetConfig(XN_USB_DEV_HANDLE pDevHandle, XN_UINT8* pnConfig)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PDEV_HANDLE(pDevHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pnConfig);

	XN_USB_DEBUG_PRINTF("XnUSBGetConfig: Getting the device config interface...\n");

	// Set busy
	nRetVal = XnUSBSetBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pDevHandle->ovlpIO.nTransferBytes = 0;
	pDevHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Change the config interface via a control transfer
	nCellRetVal = cellUsbdGetConfiguration(pDevHandle->nCtrlPipe, pnConfig, XnUSBTransferDoneCB, &pDevHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBGetConfig: cellUsbdSetInterface failed to set the config interface! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_SET_CONFIG_FAILED);
	}

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pDevHandle->ovlpIO, XN_USB_CONTROL_TIMEOUT);
	if (nRetVal != XN_STATUS_OK)
	{
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pDevHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBGetConfig: USB Control transfer failed! (%s) (0x%x)\n", XnGetStatusString(pDevHandle->ovlpIO.nRetVal), pDevHandle->ovlpIO.nUSBResult);
		nRetVal = pDevHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Make sure we got some data back
	if (pDevHandle->ovlpIO.nTransferBytes == 0)
	{
		XN_USB_DEBUG_PRINTF("XnUSBGetConfig: No data was read!\n");
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_NOT_ENOUGH_DATA);
	}

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	XN_USB_DEBUG_PRINTF("XnUSBGetConfig: Device config interface is %d\n", *pnConfig);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBSetInterface(XN_USB_DEV_HANDLE pDevHandle, XN_UINT8 nInterface, XN_UINT8 nAltInterface)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate USB init
	XN_VALIDATE_USB_INIT();

	XN_USB_DEBUG_PRINTF("XnUSBSetInterface: Setting the device interface to %d %d...\n", nInterface, nAltInterface);

	// Set busy
	nRetVal = XnUSBSetBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pDevHandle->ovlpIO.nTransferBytes = 0;
	pDevHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Change the interface via a control transfer
	nCellRetVal = cellUsbdSetInterface(pDevHandle->nCtrlPipe, nInterface, nAltInterface, XnUSBTransferDoneCB, &pDevHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSetInterface: cellUsbdSetInterface failed to set the interface! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_SET_INTERFACE_FAILED);
	}

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pDevHandle->ovlpIO, XN_USB_CONTROL_TIMEOUT);
	if (nRetVal != XN_STATUS_OK)
	{
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pDevHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSetInterface: USB Control transfer failed! (%s) (0x%x)\n", XnGetStatusString(pDevHandle->ovlpIO.nRetVal), pDevHandle->ovlpIO.nUSBResult);
		nRetVal = pDevHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// We are not expecting any data to return
	if (pDevHandle->ovlpIO.nTransferBytes != 0)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSetInterface: USB Control returned with unexpected data! (%d)\n", pDevHandle->ovlpIO.nTransferBytes);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_GOT_UNEXPECTED_BYTES);
	}

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	pDevHandle->nInterface = nInterface;
	pDevHandle->nAltInterface = nAltInterface;
	XN_USB_DEBUG_PRINTF("XnUSBSetInterface: Device alternate interface is now set to %d %d\n", nInterface, nAltInterface);

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBGetInterface(XN_USB_DEV_HANDLE pDevHandle, XN_UINT8* pnInterface, XN_UINT8* pnAltInterface)
{
	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PDEV_HANDLE(pDevHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pnInterface);
	XN_VALIDATE_OUTPUT_PTR(pnAltInterface);

	// Update the interface result
	*pnInterface = pDevHandle->nInterface;
	*pnAltInterface = pDevHandle->nAltInterface;

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBOpenEndPoint(XN_USB_DEV_HANDLE pDevHandle, XN_UINT16 nEndPointID, XnUSBEndPointType nEPType, XnUSBDirectionType nDirType, XN_USB_EP_HANDLE* pEPHandlePtr)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;
	XN_USB_EP_HANDLE pEPHandle = NULL;
	UsbConfigurationDescriptor* pUsbConfigurationDescriptor = NULL;
	UsbInterfaceDescriptor* pUsbInterfaceDescriptor = NULL;
	UsbEndpointDescriptor* pUsbEndpointDescriptor = NULL;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PDEV_HANDLE(pDevHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pEPHandlePtr);

	// Allocate memory for the endpoint handle
	XN_VALIDATE_ALIGNED_CALLOC(*pEPHandlePtr, XnUSBEPHandle, 1, XN_DEFAULT_MEM_ALIGN);
	pEPHandle = *pEPHandlePtr;

	// Get the configuration descriptor
	pUsbConfigurationDescriptor = (UsbConfigurationDescriptor*)cellUsbdScanStaticDescriptor(pDevHandle->nDevId, NULL, USB_DESCRIPTOR_TYPE_CONFIGURATION);
	if (pUsbConfigurationDescriptor == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenEndPoint: cellUsbdScanStaticDescriptor failed to get the device configuration descriptor!\n");
		XN_ALIGNED_FREE_AND_NULL(pEPHandlePtr);
		return (XN_STATUS_USB_CONFIG_QUERY_FAILED);
	}

	// Get the interface descriptor
	pUsbInterfaceDescriptor = (UsbInterfaceDescriptor*)pUsbConfigurationDescriptor;
	if ((pUsbInterfaceDescriptor = (UsbInterfaceDescriptor*)cellUsbdScanStaticDescriptor(pDevHandle->nDevId, pUsbInterfaceDescriptor, USB_DESCRIPTOR_TYPE_INTERFACE)) == NULL)
	{
		XN_USB_DEBUG_PRINTF("XnUSBOpenEndPoint: cellUsbdScanStaticDescriptor failed to get the device interface descriptor!\n");
		XN_ALIGNED_FREE_AND_NULL(pEPHandlePtr);
		return (XN_STATUS_USB_INTERFACE_QUERY_FAILED);
	}

	// Look for the right endpoint...
	pUsbEndpointDescriptor = (UsbEndpointDescriptor*)pUsbInterfaceDescriptor;
	for (XN_UINT32 nNumEndpoints = pUsbInterfaceDescriptor->bNumEndpoints; nNumEndpoints > 0; nNumEndpoints--)
	{
		// Read the endpoint descriptor
		pUsbEndpointDescriptor = (UsbEndpointDescriptor*)cellUsbdScanStaticDescriptor(pDevHandle->nDevId, pUsbEndpointDescriptor, USB_DESCRIPTOR_TYPE_ENDPOINT);
		if (pUsbEndpointDescriptor == NULL)
		{
			XN_USB_DEBUG_PRINTF("XnUSBOpenEndPoint: cellUsbdScanStaticDescriptor failed to get the device endpoint descriptor!\n");
			XN_ALIGNED_FREE_AND_NULL(pEPHandlePtr);
			return (XN_STATUS_USB_ENDPOINT_QUERY_FAILED);
		}
		else
		{
			// Is this the endpoint we're looking for?
			if (pUsbEndpointDescriptor->bEndpointAddress == nEndPointID)	
			{
				// Make sure the endpoint matches the required endpoint type
				if (nEPType == XN_USB_EP_BULK)
				{
					if (XN_USB_GET_EP_TYPE_FROM_ATTR(pUsbEndpointDescriptor->bmAttributes) != USB_ENDPOINT_TRANSFER_TYPE_BULK)
					{
						return (XN_STATUS_USB_WRONG_ENDPOINT_TYPE);
					}
				}
				else if (nEPType == XN_USB_EP_INTERRUPT)
				{
					if (XN_USB_GET_EP_TYPE_FROM_ATTR(pUsbEndpointDescriptor->bmAttributes) != USB_ENDPOINT_TRANSFER_TYPE_INTERRUPT)
					{
						return (XN_STATUS_USB_WRONG_ENDPOINT_TYPE);
					}
				}
				else if (nEPType == XN_USB_EP_ISOCHRONOUS)
				{
					return (XN_STATUS_USB_UNSUPPORTED_ENDPOINT_TYPE);
				}
				else
				{
					return (XN_STATUS_USB_UNKNOWN_ENDPOINT_TYPE);
				}

				// Make sure the endpoint matches the required direction
				if (nDirType == XN_USB_DIRECTION_IN)
				{
					if (XN_IS_USB_ENDPOINT_DIRECTION_IN(pUsbEndpointDescriptor->bEndpointAddress) == FALSE)
					{
						return (XN_STATUS_USB_WRONG_ENDPOINT_DIRECTION);
					}
				}
				else if (nDirType == XN_USB_DIRECTION_OUT)
				{
					if (XN_IS_USB_ENDPOINT_DIRECTION_OUT(pUsbEndpointDescriptor->bEndpointAddress) == FALSE)
					{
						return (XN_STATUS_USB_WRONG_ENDPOINT_DIRECTION);
					}
				}
				else
				{
					return (XN_STATUS_USB_UNKNOWN_ENDPOINT_DIRECTION);
				}

				// Open a handle to the endpoint
				pEPHandle->hEP = cellUsbdOpenPipe(pDevHandle->nDevId, pUsbEndpointDescriptor);
				if (pEPHandle->hEP < 0)
				{
					XN_USB_DEBUG_PRINTF("XnUSBOpenEndPoint: cellUsbdOpenPipe failed to open the endpoint descriptor! (%Xh)\n", pEPHandle->hEP);
					XN_ALIGNED_FREE_AND_NULL(pEPHandlePtr);
					return (XN_STATUS_USB_OPEN_ENDPOINT_FAILED);
				}

				// Update the internal endpoint struct
				pEPHandle->pDevHandle = pDevHandle;
				pEPHandle->nEPType = nEPType;
				pEPHandle->nEPDir = nDirType;

				// Init the overlapped IO event
				nRetVal = XnOSCreateEvent(&pEPHandle->ovlpIO.BusyEvent);
				if (nRetVal != XN_STATUS_OK)
				{
					XN_USB_DEBUG_PRINTF("XnUSBOpenEndPoint: failed to create the overlapped IO event!\n");
					XN_ALIGNED_FREE_AND_NULL(pEPHandlePtr);
					return (nRetVal);
				}

				pEPHandle->ovlpIO.nTransferBytes = 0;
				pEPHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

				// Init the thread data
				XnOSMemSet(&pEPHandle->ThreadData, 0, sizeof(XnUSBReadThreadData));
				pEPHandle->ThreadData.bInUse = FALSE;

				// Mark the endpoint handle as valid
				pEPHandle->bValid = TRUE;

				XN_USB_DEBUG_PRINTF("XnUSBOpenEndPoint: Success! Endpoint=%d Address=0x%x Type=%d Dir=%d\n", pEPHandle->hEP, pUsbEndpointDescriptor->bEndpointAddress, pEPHandle->nEPType, pEPHandle->nEPDir);

				// All is good...
				return (XN_STATUS_OK);
			}
		}
	}

	// If we reached this point, it means we've scanned all the endpoints and didn't find a match...
	return (XN_STATUS_USB_ENDPOINT_NOT_FOUND);
}

XN_CORE_API XnStatus XnUSBCloseEndPoint(XN_USB_EP_HANDLE pEPHandle)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PEP_HANDLE(pEPHandle);
	XN_VALIDATE_USB_PDEV_HANDLE(pEPHandle->pDevHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pEPHandle);

	// Close the endpoint handle
	XN_USB_DEBUG_PRINTF("XnUSBCloseEndPoint: Closing endpoint %d...\n", pEPHandle->hEP);

	nCellRetVal = cellUsbdClosePipe(pEPHandle->hEP);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBCloseEndPoint: cellUsbdClosePipe failed to close the endpoint! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_CLOSE_ENDPOINT_FAILED);
	}

	// Close the overlapped IO event
	XnOSCloseEvent(&pEPHandle->ovlpIO.BusyEvent);

	// Free the endpoint handle
	XN_ALIGNED_FREE_AND_NULL(pEPHandle);

	// All is good...
	XN_USB_DEBUG_PRINTF("XnUSBCloseEndPoint: Success!\n");

	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBAbortEndPoint(XN_USB_EP_HANDLE pEPHandle)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnUSBFlushEndPoint(XN_USB_EP_HANDLE pEPHandle)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnUSBResetEndPoint(XN_USB_EP_HANDLE pEPHandle)
{
	return (XN_STATUS_OS_UNSUPPORTED_FUNCTION);
}

XN_CORE_API XnStatus XnUSBSendControl(XN_USB_DEV_HANDLE pDevHandle, XnUSBControlType nType, XN_UINT8 nRequest, XN_UINT16 nValue, XN_UINT16 nIndex, XN_UCHAR* pBuffer, XN_UINT32 nBufferSize, XN_UINT32 nTimeOut)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;
	UsbDeviceRequest DeviceRequest;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PDEV_HANDLE(pDevHandle);

	// Validate input/output pointers
	if (nBufferSize != 0)
	{
		XN_VALIDATE_INPUT_PTR(pBuffer);
	}

	// Set the request type
	if (nType == XN_USB_CONTROL_TYPE_VENDOR)
	{
		DeviceRequest.bmRequestType = USB_REQTYPE_TYPE_VENDOR;
	}
	else if (nType == XN_USB_CONTROL_TYPE_CLASS)
	{
		DeviceRequest.bmRequestType = USB_REQTYPE_TYPE_CLASS;
	}
	else if (nType == XN_USB_CONTROL_TYPE_STANDARD)
	{
		DeviceRequest.bmRequestType = USB_REQTYPE_TYPE_STANDARD;
	}
	else
	{
		return (XN_STATUS_USB_WRONG_CONTROL_TYPE);
	}

	// Prepare the request
	DeviceRequest.bRequest = nRequest;
	DeviceRequest.wValue = nValue;
	DeviceRequest.wIndex = nIndex;
	DeviceRequest.wLength = nBufferSize;

	XN_USB_DEBUG_PRINTF("XnUSBSendControl: Sending a control transfer... (Type=%x Request=%d Value=%d Index=%d Size=%d)\n", DeviceRequest.bmRequestType, nRequest, nValue, nIndex, nBufferSize);

	// Set busy
	nRetVal = XnUSBSetBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pDevHandle->ovlpIO.nTransferBytes = 0;
	pDevHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Send the control transfer
	nCellRetVal = cellUsbdControlTransfer(pDevHandle->nCtrlPipe, &DeviceRequest, pBuffer, XnUSBTransferDoneCB, &pDevHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSendControl: cellUsbdControlTransfer: failed to do a control transfer! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_CONTROL_SEND_FAILED);
	}

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pDevHandle->ovlpIO, nTimeOut);
	if (nRetVal != XN_STATUS_OK)
	{
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pDevHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSendControl: USB Control transfer failed! (%s) (0x%x)\n", XnGetStatusString(pDevHandle->ovlpIO.nRetVal), pDevHandle->ovlpIO.nUSBResult);
		nRetVal = pDevHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Make sure we have written the right amount of data
	if (pDevHandle->ovlpIO.nTransferBytes != (XN_INT32)nBufferSize)
	{
		XN_USB_DEBUG_PRINTF("XnUSBSendControl: Not all of the data was written! (%d)\n", pDevHandle->ovlpIO.nTransferBytes);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_GOT_UNEXPECTED_BYTES);
	}

	XN_USB_DEBUG_PRINTF("XnUSBSendControl: Successfully written %d bytes to the control endpoint.\n", pDevHandle->ovlpIO.nTransferBytes);

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBReceiveControl(XN_USB_DEV_HANDLE pDevHandle, XnUSBControlType nType, XN_UINT8 nRequest, XN_UINT16 nValue, XN_UINT16 nIndex, XN_UCHAR* pBuffer, XN_UINT32 nBufferSize, XN_UINT32* pnBytesReceived, XN_UINT32 nTimeOut)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;
	UsbDeviceRequest DeviceRequest;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PDEV_HANDLE(pDevHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pBuffer);
	XN_VALIDATE_OUTPUT_PTR(pnBytesReceived);

	// Clear the number of received bytes
	*pnBytesReceived = 0;

	// Make sure we have a big enough buffer to read into
	if (nBufferSize == 0)
	{
		return (XN_STATUS_USB_BUFFER_TOO_SMALL);
	}

	// Set the request type
	if (nType == XN_USB_CONTROL_TYPE_VENDOR )
	{
		DeviceRequest.bmRequestType = USB_REQTYPE_TYPE_VENDOR | USB_REQTYPE_DIR_TO_HOST;
	}
	else if (nType == XN_USB_CONTROL_TYPE_CLASS)
	{
		DeviceRequest.bmRequestType = USB_REQTYPE_TYPE_CLASS | USB_REQTYPE_DIR_TO_HOST;
	}
	else if (nType == XN_USB_CONTROL_TYPE_STANDARD)
	{
		DeviceRequest.bmRequestType = USB_REQTYPE_TYPE_STANDARD | USB_REQTYPE_DIR_TO_HOST;
	}
	else
	{
		return (XN_STATUS_USB_WRONG_CONTROL_TYPE);
	}

	// Prepare the request
	DeviceRequest.bRequest = nRequest;
	DeviceRequest.wValue = nValue;
	DeviceRequest.wIndex = nIndex;
	DeviceRequest.wLength = nBufferSize;

	XN_USB_DEBUG_PRINTF("XnUSBReceiveControl: sending a control transfer... (Type=%x Request=%d Value=%d Index=%d Size=%d)\n", DeviceRequest.bmRequestType, nRequest, nValue, nIndex, nBufferSize);

	// Set busy
	nRetVal = XnUSBSetBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pDevHandle->ovlpIO.nTransferBytes = 0;
	pDevHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Send the control transfer
	nCellRetVal = cellUsbdControlTransfer(pDevHandle->nCtrlPipe, &DeviceRequest, pBuffer, XnUSBTransferDoneCB, &pDevHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBReceiveControl: cellUsbdControlTransfer: failed to do a control transfer! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_CONTROL_RECV_FAILED);
	}

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pDevHandle->ovlpIO, nTimeOut);
	if (nRetVal != XN_STATUS_OK)
	{
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pDevHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBReceiveControl: USB Control transfer failed! (%s) (0x%x)\n", XnGetStatusString(pDevHandle->ovlpIO.nRetVal), pDevHandle->ovlpIO.nUSBResult);
		nRetVal = pDevHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (nRetVal);
	}

	// Make sure we got some data back
	if (pDevHandle->ovlpIO.nTransferBytes == 0)
	{
		XN_USB_DEBUG_PRINTF("XnUSBReceiveControl: No data was read!\n");
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_NOT_ENOUGH_DATA);
	}
	else if (pDevHandle->ovlpIO.nTransferBytes > (XN_INT32)nBufferSize)
	{
		XN_USB_DEBUG_PRINTF("XnUSBReceiveControl: Too much data was read! (%d)\n", pDevHandle->ovlpIO.nTransferBytes);
		XnUSBClearBusy(&pDevHandle->ovlpIO);
		return (XN_STATUS_USB_TOO_MUCH_DATA);
	}

	// Update the number of returned bytes
	*pnBytesReceived = pDevHandle->ovlpIO.nTransferBytes;

	XN_USB_DEBUG_PRINTF("XnUSBReceiveControl: Successfully read %d bytes from the control endpoint.\n", *pnBytesReceived);

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pDevHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBReadEndPoint(XN_USB_EP_HANDLE pEPHandle, XN_UCHAR* pBuffer, XN_UINT32 nBufferSize, XN_UINT32* pnBytesReceived, XN_UINT32 nTimeOut)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PEP_HANDLE(pEPHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pBuffer);
	XN_VALIDATE_OUTPUT_PTR(pnBytesReceived);

	// Clear the number of received bytes
	*pnBytesReceived = 0;

	// Make sure we're only reading from a bulk IN endpoint
	if (pEPHandle->nEPDir != XN_USB_DIRECTION_IN)
	{
		return (XN_STATUS_USB_WRONG_ENDPOINT_DIRECTION);
	}

	if (pEPHandle->nEPType != XN_USB_EP_BULK)
	{
		return (XN_STATUS_USB_UNSUPPORTED_ENDPOINT_TYPE);
	}

	// Make sure we're reading enough bytes
	if (nBufferSize == 0)
	{
		return (XN_STATUS_USB_BUFFER_TOO_SMALL);
	}

	XN_USB_DEBUG_PRINTF("XnUSBReadEndPoint: Reading %d bytes from endpoint %d...\n", nBufferSize, pEPHandle->hEP);

	// Set busy
	nRetVal = XnUSBSetBusy(&pEPHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pEPHandle->ovlpIO.nTransferBytes = 0;
	pEPHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Read from the endpoint
	nCellRetVal = cellUsbdBulkTransfer(pEPHandle->hEP, pBuffer, nBufferSize, XnUSBTransferDoneCB, &pEPHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBReadEndPoint: cellUsbdBulkTransfer failed to do a bulk transfer! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (XN_STATUS_USB_ENDPOINT_READ_FAILED);
	}

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pEPHandle->ovlpIO, nTimeOut);
	if (nRetVal != XN_STATUS_OK)
	{
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pEPHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBReadEndPoint: USB bulk IN transfer failed! (%s) (0x%x)\n", XnGetStatusString(pEPHandle->ovlpIO.nRetVal), pEPHandle->ovlpIO.nUSBResult);
		nRetVal = pEPHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (nRetVal);
	}

	// Make sure we got some data back
	if (pEPHandle->ovlpIO.nTransferBytes == 0)
	{
		XN_USB_DEBUG_PRINTF("XnUSBReadEndPoint: No data was read!\n");
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (XN_STATUS_USB_NOT_ENOUGH_DATA);
	}

	// Update the number of returned bytes
	*pnBytesReceived = pEPHandle->ovlpIO.nTransferBytes;

	XN_USB_DEBUG_PRINTF("XnUSBReadEndPoint: Successfully read %d bytes from the endpoint.\n", *pnBytesReceived);

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pEPHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBWriteEndPoint(XN_USB_EP_HANDLE pEPHandle, XN_UCHAR* pBuffer, XN_UINT32 nBufferSize, XN_UINT32 nTimeOut)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PEP_HANDLE(pEPHandle);

	// Validate input/output pointers
	XN_VALIDATE_INPUT_PTR(pBuffer);

	// Make sure we're only reading from a bulk OUT endpoint 
	if (pEPHandle->nEPDir != XN_USB_DIRECTION_OUT)
	{
		return (XN_STATUS_USB_WRONG_ENDPOINT_DIRECTION);
	}

	if (pEPHandle->nEPType != XN_USB_EP_BULK)
	{
		return (XN_STATUS_USB_UNSUPPORTED_ENDPOINT_TYPE);
	}

	// Make sure we're writing enough bytes
	if (nBufferSize == 0)
	{
		return (XN_STATUS_USB_BUFFER_TOO_SMALL);
	}

	XN_USB_DEBUG_PRINTF("XnUSBWriteEndPoint: Writing %d bytes to endpoint %d...\n", nBufferSize, pEPHandle->hEP);

	// Set busy
	nRetVal = XnUSBSetBusy(&pEPHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pEPHandle->ovlpIO.nTransferBytes = 0;
	pEPHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Write to the endpoint
	nCellRetVal = cellUsbdBulkTransfer(pEPHandle->hEP, pBuffer, nBufferSize, XnUSBTransferDoneCB, &pEPHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBWriteEndPoint: cellUsbdBulkTransfer failed to do a bulk transfer! (%Xh)\n", nCellRetVal);
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (XN_STATUS_USB_ENDPOINT_WRITE_FAILED);
	}

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pEPHandle->ovlpIO, nTimeOut);
	if (nRetVal != XN_STATUS_OK	)
	{
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pEPHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBWriteEndPoint: USB bulk OUT transfer failed! (%s) (0x%x)\n", XnGetStatusString(pEPHandle->ovlpIO.nRetVal), pEPHandle->ovlpIO.nUSBResult);
		nRetVal = pEPHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (nRetVal);
	}

	// Make sure we have written the right amount of data
	if (pEPHandle->ovlpIO.nTransferBytes != (XN_INT32)nBufferSize)
	{
		XN_USB_DEBUG_PRINTF("XnUSBWriteEndPoint: Not all of the data was written! (%d)\n", pEPHandle->ovlpIO.nTransferBytes);
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (XN_STATUS_USB_GOT_UNEXPECTED_BYTES);
	}

	XN_USB_DEBUG_PRINTF("XnUSBReadEndPoint: Successfully written %d bytes to the endpoint.\n", pEPHandle->ovlpIO.nTransferBytes);

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pEPHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBQueueReadEndPoint(XN_USB_EP_HANDLE pEPHandle, XN_UCHAR* pBuffer, XN_UINT32 nBufferSize, XN_UINT32 nTimeOut)
{
	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PEP_HANDLE(pEPHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pBuffer);

	// Make sure we're only reading from a bulk IN endpoint
	if (pEPHandle->nEPDir != XN_USB_DIRECTION_IN)
	{
		return (XN_STATUS_USB_WRONG_ENDPOINT_DIRECTION);
	}

	if (pEPHandle->nEPType != XN_USB_EP_BULK)
	{
		return (XN_STATUS_USB_UNSUPPORTED_ENDPOINT_TYPE);
	}

	// Make sure we're reading enough bytes
	if (nBufferSize == 0)
	{
		return (XN_STATUS_USB_BUFFER_TOO_SMALL);
	}

	XN_USB_DEBUG_PRINTF("XnUSBQueueReadEndPoint: Queuing to read %d bytes from endpoint %d...\n", nBufferSize, pEPHandle->hEP);

	// Set busy
	nRetVal = XnUSBSetBusy(&pEPHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// Prepare the transfer context
	pEPHandle->ovlpIO.nTransferBytes = 0;
	pEPHandle->ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

	// Read from the endpoint
	nCellRetVal = cellUsbdBulkTransfer(pEPHandle->hEP, pBuffer, nBufferSize, XnUSBTransferDoneCB, &pEPHandle->ovlpIO);
	if (nCellRetVal != CELL_OK)
	{
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		XN_USB_DEBUG_PRINTF("XnUSBQueueReadEndPoint: cellUsbdBulkTransfer failed to do a bulk transfer! (%Xh)\n", nCellRetVal);
		return (XN_STATUS_USB_ENDPOINT_READ_FAILED);
	}

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBFinishReadEndPoint(XN_USB_EP_HANDLE pEPHandle, XN_UINT32* pnBytesReceived, XN_UINT32 nTimeOut)
{	
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PEP_HANDLE(pEPHandle);

	// Validate input/output pointers
	XN_VALIDATE_OUTPUT_PTR(pnBytesReceived);

	// Wait for the USB transfer to complete
	nRetVal = XnUSBWaitWithTimeout(&pEPHandle->ovlpIO, nTimeOut);
	if (nRetVal != XN_STATUS_OK)
	{
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (nRetVal);
	}

	// Check the transfer status
	if (pEPHandle->ovlpIO.nRetVal != XN_STATUS_OK)
	{
		XN_USB_DEBUG_PRINTF("XnUSBFinishReadEndPoint: USB bulk IN transfer failed! (%s) (0x%x)\n", XnGetStatusString(pEPHandle->ovlpIO.nRetVal), pEPHandle->ovlpIO.nUSBResult);
		nRetVal = pEPHandle->ovlpIO.nRetVal;
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (nRetVal);
	}

	// Make sure we got some data back
	if (pEPHandle->ovlpIO.nTransferBytes == 0)
	{
		XN_USB_DEBUG_PRINTF("XnUSBFinishReadEndPoint: No data was read!\n");
		XnUSBClearBusy(&pEPHandle->ovlpIO);
		return (XN_STATUS_USB_NOT_ENOUGH_DATA);
	}

	// Update the number of returned bytes
	*pnBytesReceived = pEPHandle->ovlpIO.nTransferBytes;

	XN_USB_DEBUG_PRINTF("XnUSBFinishReadEndPoint: Successfully read %d bytes from the endpoint.\n", *pnBytesReceived);

	// Clear the busy state
	nRetVal = XnUSBClearBusy(&pEPHandle->ovlpIO);
	XN_IS_STATUS_OK(nRetVal);

	// All is good...
	return (XN_STATUS_OK);

}

XN_THREAD_PROC XnUSBReadThreadMain(XN_THREAD_PARAM pThreadParam)
{
	// ZZZZ This entire function needs to be re-written to use a double buffer (since the PS3 can't queue multiple request)

	// Local function variables
	XN_INT32 nCellRetVal = 0;
	XnStatus nRetVal = XN_STATUS_OK;
	XnUSBReadThreadData* pThreadData = (XnUSBReadThreadData*)pThreadParam;
	XN_UINT32 nBufIdx = 0;

	// Make sure we have a valid thread data pointer
	if (pThreadData == NULL)
	{
		XN_THREAD_PROC_RETURN(XN_STATUS_ERROR);
	}

	XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: Read thread is alive!\n");

	// ZZZZ Fix me!!! Must add a way to do priorities on the PS3 in a pretty way
	sys_ppu_thread_t ThreadID;
	sys_ppu_thread_get_id(&ThreadID);
	sys_ppu_thread_set_priority(ThreadID, XN_USB_READ_THREAD_PRIORITY);

	// Queue the first read commands
	for (nBufIdx=0; nBufIdx<pThreadData->nNumBuffers; nBufIdx++)
	{
		pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nTransferBytes = 0;
		pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;
	
		XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: Queue buffer %d\n", nBufIdx);

		nCellRetVal = cellUsbdBulkTransfer(pThreadData->pEPHandle->hEP, &pThreadData->pBuffersInfo[nBufIdx].pBuffer[0], pThreadData->nBufferSize, XnUSBTransferDoneCB, &pThreadData->pBuffersInfo[nBufIdx].ovlpIO);
		if (nCellRetVal != CELL_OK)
		{
			XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: cellUsbdBulkTransfer failed to queue bulk transfer for buffer %d! (%Xh)\n", nBufIdx, nCellRetVal);
		}	
	}

	// Loop forever (or until we're asked to die...)
	while (1)
	{
		for (nBufIdx=0; nBufIdx<pThreadData->nNumBuffers; nBufIdx++)
		{
			// Time to die?
			if(pThreadData->bKillReadThread)
			{
				goto EndThread;
			}	

			XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: Waiting for buffer %d\n", nBufIdx);

			// Wait for the transfer to come back and check the event status
			nRetVal = XnOSWaitEvent(pThreadData->pBuffersInfo[nBufIdx].ovlpIO.BusyEvent, pThreadData->nTimeOut);
			if (nRetVal != XN_STATUS_OK)
			{
				XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: wait error for buffer %d (%s)!\n", nBufIdx, XnGetStatusString(nRetVal));
			}
			else
			{
				// Check the USB transfer status
				if (pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nRetVal != XN_STATUS_OK)
				{
					XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: USB transfer failed for buffer %d! (%s) (0x%x)\n", nBufIdx, XnGetStatusString(pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nRetVal), pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nUSBResult);
				}
				else
				{
					// All is good so call the user callback function
					XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: buffer %d is ready! calling the CB function...\n", nBufIdx);
					pThreadData->pCallbackFunction(pThreadData->pBuffersInfo[nBufIdx].pBuffer, pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nTransferBytes, pThreadData->pCallbackData);

					// Queue another read from the endpoint
					pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nTransferBytes = 0;
					pThreadData->pBuffersInfo[nBufIdx].ovlpIO.nRetVal = XN_STATUS_USB_TRANSFER_PENDING;

					XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: Queue buffer %d\n", nBufIdx);

					nCellRetVal = cellUsbdBulkTransfer(pThreadData->pEPHandle->hEP, &pThreadData->pBuffersInfo[nBufIdx].pBuffer[0], pThreadData->nBufferSize, XnUSBTransferDoneCB, &pThreadData->pBuffersInfo[nBufIdx].ovlpIO);
					if (nCellRetVal != CELL_OK)
					{
						XN_USB_DEBUG_PRINTF("XnUSBReadThreadMain: cellUsbdBulkTransfer failed to queue bulk transfer for buffer %d! (%Xh)\n", nBufIdx, nCellRetVal);
					}	
				}
			}
		}
	}

EndThread:
	XN_USB_DEBUG_PRINTF("XnUSBInitReadThread: Read thread is terminating!\n");

	XN_THREAD_PROC_RETURN (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBInitReadThread(XN_USB_EP_HANDLE pEPHandle, XN_UINT32 nBufferSize, XN_UINT32 nNumBuffers, XN_UINT32 nTimeOut, XnUSBReadCallbackFunctionPtr pCallbackFunction, void* pCallbackData)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;
	XnUSBReadThreadData* pThreadData = NULL;
	XN_UINT32 nBufIdx = 0;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PEP_HANDLE(pEPHandle);

	// Validate input/output pointers
	XN_VALIDATE_INPUT_PTR(pCallbackFunction);

	XN_USB_DEBUG_PRINTF("XnUSBInitReadThread: Going to create a read thread from endpoint %d (NumBufs=%d BuSize=%d)\n", pEPHandle->hEP, nNumBuffers, nBufferSize);

	// Get the endpoint's thread data
	pThreadData = &pEPHandle->ThreadData;

	// Make sure this endpoint doesn't already have a read thread active
	if (pThreadData->bInUse == TRUE)
	{
		return (XN_STATUS_USB_READTHREAD_ALREADY_INIT);
	}

	// Clear & init the thread data
	XnOSMemSet(pThreadData, 0, sizeof(XnUSBReadThreadData));

	pThreadData->pEPHandle = pEPHandle;
	pThreadData->nBufferSize = nBufferSize;
	pThreadData->nNumBuffers = nNumBuffers;
	pThreadData->nTimeOut = nTimeOut;
	pThreadData->pCallbackFunction = pCallbackFunction;
	pThreadData->pCallbackData = pCallbackData;
	pThreadData->bKillReadThread = FALSE;

	// Allocate & init the thread's buffers info
	XN_VALIDATE_ALIGNED_CALLOC(pThreadData->pBuffersInfo, XnUSBBuffersInfo, nNumBuffers, XN_DEFAULT_MEM_ALIGN);

	for (nBufIdx=0; nBufIdx<nNumBuffers; nBufIdx++)
	{
		// Add cleanup memory here!
		nRetVal = XnOSCreateEvent(&pThreadData->pBuffersInfo[nBufIdx].ovlpIO.BusyEvent);
		XN_IS_STATUS_OK(nRetVal); // Add cleanup memory here!
		//XN_VALIDATE_ALIGNED_CALLOC(pThreadData->pBuffersInfo[nBufIdx].pBuffer, XN_UCHAR, nBufferSize, XN_DEFAULT_MEM_ALIGN); // Add cleanup memory here!
		cellUsbdAllocateMemory((void**)&(pThreadData->pBuffersInfo[nBufIdx].pBuffer), 65536);
		if (pThreadData->pBuffersInfo[nBufIdx].pBuffer == NULL)
		{
			return (XN_STATUS_ALLOC_FAILED);
		}
		pThreadData->pBuffersInfo[nBufIdx].bFailed = FALSE;
	}

	// Create the read thread
	nRetVal = XnOSCreateThread(XnUSBReadThreadMain, (XN_THREAD_PARAM)&pEPHandle->ThreadData, &pThreadData->hReadThread);
	XN_IS_STATUS_OK(nRetVal); // Add cleanup memory here!

	XN_USB_DEBUG_PRINTF("XnUSBInitReadThread: Successfully started the read thread!\n");

	// Mark the read thread as active in the endpoint
	pThreadData->bInUse = TRUE;

	// All is good...
	return (XN_STATUS_OK);
}

XN_CORE_API XnStatus XnUSBShutdownReadThread(XN_USB_EP_HANDLE pEPHandle)
{
	// Local function variables
	XnStatus nRetVal = XN_STATUS_OK;
	XnUSBReadThreadData* pThreadData = NULL;
	XN_UINT32 nBufIdx = 0;

	// Validate USB init
	XN_VALIDATE_USB_INIT();
	XN_VALIDATE_USB_PEP_HANDLE(pEPHandle);

	XN_USB_DEBUG_PRINTF("XnUSBShutdownReadThread: Going to shutdown the read thread for endpoint %d...\n", pEPHandle->hEP);

	// Get the endpoint's thread data
	pThreadData = &pEPHandle->ThreadData;

	// Make sure this endpoint have a read thread active
	if (pThreadData->bInUse == FALSE)
	{
		return (XN_STATUS_USB_READTHREAD_NOT_INIT);
	}

	// Make sure the read thread is valid, and if so shutdown it.
	if (pThreadData->hReadThread != NULL)
	{
		pThreadData->bKillReadThread = TRUE;

		nRetVal = XnOSWaitForThreadExit(pThreadData->hReadThread, XN_USB_READ_THREAD_KILL_TIMEOUT);
		if (nRetVal != XN_STATUS_OK)
		{
			XnOSTerminateThread(&pThreadData->hReadThread);
		}
		else
		{
			XnOSCloseThread(&pThreadData->hReadThread);
		}
	}
	
	// Free the thread's buffers info
	if (pThreadData->pBuffersInfo != NULL)
	{
		for (nBufIdx=0; nBufIdx<pThreadData->nNumBuffers; nBufIdx++)
		{
			if (pThreadData->pBuffersInfo[nBufIdx].ovlpIO.BusyEvent != NULL)
			{
				XnOSCloseEvent(&pThreadData->pBuffersInfo[nBufIdx].ovlpIO.BusyEvent);
			}

			if (pThreadData->pBuffersInfo[nBufIdx].pBuffer != NULL)
			{
				//XN_ALIGNED_FREE_AND_NULL(pThreadData->pBuffersInfo[nBufIdx].pBuffer);
				cellUsbdFreeMemory(pThreadData->pBuffersInfo[nBufIdx].pBuffer);
			}
		}

		XN_ALIGNED_FREE_AND_NULL(pThreadData->pBuffersInfo);
	}

	XN_USB_DEBUG_PRINTF("XnUSBInitReadThread: Successfully shutdown the read thread!\n");

	// Mark the read thread as not active in the endpoint
	pThreadData->bInUse = FALSE;

	// All is good...
	return (XN_STATUS_OK);
}
