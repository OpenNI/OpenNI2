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
#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"

#define OZ_RESOLUTION_X 320
#define OZ_RESOLUTION_Y 240

class OzStream : public oni::driver::StreamBase
{
public:
	~OzStream()
	{
		stop();
	}

	OniStatus start()
	{
		xnOSCreateThread(threadFunc, this, &m_threadHandle);

		return ONI_STATUS_OK;
	}

	void stop()
	{
		m_running = false;
	}

	virtual OniStatus SetVideoMode(OniVideoMode*) = 0;
	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode) = 0;

	OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
		{
			if (*pDataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", *pDataSize, (int)sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}
			return GetVideoMode((OniVideoMode*)data);
		}

		return ONI_STATUS_NOT_IMPLEMENTED;
	}

	OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
		{
			if (dataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", dataSize, (int)sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}
			return SetVideoMode((OniVideoMode*)data);
		}

		return ONI_STATUS_NOT_IMPLEMENTED;
	}

	virtual void Mainloop() = 0;
protected:
	// Thread
	static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam)
	{
		OzStream* pStream = (OzStream*)pThreadParam;
		pStream->m_running = true;
		pStream->Mainloop();

		XN_THREAD_PROC_RETURN(XN_STATUS_OK);
	}


	int singleRes(int x, int y) {return y*OZ_RESOLUTION_X+x;}

	bool m_running;

	XN_THREAD_HANDLE m_threadHandle;
};

class OzDepthStream : public OzStream
{
public:
	OniStatus SetVideoMode(OniVideoMode*) {return ONI_STATUS_NOT_IMPLEMENTED;}
	OniStatus GetVideoMode(OniVideoMode* pVideoMode)
	{
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		pVideoMode->fps = 30;
		pVideoMode->resolutionX = OZ_RESOLUTION_X;
		pVideoMode->resolutionY = OZ_RESOLUTION_Y;
		return ONI_STATUS_OK;
	}

private:

	void Mainloop()
	{
		int frameId = 1;
		int xdir = 1;
		int ydir = 1;
		struct {int x, y;} center = {0,0};
		while (m_running)
		{
//			printf("Tick");
			OniFrame* pFrame = getServices().acquireFrame();

			if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}

			// Fill frame
			xnOSMemSet(pFrame->data, 0, pFrame->dataSize);

			OniDepthPixel* pDepth = (OniDepthPixel*)pFrame->data;

			for (int y1 = XN_MAX(center.y-10, 0); y1 < XN_MIN(center.y+10, OZ_RESOLUTION_Y); ++y1)
				for (int x1 = XN_MAX(center.x-10, 0); x1 < XN_MIN(center.x+10, OZ_RESOLUTION_X); ++x1)
					if ((x1-center.x)*(x1-center.x)+(y1-center.y)*(y1-center.y) < 70)
						pDepth[singleRes(x1, y1)] = OniDepthPixel(1000+(x1-y1)*3);

//			pDepth[singleRes(center.x, center.y)] = 1000;

			center.x += xdir;
			center.y += ydir;

			if (center.x < abs(xdir) || center.x > OZ_RESOLUTION_X-1-abs(xdir)) xdir*=-1;
			if (center.y < abs(ydir) || center.y > OZ_RESOLUTION_Y-1-abs(ydir)) ydir*=-1;

			for (int i = 0; i < OZ_RESOLUTION_X; ++i) pDepth[i] = 2000;
			pDepth[0] = 2000;

			// Fill metadata
			pFrame->frameIndex = frameId;

			pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
			pFrame->videoMode.resolutionX = OZ_RESOLUTION_X;
			pFrame->videoMode.resolutionY = OZ_RESOLUTION_Y;
			pFrame->videoMode.fps = 30;

			pFrame->width = OZ_RESOLUTION_X;
			pFrame->height = OZ_RESOLUTION_Y;

			pFrame->cropOriginX = pFrame->cropOriginY = 0;
			pFrame->croppingEnabled = FALSE;

			pFrame->sensorType = ONI_SENSOR_DEPTH;
			pFrame->stride = OZ_RESOLUTION_X*sizeof(OniDepthPixel);
			pFrame->timestamp = frameId*33000;

			raiseNewFrame(pFrame);
			getServices().releaseFrame(pFrame);

			frameId++;

			xnOSSleep(33);
		}
	}
};

class OzImageStream : public OzStream
{
public:
	OniStatus SetVideoMode(OniVideoMode*) {return ONI_STATUS_NOT_IMPLEMENTED;}
	OniStatus GetVideoMode(OniVideoMode* pVideoMode)
	{
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		pVideoMode->fps = 30;
		pVideoMode->resolutionX = OZ_RESOLUTION_X;
		pVideoMode->resolutionY = OZ_RESOLUTION_Y;
		return ONI_STATUS_OK;
	}

private:

	void Mainloop()
	{
		int frameId = 1;
		int xdir = -3;
		int ydir = 1;
		struct {int x, y;} center = {160,120};
		while (m_running)
		{
			xnOSSleep(33);
			//			printf("Tick");
			OniFrame* pFrame = getServices().acquireFrame();

			if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}

			// Fill frame
			xnOSMemSet(pFrame->data, 0, pFrame->dataSize);

			OniRGB888Pixel* pImage = (OniRGB888Pixel*)pFrame->data;


			for (int y = XN_MAX(center.y-10, 0); y < XN_MIN(center.y+10, OZ_RESOLUTION_Y); ++y)
				for (int x = XN_MAX(center.x-10, 0); x < XN_MIN(center.x+10, OZ_RESOLUTION_X); ++x)
					if ((x-center.x)*(x-center.x)+(y-center.y)*(y-center.y) < 70)
				{
					pImage[singleRes(x, y)].r = (char)(255*(x/(double)OZ_RESOLUTION_X));
					pImage[singleRes(x, y)].g = (char)(255*(y/(double)OZ_RESOLUTION_Y));
					pImage[singleRes(x, y)].b = (char)(255*((OZ_RESOLUTION_X-x)/(double)OZ_RESOLUTION_X));
				}
//			pImage[singleRes(center.x, center.y)].r = 255;

			center.x += xdir;
			center.y += ydir;

			if (center.x < abs(xdir) || center.x > OZ_RESOLUTION_X-1-abs(xdir)) xdir*=-1;
			if (center.y < abs(ydir) || center.y > OZ_RESOLUTION_Y-1-abs(ydir)) ydir*=-1;



			pImage[0].b = (unsigned char)255;

			// 			for (int y = 0; y < OZ_RESOLUTION_Y; ++y)
			// 			{
			// 				pDepth[y*OZ_RESOLUTION_X+(OZ_RESOLUTION_Y-y)] = pDepth[y*OZ_RESOLUTION_X+(y)] = 500+y;
			// 			}

			// Fill metadata
			pFrame->frameIndex = frameId;

			pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
			pFrame->videoMode.resolutionX = OZ_RESOLUTION_X;
			pFrame->videoMode.resolutionY = OZ_RESOLUTION_Y;
			pFrame->videoMode.fps = 30;

			pFrame->width = OZ_RESOLUTION_X;
			pFrame->height = OZ_RESOLUTION_Y;

			pFrame->cropOriginX = pFrame->cropOriginY = 0;
			pFrame->croppingEnabled = FALSE;

			pFrame->sensorType = ONI_SENSOR_COLOR;
			pFrame->stride = OZ_RESOLUTION_X*3;
			pFrame->timestamp = frameId*33000;

			raiseNewFrame(pFrame);
			getServices().releaseFrame(pFrame);

			frameId++;
		}
	}
};

class OzDevice : public oni::driver::DeviceBase
{
public:
	OzDevice(OniDeviceInfo* pInfo, oni::driver::DriverServices& driverServices) : m_pInfo(pInfo), m_driverServices(driverServices)
	{
		m_numSensors = 2;

		m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
		m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
		m_sensors[0].numSupportedVideoModes = 1;
		m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		m_sensors[0].pSupportedVideoModes[0].fps = 30;
		m_sensors[0].pSupportedVideoModes[0].resolutionX = OZ_RESOLUTION_X;
		m_sensors[0].pSupportedVideoModes[0].resolutionY = OZ_RESOLUTION_Y;

		m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
		m_sensors[1].sensorType = ONI_SENSOR_COLOR;
		m_sensors[1].numSupportedVideoModes = 1;
		m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_sensors[1].pSupportedVideoModes[0].fps = 30;
		m_sensors[1].pSupportedVideoModes[0].resolutionX = OZ_RESOLUTION_X;
		m_sensors[1].pSupportedVideoModes[0].resolutionY = OZ_RESOLUTION_Y;

	}
	OniDeviceInfo* GetInfo()
	{
		return m_pInfo;
	}

	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
	{
		*numSensors = m_numSensors;
		*pSensors = m_sensors;

		return ONI_STATUS_OK;
	}

	oni::driver::StreamBase* createStream(OniSensorType sensorType)
	{
		if (sensorType == ONI_SENSOR_DEPTH)
		{
			OzDepthStream* pDepth = XN_NEW(OzDepthStream);
			return pDepth;
		}
		if (sensorType == ONI_SENSOR_COLOR)
		{
			OzImageStream* pImage = XN_NEW(OzImageStream);
			return pImage;
		}

		m_driverServices.errorLoggerAppend("OzDevice: Can't create a stream of type %d", sensorType);
		return NULL;
	}

	void destroyStream(oni::driver::StreamBase* pStream)
	{
		XN_DELETE(pStream);
	}

	OniStatus  getProperty(int propertyId, void* data, int* pDataSize)
	{
		OniStatus rc = ONI_STATUS_OK;

		switch (propertyId)
		{
		case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
			{
				if (*pDataSize == sizeof(OniVersion))
				{
					OniVersion* version = (OniVersion*)data;
					version->major = version->minor = version->maintenance = version->build = 2;
				}
				else
				{
					m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
					rc = ONI_STATUS_ERROR;
				}
			}
			break;
		default:
			m_driverServices.errorLoggerAppend("Unknown property: %d\n", propertyId);
			rc = ONI_STATUS_ERROR;
		}
		return rc;
	}
private:
	OzDevice(const OzDevice&);
	void operator=(const OzDevice&);

	OniDeviceInfo* m_pInfo;
	int m_numSensors;
	OniSensorInfo m_sensors[10];
	oni::driver::DriverServices& m_driverServices;
};


class OzDriver : public oni::driver::DriverBase
{
public:
	OzDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
	{}

	virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* /*mode*/)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
		{
			if (xnOSStrCmp(iter->Key()->uri, uri) == 0)
			{
				// Found
				if (iter->Value() != NULL)
				{
					// already using
					return iter->Value();
				}

				OzDevice* pDevice = XN_NEW(OzDevice, iter->Key(), getServices());
				iter->Value() = pDevice;
				return pDevice;
			}
		}

		getServices().errorLoggerAppend("Looking for '%s'", uri);
		return NULL;
	}

	virtual void deviceClose(oni::driver::DeviceBase* pDevice)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
		{
			if (iter->Value() == pDevice)
			{
				iter->Value() = NULL;
				XN_DELETE(pDevice);
				return;
			}
		}

		// not our device?!
		XN_ASSERT(FALSE);
	}

	virtual OniStatus tryDevice(const char* uri)
	{
		if (xnOSStrCmp(uri, "Dummy") != 0 &&
			xnOSStrCmp(uri, "Oz") != 0 &&
			xnOSStrCmp(uri, "PingPong") != 0)
		{
			return ONI_STATUS_ERROR;
		}


		OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
		xnOSStrCopy(pInfo->uri, uri, ONI_MAX_STR);
		xnOSStrCopy(pInfo->vendor, "Table Tennis", ONI_MAX_STR);
		m_devices[pInfo] = NULL;

		deviceConnected(pInfo);

		return ONI_STATUS_OK;
	}

	void shutdown() {}

protected:
	/*
	static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam)
	{
		OzModule* pModule = (OzModule*)pThreadParam;

		for (int i = 0; i < 10; ++i)
		{
			printf("%d\n", i);
			Sleep(1000);
		}

		OzDevice* pDevice4 = new OzDevice;
		xnOSStrCopy(pDevice4->GetInfo()->uri, "Oz4", 256);
		pModule->m_deviceConnected.Raise(pDevice4, pDevice4->GetInfo());

		for (int i = 0; i < 3; ++i)
		{
			printf("%d\n", i);
			Sleep(1000);
		}

		OzDevice* pDevice2 = new OzDevice;
		xnOSStrCopy(pDevice2->GetInfo()->uri, "Oz2", 256);
		pModule->m_deviceConnected.Raise(pDevice2, pDevice2->GetInfo());

		for (int i = 0; i < 5; ++i)
		{
			printf("%d\n", i);
			Sleep(1000);
		}

		pModule->m_deviceDisconnected.Raise(pDevice2);
		delete pDevice2;

		OzDevice* pDevice3 = new OzDevice;
		xnOSStrCopy(pDevice3->GetInfo()->uri, "Oz3", 256);
		pModule->m_deviceConnected.Raise(pDevice3, pDevice3->GetInfo());


		XN_THREAD_PROC_RETURN(XN_STATUS_OK);
	}
	*/

	XN_THREAD_HANDLE m_threadHandle;

	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> m_devices;
};

ONI_EXPORT_DRIVER(OzDriver);
