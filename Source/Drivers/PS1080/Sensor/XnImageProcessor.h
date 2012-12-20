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
#ifndef __XN_IMAGE_PROCESSOR_H__
#define __XN_IMAGE_PROCESSOR_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnFrameStreamProcessor.h"
#include "XnSensorImageStream.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
class XnImageProcessor : public XnFrameStreamProcessor
{
public:
	XnImageProcessor(XnSensorImageStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager, XnBool bCompressedOutput = FALSE);
	virtual ~XnImageProcessor();

	XnStatus Init();

protected:
	//---------------------------------------------------------------------------
	// Overridden Functions
	//---------------------------------------------------------------------------
	virtual void OnEndOfFrame(const XnSensorProtocolResponseHeader* pHeader);
	virtual void OnFrameReady(XnUInt32 nFrameID, XnUInt64 nFrameTS);

	//---------------------------------------------------------------------------
	// Helper Functions
	//---------------------------------------------------------------------------
	inline XnSensorImageStream* GetStream()
	{
		return (XnSensorImageStream*)XnFrameStreamProcessor::GetStream();
	}

	XnUInt32 GetActualXRes() { return m_nActualXRes; }
	XnUInt32 GetActualYRes() { return m_nActualYRes; }

private:
	XnUInt32 CalculateExpectedSize();
	void CalcActualRes();
	static XnStatus XN_CALLBACK_TYPE ActualResChangedCallback(const XnProperty* pSender, void* pCookie);

	XnUInt32 m_nActualXRes;
	XnUInt32 m_nActualYRes;

	XnCallbackHandle m_hXResCallback;
	XnCallbackHandle m_hYResCallback;
	XnCallbackHandle m_hXCropCallback;
	XnCallbackHandle m_hYCropCallback;
	XnCallbackHandle m_hCropEnabledCallback;

	XnBool m_bCompressedOutput;
};

#endif //__XN_IMAGE_PROCESSOR_H__
