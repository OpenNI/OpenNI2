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
#undef XN_CROSS_PLATFORM
#include <XnOS.h>

#define XN_EVENT_BIT_PATTERN 1

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XN_CORE_API XnStatus XnOSCreateEvent(XN_EVENT_HANDLE* pEventHandle)
{
	return XnOSCreateNamedEvent(pEventHandle, NULL);
}

XN_CORE_API XnStatus XnOSCreateNamedEvent(XN_EVENT_HANDLE* pEventHandle, const XN_CHAR* cpEventName)
{
	XN_VALIDATE_INPUT_PTR(pEventHandle);

	XN_VALIDATE_ALLOC(*pEventHandle, sys_event_queue_t);

	sys_event_queue_attribute_t attrib;

	sys_event_queue_attribute_initialize(attrib);
	if (cpEventName != NULL)
	{
		sys_event_queue_attribute_name_set(attrib.name, cpEventName);
	}

	int nErr = sys_event_queue_create(*pEventHandle, &attrib, SYS_EVENT_QUEUE_LOCAL, 1);
	if (nErr != CELL_OK)
	{
		return XN_STATUS_OS_EVENT_CREATION_FAILED;
	}
	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSCloseEvent(XN_EVENT_HANDLE* pEventHandle)
{
	XN_VALIDATE_INPUT_PTR(pEventHandle);
	XN_RET_IF_NULL(*pEventHandle, XN_STATUS_OS_EVENT_CLOSE_FAILED);
	int nErr = sys_event_queue_destroy(**pEventHandle, SYS_EVENT_QUEUE_DESTROY_FORCE);
	XN_FREE_AND_NULL(*pEventHandle);

	if (nErr != CELL_OK)
	{
		return XN_STATUS_OS_EVENT_CLOSE_FAILED;
	}
	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSSetEvent(const XN_EVENT_HANDLE EventHandle)
{
	XN_RET_IF_NULL(EventHandle, XN_STATUS_OS_INVALID_EVENT);

	sys_ppu_thread_t my_id;
	sys_ppu_thread_get_id(&my_id);
	sys_event_port_t port;
	sys_event_port_create(&port, SYS_EVENT_PORT_LOCAL, (uint64_t)my_id);
	sys_event_port_connect_local(port, *EventHandle);
	int nErr = sys_event_port_send(port, 0, 0, 0);
	sys_event_port_disconnect(port);
	sys_event_port_destroy(port);

	if (nErr != CELL_OK)
	{
		return XN_STATUS_OS_EVENT_SET_FAILED;
	}
	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSResetEvent(const XN_EVENT_HANDLE EventHandle)
{
	return XN_STATUS_OK;
}

XN_CORE_API XnStatus XnOSWaitEvent(const XN_EVENT_HANDLE EventHandle, XN_UINT32 nMilliseconds)
{
	XN_RET_IF_NULL(EventHandle, XN_STATUS_OS_INVALID_EVENT);

	sys_event_t event;
	int nErr = sys_event_queue_receive(*EventHandle, &event, nMilliseconds*1000);

	if (nErr != CELL_OK)
	{
		switch (nErr)
		{
			case ESRCH:
				return XN_STATUS_OS_INVALID_EVENT;
			case ECANCELED:
				return XN_STATUS_OS_EVENT_CANCELED;
			case EINVAL:	
				return XN_STATUS_OS_INVALID_EVENT;
			case ETIMEDOUT:
				return XN_STATUS_OS_EVENT_TIMEOUT;
			default: 
				return XN_STATUS_OS_EVENT_WAIT_FAILED;
		}	
	}

	return XN_STATUS_OK;
}
