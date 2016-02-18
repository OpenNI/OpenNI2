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
#ifndef XNONICOLORSTREAM_H
#define XNONICOLORSTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnOniMapStream.h"

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class XnOniColorStream :
	public XnOniMapStream
{
public:
	XnOniColorStream(XnSensor* pSensor, XnOniDevice* pDevice);

	static void GetAllowedOniOutputFormatForInputFormat(XnIOImageFormats inputFormat, OniPixelFormat *aOniFormats, int *nOniFormats);

	static XnBool IsSupportedInputFormat(XnIOImageFormats inputFormat, OniPixelFormat oniFormat);

	static XnBool IsPreferredInputFormat(XnIOImageFormats inputFormat, XnIOImageFormats thanFormat, OniPixelFormat oniFormat);
};

#endif // XNONICOLORSTREAM_H
