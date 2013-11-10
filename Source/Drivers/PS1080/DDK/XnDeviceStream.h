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
#ifndef XNDEVICESTREAM_H
#define XNDEVICESTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnDeviceModule.h>
#include <DDK/XnActualIntProperty.h>
#include <DDK/XnActualStringProperty.h>
#include <XnOS.h>
#include <Core/XnBuffer.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

/** Represents a stream in a device. */
class XnDeviceStream : public XnDeviceModule
{
public:
	XnDeviceStream(const XnChar* csType, const XnChar* csName);
	~XnDeviceStream() { Free(); }

	XnStatus Init();
	XnStatus Free();

	void SetServices(oni::driver::StreamServices& services) { m_pServices = &services; }
	void AddRefToFrame(OniFrame* pFrame) { m_pServices->addFrameRef(pFrame); }
	void ReleaseFrame(OniFrame* pFrame) { m_pServices->releaseFrame(pFrame); }

	void AddRef();
	XnUInt32 DecRef();

	void OpenAddRef();
	XnUInt32 OpenDecRef();
	XnUInt32 GetOpenRefCount() const { return m_nOpenRefCount; }

	typedef void (*NewDataCallbackPtr)(XnDeviceStream* pSender, OniFrame* pFrame, void* pCookie);

	/** Sets a function callback to be called when new data is available. */
	void SetNewDataCallback(NewDataCallbackPtr pFunc, void* pCookie);

	/** Notifies new data is available in this stream. */
	virtual void NewDataAvailable(OniFrame* pFrame);

	//---------------------------------------------------------------------------
	// Getters
	//---------------------------------------------------------------------------
	inline const XnChar* GetType() const { return m_Type.GetValue(); }
	inline XnBool IsOpen() const { return (XnBool)m_IsOpen.GetValue(); }
	inline OniPixelFormat GetOutputFormat() const { return (OniPixelFormat)m_OutputFormat.GetValue(); }
	inline XnBool IsMirrored() const { return (XnBool)m_IsMirrored.GetValue(); }
	inline XnUInt32 GetRequiredDataSize() const { return (XnUInt32)m_RequiredSize.GetValue(); }
	inline XnUInt32 GetLastFrameID() const { return m_nFrameID; }

	//---------------------------------------------------------------------------
	// Setters
	//---------------------------------------------------------------------------
	virtual XnStatus Open();
	virtual XnStatus Close();
	virtual XnStatus SetOutputFormat(OniPixelFormat nOutputFormat);
	virtual XnStatus SetMirror(XnBool bIsMirrored);

	inline XN_CRITICAL_SECTION_HANDLE* GetOpenLock() { return &m_hOpenLock; }

protected:
	//---------------------------------------------------------------------------
	// Properties Getters
	//---------------------------------------------------------------------------
	inline XnActualIntProperty& IsOpenProperty() { return m_IsOpen; }
	inline XnActualIntProperty& OutputFormatProperty() { return m_OutputFormat; }
	inline XnActualIntProperty& RequiredSizeProperty() { return m_RequiredSize; }
	inline XnActualIntProperty& IsMirroredProperty() { return m_IsMirrored; }

protected:
	oni::driver::StreamServices& GetServices() { return *m_pServices; }

	//---------------------------------------------------------------------------
	// Virtual Functions
	//---------------------------------------------------------------------------

	/** Performs mirror on the given stream output. */
	virtual XnStatus Mirror(OniFrame* pFrame) const = 0;
	/** Calculates the required size. */
	virtual XnStatus CalcRequiredSize(XnUInt32* pnRequiredSize) const = 0;

	//---------------------------------------------------------------------------
	// Utility Functions
	//---------------------------------------------------------------------------
	XnStatus RegisterRequiredSizeProperty(XnProperty* pProperty);
	inline XN_CRITICAL_SECTION_HANDLE* GetLock() { return &m_hCriticalSection; }

private:
	XnStatus UpdateRequiredSize();

	static XnStatus XN_CALLBACK_TYPE UpdateRequiredSizeCallback(const XnProperty* pSenser, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetIsOpenCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetOutputFormatCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetIsMirrorCallback(XnActualIntProperty* pSender, XnUInt64 nValue, void* pCookie);

	//---------------------------------------------------------------------------
	// Members
	//---------------------------------------------------------------------------
	oni::driver::StreamServices* m_pServices;
	XnActualIntProperty m_IsStream;
	XnActualStringProperty m_Type;
	XnActualIntProperty m_IsOpen;
	XnActualIntProperty m_RequiredSize;
	XnActualIntProperty m_OutputFormat;
	XnActualIntProperty m_IsMirrored;

	XnUInt32 m_nFrameID;
	NewDataCallbackPtr m_pNewDataCallback;
	void* m_pNewDataCallbackCookie;

	XnUInt32 m_nRefCount;
	XnUInt32 m_nOpenRefCount;
	XN_CRITICAL_SECTION_HANDLE m_hCriticalSection;
	XN_CRITICAL_SECTION_HANDLE m_hOpenLock;
};

#endif // XNDEVICESTREAM_H
