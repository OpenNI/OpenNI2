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
#ifndef TESTSTREAM_H
#define TESTSTREAM_H

#include <Driver\OniDriverAPI.h>
#include <XnLib.h>
#include <XnOSCpp.h>

#define DEFAULT_RESOLUTION_X 640
#define DEFAULT_RESOLUTION_Y 480
#define DEFAULT_FPS 30

class TestStream : public oni::driver::StreamBase
{
public:
	TestStream(OniSensorType sensorType);
	virtual ~TestStream();
	virtual OniStatus start();
	virtual void stop();
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
	virtual OniStatus invoke(int commandId, void* data, int dataSize);

	static OniPixelFormat getDefaultPixelFormat(OniSensorType sensorType);

protected:
	OniStatus setVideoMode(OniVideoMode*);
	OniStatus getVideoMode(OniVideoMode* pVideoMode);

	bool m_running;
	OniVideoMode m_videoMode;
	OniSensorType m_sensorType;
	int m_frameIndex;

	XN_THREAD_HANDLE m_threadHandle;

	xnl::CriticalSection m_cs;
	xnl::OSEvent m_osEvent;
};

#endif // TESTSTREAM_H
