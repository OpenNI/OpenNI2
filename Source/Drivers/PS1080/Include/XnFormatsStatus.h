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
#ifndef _XN_FORMATS_STATUS_H_
#define _XN_FORMATS_STATUS_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnStatus.h>
#include <XnCommon.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
XN_PS_STATUS_MESSAGE_MAP_START(XN_ERROR_GROUP_FORMATS)
XN_STATUS_MESSAGE(XN_STATUS_FORMATS_NOT_INIT, "Xiron Formats library was not initialized!")
XN_STATUS_MESSAGE(XN_STATUS_FORMATS_ALREADY_INIT, "Xiron Formats library was already initialized!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_DEPTH_FORMAT, "Invalid Xiron I/O stream depth format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_IMAGE_FORMAT, "Invalid Xiron I/O stream image format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_MISC_FORMAT, "Invalid Xiron I/O stream misc format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_AUDIO_FORMAT, "Invalid Xiron I/O stream audio format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_COMPRESSION_INIT_FAILED, "Xiron I/O compression initialization failed!")
XN_STATUS_MESSAGE(XN_STATUS_IO_COMPRESSION_FAILED, "Xiron I/O compression failed!")
XN_STATUS_MESSAGE(XN_STATUS_IO_DECOMPRESSION_FAILED, "Xiron I/O decompression failed!")
XN_STATUS_MESSAGE(XN_STATUS_IO_COMPRESSED_BUFFER_TOO_SMALL, "The compressed input buffer is too small to be valid!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_COMPRESSED_BUFFER_SIZE, "Invalid compressed buffer size!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_DEPTH_COMPRESSION_FORMAT, "Invalid depth compression format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_IMAGE_COMPRESSION_FORMAT, "Invalid image stream compression format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_MISC_COMPRESSION_FORMAT, "Invalid misc stream compression format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_AUDIO_COMPRESSION_FORMAT, "Invalid audio stream compression format!")
XN_STATUS_MESSAGE(XN_STATUS_IO_UNSUPPORTED_COMPRESSION_FORMAT, "This compression format is no longer supported!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_PACKED_BUFFER, "Invalid Xiron I/O packed stream buffer!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_HEADER, "Invalid Xiron I/O stream header!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_DEPTH_BUFFER, "Invalid Xiron I/O stream depth buffer!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_IMAGE_BUFFER, "Invalid Xiron I/O stream image buffer!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_MISC_BUFFER, "Invalid Xiron I/O stream misc buffer!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_AUDIO_BUFFER, "Invalid Xiron I/O stream audio buffer!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_DEPTH_BUFFER_SIZE, "Invalid Xiron I/O stream depth buffer size!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_IMAGE_BUFFER_SIZE, "Invalid Xiron I/O stream image buffer size!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_MISC_BUFFER_SIZE, "Invalid Xiron I/O stream misc buffer size!")
XN_STATUS_MESSAGE(XN_STATUS_IO_INVALID_STREAM_AUDIO_BUFFER_SIZE, "Invalid Xiron I/O stream audio buffer size!")
XN_STATUS_MESSAGE(XN_STATUS_INVALID_OUTPUT_FORMAT_FOR_RESOLUTION, "Pixel format is not supported for this resolution!")
XN_PS_STATUS_MESSAGE_MAP_END(XN_ERROR_GROUP_FORMATS)

#endif //_XN_FORMATS_STATUS_H_
