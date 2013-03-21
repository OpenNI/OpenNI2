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
#ifndef __XN_UNCOMPRESSED_YUV_IMAGE_PROCESSOR_H__
#define __XN_UNCOMPRESSED_YUV_IMAGE_PROCESSOR_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnImageProcessor.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

class XnPassThroughImageProcessor : public XnImageProcessor
{
public:
	XnPassThroughImageProcessor(XnSensorImageStream* pStream, XnSensorStreamHelper* pHelper, XnFrameBufferManager* pBufferManager);
	~XnPassThroughImageProcessor();

	//---------------------------------------------------------------------------
	// Overridden Functions
	//---------------------------------------------------------------------------
protected:
	virtual void ProcessFramePacketChunk(const XnSensorProtocolResponseHeader* pHeader, const XnUChar* pData, XnUInt32 nDataOffset, XnUInt32 nDataSize);
};

#endif //__XN_UNCOMPRESSED_YUV_IMAGE_PROCESSOR_H__
