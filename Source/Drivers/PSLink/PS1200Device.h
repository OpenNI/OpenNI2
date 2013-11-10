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
#ifndef PS1200DEVICE_H
#define PS1200DEVICE_H

#include "PrimeClient.h"
#include <XnUSB.h>

namespace xn
{

class ClientUSBConnectionFactory;

class PS1200Device : public PrimeClient
{
public:
	PS1200Device();
	virtual ~PS1200Device();

	virtual XnStatus Init(const XnChar* strConnString, XnTransportType transportType);
	virtual void Shutdown();
	virtual XnBool IsInitialized() const;

	XnStatus SetUsbAltInterface(XnUInt8 altInterface);
	XnStatus GetUsbAltInterface(XnUInt8& altInterface) const;

	virtual XnStatus UsbTest(XnUInt32 nSeconds, XnUInt32& endpointsCount, XnUsbTestEndpointResult* endpoints);

protected:
	const ClientUSBConnectionFactory* GetConnectionFactory() const;
	ClientUSBConnectionFactory* GetConnectionFactory();
	IConnectionFactory* CreateConnectionFactory(XnTransportType transportType);

private:
	static const XnUInt32 WAIT_FOR_FREE_BUFFER_TIMEOUT_MS;
	static const XnUInt32 USB_READ_BUFFERS;
	static const XnUInt32 USB_READ_BUFFER_PACKETS;
	static const XnUInt32 USB_READ_TIMEOUT;
	static const XnUSBEndPointType ENDPOINTS_TYPE;
	static const XnUInt32 NUM_OUTPUT_CONNECTIONS;
	static const XnUInt32 NUM_INPUT_CONNECTIONS;
	static const XnUInt32 PRE_CONTROL_RECEIVE_SLEEP;

	//Data members
	XnBool m_bInitialized;

	XnCallbackHandle m_hInputInterruptCallback;

	//Hide assignment operator
	PS1200Device& operator=(const PS1200Device&);
};

}

#endif // PS1200DEVICE_H
