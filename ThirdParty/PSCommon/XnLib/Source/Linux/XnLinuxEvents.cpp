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
#include <XnOS.h>
#include <XnLog.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <errno.h>

struct _XnEvent
{
	XnInt fd;
	XnBool manualReset;
};

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XN_C_API XnStatus xnOSCreateEvent(XN_EVENT_HANDLE* pEventHandle, XnBool manualReset)
{
	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(pEventHandle);
	
	*pEventHandle = NULL;
	
	_XnEvent* pEvent = XN_NEW(_XnEvent);
	pEvent->fd = eventfd(0, EFD_NONBLOCK);

	if (pEvent->fd == -1)
	{
		XN_DELETE(pEvent);
		XN_LOG_WARNING_RETURN(XN_STATUS_OS_EVENT_CREATION_FAILED, XN_MASK_OS, "Failed to create event: eventfd errno is %d", errno);
	}
	
	pEvent->manualReset = manualReset;
	
	*pEventHandle = pEvent;
	
	// All is good...
	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnOSCloseEvent(XN_EVENT_HANDLE* pEventHandle)
{
	// Validate the input/output pointers (to make sure none of them is NULL)
	XN_VALIDATE_INPUT_PTR(pEventHandle);

	// Make sure the actual event handle isn't NULL
	XN_RET_IF_NULL(*pEventHandle, XN_STATUS_OS_INVALID_EVENT);
	
	_XnEvent* pEvent = *pEventHandle;
	
	close(pEvent->fd);
	
	XN_DELETE(pEvent);

	*pEventHandle = NULL;
	return (XN_STATUS_OK);
}

XN_C_API XnStatus xnOSSetEvent(const XN_EVENT_HANDLE EventHandle)
{
	// Make sure the actual event handle isn't NULL
	XN_RET_IF_NULL(EventHandle, XN_STATUS_OS_INVALID_EVENT);

	_XnEvent* pEvent = EventHandle;
	XnUInt64 nValue = 1;
	if (-1 == write(pEvent->fd, &nValue, sizeof(nValue)))
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_OS_EVENT_SET_FAILED, XN_MASK_OS, "Failed to set event: read errno is %d", errno);
	}
	
	return XN_STATUS_OK;
}

XN_C_API XnStatus xnOSResetEvent(const XN_EVENT_HANDLE EventHandle)
{
	XnUInt64 nValue;

	// Make sure the actual event handle isn't NULL
	XN_RET_IF_NULL(EventHandle, XN_STATUS_OS_INVALID_EVENT);

	_XnEvent* pEvent = EventHandle;
	
	// read from the event until it is clear (reading fails with EAGAIN)
	while (-1 != read(pEvent->fd, &nValue, sizeof(nValue)));

	if (errno != EAGAIN)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_OS_EVENT_RESET_FAILED, XN_MASK_OS, "Failed to reset event: read errno is %d", errno);
	}

	return XN_STATUS_OK;
}

XN_C_API XnBool xnOSIsEventSet(const XN_EVENT_HANDLE EventHandle)
{
	return (xnOSWaitEvent(EventHandle, 0) == XN_STATUS_OK);
}

XN_C_API XnStatus xnOSWaitEvent(const XN_EVENT_HANDLE EventHandle, XnUInt32 nMilliseconds)
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(EventHandle->fd, &rfds);

	struct timeval tv;
	tv.tv_sec = nMilliseconds / 1000;
	tv.tv_usec = (nMilliseconds % 1000) * 1000;

	int ret = select(EventHandle->fd + 1, &rfds, NULL, NULL, &tv);
	if (ret == 0)
	{
		return XN_STATUS_OS_EVENT_TIMEOUT;
	}
	else if (ret == -1)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_OS_EVENT_WAIT_FAILED, XN_MASK_OS, "Failed to wait event: eventfd errno is %d", errno);
	}
	else
	{
		if (!EventHandle->manualReset)
		{
			xnOSResetEvent(EventHandle);
		}

		return XN_STATUS_OK;
	}
}

XN_C_API XnStatus xnOSWaitMultipleEvents(XnUInt32 nCount, const XN_EVENT_HANDLE EventHandles[], XnUInt32 nMilliseconds, XnUInt32* pnIndex)
{
	*pnIndex = 0;

	fd_set rfds;
	FD_ZERO(&rfds);
	
	int max_fd = -1;
	
	for (XnUInt32 i = 0; i < nCount; ++i)
	{
		XN_RET_IF_NULL(EventHandles[i], XN_STATUS_OS_INVALID_EVENT);
		FD_SET(EventHandles[i]->fd, &rfds);
		if (EventHandles[i]->fd > max_fd)
		{
			max_fd = EventHandles[i]->fd;
		}
	}
	
	struct timeval tv;
	tv.tv_sec = nMilliseconds / 1000;
	tv.tv_usec = (nMilliseconds % 1000) * 1000;
	
	int ret = select(max_fd + 1, &rfds, NULL, NULL, &tv);
	if (ret == 0)
	{
//		printf("timeout on %d\n", max_fd-1);
		return XN_STATUS_OS_EVENT_TIMEOUT;
	}
	else if (ret == -1)
	{
		XN_LOG_WARNING_RETURN(XN_STATUS_OS_EVENT_CREATION_FAILED, XN_MASK_OS, "Failed to create event: eventfd errno is %d", errno);
	}
	else
	{
		for (XnUInt32 i = 0; i < nCount; ++i)
		{
			if (FD_ISSET(EventHandles[i]->fd, &rfds))
			{
				*pnIndex = i;

				if (!EventHandles[i]->manualReset)
				{
//					printf("auto resetting %d\n", EventHandles[i]->fd);
					xnOSResetEvent(EventHandles[i]);
				}

				return XN_STATUS_OK;
			}
		}
		
		// shouldn't get here
		XN_LOG_ERROR_RETURN(XN_STATUS_ERROR, XN_MASK_OS, "Internal error - no fd in set!");
	}
}