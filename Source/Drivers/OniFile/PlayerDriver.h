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
/// @file
/// Contains the declaration of Driver class that implements an OpenNI driver,
/// which manages virtual OpenNI devices. Those devices read their data from
/// *.ONI files.
#ifndef PLAYERDRIVER_H
#define PLAYERDRIVER_H

#ifndef _ONIFILESPLAYER_DRIVER_H_
#define  _ONIFILESPLAYER_DRIVER_H_ 1

#include "Driver/OniDriverAPI.h"
#include "XnString.h"
#include "XnOSCpp.h"

namespace oni_file {

/// Implements an OpenNI driver, which manages virtual OpenNI devices. Those
/// devics read their data from *.ONI files.
class PlayerDriver : public oni::driver::DriverBase
{
public:
    /// @copydoc OniDriverBase::OniDriverBase(OniDriverServices*)
    PlayerDriver(OniDriverServices* pDriverServices);

    /// @copydoc OniDriverBase::DeviceOpen(const char*)
    /// @todo Document it.
    oni::driver::DeviceBase* deviceOpen(const char* strUri, const char* mode);

	void deviceClose(oni::driver::DeviceBase* pDevice);

    /// @copydoc OniDriverBase::Shutdown()
    virtual void shutdown();

    /// Verifies that the given URI's scheme is known, and emits a message which
    /// notifies OpenNI that a device has been connected.
    /// @param[in] strUri The requested URI which points to a *.ONI file.
    /// @returns ONI_STATUS_OK whenever the URI is supported, or ONI_STATUS_ERROR
    /// when it's not.
    virtual OniStatus tryDevice(const char* strUri);

private:

	static XnStatus XN_CALLBACK_TYPE FileOpen(void* pCookie);
	static XnStatus XN_CALLBACK_TYPE FileRead(void* pCookie, void* pBuffer, XnUInt32 nSize, XnUInt32* pnBytesRead);
	static void     XN_CALLBACK_TYPE FileClose(void* pCookie);

	static void     XN_CALLBACK_TYPE EOFReached(void* pCookie, const char *strUri);

	// Handle to the opened file.
	XN_FILE_HANDLE m_fileHandle;

	// Path to file.
	xnl::String m_filePath;
};

} // namespace oni_files_player

#endif // _ONIFILESPLAYER_DRIVER_H_

#endif // PLAYERDRIVER_H
