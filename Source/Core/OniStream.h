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
#ifndef _ONI_IMPL_STREAM_H_
#define _ONI_IMPL_STREAM_H_

#include "OniDriverHandler.h"
#include "OniCommon.h"
#include "OniFrameHolder.h"
#include "XnEvent.h"
#include "XnErrorLogger.h"
#include "XnHash.h"
#include "XnLockable.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

class Device;
class FrameHolder;
class Recorder;

class VideoStream
{
public:
	VideoStream(void* streamHandle, const OniSensorInfo* pSensorInfo, Device& device, const DriverHandler& driverHandler, xnl::ErrorLogger& errorLogger);
	virtual ~VideoStream();

	OniStatus start();
	void stop();
	OniBool isStarted();

	OniStatus readFrame(OniFrame** pFrame);
	
	OniFrame* peekFrame();
	void lockFrame();
	void unlockFrame();

	OniStatus setProperty(int propertyId, const void* data, int dataSize);
	OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	OniBool isPropertySupported(int propertyId);
	OniStatus invoke(int commandId, const void* data, int dataSize);
	OniBool isCommandSupported(int commandId);
	void notifyAllProperties();

	void frameRelease(OniFrame* pFrame);
	void frameAddRef(OniFrame* pFrame);

	void initializeFramePool(int dataSize, int poolSize);
	void initializeFramePool(OniGeneralBuffer* buffers, int numBuffers, int dataSize);

	OniStatus registerNewFrameCallback(OniGeneralCallback handler, void* pCookie, XnCallbackHandle* pHandle);
	void unregisterNewFrameCallback(XnCallbackHandle handle);

	const OniSensorInfo* getSensorInfo() const;

	Device& getDevice();

	void* getHandle() const;

	void setFrameHolder(FrameHolder* pFrameHolder);
	FrameHolder* getFrameHolder();

	void raiseNewFrameEvent();
	XnStatus waitForNewFrameEvent();

	void setContextNewFrameEvent(xnl::OSEvent* pContextNewFrameEvent);

	static VideoStream* getFrameStream(OniFrame* pFrame);
    OniStatus addRecorder(Recorder& aRecorder);
    OniStatus removeRecorder(Recorder& aRecorder);

	OniStatus convertDepthToWorldCoordinates(float depthX, float depthY, float depthZ, float* pWorldX, float* pWorldY, float* pWorldZ);
	OniStatus convertWorldToDepthCoordinates(float worldX, float worldY, float worldZ, float* pDepthX, float* pDepthY, float* pDepthZ);
	OniStatus convertDepthToColorCoordinates(VideoStream* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY);

protected:
	XN_EVENT_HANDLE m_newFrameInternalEvent;
	XN_EVENT_HANDLE m_newFrameInternalEventForFrameHolder;

	xnl::ErrorLogger& m_errorLogger;

private:
	FrameHolder* m_pFrameHolder;

	VideoStream(const VideoStream& other);
	VideoStream& operator=(const VideoStream& other);

	xnl::EventNoArgs m_newFrameEvent;
	XN_THREAD_HANDLE m_newFrameThread;

	OniSensorInfo* m_pSensorInfo;

	static XN_THREAD_PROC newFrameThread(XN_THREAD_PARAM pThreadParam);
	void newFrameThreadMainloop();
	bool m_running;

	static void ONI_CALLBACK_TYPE stream_NewFrame(void* streamHandle, OniDriverFrame* pFrame, void* pCookie);
	static void ONI_CALLBACK_TYPE stream_PropertyChanged(void* streamHandle, int propertyId, const void* data, int dataSize, void* pCookie);

	void refreshWorldConversionCache();

	Device& m_device;
	const DriverHandler& m_driverHandler;
	void* m_streamHandle;

	xnl::OSEvent* m_pContextNewFrameEvent;

	xnl::CriticalSection m_cs;

	OniBool m_started;

    // XnLib does not provide a set container. I decided to use this odd
    // Recorder* -> Recorder* map to mimic a set.
    typedef xnl::Lockable<xnl::Hash<Recorder*, Recorder*> > Recorders;
    Recorders m_recorders;

	struct WorldConversionCache
	{
		float xzFactor;
		float yzFactor;
		float coeffX;
		float coeffY;
		int resolutionX;
		int resolutionY;
		int halfResX;
		int halfResY;
	} m_worldConvertCache;
};

ONI_NAMESPACE_IMPLEMENTATION_END

#endif // _ONI_IMPL_STREAM_H_
