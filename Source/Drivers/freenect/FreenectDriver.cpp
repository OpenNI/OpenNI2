#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
//#include "KinectDriver.h"
#include "FreenectDeviceNI.h"
#include "XnHash.h"

#include <iostream>
#include <sstream>


using namespace oni::driver;

static const char VENDOR_VAL[] = "Microsoft";
static const char NAME_VAL[] = "Kinect";



// EXPERIMENTAL with modified libfreenect and double-inheritance

class FreenectDriver : public DriverBase, private Freenect::Freenect
{
private:
	//Freenect::Freenect freenect;
	// from Freenect::Freenect - freenect_context* m_ctx
	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> devices;

public:
	
	FreenectDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
	{
		freenect_set_log_level(m_ctx, FREENECT_LOG_DEBUG);
		// MOTOR doesn't work with k4w branch; todo: fix it
		//freenect_select_subdevices(m_ctx, static_cast<freenect_device_flags>(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));
		freenect_select_subdevices(m_ctx, static_cast<freenect_device_flags>(FREENECT_DEVICE_CAMERA));
	}
	~FreenectDriver()
	{}
	
	OniStatus initialize(DeviceConnectedCallback connectedCallback, DeviceDisconnectedCallback disconnectedCallback, DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie)
	{
		DriverBase::initialize(connectedCallback, disconnectedCallback, deviceStateChangedCallback, pCookie);
		std::cout << "number of freenect devices: " << Freenect::deviceCount() << std::endl;
		for (int i = 0; i < Freenect::deviceCount(); i++)
		{
			OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
			std::ostringstream uri;
			uri << "freenect:" << i;
			xnOSStrCopy(pInfo->uri, uri.str().c_str(), ONI_MAX_STR);
			xnOSStrCopy(pInfo->vendor, VENDOR_VAL, ONI_MAX_STR);
			xnOSStrCopy(pInfo->name, NAME_VAL, ONI_MAX_STR);
			devices[pInfo] = NULL;
			//deviceConnected(pInfo);
			//deviceStateChanged(pInfo, 0);	
		}
		return ONI_STATUS_OK;
	}	
	
	DeviceBase* deviceOpen(const char* uri)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = devices.Begin(); iter != devices.End(); ++iter)
		{
			if (xnOSStrCmp(iter->Key()->uri, uri) == 0)
			{
				// found
				if (iter->Value() != NULL)
				{
					// already using
					return iter->Value();
				}
				else
				{
					int id;
					std::istringstream(iter->Key()->uri) >> id;
					FreenectDeviceNI * freenect_device = &createDevice<FreenectDeviceNI>(id);
					iter->Value() = freenect_device;
					return freenect_device;
				}
			}
		}
		
		getServices().errorLoggerAppend("Looking for '%s'", uri);
		return NULL;	
	}
	
	void deviceClose(DeviceBase* pDevice)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = devices.Begin(); iter != devices.End(); ++iter)
		{
			if (iter->Value() == pDevice)
			{
				iter->Value() = NULL;
				int id;
				std::istringstream(iter->Key()->uri) >> id;
				Freenect::deleteDevice(id);
				return;
			}
		}
		
		// not our device?!
		XN_ASSERT(FALSE);
	}
	
	void shutdown()
	{}

	/*

	virtual OniStatus tryDevice(const char* uri);

	virtual void* enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount);
	virtual void disableFrameSync(void* frameSyncGroup);
	void StatusUpdate(const OLECHAR* instanceName, bool isConnected);
	*/
};


// PARTIAL REFERENCE FROM TestDevice.cpp 
/*
class TestDriver : public oni::driver::DriverBase
{
public:
	virtual OniStatus tryDevice(const char* uri)
	{
		if (xnOSStrCmp(uri, "Test"))
		{
			return ONI_STATUS_ERROR;
		}


		OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
		xnOSStrCopy(pInfo->uri, uri, ONI_MAX_STR);
		xnOSStrCopy(pInfo->vendor, "Test", ONI_MAX_STR);
		m_devices[pInfo] = NULL;

		deviceConnected(pInfo);

		return ONI_STATUS_OK;
	}
};
*/

// REFERENCE ENDS HERE





/*  ORIGINAL

//class FreenectDriver : public kinect_device::KinectDriver, private Freenect::Freenect
//class FreenectDriver : public kinect_device::KinectDriver
class FreenectDriver : public DriverBase
{
private:
	Freenect::Freenect freenect;
	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> m_devices;

public:
	
	FreenectDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
	{
	}
	~FreenectDriver()
	{}
	
	OniStatus initialize(DeviceConnectedCallback connectedCallback, DeviceDisconnectedCallback disconnectedCallback, DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie)
	{
		DriverBase::initialize(connectedCallback, disconnectedCallback, deviceStateChangedCallback, pCookie);
		std::cout << "number of devices: " << freenect.deviceCount() << std::endl;
		for (int i = 0; i < freenect.deviceCount(); i++)
		{
			OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
			std::ostringstream uri;
			uri << "freenect:" << i;
			xnOSStrCopy(pInfo->uri, uri.str().c_str(), ONI_MAX_STR);
			xnOSStrCopy(pInfo->vendor, VENDOR_VAL, ONI_MAX_STR);
			xnOSStrCopy(pInfo->name, NAME_VAL, ONI_MAX_STR);
			m_devices[pInfo] = NULL;
			//deviceConnected(pInfo);
			//deviceStateChanged(pInfo, 0);	
		}
		return ONI_STATUS_OK;
	}	
	
	DeviceBase* deviceOpen(const char* uri)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
		{
			if (xnOSStrCmp(iter->Key()->uri, uri) == 0)
			{
				// found
				if (iter->Value() != NULL)
				{
					// already using
					return iter->Value();
				}
				else
				{
					int id;
					std::istringstream(iter->Key()->uri) >> id;
					FreenectDeviceNI * freenect_device = &freenect.createDevice<FreenectDeviceNI>(id);
					//iter->Value() = freenect_device;
					std::cout << "number of devices: " << freenect.deviceCount() << std::endl;
					//return freenect_device;
				}
			}
		}		
		return NULL;	
	}
	
	void deviceClose(DeviceBase* pDevice)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
		{
			if (iter->Value() == pDevice)
			{
				iter->Value() = NULL;
				XN_DELETE(pDevice);
				int id;
				std::istringstream(iter->Key()->uri) >> id;
				freenect.deleteDevice(id);
				return;
			}
		}
		// not our device?!
		XN_ASSERT(FALSE);
	}
	
	void shutdown()
	{}

	/*

	virtual OniStatus tryDevice(const char* uri);

	virtual void* enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount);
	virtual void disableFrameSync(void* frameSyncGroup);
	void StatusUpdate(const OLECHAR* instanceName, bool isConnected);
	*/
//};
