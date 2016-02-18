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
#ifndef XNPIXELSTREAM_H
#define XNPIXELSTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <DDK/XnFrameStream.h>
#include <DDK/XnActualGeneralProperty.h>
#include <XnArray.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

/** Represents a stream that is pixel based (meaning, its output data is a matrix of data). */
class XnPixelStream : public XnFrameStream
{
public:
	XnPixelStream(const XnChar* csType, const XnChar* csName, XnBool bAllowCustomResolutions);
	~XnPixelStream() { Free(); }

	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	XnStatus Init();

	//---------------------------------------------------------------------------
	// Getters
	//---------------------------------------------------------------------------
	inline XnResolutions GetResolution() const { return (XnResolutions)m_Resolution.GetValue(); }
	inline XnUInt32 GetXRes() const { return (XnUInt32)m_XRes.GetValue(); }
	inline XnUInt32 GetYRes() const { return (XnUInt32)m_YRes.GetValue(); }
	inline XnUInt32 GetBytesPerPixel() const { return (XnUInt32)m_BytesPerPixel.GetValue(); }
	inline const OniCropping* GetCropping() const { return (OniCropping*)m_Cropping.GetValue().data; }
	inline const xnl::Array<XnCmosPreset>& GetSupportedModes() const { return m_supportedModesData; }

protected:
	XnStatus AddSupportedModes(XnCmosPreset* aPresets, XnUInt32 nCount);
	XnStatus ValidateSupportedMode(const XnCmosPreset& preset);

	/** Notifies new data is available in this stream. */
	virtual void NewDataAvailable(OniFrame* pFrame);

	//---------------------------------------------------------------------------
	// Properties Getters
	//---------------------------------------------------------------------------
	inline XnActualIntProperty& IsPixelStreamProperty() { return m_IsPixelStream; }
	inline XnActualIntProperty& ResolutionProperty() { return m_Resolution; }
	inline XnActualIntProperty& XResProperty() { return m_XRes; }
	inline XnActualIntProperty& YResProperty() { return m_YRes; }
	inline XnActualIntProperty& BytesPerPixelProperty() { return m_BytesPerPixel; }
	inline XnActualGeneralProperty& CroppingProperty() { return m_Cropping; }

	//---------------------------------------------------------------------------
	// Setters
	//---------------------------------------------------------------------------
	virtual XnStatus SetResolution(XnResolutions nResolution);
	virtual XnStatus SetXRes(XnUInt32 nXRes);
	virtual XnStatus SetYRes(XnUInt32 nYRes);
	virtual XnStatus SetCropping(const OniCropping* pCropping);

	//---------------------------------------------------------------------------
	// Virtual Methods
	//---------------------------------------------------------------------------

	/** Crops the stream output. */
	virtual XnStatus CropImpl(OniFrame* pFrame, const OniCropping* pCropping);

	//---------------------------------------------------------------------------
	// Overridden Methods
	//---------------------------------------------------------------------------
	XnStatus Mirror(OniFrame* pFrame) const;
	XnStatus CalcRequiredSize(XnUInt32* pnRequiredSize) const;

	XnStatus ValidateCropping(const OniCropping* pCropping);

private:
	class XnResolutionProperty : public XnActualIntProperty
	{
	public:
		XnResolutionProperty(XnUInt32 propertyId, const XnChar* strName, XnUInt64 nInitialValue = 0, const XnChar* strModule = "");
		XnBool ConvertValueToString(XnChar* csValue, const void* pValue) const;
	};

	XnStatus OnResolutionChanged();
	XnStatus OnOutputFormatChanged();
	XnStatus FixCropping();
	XnStatus GetSupportedModes(XnCmosPreset* aPresets, XnUInt32& nCount);

	static XnStatus XN_CALLBACK_TYPE SetResolutionCallback(XnActualIntProperty* pSenser, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetXResCallback(XnActualIntProperty* pSenser, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetYResCallback(XnActualIntProperty* pSenser, XnUInt64 nValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE SetCroppingCallback(XnActualGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ResolutionValueChangedCallback(const XnProperty* pSenser, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE OutputFormatValueChangedCallback(const XnProperty* pSenser, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE FixCroppingCallback(const XnProperty* pSenser, void* pCookie);
	static XnStatus XN_CALLBACK_TYPE ReadCroppingFromFileCallback(XnGeneralProperty* pSender, const XnChar* csINIFile, const XnChar* csSection);
	static XnStatus XN_CALLBACK_TYPE GetSupportedModesCallback(const XnGeneralProperty* pSender, const OniGeneralBuffer& gbValue, void* pCookie);
	//---------------------------------------------------------------------------
	// Members
	//---------------------------------------------------------------------------
	XnActualIntProperty m_IsPixelStream;
	XnResolutionProperty m_Resolution;
	XnActualIntProperty m_XRes;
	XnActualIntProperty m_YRes;
	XnActualIntProperty m_BytesPerPixel;
	XnActualGeneralProperty m_Cropping;

	OniCropping m_CroppingData;

	XnActualIntProperty m_SupportedModesCount;
	XnGeneralProperty m_SupportedModes;

	xnl::Array<XnCmosPreset> m_supportedModesData;
	XnBool m_bAllowCustomResolutions;
};

#endif // XNPIXELSTREAM_H
