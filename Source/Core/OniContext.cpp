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
#include "OniContext.h"
#include "OniFileRecorder.h"
#include "OniStreamFrameHolder.h"
#include <XnLog.h>
#include <XnOSCpp.h>

static const char* ONI_CONFIGURATION_FILE = "OpenNI.ini";
static const char* ONI_DEFAULT_DRIVERS_REPOSITORY = "OpenNI2" XN_FILE_DIR_SEP "Drivers";

#define XN_MASK_ONI_CONTEXT "OniContext"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

OniBool Context::s_valid = FALSE;

Context::Context() : m_errorLogger(xnl::ErrorLogger::GetInstance()), m_autoRecording(false), m_autoRecordingStarted(false), m_initializationCounter(0), m_lastFPSPrint(0)
{
	m_overrideDevice[0] = '\0';
	m_driverRepo[0] = '\0';
}

Context::~Context()
{
	s_valid = FALSE;
}

OniStatus Context::initialize()
{
	m_initializationCounter++;
	if (m_initializationCounter > 1)
	{
		xnLogVerbose(XN_MASK_ONI_CONTEXT, "Initialize: Already initialized");
		return ONI_STATUS_OK;
	}

	XnStatus rc = resolvePathToOpenNI();
	if (rc != XN_STATUS_OK)
	{
		return OniStatusFromXnStatus(rc);
	}
	
	rc = configure();
	if (rc != XN_STATUS_OK)
	{
		return OniStatusFromXnStatus(rc);
	}

	s_valid = TRUE;

	rc = loadLibraries();
	if (rc == XN_STATUS_OK)
	{
		m_errorLogger.Clear();
	}

	return OniStatusFromXnStatus(rc);
}

// Dummy function used only for taking its address for the sake of xnOSGetModulePathForProcAddress.
static void dummyFunctionToTakeAddress() {}

XnStatus Context::resolvePathToOpenNI()
{
	XnStatus rc = XN_STATUS_OK;

	XnChar strModulePath[XN_FILE_MAX_PATH];
	rc = xnOSGetModulePathForProcAddress(reinterpret_cast<void*>(&dummyFunctionToTakeAddress), strModulePath);
	if (rc != XN_STATUS_OK)
	{
		m_errorLogger.Append("Couldn't get the OpenNI shared library module's path: %s", xnGetStatusString(rc));
		return rc;
	}

	rc = xnOSGetDirName(strModulePath, m_pathToOpenNI, sizeof(m_pathToOpenNI));
	if (rc != XN_STATUS_OK)
	{
		// Very unlikely to happen, but just in case.
		m_errorLogger.Append("Couldn't get the OpenNI shared library module's directory: %s", xnGetStatusString(rc));
		return rc;
	}

	return XN_STATUS_OK;
}

XnStatus Context::resolveConfigurationFile(char* strOniConfigurationFile)
{
	XnStatus rc = XN_STATUS_OK;
	XnBool bExists;

#if XN_PLATFORM == XN_PLATFORM_ANDROID_ARM
	// support for applications
	xnOSGetApplicationFilesDir(strOniConfigurationFile, XN_FILE_MAX_PATH);
	rc = xnOSAppendFilePath(strOniConfigurationFile, ONI_CONFIGURATION_FILE, XN_FILE_MAX_PATH);
	XN_IS_STATUS_OK(rc);

	xnOSDoesFileExist(strOniConfigurationFile, &bExists);

	if (!bExists)
	{
		// support for native use - search in current dir
		rc = xnOSStrCopy(strOniConfigurationFile, ONI_CONFIGURATION_FILE, XN_FILE_MAX_PATH);
		XN_IS_STATUS_OK(rc);
	}

#else
	xnOSStrCopy(strOniConfigurationFile, m_pathToOpenNI, XN_FILE_MAX_PATH);
	rc = xnOSAppendFilePath(strOniConfigurationFile, ONI_CONFIGURATION_FILE, XN_FILE_MAX_PATH);
	XN_IS_STATUS_OK(rc);
#endif

	xnOSDoesFileExist(strOniConfigurationFile, &bExists);

	if (!bExists)
	{
		strOniConfigurationFile[0] = '\0';
	}

	return XN_STATUS_OK;
}

XnStatus Context::configure()
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	XnChar strOniConfigurationFile[XN_FILE_MAX_PATH];
	XnStatus rc = resolveConfigurationFile(strOniConfigurationFile);
	if (rc != XN_STATUS_OK)
	{
		return OniStatusFromXnStatus(rc);
	}

#ifdef ONI_PLATFORM_ANDROID_OS
	xnLogSetMaskMinSeverity(XN_LOG_MASK_ALL, (XnLogSeverity)0);
	xnLogSetAndroidOutput(TRUE);
#endif
	
	// First, we should process the log related configuration as early as possible.
	rc = xnLogInitFromINIFile(strOniConfigurationFile, "Log");
	if (XN_STATUS_OK != rc)
	{
		return ONI_STATUS_ERROR;
	}

	// Now that log was configured, we can start issues some log entries
	xnLogVerbose(XN_MASK_ONI_CONTEXT, "OpenNI %s", ONI_VERSION_STRING);
	if (strOniConfigurationFile[0] != '\0')
	{
		xnLogVerbose(XN_MASK_ONI_CONTEXT, "Configuration file found at '%s'", strOniConfigurationFile);
	}

	// Then, process the other device configurations.
	rc = xnOSReadStringFromINI(strOniConfigurationFile, "Device", "Override", m_overrideDevice, sizeof(m_overrideDevice));
	if (rc == XN_STATUS_OK)
	{
		xnLogWarning(XN_MASK_ONI_CONTEXT, "Device will be overridden with '%s'", m_overrideDevice);
	}

	XnChar autoRecordingName[XN_FILE_MAX_PATH];
	rc = xnOSReadStringFromINI(strOniConfigurationFile, "Device", "RecordTo", autoRecordingName, XN_FILE_MAX_PATH);
	if (rc == XN_STATUS_OK)
	{
		OniStatus oniRc = recorderOpen(autoRecordingName, &m_autoRecorder);
		if (oniRc == ONI_STATUS_OK)
		{
			m_autoRecording = true;
		}
	}

	XnChar strRepo[XN_FILE_MAX_PATH];
	strRepo[0] = '\0';

#if XN_PLATFORM != XN_PLATFORM_ANDROID_ARM
	xnOSStrCopy(strRepo, ONI_DEFAULT_DRIVERS_REPOSITORY, sizeof(strRepo));
#endif

	// check if repo was overridden
	XnChar strTemp[XN_INI_MAX_LEN];
	rc = xnOSReadStringFromINI(strOniConfigurationFile, "Drivers", "Repository", strTemp, sizeof(strTemp));
	if (rc == XN_STATUS_OK)
	{
		xnOSStrCopy(strRepo, strTemp, sizeof(strRepo));
	}

	rc = xnOSStrCopy(m_driverRepo, m_pathToOpenNI, sizeof(m_driverRepo));
	XN_IS_STATUS_OK(nRetVal);
	rc = xnOSAppendFilePath(m_driverRepo, strRepo, sizeof(m_driverRepo));
	if (rc != XN_STATUS_OK)
	{
		xnLogError(XN_MASK_ONI_CONTEXT, "Driver path is too long!");
		m_errorLogger.Append("The driver path gets too long");
		return rc;
	}

#if XN_PLATFORM == XN_PLATFORM_ANDROID_ARM
	m_driversList.AddLast("libOniFile.so");
	m_driversList.AddLast("libPS1080.so");
	m_driversList.AddLast("libPSLink.so");
	m_driversList.AddLast("libSD.so");
#endif

	// check if driver list is overridden
	XnChar strDriversList[2048];
	rc = xnOSReadStringFromINI(strOniConfigurationFile, "Drivers", "List", strDriversList, sizeof(strDriversList));
	if (rc == XN_STATUS_OK)
	{
		m_driversList.Clear();

		// parse list
		xnl::FileName driver;
		int driverLen = 0;

		for (XnChar* c = strDriversList; ; ++c)
		{
			if (*c == ',' || *c == '\0')
			{
				driver[driverLen++] = '\0';
				rc = m_driversList.AddLast(driver);
				XN_IS_STATUS_OK(rc);
				driverLen = 0;

				if (*c == '\0')
					break;
			}
			else
			{
				driver[driverLen++] = *c;
			}
		}
	}

	return (XN_STATUS_OK);
}

XnStatus Context::loadLibraries()
{
	XnStatus nRetVal;

	xnLogVerbose(XN_MASK_ONI_CONTEXT, "Using '%s' as driver path", m_driverRepo);

	if (m_driversList.IsEmpty())
	{
		// search repo for drivers
		XnInt32 nFileCount = 0;
		XnChar cpSearchString[XN_FILE_MAX_PATH] = "";

		xnLogVerbose(XN_MASK_ONI_CONTEXT, "Looking for drivers at '%s'", m_driverRepo);

		// Build the search pattern string
		XN_VALIDATE_STR_APPEND(cpSearchString, m_driverRepo, XN_FILE_MAX_PATH, nRetVal);
		XN_VALIDATE_STR_APPEND(cpSearchString, XN_FILE_DIR_SEP, XN_FILE_MAX_PATH, nRetVal);
		XN_VALIDATE_STR_APPEND(cpSearchString, XN_SHARED_LIBRARY_PREFIX, XN_FILE_MAX_PATH, nRetVal);
		XN_VALIDATE_STR_APPEND(cpSearchString, XN_FILE_ALL_WILDCARD, XN_FILE_MAX_PATH, nRetVal);
		XN_VALIDATE_STR_APPEND(cpSearchString, XN_SHARED_LIBRARY_POSTFIX, XN_FILE_MAX_PATH, nRetVal);

		nRetVal = xnOSCountFiles(cpSearchString, &nFileCount);
		if (nRetVal != XN_STATUS_OK || nFileCount == 0)
		{
			xnLogError(XN_MASK_ONI_CONTEXT, "Found no drivers matching '%s'", cpSearchString);
			m_errorLogger.Append("Found no files matching '%s'", cpSearchString);
			return XN_STATUS_NO_MODULES_FOUND;
		}

		nRetVal = m_driversList.SetSize(nFileCount);
		XN_IS_STATUS_OK(nRetVal);

		typedef XnChar MyFileName[XN_FILE_MAX_PATH];
		MyFileName* acsFileList = XN_NEW_ARR(MyFileName, nFileCount);

		nRetVal = xnOSGetFileList(cpSearchString, NULL, acsFileList, nFileCount, &nFileCount);
		XN_IS_STATUS_OK(nRetVal);

		for (int i = 0; i < nFileCount; ++i)
		{
			m_driversList[i] = acsFileList[i];
		}

		XN_DELETE_ARR(acsFileList);
	}

	// Save directory
	XnChar workingDir[XN_FILE_MAX_PATH];
	xnOSGetCurrentDir(workingDir, XN_FILE_MAX_PATH);
	// Change directory
	xnOSSetCurrentDir(m_driverRepo);

	for (XnUInt32 i = 0; i < m_driversList.GetSize(); ++i)
	{
		xnLogVerbose(XN_MASK_ONI_CONTEXT, "Loading device driver '%s'...", m_driversList[i].getData());
		DeviceDriver* pDeviceDriver = XN_NEW(DeviceDriver, m_driversList[i].getData(), m_frameManager, m_errorLogger);
		if (pDeviceDriver == NULL || !pDeviceDriver->isValid())
		{
			xnLogWarning(XN_MASK_ONI_CONTEXT, "Couldn't use file '%s' as a device driver", m_driversList[i].getData());
			m_errorLogger.Append("Couldn't understand file '%s' as a device driver", m_driversList[i].getData());
			XN_DELETE(pDeviceDriver);
			continue;
		}
		OniCallbackHandle dummy;
		pDeviceDriver->registerDeviceConnectedCallback(deviceDriver_DeviceConnected, this, dummy);
		pDeviceDriver->registerDeviceDisconnectedCallback(deviceDriver_DeviceDisconnected, this, dummy);
		pDeviceDriver->registerDeviceStateChangedCallback(deviceDriver_DeviceStateChanged, this, dummy);
		if (!pDeviceDriver->initialize())
		{
			xnLogVerbose(XN_MASK_ONI_CONTEXT, "Couldn't use file '%s' as a device driver", m_driversList[i].getData());
			m_errorLogger.Append("Couldn't initialize device driver from file '%s'", m_driversList[i].getData());
			XN_DELETE(pDeviceDriver);
			continue;
		}
		m_cs.Lock();
		m_deviceDrivers.AddLast(pDeviceDriver);
		m_cs.Unlock();
	}

	// Return to directory
	xnOSSetCurrentDir(workingDir);

	if (m_deviceDrivers.Size() == 0)
	{
		xnLogError(XN_MASK_ONI_CONTEXT, "Found no valid drivers");
		m_errorLogger.Append("Found no valid drivers");
		return XN_STATUS_NO_MODULES_FOUND;
	}

	return XN_STATUS_OK;
}

void Context::shutdown()
{
	--m_initializationCounter;
	if (m_initializationCounter > 0)
	{
		xnLogVerbose(XN_MASK_ONI_CONTEXT, "Shutdown: still need %d more shutdown calls (to match initializations)", m_initializationCounter);
		return;
	}
	if (!s_valid)
	{
		return;
	}

	s_valid = FALSE;

	m_cs.Lock();

    // Close all recorders.
    while (m_recorders.Begin() != m_recorders.End())
    {
        Recorder* pRecorder = *m_recorders.Begin();
        recorderClose(pRecorder);
    }

	// Destroy all streams
	while (m_streams.Begin() != m_streams.End())
	{
		VideoStream* pStream = *m_streams.Begin();
		streamDestroy(pStream);
	}

	// Close all devices
	while (m_devices.Begin() != m_devices.End())
	{
		Device* pDevice = *m_devices.Begin();
		m_devices.Remove(pDevice);
		pDevice->close();
		XN_DELETE(pDevice);
	}

	for (xnl::List<DeviceDriver*>::Iterator iter = m_deviceDrivers.Begin(); iter != m_deviceDrivers.End(); ++iter)
	{
		DeviceDriver* pDriver = *iter;
		XN_DELETE(pDriver);
	}
	m_deviceDrivers.Clear();

	m_cs.Unlock();

	m_overrideDevice[0] = '\0';
	m_driverRepo[0] = '\0';
	m_pathToOpenNI[0] = '\0';
	m_driversList.Clear();

	xnLogVerbose(XN_MASK_ONI_CONTEXT, "Shutdown: successful.");
	xnLogClose();
}

OniStatus Context::registerDeviceConnectedCallback(OniDeviceInfoCallback handler, void* pCookie, OniCallbackHandle& handle)
{
	return OniStatusFromXnStatus(m_deviceConnectedEvent.Register(handler, pCookie, (XnCallbackHandle&)handle));
}
void Context::unregisterDeviceConnectedCallback(OniCallbackHandle handle)
{
	m_deviceConnectedEvent.Unregister((XnCallbackHandle)handle);
}
OniStatus Context::registerDeviceDisconnectedCallback(OniDeviceInfoCallback handler, void* pCookie, OniCallbackHandle& handle)
{
	return OniStatusFromXnStatus(m_deviceDisconnectedEvent.Register(handler, pCookie, (XnCallbackHandle&)handle));
}
void Context::unregisterDeviceDisconnectedCallback(OniCallbackHandle handle)
{
	m_deviceDisconnectedEvent.Unregister((XnCallbackHandle)handle);
}
OniStatus Context::registerDeviceStateChangedCallback(OniDeviceStateCallback handler, void* pCookie, OniCallbackHandle& handle)
{
	return OniStatusFromXnStatus(m_deviceStateChangedEvent.Register(handler, pCookie, (XnCallbackHandle&)handle));
}
void Context::unregisterDeviceStateChangedCallback(OniCallbackHandle handle)
{
	m_deviceStateChangedEvent.Unregister((XnCallbackHandle)handle);
}

OniStatus Context::getDeviceList(OniDeviceInfo** pDevices, int* pDeviceCount)
{
	m_cs.Lock();

	*pDeviceCount = m_devices.Size();
	*pDevices = XN_NEW_ARR(OniDeviceInfo, *pDeviceCount);

	int idx = 0;
	for (xnl::List<Device*>::ConstIterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter, ++idx)
	{
		xnOSMemCopy((*pDevices)+idx, (*iter)->getInfo(), sizeof(OniDeviceInfo));
	}

	m_cs.Unlock();
	return ONI_STATUS_OK;

}
OniStatus Context::releaseDeviceList(OniDeviceInfo* pDevices)
{
	XN_DELETE_ARR(pDevices);
	return ONI_STATUS_OK;
}

OniStatus Context::deviceOpen(const char* uri, const char* mode, OniDeviceHandle* pDevice)
{
	oni::implementation::Device* pMyDevice = NULL;

	const char* deviceURI = uri;
	if (xnOSStrLen(m_overrideDevice) > 0)
		deviceURI = m_overrideDevice;

	xnLogVerbose(XN_MASK_ONI_CONTEXT, "Trying to open device by URI '%s'", deviceURI == NULL ? "(NULL)" : deviceURI);

	m_cs.Lock();

	if (deviceURI == NULL)
	{
		// Default
		if (m_devices.Size() == 0)
		{
			m_errorLogger.Append("DeviceOpen using default: no devices found");
			xnLogError(XN_MASK_ONI_CONTEXT, "Can't open default device - none found");
			m_cs.Unlock();
			return ONI_STATUS_ERROR;
		}

		pMyDevice = *m_devices.Begin();
	}
	else
	{
		for (xnl::List<Device*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
		{
			if (xnOSStrCmp((*iter)->getInfo()->uri, deviceURI) == 0)
			{
				pMyDevice = *iter;
			}
		}
	}

	if (pMyDevice == NULL)
	{
		for (xnl::List<DeviceDriver*>::Iterator iter = m_deviceDrivers.Begin(); iter != m_deviceDrivers.End() && pMyDevice == NULL; ++iter)
		{
			if ((*iter)->tryDevice(deviceURI))
			{
				for (xnl::List<Device*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
				{
					if (xnOSStrCmp((*iter)->getInfo()->uri, deviceURI) == 0)
					{
						pMyDevice = *iter;
						break;
					}
				}
			}
			else
			{
//					printf("Not yet\n");
			}
		}
	}

	m_cs.Unlock();

	if (pMyDevice == NULL)
	{
		xnLogError(XN_MASK_ONI_CONTEXT, "Couldn't open device '%s'", uri);
		m_errorLogger.Append("DeviceOpen: Couldn't open device '%s'", uri);
		return ONI_STATUS_NO_DEVICE;
	}

	_OniDevice* pDeviceHandle = XN_NEW(_OniDevice);
	if (pDeviceHandle == NULL)
	{
		m_errorLogger.Append("Couldn't allocate memory for DeviceHandle");
		return ONI_STATUS_ERROR;
	}
	*pDevice = pDeviceHandle;
	pDeviceHandle->pDevice = pMyDevice;

	return pMyDevice->open(mode);
}

OniStatus Context::deviceClose(OniDeviceHandle device)
{
	if (device == NULL)
	{
		return ONI_STATUS_ERROR;
	}
	Device* pDevice = device->pDevice;

	pDevice->close();

	XN_DELETE(device);
	return ONI_STATUS_OK;
}

const OniSensorInfo* Context::getSensorInfo(OniDeviceHandle device, OniSensorType sensorType)
{
	Device* pDevice = device->pDevice;

	OniSensorInfo *pSensorInfos;
	int sensors = ONI_MAX_SENSORS;
	pDevice->getSensorInfoList(&pSensorInfos, sensors);

	for (int i = 0; i < sensors; ++i)
	{
		if (pSensorInfos[i].sensorType == sensorType)
		{
			return (&pSensorInfos[i]);
		}
	}

	return NULL;
}

const OniSensorInfo* Context::getSensorInfo(OniStreamHandle stream)
{
	if (stream == NULL || stream->pStream == NULL)
	{
		m_errorLogger.Append("Invalid stream");
		return NULL;
	}

	return stream->pStream->getSensorInfo();
}

OniStatus Context::createStream(OniDeviceHandle device, OniSensorType sensorType, OniStreamHandle* pStream)
{

	// Create the stream.
	Device* pDevice = device->pDevice;
	VideoStream* pMyStream = pDevice->createStream(sensorType);
	if (pMyStream == NULL)
	{
		m_errorLogger.Append("Context: Couldn't create stream from device:%08x, source: %d", device, sensorType);
		return ONI_STATUS_ERROR;
	}

	pMyStream->setNewFrameCallback(newFrameCallback, this);

	// Create stream frame holder and connect it to the stream.
	StreamFrameHolder* pFrameHolder = XN_NEW(StreamFrameHolder, m_frameManager, pMyStream);
	if (pFrameHolder == NULL)
	{
		m_errorLogger.Append("Context: Couldn't create stream frame holder from device:%08x, source: %d", device, sensorType);
		XN_DELETE(pMyStream);
		return ONI_STATUS_ERROR;
	}
	pMyStream->setFrameHolder(pFrameHolder);

	// Create handle object.
	_OniStream* pStreamHandle = XN_NEW(_OniStream);
	if (pStreamHandle == NULL)
	{
		m_errorLogger.Append("Couldn't allocate memory for StreamHandle");
		XN_DELETE(pFrameHolder);
		pFrameHolder = NULL;
		XN_DELETE(pMyStream);
		pMyStream = NULL;
		return ONI_STATUS_ERROR;
	}
	*pStream = pStreamHandle;
	pStreamHandle->pStream = pMyStream;

	m_cs.Lock();
	m_streams.AddLast(pMyStream);
	m_cs.Unlock();

	if (m_autoRecording)
	{
		m_streamsToAutoRecord.Lock();
		m_streamsToAutoRecord.AddLast(*pStream);
		m_streamsToAutoRecord.Unlock();
	}

	return ONI_STATUS_OK;
}

OniStatus Context::streamDestroy(OniStreamHandle stream)
{
	OniStatus rc = ONI_STATUS_OK;

	if (stream == NULL)
	{
		return ONI_STATUS_OK;
	}

	if (m_autoRecording)
	{
		m_streamsToAutoRecord.Lock();
		m_streamsToAutoRecord.Remove(stream);
		m_streamsToAutoRecord.Unlock();
	}

	VideoStream* pStream = stream->pStream;
	rc = streamDestroy(pStream);
	if (rc == ONI_STATUS_OK)
	{
		XN_DELETE(stream);
	}
	return rc;
}

OniStatus Context::streamDestroy(VideoStream* pStream)
{
	OniStatus rc = ONI_STATUS_OK;

	if (pStream == NULL)
	{
		return ONI_STATUS_OK;
	}

	// Make sure the stream is stopped.
	pStream->stop();

	m_cs.Lock();

	// Remove the stream from the streams list.
	m_streams.Remove(pStream);

	m_cs.Unlock();

	// Lock stream's frame holder.
	FrameHolder* pFrameHolder = pStream->getFrameHolder();
	pFrameHolder->setEnabled(FALSE);
	pFrameHolder->lock();
	pFrameHolder->clear();

	// Get the frame holder's streams.
	int numStreams = pFrameHolder->getNumStreams();
	xnl::Array<VideoStream*> pStreamList(numStreams);
	pStreamList.SetSize(numStreams);
	pFrameHolder->getStreams(pStreamList.GetData(), &numStreams);

	// Change holder to all the streams (allocate new StreamFrameHolder).
	for (int i = 0; i < numStreams; ++i)
	{
		if (pStreamList[i] != pStream)
		{
			// Allocate new frame holder.
			StreamFrameHolder* pStreamFrameHolder = XN_NEW(StreamFrameHolder, m_frameManager, pStreamList[i]);
			if (pStreamFrameHolder == NULL)
			{
				rc = ONI_STATUS_ERROR;
				continue;
			}

			// Replace the holder in the stream.
			pStreamList[i]->setFrameHolder(pStreamFrameHolder);
		}
	}

	pFrameHolder->unlock();

	// Delete the stream object and handle.
	XN_DELETE(pStream);

	// Delete the frame holder.
	XN_DELETE(pFrameHolder);

	return rc;
}

OniStatus Context::readFrame(OniStreamHandle stream, OniFrame** pFrame)
{
	// Make sure frame is available.
	int streamIndex;
	OniStatus rc = waitForStreams(&stream, 1, &streamIndex, ONI_TIMEOUT_FOREVER);
	if (rc != ONI_STATUS_OK)
	{
		return rc;
	}

	// Get the actual frame.
	_OniStream* pStream = (_OniStream*)stream;
	return pStream->pStream->readFrame(pFrame);
}

void Context::frameRelease(OniFrame* pFrame)
{
	m_frameManager.release(pFrame);
}

void Context::frameAddRef(OniFrame* pFrame)
{
	m_frameManager.addRef(pFrame);
}

OniStatus Context::waitForStreams(OniStreamHandle* pStreams, int streamCount, int* pStreamIndex, int timeout)
{
	if (m_autoRecording && !m_autoRecordingStarted)
	{
		m_streamsToAutoRecord.Lock();
		for (xnl::List<OniStreamHandle>::ConstIterator iter = m_streamsToAutoRecord.Begin(); iter != m_streamsToAutoRecord.End(); ++iter)
		{
			m_autoRecorder->pRecorder->attachStream(*(*iter)->pStream, true);
		}
		m_streamsToAutoRecord.Unlock();

		m_autoRecorder->pRecorder->start();
		m_autoRecordingStarted = true;
	}

	static const int MAX_WAITED_STREAMS = 50;
	Device* deviceList[MAX_WAITED_STREAMS];
	VideoStream* streamsList[MAX_WAITED_STREAMS];

	unsigned long long oldestTimestamp = XN_MAX_UINT64;
	int oldestIndex = -1;

	if (streamCount > MAX_WAITED_STREAMS)
	{
		m_errorLogger.Append("Cannot wait on more than %d streams", MAX_WAITED_STREAMS);
		return ONI_STATUS_NOT_SUPPORTED;
	}

	int numDevices = 0;

	for (int i = 0; i < streamCount; ++i)
	{
		if (pStreams[i] == NULL)
		{
			continue;
		}

		streamsList[i] =  ((_OniStream*)pStreams[i])->pStream;

		Device* pDevice = &streamsList[i]->getDevice();

		// Check if device already exists.
		bool found = false;
		for (int j = 0; j < numDevices; ++j)
		{
			if (deviceList[j] == pDevice)
			{
				found = true;
				break;
			}
		}

		// Add new device to list.
		if (!found)
		{
			deviceList[numDevices] = pDevice;
			++numDevices;
		}
	}

	XN_EVENT_HANDLE hEvent = getThreadEvent();

	XnUInt64 passedTime;
	XnOSTimer workTimer;
	XnUInt32 timeToWait = timeout;
	xnOSStartTimer(&workTimer);

	do
	{
		for (int i = 0; i < streamCount; ++i)
		{
			if (pStreams[i] == NULL)
				continue;

			VideoStream* pStream = ((_OniStream*)pStreams[i])->pStream;
			pStream->lockFrame();
			OniFrame* pFrame = pStream->peekFrame();
			if (pFrame != NULL && pFrame->timestamp < oldestTimestamp)
			{
				oldestTimestamp = pFrame->timestamp;
				oldestIndex = i;
			}
			pStream->unlockFrame();
		}

		if (oldestIndex != -1)
		{
			*pStreamIndex = oldestIndex;
			break;
		}

		// 'Poke' the driver to attempt to receive more frames.
		for (int j = 0; j < numDevices; ++j)
		{
			deviceList[j]->tryManualTrigger();
		}

		if(timeout != ONI_TIMEOUT_FOREVER)
		{
			xnOSQueryTimer(workTimer, &passedTime);
			if((int)passedTime < timeout)
				timeToWait = timeout - (int)passedTime;
			else
				timeToWait = 0;
		}
	} while (XN_STATUS_OK == xnOSWaitEvent(hEvent, timeToWait));
	
	xnOSStopTimer(&workTimer);

	if (oldestIndex != -1)
	{
		return ONI_STATUS_OK;
	}

	m_errorLogger.Append("waitForStreams: timeout reached");
	return ONI_STATUS_TIME_OUT;
}

OniStatus Context::enableFrameSync(OniStreamHandle* pStreams, int numStreams, OniFrameSyncHandle* pFrameSyncHandle)
{
	// Verify parameters.
	if (pFrameSyncHandle == NULL)
	{
		return ONI_STATUS_BAD_PARAMETER;
	}

	xnl::Array<VideoStream*> pStreamList(numStreams);
	DeviceDriver* pDeviceDriver = NULL;

	// Set the size of the arrays, so they can be filled.
	pStreamList.SetSize(numStreams);

	// Check validity and fill the arrays.
	for (int i = 0; i < numStreams; ++i)
	{
		// Make sure stream's device is valid and is same as device of other streams. 
		if (pDeviceDriver == NULL)
		{
			pDeviceDriver = pStreams[i]->pStream->getDevice().getDeviceDriver();
		}
		else
		{
			// Check whether device is different than previous devices.
			if (pDeviceDriver != pStreams[i]->pStream->getDevice().getDeviceDriver())
			{
				// Frame sync groups using streams from different drivers is not supported.
				m_errorLogger.Append("EnableFrameSync: can't sync streams from different drivers");
				return ONI_STATUS_NOT_SUPPORTED;
			}
		}

		// Make sure stream does not already belong to stream group.
		/*if (pStreams[i]->pStream->GetFrameSyncGroup() != NULL)
		{
			// TODO: add ONI_STATUS_ALREADY_EXISTS?
			return ONI_STATUS_ERROR;
		}*/

		// Store the stream pointer.
		pStreamList[i] = pStreams[i]->pStream;
	}

	return enableFrameSyncEx(pStreamList.GetData(), numStreams, pDeviceDriver, pFrameSyncHandle);
}

OniStatus Context::enableFrameSyncEx(VideoStream** pStreams, int numStreams, DeviceDriver* pDeviceDriver, OniFrameSyncHandle* pFrameSyncHandle)
{
	// Make sure the device driver is valid.
	if (pDeviceDriver == NULL)
	{
		return ONI_STATUS_ERROR;
	}

	// Create the new frame sync group (it will link all the streams).
	SyncedStreamsFrameHolder* pSyncedStreamsFrameHolder = XN_NEW(SyncedStreamsFrameHolder, 
																	m_frameManager, pStreams, numStreams);
	XN_VALIDATE_PTR(pSyncedStreamsFrameHolder, ONI_STATUS_ERROR);

	// Configure frame-sync group in driver.
	void* driverHandle = pDeviceDriver->enableFrameSync(pStreams, numStreams);
	XN_VALIDATE_PTR(driverHandle, ONI_STATUS_ERROR);

	// Return the frame sync handle.
	*pFrameSyncHandle = XN_NEW(_OniFrameSync);
	if (*pFrameSyncHandle == NULL)
	{
		m_errorLogger.Append("Couldn't allocate memory for FrameSyncHandle");
		return ONI_STATUS_ERROR;
	}
	(*pFrameSyncHandle)->pSyncedStreamsFrameHolder = pSyncedStreamsFrameHolder;
	(*pFrameSyncHandle)->pDeviceDriver = pDeviceDriver;
	(*pFrameSyncHandle)->pFrameSyncHandle = driverHandle;

	// Update the frame holders of all the streams.
	pSyncedStreamsFrameHolder->lock();
	for (int j = 0; j < numStreams; ++j)
	{
		FrameHolder* pOldFrameHolder = pStreams[j]->getFrameHolder();
		pOldFrameHolder->lock();
		pOldFrameHolder->setStreamEnabled(pStreams[j], FALSE);
		pStreams[j]->setFrameHolder(pSyncedStreamsFrameHolder);
		pOldFrameHolder->unlock();
		XN_DELETE(pOldFrameHolder);
	}
	pSyncedStreamsFrameHolder->unlock();

	return ONI_STATUS_OK;

}

void Context::disableFrameSync(OniFrameSyncHandle frameSyncHandle)
{
	if (frameSyncHandle == NULL)
	{
		m_errorLogger.Append("Disable Frame Sync: Invalid handle");
		return;
	}

	// Disable the frame sync in the driver.
	frameSyncHandle->pDeviceDriver->disableFrameSync(frameSyncHandle->pFrameSyncHandle);

	// Disable and clear the synced stream frame holder.
	frameSyncHandle->pSyncedStreamsFrameHolder->setEnabled(FALSE);
	frameSyncHandle->pSyncedStreamsFrameHolder->lock();
	frameSyncHandle->pSyncedStreamsFrameHolder->clear();

	// Get the stream list from the holder.
	int numStreams = frameSyncHandle->pSyncedStreamsFrameHolder->getNumStreams();
	xnl::Array<VideoStream*> pStreamList(numStreams);
	pStreamList.SetSize(numStreams);
	frameSyncHandle->pSyncedStreamsFrameHolder->getStreams(pStreamList.GetData(), &numStreams);

	// Change holder to all the streams (allocate new StreamFrameHolder).
	for (int i = 0; i < numStreams; ++i)
	{
		// Allocate new frame holder.
		StreamFrameHolder* pStreamFrameHolder = XN_NEW(StreamFrameHolder, m_frameManager, pStreamList[i]);
		if (pStreamFrameHolder == NULL)
		{
			// TODO: error!!!
			continue;
		}

		// Replace the holder in the stream.
		pStreamList[i]->setFrameHolder(pStreamFrameHolder);
	}
	frameSyncHandle->pSyncedStreamsFrameHolder->unlock();

	// Delete the frame sync group (it will remove the link from all the streams).
	XN_DELETE(frameSyncHandle->pSyncedStreamsFrameHolder);
	XN_DELETE(frameSyncHandle);
}

const char* Context::getExtendedError()
{
	return m_errorLogger.GetExtendedError();
}

void ONI_CALLBACK_TYPE Context::deviceDriver_DeviceConnected(Device* pDevice, void* pCookie)
{
	Context* pContext = (Context*)pCookie;

	pContext->m_cs.Lock();
	pContext->m_devices.AddLast(pDevice);
	pContext->m_cs.Unlock();

	pContext->m_deviceConnectedEvent.Raise(pDevice->getInfo());
}
void ONI_CALLBACK_TYPE Context::deviceDriver_DeviceDisconnected(Device* pDevice, void* pCookie)
{
	Context* pContext = (Context*)pCookie;

	pContext->m_cs.Lock();
	pContext->m_devices.Remove(pDevice);
	pContext->m_cs.Unlock();

	pContext->m_deviceDisconnectedEvent.Raise(pDevice->getInfo());
}
void ONI_CALLBACK_TYPE Context::deviceDriver_DeviceStateChanged(Device* pDevice, OniDeviceState deviceState, void* pCookie)
{
	Context* pContext = (Context*)pCookie;
	pContext->m_deviceStateChangedEvent.Raise(pDevice->getInfo(), deviceState);
}

OniStatus Context::recorderOpen(const char* fileName, OniRecorderHandle* pRecorder)
{
    // Validate parameters.
    if (NULL == pRecorder || NULL == fileName)
    {
        return ONI_STATUS_BAD_PARAMETER;
    }
    // Allocate the handle.
    *pRecorder = XN_NEW(_OniRecorder);
    if (NULL == *pRecorder)
    {
        return ONI_STATUS_ERROR;
    }
    // Create the recorder itself.
	(*pRecorder)->pRecorder = XN_NEW(FileRecorder, m_frameManager, m_errorLogger, *pRecorder);

    if (NULL == (*pRecorder)->pRecorder)
    {
        XN_DELETE(*pRecorder);
        return ONI_STATUS_ERROR;
    }
    // Try to initialize the recorder, and add it to the list of known
    // recorders upon successful initialization.
    OniStatus status = (*pRecorder)->pRecorder->initialize(fileName);
    if (ONI_STATUS_OK == status) 
    {
        m_recorders.AddLast((*pRecorder)->pRecorder);
    }
    else
    {
        XN_DELETE((*pRecorder)->pRecorder);
    }
    return status;
}
OniStatus Context::recorderClose(OniRecorderHandle* pRecorder)
{
    // Validate parameters.
    if (NULL == pRecorder)
    {
        return ONI_STATUS_BAD_PARAMETER;
    }

    // NOTE:
    //  The way handles are related to Recorder instance can be depicted by such
    //  a diagram:
    //
    //  +----------------------------+ points to 
    //  | OniRecorderHandle handle_1 |-----------------+
    //  +----------------------------+                 |
    //  +----------------------------+ points to +-----v------------------+
    //  | OniRecorderHandle handle_2 |---------->| _OniRecorder instance  |
    //  +----------------------------+           |------------------------|
    //                                           | Recorder* pRecorder    |
    //  +-------------------+          points to +-----|------------------+
    //  | Recorder instance |<-------------------------+
    //  +-------------------+
    //
    // As you see, there might be two instances of OniRecorderHandle, which point
    // to the same Recorder instance.
    //
    // Handles do not support any reference-counting, and thus whenever somebody
    // destroys a Recorder instance, the instance becomes nonexistent for every
    // handle out there in your program.
    //
    // Moreover, a Recorder instance might own a handle to itself, and whenever
    // the Recorder instance is being destroyed, it NULL-fies the pRecorder
    // field in _OniRecorder structure.
    if (NULL != *pRecorder)
    {
        recorderClose((*pRecorder)->pRecorder);
    }

    // Delete the _OniRecorder data structure.
    XN_DELETE(*pRecorder);

    // Ensure, that the client no longer considers the handle being a valid one.
    *pRecorder = NULL;

    return ONI_STATUS_OK;
}
OniStatus Context::recorderClose(Recorder* pRecorder)
{
    // Validate parameters.
    if (NULL == pRecorder)
    {
        return ONI_STATUS_BAD_PARAMETER;
    }
    pRecorder->stop();
    pRecorder->detachAllStreams();
    m_recorders.Remove(pRecorder);
    XN_DELETE(pRecorder);
    return ONI_STATUS_OK;
}

void Context::clearErrorLogger()
{
	m_errorLogger.Clear();
}

void Context::addToLogger(const XnChar* cpFormat, ...)
{
	va_list args;
	va_start(args, cpFormat);
	m_errorLogger.AppendV(cpFormat, args);
	va_end(args);
}

void Context::onNewFrame()
{
	XnUInt64 nNow;
	xnOSGetHighResTimeStamp(&nNow);
	nNow /= 1000000;

	m_cs.Lock();
	for (xnl::Hash<XN_THREAD_ID, XN_EVENT_HANDLE>::Iterator it = m_waitingThreads.Begin(); it != m_waitingThreads.End(); ++it)
	{
		xnOSSetEvent(it->Value());
	}

	if (nNow != m_lastFPSPrint)
	{
		XnChar fpsInfo[2048] = "";
		XnUInt32 written = 0;
		XnUInt32 writtenNow = 0;
		xnOSStrFormat(fpsInfo + written, sizeof(fpsInfo) - written, &writtenNow, "[FPS] ");
		written += writtenNow;

		for (xnl::List<VideoStream*>::Iterator it = m_streams.Begin(); it != m_streams.End(); ++it)
		{
			VideoStream* pStream = *it;
			if (written > sizeof(fpsInfo))
			{
				break;
			}

			xnOSStrFormat(fpsInfo + written, sizeof(fpsInfo) - written, &writtenNow, "%s: %.2f ", 
				pStream->getSensorName(), pStream->calcCurrentFPS());
			written += writtenNow;
		}

		xnLogVerbose(XN_MASK_ONI_CONTEXT, "%s", fpsInfo);
		m_lastFPSPrint = nNow;
	}
	m_cs.Unlock();
}

void XN_CALLBACK_TYPE Context::newFrameCallback(void* pCookie)
{
	Context* pThis = (Context*)pCookie;
	pThis->onNewFrame();
}

XN_EVENT_HANDLE Context::getThreadEvent()
{
	XN_THREAD_ID tid;
	XN_EVENT_HANDLE hEvent = NULL;
	xnOSGetCurrentThreadID(&tid);

	m_cs.Lock();
	
	if (XN_STATUS_OK != m_waitingThreads.Get(tid, hEvent))
	{
		xnOSCreateEvent(&hEvent, FALSE);
		m_waitingThreads.Set(tid, hEvent);
	}

	m_cs.Unlock();

	return hEvent;
}

ONI_NAMESPACE_IMPLEMENTATION_END
