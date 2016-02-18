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
// PSLinkConsole.cpp : Defines the entry point for the console application.
//

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <OpenNI.h>
#include <PSLink.h>
#include <XnStringsHash.h>
#include <XnList.h>
#include <XnArray.h>
#include <XnBitSet.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

using namespace openni;

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------
#define CHECK_RC(what, nRetVal) \
	if (nRetVal != XN_STATUS_OK) \
	{ \
		printf("Failed to %s: %s\n", what, OpenNI::getExtendedError()); \
		XN_ASSERT(FALSE); \
		return nRetVal; \
	}

#define CHECK_RC_NO_RET(what, nRetVal) \
	if (nRetVal != XN_STATUS_OK) \
	{ \
		printf("Failed to %s: %s\n", what, OpenNI::getExtendedError()); \
		XN_ASSERT(FALSE); \
	}

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define WAIT_FOR_DEVICE_TIMEOUT 10000
#define WAIT_FOR_DEVICE_CHECK_INTERVAL_MS 5000
#define XN_MASK_PRIME_CONSOLE "PSLinkConsole"
#define MAX_STREAMS_COUNT 10

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
typedef int (*CommandHandler)(int argc, const char* argv[]);

typedef struct  
{
	const XnChar* name;
	CommandHandler handler;
} Command;

typedef struct
{
	XnFwStreamType type;
	const char* name;
} FwStreamName;

typedef struct
{
	XnUsbInterfaceType type;
	const char* name;
} UsbInterfaceName;

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
static Device g_device;
static xnl::Array<VideoStream*> g_streams;
static xnl::List<const XnChar*> g_commandsList;
static XnStringsHashT<Command> g_commands;
static XnBool g_continue = TRUE;

static FwStreamName g_fwStreamNames[] = {
	{ XN_FW_STREAM_TYPE_COLOR, "Color" },
	{ XN_FW_STREAM_TYPE_IR, "IR" },
	{ XN_FW_STREAM_TYPE_SHIFTS, "Depth" },
	{ XN_FW_STREAM_TYPE_AUDIO, "Audio" },
	{ XN_FW_STREAM_TYPE_DY, "DY" },
	{ XN_FW_STREAM_TYPE_LOG, "Log" },
};

static UsbInterfaceName g_usbInterfaceNames[] = {
	{ PS_USB_INTERFACE_DONT_CARE, "ANY"	},
	{ PS_USB_INTERFACE_ISO_ENDPOINTS, "ISO"	},
	{ PS_USB_INTERFACE_BULK_ENDPOINTS, "BULK" },
};

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------
void ExecuteCommandsFromStream(FILE* inputStream, XnBool bPrompt);

//---------------------------------------------------------------------------
// Utilities
//---------------------------------------------------------------------------

//pnTokens is max args on input, actual number of args on output
void SplitStr(XnChar* str, const XnChar* strTokens[], int* pnTokens)
{
	XnUInt32 nMaxTokens = *pnTokens;
	XnUInt32 nToken = 0;

	XnBool bFirstAfterDelim = TRUE;

	for (XnChar* p = str; *p != '\0' && nToken < nMaxTokens ; ++p)
	{
		if (*p == ' ' || *p == '\n')
		{
			*p = '\0';
			bFirstAfterDelim = TRUE;
		}
		else if (bFirstAfterDelim)
		{
			strTokens[nToken++] = p;
			bFirstAfterDelim = FALSE;
		}
	}

	*pnTokens = nToken;
}

//MyAtoi() accepts hex (with 0x) or decimal values from string
XnUInt32 MyAtoi(const XnChar* str)
{
	XnUInt32 nValue = 0;
	sscanf(str, "%i", &nValue);
	return nValue;
}

XnChar* ToLower(XnChar* str)
{
	for (XnChar* p = str; *p != '\0'; ++p)
	{
		*p = (XnChar)tolower(*p);
	}
	return str;
}

const char* fwStreamTypeToName(XnFwStreamType type)
{
	for (size_t i = 0; i < sizeof(g_fwStreamNames) / sizeof(g_fwStreamNames[0]); ++i)
	{
		if (g_fwStreamNames[i].type == type)
		{
			return g_fwStreamNames[i].name;
		}
	}

	XN_ASSERT(FALSE);
	return NULL;
}

XnFwStreamType fwStreamNameToType(const char* name)
{
	for (size_t i = 0; i < sizeof(g_fwStreamNames) / sizeof(g_fwStreamNames[0]); ++i)
	{
		if (xnOSStrCaseCmp(g_fwStreamNames[i].name, name) == 0)
		{
			return g_fwStreamNames[i].type;
		}
	}

	return (XnFwStreamType)-1;
}

const XnChar* fwPixelFormatToName(XnFwPixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case XN_FW_PIXEL_FORMAT_SHIFTS_9_3:
		return "Shifts9.3";
	case XN_FW_PIXEL_FORMAT_GRAYSCALE16:
		return "Grayscale16";
	case XN_FW_PIXEL_FORMAT_YUV422:
		return "YUV422";
	case XN_FW_PIXEL_FORMAT_BAYER8:
		return "BAYER8";
	default:
		XN_ASSERT(FALSE);
		return "UNKNOWN";
	}
}

XnFwPixelFormat fwPixelFormatNameToType(const XnChar* name)
{
	if (xnOSStrCmp(name, "Shifts9.3") == 0)
		return XN_FW_PIXEL_FORMAT_SHIFTS_9_3;
	else if (xnOSStrCmp(name, "Grayscale16") == 0)
		return XN_FW_PIXEL_FORMAT_GRAYSCALE16;
	else if (xnOSStrCmp(name, "YUV422") == 0)
		return XN_FW_PIXEL_FORMAT_YUV422;
	else if (xnOSStrCmp(name, "BAYER8") == 0)
		return XN_FW_PIXEL_FORMAT_BAYER8;
	else
	{
		XN_ASSERT(FALSE);
		return (XnFwPixelFormat)(-1);
	}
}

const XnChar* fwCompressionTypeToName(XnFwCompressionType compression)
{
	switch (compression)
	{
	case XN_FW_COMPRESSION_NONE:
		return "None";
	case XN_FW_COMPRESSION_8Z:
		return "8z";
	case XN_FW_COMPRESSION_16Z:
		return "16z";
	case XN_FW_COMPRESSION_24Z:
		return "24z";
	case XN_FW_COMPRESSION_6_BIT_PACKED:
		return "6bit";
	case XN_FW_COMPRESSION_10_BIT_PACKED:
		return "10bit";
	case XN_FW_COMPRESSION_11_BIT_PACKED:
		return "11bit";
	case XN_FW_COMPRESSION_12_BIT_PACKED:
		return "12bit";
	default:
		XN_ASSERT(FALSE);
		return "UNKNOWN";
	}
}

XnFwCompressionType fwCompressionNameToType(const XnChar* name)
{
	if (xnOSStrCmp(name, "None") == 0)
		return XN_FW_COMPRESSION_NONE;
	else if (xnOSStrCmp(name, "8z") == 0)
		return XN_FW_COMPRESSION_8Z;
	else if (xnOSStrCmp(name, "16z") == 0)
		return XN_FW_COMPRESSION_16Z;
	else if (xnOSStrCmp(name, "24z") == 0)
		return XN_FW_COMPRESSION_24Z;
	else if (xnOSStrCmp(name, "6bit") == 0)
		return XN_FW_COMPRESSION_6_BIT_PACKED;
	else if (xnOSStrCmp(name, "10bit") == 0)
		return XN_FW_COMPRESSION_10_BIT_PACKED;
	else if (xnOSStrCmp(name, "11bit") == 0)
		return XN_FW_COMPRESSION_11_BIT_PACKED;
	else if (xnOSStrCmp(name, "12bit") == 0)
		return XN_FW_COMPRESSION_12_BIT_PACKED;
	else
	{
		XN_ASSERT(FALSE);
		return (XnFwCompressionType)-1;
	}
}

const char* fwVideoModeToString(XnFwStreamVideoMode videoMode)
{
	static char buffer[256];
	XnUInt32 charsWritten = 0;
	xnOSStrFormat(buffer, sizeof(buffer), &charsWritten, "%ux%u@%u (%s, %s)", 
		videoMode.m_nXRes, videoMode.m_nYRes, videoMode.m_nFPS, 
		fwPixelFormatToName(videoMode.m_nPixelFormat),
		fwCompressionTypeToName(videoMode.m_nCompression));
	return buffer;
}

//---------------------------------------------------------------------------
// Framework
//---------------------------------------------------------------------------
void RunCommand(int argc, const char* argv[])
{
	char commandName[XN_FILE_MAX_PATH];
	xnOSStrCopy(commandName, argv[0], sizeof(commandName));

	if (commandName[0] == '#')
	{
		//This is a comment - ignore it
		return;
	}

	ToLower(commandName);

	Command command;
	if (XN_STATUS_OK != g_commands.Get(commandName, command))
	{
		printf("Invalid command '%s'. Try 'Help' for all available commands.\n\n", argv[0]);
		return;
	}

	command.handler(argc, argv);
}

void RunCommand(XnChar* strCmdLine)
{
	enum {CMD_MAX_ARGS = 256};
	const char* argv[CMD_MAX_ARGS] = {NULL};
	int argc = CMD_MAX_ARGS;

	SplitStr(strCmdLine, argv, &argc);

	if (argc == 0)
	{
		//Ignore empty lines
		return;
	}

	RunCommand(argc, argv);
}

void ExecuteCommandsFromStream(FILE* pStream, XnBool bPrompt)
{
	XnChar strCmdLine[1024];
	XnBool bEOF = FALSE;

	while (g_continue && !bEOF)
	{
		if (bPrompt)
		{
			printf("PSLinkConsole>");
		}

		// read command from stream 
		if (fgets(strCmdLine, sizeof(strCmdLine), pStream) != 0)
		{
			RunCommand(strCmdLine);
		}
		else
		{
			if (ferror(pStream))
			{
				printf("Error reading from input stream: %d\n", errno);
			}
			//if fgets returns 0 and this is not an error then it's eof
			bEOF = TRUE;
		}
	} // commands loop
}

void RegisterCommand(const XnChar* cmd, CommandHandler handler)
{
	Command command;
	command.name = cmd;
	command.handler = handler;

	XnChar commandName[XN_FILE_MAX_PATH];
	xnOSStrCopy(commandName, cmd, sizeof(commandName));
	ToLower(commandName);
	g_commands.Set(commandName, command);

	g_commandsList.AddLast(cmd);
}

int RunScript(const XnChar* strFileName) 
{
	XnStatus nRetVal = XN_STATUS_OK;

	XnBool bExists = FALSE;
	nRetVal = xnOSDoesFileExist(strFileName, &bExists);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed checking for file existence: %s\n", OpenNI::getExtendedError());
		return -1;
	}

	if (!bExists)
	{
		printf("Script file '%s' not found\n", strFileName);
		return -2;
	}

	printf("Running script file '%s'\n", strFileName);
	FILE* pScriptFile = fopen(strFileName, "r");
	if (pScriptFile == NULL)
	{
		printf("Failed to open script file '%s'\n", strFileName);
		return -3;
	}

	ExecuteCommandsFromStream(pScriptFile, FALSE);
	fclose(pScriptFile);

	return nRetVal;
}

//---------------------------------------------------------------------------
// Commands
//---------------------------------------------------------------------------
int Help(int /*argc*/, const char* /*argv*/[])
{
	printf("Supported commands:\n");
	for (xnl::List<const XnChar*>::Iterator it = g_commandsList.Begin(); it != g_commandsList.End(); ++it)
	{
		printf("\t%s\n", *it);
	}

	return 0;
}

int BeginUpload(int /*argc*/, const char* /*argv*/[])
{
	Status nRetVal = g_device.invoke(PS_COMMAND_BEGIN_FIRMWARE_UPDATE, NULL, 0);
	if (nRetVal == STATUS_OK)
	{
		printf("Begin upload successful\n\n");
		return 0;
	}
	else
	{
		printf("Begin upload failed: %s\n\n", OpenNI::getExtendedError());
		return nRetVal;
	}
}

int EndUpload(int /*argc*/, const char* /*argv*/[])
{
	Status nRetVal = g_device.invoke(PS_COMMAND_END_FIRMWARE_UPDATE, NULL, 0);
	if (nRetVal == STATUS_OK)
	{
		printf("End upload successful\n\n");
		return 0;
	}
	else
	{
		printf("End upload failed: %s\n\n", OpenNI::getExtendedError());
		return nRetVal;
	}
}

int Upload(int argc, const char* argv[])
{
	if (argc < 2 || (argc >= 3 && xnOSStrCaseCmp(argv[2], "factory") != 0))
	{
		printf("Usage: %s <fileName> [factory]\n", argv[0]);
		printf("Note: filename can not contain any spaces.\n\n");
		return -1;
	}

	XnCommandUploadFile uploadCommand;
	uploadCommand.uploadToFactory = (argc > 2);
	uploadCommand.filePath = argv[1];

	printf("Uploading file '%s'...", uploadCommand.filePath);

	Status nRetVal = g_device.invoke(PS_COMMAND_UPLOAD_FILE, uploadCommand);
	printf("\n");
	if (nRetVal == STATUS_OK)
	{
		printf("File uploaded successfully\n\n");
		return 0;
	}
	else
	{
		printf("Failed to upload file '%s': %s\n\n", uploadCommand.filePath, OpenNI::getExtendedError());
		return nRetVal;
	}
}

int Dir(int /*argc*/, const char* /*argv*/[])
{
	XnFwFileEntry files[50];
	XnCommandGetFileList args;
	args.count = sizeof(files) / sizeof(files[0]);
	args.files = files;

	Status nRetVal = g_device.invoke(PS_COMMAND_GET_FILE_LIST, args);
	if (nRetVal != STATUS_OK)
	{
		printf("Failed to get file list: %s\n\n", OpenNI::getExtendedError());
		return nRetVal;
	}
	else
	{
		// print list
		printf("\n");
		printf("%-4s  %-32s  %-8s  %-10s  %-6s  %-6s  %-15s\n", "ZONE", "NAME", "VERSION", "ADDRESS", "SIZE", "CRC", "FLAGS");
		printf("%-4s  %-32s  %-8s  %-10s  %-6s  %-6s  %-15s\n", "====", "====", "=======", "=======", "====", "===", "=====");

		for (XnUInt32 i = 0; i < args.count; ++i)
		{
			XnFwFileEntry& file = files[i];
			printf("%-4u  %-32s  %01u.%01u.%01u.%02u  0x%08x  %6u  0x%04x  ", 
				file.zone, file.name, 
				file.version.major, file.version.minor, file.version.maintenance, file.version.build,
				file.address, file.size, file.crc);

			// flags
			if ((file.flags & XN_FILE_FLAG_BAD_CRC) != 0)
			{
				printf("CORRUPT");
			}

			printf("\n");
		}
		printf("\n");
	}

	return 0;
}

int Download(int argc, const char* argv[])
{
	if (argc < 4)
	{
		printf("Usage: %s <zone> <firmware file name> <host target file>\n", argv[0]);
		printf("Note: filename can not contain any spaces.\n\n");
		return -1;
	}

	XnCommandDownloadFile args;
	args.zone = (uint16_t)MyAtoi(argv[1]);
	args.firmwareFileName = argv[2];
	args.targetPath = argv[3];

	Status nRetVal = g_device.invoke(PS_COMMAND_DOWNLOAD_FILE, args);
	if (nRetVal != STATUS_OK)
	{
		printf("Failed to download file '%s' from zone %u: %s\n\n", args.firmwareFileName, args.zone, OpenNI::getExtendedError());
	}

	return nRetVal;
}

int PrintFirmwareVersion(int /*argc*/, const char* /*argv*/[])
{
	char strVersion[200];
	int size = sizeof(strVersion);
	Status nRetVal = g_device.getProperty(DEVICE_PROPERTY_FIRMWARE_VERSION, strVersion, &size);
	if (nRetVal != STATUS_OK)
	{
		printf("Failed to get firmware version!");
		return -1;
	}

	printf("FW version from device: %s\n\n", strVersion);
	return 0;
}

int DumpStream(int argc, const char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s <StreamID> <on|off>\n\n", argv[0]);
		return -1;
	}

	XnUInt16 nStreamID = (XnUInt16)MyAtoi(argv[1]);
	XnBool bDumpOn = (xnOSStrCaseCmp(argv[2], "on") == 0);
	if (STATUS_OK != g_device.setProperty(PS_PROPERTY_DUMP_DATA, bDumpOn))
	{
		printf("Failed to toggle dump for stream\n");
		return -2;
	}

	if (bDumpOn)
	{
		printf("Dumping stream %u to directory 'Log'\n\n", nStreamID);
	}
	else
	{
		printf("Stream %u dump is now off\n\n", nStreamID);
	}

	return 0;
}

int DumpEP(int argc, const char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s <EndpointID> <on|off>\n\n", argv[0]);
		return -1;
	}

	XnCommandDumpEndpoint args;

	args.endpoint = (uint8_t)MyAtoi(argv[1]);
	args.enabled = xnOSStrCaseCmp(argv[2], "on") == 0;

	Status nRetVal = g_device.invoke(PS_COMMAND_DUMP_ENDPOINT, args);
	if (nRetVal != STATUS_OK)
	{
		printf("Failed to set endpoint dump: %s\n", OpenNI::getExtendedError());
		return 1;
	}

	if (args.enabled)
	{
		printf("Dumping endpoint %u to directory 'Log'\n\n", args.endpoint);
	}
	else
	{
		printf("Endpoint %u dump is now off\n\n", args.endpoint);
	}

	return 0;
}

int EnumerateStreams(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <stream type|ALL>\n", argv[0]);
		printf("Enumerate available stream of a certain type, where stream type is one of the following:\n");
		printf("\tDepth, IR, Log, Image, User, Hands, Gestures, DY\n\n");
		return -1;
	}

	XnFwStreamType type = (XnFwStreamType)-1;
	if (xnOSStrCaseCmp(argv[1], "all") != 0)
	{
		type = fwStreamNameToType(argv[1]);
		if (type == (XnFwStreamType)-1)
		{
			printf("Bad stream type '%s'\n\n", argv[1]);
			return -2;
		}
	}

	XnFwStreamInfo streams[20];
	XnCommandGetFwStreamList args;
	args.count = sizeof(streams)/sizeof(streams[0]);
	args.streams = streams;
	Status nRetVal = g_device.invoke(LINK_COMMAND_GET_FW_STREAM_LIST, args);
	if (nRetVal == STATUS_OK)
	{
		int index = 0;
		for (uint32_t i = 0; i < args.count; ++i)
		{
			if (type == (XnFwStreamType)-1 || type == streams[i].type)
			{
				printf("\t[%u] stream type='%s', creationInfo='%s'\n", 
					index++,
					fwStreamTypeToName(streams[i].type), 
					streams[i].creationInfo);
			}
		}
	}
	else
	{
		printf("Failed to enumerate streams: %s\n", OpenNI::getExtendedError());
	}
	printf("\n");

	return nRetVal;
}

int CreateStream(int argc, const char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s <stream type> <creation info>\n", argv[0]);
		printf("Creates a stream, where node type and creation info are values returned from the Enum command.\n\n");
		return -1;
	}

	XnFwStreamType streamType = fwStreamNameToType(argv[1]);
	if (streamType == (XnFwStreamType)-1)
	{
		printf("Bad stream type '%s'.\n\n", argv[1]);
		return -2;
	}

	XnCommandCreateStream args;
	args.type = streamType;
	args.creationInfo = argv[2];

	Status nRetVal = g_device.invoke(LINK_COMMAND_CREATE_FW_STREAM, args);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully created stream of type %s with ID %u.\n\n", argv[1], args.id);
	}
	else
	{
		printf("Failed to create stream: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int DestroyStream(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <streamID>\n", argv[0]);
		printf("Destroys a stream, where stream ID is an ID returned from the Create command.\n");
		return -1;
	}

	XnCommandDestroyStream args;
	args.id = MyAtoi(argv[1]);
	Status nRetVal = g_device.invoke(LINK_COMMAND_DESTROY_FW_STREAM, args);
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully destroyed stream %u\n\n", args.id);
	}
	else
	{
		printf("Failed to destroy stream: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int StartStream(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <StreamID>\n\n", argv[0]);
		return -1;
	}

	XnCommandStartStream args;
	args.id = MyAtoi(argv[1]);
	Status nRetVal = g_device.invoke(LINK_COMMAND_START_FW_STREAM, args);
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully started stream %u\n\n", args.id);
	}
	else
	{
		printf("Failed to start stream %u: %s\n\n", args.id, OpenNI::getExtendedError());
	}

	return nRetVal;
}

int StopStream(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <StreamID>\n\n", argv[0]);
		return -1;
	}

	XnCommandStopStream args;
	args.id = MyAtoi(argv[1]);
	Status nRetVal = g_device.invoke(LINK_COMMAND_STOP_FW_STREAM, args);
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully stopped stream %u\n\n", args.id);
	}
	else
	{
		printf("Failed to stop stream %u: %s\n\n", args.id, OpenNI::getExtendedError());
	}

	return nRetVal;
}

int PrintModes(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <StreamID>\n", argv[0]);
		printf("Shows a list of supported map output modes for the specified stream.\n\n");
		return -1;
	}

	XnFwStreamVideoMode modes[50];
	XnCommandGetFwStreamVideoModeList args;
	args.videoModes = modes;
	args.count = sizeof(modes)/sizeof(modes[0]);
	args.streamId = MyAtoi(argv[1]);

	Status nRetVal = g_device.invoke(LINK_COMMAND_GET_FW_STREAM_VIDEO_MODE_LIST, args);
	if (nRetVal != STATUS_OK)
	{
		printf("Failed getting video modes list for stream %d: %s\n\n", args.streamId, OpenNI::getExtendedError());
		return -2;
	}

	printf("Got %u modes:\n", args.count);
	for (uint32_t i = 0; i < args.count; i++)
	{
		printf("\t[%u] %s\n", i, fwVideoModeToString(modes[i]));
	}
	printf("\n");

	return 0;
}

int SetMode(int argc, const char* argv[])
{
	if (argc < 7)
	{
		printf("Usage: %s <StreamID> <XRes> <YRes> <FPS> <format> <compression>\n", argv[0]);
		printf("Sets the video mode of the specified stream.\n");
		printf("Allowed formats: ");
		for (int i = 1; i <= XN_FW_PIXEL_FORMAT_BAYER8; ++i)
			printf("%s, ", fwPixelFormatToName((XnFwPixelFormat)i));
		printf("\n");
		printf("Allowed compressions: ");
		for (int i = 0; i <= XN_FW_COMPRESSION_12_BIT_PACKED; ++i)
			printf("%s, ", fwCompressionTypeToName((XnFwCompressionType)i));
		printf("\n\n");
		return -1;
	}

	XnCommandSetFwStreamVideoMode args;
	args.streamId = MyAtoi(argv[1]);
	args.videoMode.m_nXRes = MyAtoi(argv[2]);
	args.videoMode.m_nYRes = MyAtoi(argv[3]);
	args.videoMode.m_nFPS = MyAtoi(argv[4]);
	args.videoMode.m_nPixelFormat = fwPixelFormatNameToType(argv[5]);
	args.videoMode.m_nCompression = fwCompressionNameToType(argv[6]);

	Status nRetVal = g_device.invoke(LINK_COMMAND_SET_FW_STREAM_VIDEO_MODE, args);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully set video mode.\n\n");
	}
	else
	{
		printf("Failed to set video mode: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int PrintMode(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <StreamID>\n", argv[0]);
		printf("Shows the current map output mode of the specified stream.\n\n");
		return -1;
	}

	XnCommandSetFwStreamVideoMode args;
	args.streamId = MyAtoi(argv[1]);

	Status nRetVal = g_device.invoke(LINK_COMMAND_GET_FW_STREAM_VIDEO_MODE, args);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Video mode of stream %u: %s\n\n", args.streamId, fwVideoModeToString(args.videoMode));
	}
	else
	{
		printf("Failed to set video mode: %s\n\n", OpenNI::getExtendedError());
	}

	return 0;
}

xnl::Array<XnI2CDeviceInfo>& GetI2CDeviceList()
{
	static xnl::Array<XnI2CDeviceInfo> s_i2cDevices;

	if (s_i2cDevices.GetSize() == 0)
	{
		XnI2CDeviceInfo devices[20];

		XnCommandGetI2CDeviceList args;
		args.devices = devices;
		args.count = sizeof(devices)/sizeof(devices[0]);

		if (STATUS_OK != g_device.invoke(PS_COMMAND_GET_I2C_DEVICE_LIST, args))
		{
			printf("Failed getting device list: %s\n\n", OpenNI::getExtendedError());
		}
		else
		{
			s_i2cDevices.SetData(args.devices, args.count);
		}
	}

	return s_i2cDevices;
}

XnStatus GetI2CDeviceIDFromName(const char* deviceName, uint32_t* result)
{
	XnStatus nRetVal = XN_STATUS_NO_MATCH;
	
	xnl::Array<XnI2CDeviceInfo>& devices = GetI2CDeviceList();

	nRetVal = XN_STATUS_NO_MATCH;
	for (XnUInt32 i = 0; i < devices.GetSize() && nRetVal==XN_STATUS_NO_MATCH; i++)
	{
		if(xnOSStrCaseCmp(devices[i].name, deviceName) == 0)
		{
			nRetVal = XN_STATUS_OK;
			*result = (XnUInt8)devices[i].id;
		}
	}

	return nRetVal;
}

int WriteI2C(int argc, const char* argv[])
{
	if (argc < 6)
	{
		printf("Usage: %s <Device ID or Name> <AddressSize> <Address> <ValueSize> <Value> [Mask]\n", argv[0]);
		printf("Note - each parameter may be in hex, indicated by an '0x' prefix.\n\n");
		return -1;
	}

	XnCommandI2C args;

	//Try to get parse ID by name, and then by number
	args.deviceID = 0;
	if (GetI2CDeviceIDFromName(argv[1], &args.deviceID) == XN_STATUS_NO_MATCH)
		args.deviceID = MyAtoi(argv[1]);

	args.addressSize = MyAtoi(argv[2]);
	args.address = MyAtoi(argv[3]);
	args.valueSize = MyAtoi(argv[4]);
	args.value = MyAtoi(argv[5]);
	args.mask = (argc > 6) ? MyAtoi(argv[6]) : 0xFFFFFFFF;

	Status nRetVal = g_device.invoke(PS_COMMAND_I2C_WRITE, args);
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully written I2C value.\n\n");
	}
	else
	{
		printf("Failed to write I2C value: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int ReadI2C(int argc, const char* argv[])
{
	if (argc < 5)
	{
		printf("Usage: %s <Device ID or Name> <AddressSize> <Address> <ValueSize>\n", argv[0]);
		printf("Note - each parameter may be in hex, indicated by an '0x' prefix.\n\n");
		return -1;
	}

	XnCommandI2C args;

	//Try to get parse ID by name, and then by number
	args.deviceID = 0;
	if (GetI2CDeviceIDFromName(argv[1], &args.deviceID) == XN_STATUS_NO_MATCH)
		args.deviceID = MyAtoi(argv[1]);

	args.addressSize = MyAtoi(argv[2]);
	args.address = MyAtoi(argv[3]);
	args.valueSize = MyAtoi(argv[4]);
	args.value = 0;

	Status nRetVal = g_device.invoke(PS_COMMAND_I2C_READ, args);
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully read I2C value: %u (0x%X)\n\n", args.value, args.value);
	}
	else
	{
		printf("Failed to read I2C value: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int WriteAHB(int argc, const char* argv[])
{
	if (argc < 5)
	{
		printf("Usage: %s <Address> <Value> <BitOffset> <BitWidth>\n", argv[0]);
		printf("Note - each parameter may be in hex, indicated by an '0x' prefix.\n\n");
		return -1;
	}

	XnCommandAHB args;
	args.address = MyAtoi(argv[1]);
	args.value = MyAtoi(argv[2]);
	args.offsetInBits = MyAtoi(argv[3]);
	args.widthInBits = MyAtoi(argv[4]);

	Status nRetVal = g_device.invoke(PS_COMMAND_AHB_WRITE, args);
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully written AHB value.\n\n");
	}
	else
	{
		printf("Failed to write AHB value: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int ReadAHB(int argc, const char* argv[])
{
	if (argc < 4)
	{
		printf("Usage: %s <Address> <BitOffset> <BitWidth>\n", argv[0]);
		printf("Note - each parameter may be in hex, indicated by an '0x' prefix.\n\n");
		return -1;
	}

	XnCommandAHB args;
	args.address = MyAtoi(argv[1]);
	args.offsetInBits = MyAtoi(argv[2]);
	args.widthInBits = MyAtoi(argv[3]);

	Status nRetVal = g_device.invoke(PS_COMMAND_AHB_READ, args);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully read AHB value: %u (0x%X)\n\n", args.value, args.value);
	}
	else
	{
		printf("Failed to read I2C value: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int SoftReset(int /*argc*/, const char* /*argv*/[])
{
	printf("Resetting device...");
	Status nRetVal = g_device.invoke(PS_COMMAND_SOFT_RESET, NULL, 0);
	printf("\n");
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully executed Soft Reset.\n\n");
	}
	else
	{
		printf("Failed to execute soft reset: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int HardReset(int /*argc*/, const char* /*argv*/[])
{
	printf("Resetting device...");
	Status nRetVal = g_device.invoke(PS_COMMAND_POWER_RESET, NULL, 0);
	printf("\n");
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully executed Hard Reset.\n\n");
		printf("**********************************************\n");
		printf("Warning: console must be restarted!\n");
		printf("**********************************************\n\n");
	}
	else
	{
		printf("Failed to execute hard reset: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}
// Retrieve data from firmware according to the command number received from user
int ReadDebugData(int argc, const char* argv[])
{

    XnStatus nRetVal = XN_STATUS_OK;
    XnCommandDebugData commandDebugData = {0};
    if (argc < 2)
    {
        printf("\nUsage: %s <Command ID> \n\n", argv[0]);
        return -1;
    }
    
    XnUInt8 debugData[1024];
    commandDebugData.dataID = (uint16_t)MyAtoi(argv[1]);
    commandDebugData.dataSize = sizeof(debugData);
    commandDebugData.data = debugData;

    nRetVal = g_device.invoke(PS_COMMAND_READ_DEBUG_DATA, &commandDebugData, sizeof(commandDebugData));
    if (nRetVal == STATUS_OK)
    {
        printf("\nCommand: %x Data size: %d \nData:" ,commandDebugData.dataID, commandDebugData.dataSize);
        for(int i = 0; i < commandDebugData.dataSize; i++)
        {
            if(i % 8 == 0)
            {
                printf("\n");
            }
            printf("%02x ",commandDebugData.data[i]);
        }
        printf("\n\n");
    }
    else
    {
        printf("Failed to retrieve data: %s\n\n", OpenNI::getExtendedError());
    }

    return nRetVal;
}

// Enables/Disables the BIST (XN_LINK_PROP_ID_ACC_ENABLED)
int Acc(int argc, const char* argv[])
{
    XnBool bAccEnabled;
    Status nRetVal;
    if(argc == 1)
    {
        nRetVal = g_device.getProperty(LINK_PROP_ACC_ENABLED, &bAccEnabled);
        if (nRetVal == STATUS_OK)
        {
            printf("Acc is %s.\n\n", (bAccEnabled ? "on" : "off"));
        }
        else
        {
            printf("Failed to get Acc: %s\n\n", OpenNI::getExtendedError());
        }

        return nRetVal;
    }

    if((xnOSStrCaseCmp(argv[1], "on") != 0) && (xnOSStrCaseCmp(argv[1], "off") != 0))
    {
        printf("Usage: %s <on|off>\n\n", argv[0]);
        return -1;
    }
	const XnChar* strAccActive = argv[1];
	bAccEnabled = (xnOSStrCaseCmp(strAccActive, "on") == 0);
	nRetVal = g_device.setProperty(LINK_PROP_ACC_ENABLED, bAccEnabled);
	if (nRetVal == STATUS_OK)
	{
		printf("Acc is now %s.\n\n", strAccActive);
	}
	else
	{
		printf("Failed to set Acc %s: %s\n\n", strAccActive, OpenNI::getExtendedError());
	}

	return nRetVal;
}
// Enables/Disables the VDD - Valid Depth Detect (XN_LINK_PROP_ID_VDD_ENABLED) 
//on - Safety mechanism is on | off - reduce power
int VDD(int argc, const char* argv[])
{
    XnBool bAccEnabled;
    Status nRetVal;
    if(argc == 1)
    {
        nRetVal = g_device.getProperty(LINK_PROP_VDD_ENABLED, &bAccEnabled);
        if (nRetVal == STATUS_OK)
        {
            printf("VDD is %s.\n\n", (bAccEnabled ? "on" : "off"));
        }
        else
        {
            printf("Failed to get VDD: %s\n\n", OpenNI::getExtendedError());
        }

        return nRetVal;
    }

    if((xnOSStrCaseCmp(argv[1], "on") != 0) && (xnOSStrCaseCmp(argv[1], "off") != 0))
    {
        printf("Usage: %s <on|off>\n\n", argv[0]);
        return -1;
    }
	const XnChar* strAccActive = argv[1];
	bAccEnabled = (xnOSStrCaseCmp(strAccActive, "on") == 0);
	nRetVal = g_device.setProperty(LINK_PROP_VDD_ENABLED, bAccEnabled);
	if (nRetVal == STATUS_OK)
	{
		printf("VDD is now %s.\n\n", strAccActive);
	}
	else
	{
		printf("Failed to set VDD %s: %s\n\n", strAccActive, OpenNI::getExtendedError());
	}

	return nRetVal;
}

// Enables/Disables the Periodic BIST (XN_LINK_PROP_ID_PERIODIC_BIST_ENABLED)
int PeriodicBist(int argc, const char* argv[])
{
    XnBool bAccEnabled;
    Status nRetVal;
    if(argc == 1)
    {
        nRetVal = g_device.getProperty(LINK_PROP_PERIODIC_BIST_ENABLED, &bAccEnabled);
        if (nRetVal == STATUS_OK)
        {
            printf("Periodic BIST is %s.\n\n", (bAccEnabled ? "on" : "off"));
        }
        else
        {
            printf("Failed to get Periodic BIST: %s\n\n", OpenNI::getExtendedError());
        }

        return nRetVal;
    }

    if((xnOSStrCaseCmp(argv[1], "on") != 0) && (xnOSStrCaseCmp(argv[1], "off") != 0))
    {
        printf("Usage: %s <on|off>\n\n", argv[0]);
        return -1;
    }
	const XnChar* strAccActive = argv[1];
	bAccEnabled = (xnOSStrCaseCmp(strAccActive, "on") == 0);
	nRetVal = g_device.setProperty(LINK_PROP_PERIODIC_BIST_ENABLED, bAccEnabled);
	if (nRetVal == STATUS_OK)
	{
		printf("Periodic BIST is now %s.\n\n", strAccActive);
	}
	else
	{
		printf("Failed to set Periodic BIST %s: %s\n\n", strAccActive, OpenNI::getExtendedError());
	}

	return nRetVal;
}
xnl::Array<XnFwLogMask>& GetLogMaskList()
{
	static xnl::Array<XnFwLogMask> s_masks;

	if (s_masks.GetSize() == 0)
	{
		XnFwLogMask masks[20];
		XnCommandGetLogMaskList args;
		args.masks = masks;
		args.count = sizeof(masks)/sizeof(masks[0]);

		if (STATUS_OK != g_device.invoke(PS_COMMAND_GET_LOG_MASK_LIST, args))
		{
			printf("Failed getting masks list: %s\n\n", OpenNI::getExtendedError());
		}
		else
		{
			s_masks.SetData(args.masks, args.count);
		}
	}

	return s_masks;
}

XnStatus GetLogIDFromName(const char* mask, uint32_t* result)
{
	XnStatus nRetVal = XN_STATUS_NO_MATCH;

	xnl::Array<XnFwLogMask>& masks = GetLogMaskList();

	nRetVal = XN_STATUS_NO_MATCH;
	for (XnUInt32 i = 0; i < masks.GetSize() && nRetVal==XN_STATUS_NO_MATCH; i++)
	{
		if(xnOSStrCaseCmp(masks[i].name, mask) == 0)
		{
			nRetVal = XN_STATUS_OK;
			*result = (XnUInt8)masks[i].id;
		}
	}

	return nRetVal;
}

int Log(int argc, const char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;
	bool isOnOffCommnad = (argc == 2) && ((xnOSStrCaseCmp(argv[1], "on") == 0) || (xnOSStrCaseCmp(argv[1], "off") == 0));
	bool isStartStopCommnad = (argc == 3) && ((xnOSStrCaseCmp(argv[1], "open") == 0) || (xnOSStrCaseCmp(argv[1], "close") == 0));
	
	if (!isOnOffCommnad && !isStartStopCommnad)
	{
		printf("Usage: %s <on|off> or <open|close> <stream name or id>\n\n", argv[0]);
		return -1;
	}

	if (isStartStopCommnad)
	{
		//Try to get parse ID by name, and then by number
		uint32_t logID = 0;
		if (GetLogIDFromName(argv[1], &logID) == XN_STATUS_NO_MATCH)
		{
			logID = (XnUInt8)MyAtoi(argv[2]);
		}

		//Start command
		XnCommandSetLogMaskState args;
		args.mask = logID;
		args.enabled = xnOSStrCaseCmp(argv[1], "open") == 0;
		if (STATUS_OK == g_device.invoke(PS_COMMAND_SET_LOG_MASK_STATE, args))
		{
			printf("Sent %s command for log #%d", argv[1], (int)logID);
		}
		else
		{
			printf("Failed to send %s command for log #%d: %s\n\n", argv[1], (int)logID, OpenNI::getExtendedError());
		}
	}
	//on/off command
	else
	{
		const XnChar* strLogOn = argv[1];
		XnBool bLogOn = (xnOSStrCaseCmp(strLogOn, "on") == 0);

		if (bLogOn)
		{
			nRetVal = g_device.invoke(PS_COMMAND_START_LOG, NULL, 0);
		}
		else
		{
			nRetVal = g_device.invoke(PS_COMMAND_STOP_LOG, NULL, 0);
		}

		if (nRetVal == STATUS_OK)
		{
			if (bLogOn)
			{
				printf("Saving firmware log to 'Log'\n\n");
			}
			else
			{
				printf("Firmware log is now off\n\n");
			}
		}
		else
		{
			printf("Failed to set log %s: %s\n\n", strLogOn, OpenNI::getExtendedError());
		}
	}

	return nRetVal;
}

int Script(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <fileName>\n\n", argv[0]);
		return -1;
	}

	return RunScript(argv[1]);
}

int PrintBootStatus(int /*argc*/, const char* /*argv*/[])
{
	XnBootStatus bootStatus;
	Status nRetVal = g_device.getProperty(LINK_PROP_BOOT_STATUS, &bootStatus);
	if (nRetVal != STATUS_OK)
	{
		printf("Failed to get boot status: %s\n\n", OpenNI::getExtendedError());
		return nRetVal;
	}
	else
	{
		enum{MAX_STR_LEN = 256};
		char bootZoneStr[MAX_STR_LEN], bootZoneErr[MAX_STR_LEN];

		//Helper macro to write enum names
		#define CASE_ENUM_TOSTRING(enumValue, enumStr, targetStr) \
				case enumValue: sprintf(targetStr, "%s", enumStr); break;

		//Get XnLinkBootZone string
		switch(bootStatus.zone){
			CASE_ENUM_TOSTRING(XN_ZONE_FACTORY, "FACTORY_ZONE", bootZoneStr)
			CASE_ENUM_TOSTRING(XN_ZONE_UPDATE, "UPDATE_ZONE", bootZoneStr)
			default:
				sprintf(bootZoneStr, "Unexpected - %d",bootStatus.zone);
		}

		//Get XnLinkBootErrorCode string
		switch(bootStatus.errorCode){
			CASE_ENUM_TOSTRING(XN_BOOT_OK, "BOOT_OK", bootZoneErr)
			CASE_ENUM_TOSTRING(XN_BOOT_BAD_CRC, "BAD_CRC", bootZoneErr)
			CASE_ENUM_TOSTRING(XN_BOOT_UPLOAD_IN_PROGRESS, "UPLOAD_IN_PROGRESS", bootZoneErr)
			CASE_ENUM_TOSTRING(XN_BOOT_FW_LOAD_FAILED, "FW_LOAD_FAILED", bootZoneErr)
			default:
				sprintf(bootZoneErr, "Unexpected - %d",bootStatus.errorCode);
		}

		printf("Zone: %s\nError code: %s\n\n", bootZoneStr, bootZoneErr);

		return nRetVal;
	}
}

int PrintLogFilesList(int /*argc*/, const char* /*argv*/[])
{
	xnl::Array<XnFwLogMask>& masks = GetLogMaskList();

	for (XnUInt32 i = 0; i < masks.GetSize(); ++i)
	{
		printf("%4u %s\n", masks[i].id, masks[i].name);
	}

	printf("\n");

	return 0;
}

int PrintI2CList(int /*argc*/, const char* /*argv*/[])
{
	xnl::Array<XnI2CDeviceInfo>& deviceList = GetI2CDeviceList();
    
    printf("%2s %32s %9s %8s", "ID", "NAME", "MASTER-ID", "SLAVE-ID\n");
    printf("%2s %32s %9s %8s", "==", "====", "=========", "========\n");

	for (XnUInt32 i = 0; i < deviceList.GetSize(); ++i)
	{
        printf("%2u %32s %9u %8u\n", deviceList[i].id, deviceList[i].name, deviceList[i].masterId, deviceList[i].slaveId);
	}

	return 0;
}

int PrintBistList(int /*argc*/, const char* /*argv*/[])
{
	XnBistInfo tests[20];
	XnCommandGetBistList args;
	args.tests = tests;
	args.count = sizeof(tests)/sizeof(tests[0]);

	if (STATUS_OK != g_device.invoke(PS_COMMAND_GET_BIST_LIST, args))
	{
		printf("Failed getting tests list: %s\n\n", OpenNI::getExtendedError());
		return 1;
	}
	else
	{
		for (XnUInt32 i = 0; i < args.count; ++i)
		{
			printf("%4u %s\n", tests[i].id, tests[i].name);
		}

		printf("\n");
	}

	return 0;
}
int RunBist(int argc, const char* argv[])
{
    Status nRetVal = STATUS_OK;

    if (argc < 2)
    {
        printf("Usage: %s ALL | <Test>...\n\n", argv[0]);
        return -1;
    }

    XnBistInfo bistInfos[20];
    XnCommandGetBistList supportedTests;
    supportedTests.tests = bistInfos;
    supportedTests.count = sizeof(bistInfos)/sizeof(bistInfos[0]);

    if (STATUS_OK != g_device.invoke(PS_COMMAND_GET_BIST_LIST, supportedTests))
    {
        printf("Failed getting tests list: %s\n\n", OpenNI::getExtendedError());
        return -2;
    }

    xnl::BitSet requestedTests;

    if (xnOSStrCaseCmp(argv[1], "ALL") == 0)
    {
        for (XnUInt32 i = 0; i < supportedTests.count; ++i)
        {
            requestedTests.Set(supportedTests.tests[i].id, TRUE);
        }
    }
    else
    {
        for (int i = 1; i < argc; ++i)
        {
            requestedTests.Set(MyAtoi(argv[i]), TRUE);
        }
    }

    XnUInt8 response[512];
    XnCommandExecuteBist args;
    args.extraData = response;

    for (XnUInt32 i = 0; i < requestedTests.GetSize(); ++i)
    {
        if (!requestedTests.IsSet(i))
        {
            continue;
        }

        // search for test in list (for its name)
        const XnChar* testName = "Unknown";
        for (XnUInt32 j = 0; j < supportedTests.count; ++j)
        {
            if (supportedTests.tests[j].id == i)
            {
                testName = supportedTests.tests[j].name;
                break;
            }
        }

        printf("Executing test %u (%s)...\n", i, testName);

        args.id = i;
        args.extraDataSize = sizeof(response);
        nRetVal = g_device.invoke(PS_COMMAND_EXECUTE_BIST, args);
        if (nRetVal != STATUS_OK)
        {
            printf("\nFailed to execute: %s\n\n", OpenNI::getExtendedError());
            return nRetVal;
        }

        printf("Test %u (%s) ", i, testName);

        if (args.errorCode != 0)
        {
            printf("Failed (error code 0x%04X).", args.errorCode);
        }
        else
        {
            printf("Passed.");
        }

        printf("\n");

        // extra data
        if (args.extraDataSize > 0)
        {
            printf("Extra Data: ");
            for (XnUInt32 j = 0; j < args.extraDataSize; ++j)
            {
                printf("%02X ", args.extraData[j]);
            }
            printf("\n");
        }
    }

    printf("\n");

    return (XN_STATUS_OK);
}
//Prints option list in which the user can choose to get the temperature
int PrintTempList(int /*argc*/, const char* /*argv*/[])
{
    XnTempInfo tempInfo[20];
    XnCommandGetTempList args;
    args.pTempInfos = tempInfo;
    args.count = sizeof(tempInfo)/sizeof(tempInfo[0]);

    if (STATUS_OK != g_device.invoke(PS_COMMAND_GET_TEMP_LIST, args)) 
    {
        printf("Failed getting Temperature list: %s\n\n", OpenNI::getExtendedError());
        return 1;
    }
    else
    {
        for (XnUInt32 i = 0; i < args.count; ++i)
        {
            printf("%4u %s\n", tempInfo[i].id, tempInfo[i].name);
        }

        printf("\n");
    }

    return 0;
}
int ReadTemps(int argc, const char* argv[])
{
	Status nRetVal = STATUS_OK;

	if (argc < 2)
	{
		printf("Usage: %s ALL | <Sensor ID> | <Sensor name>...\n\n", argv[0]);
		return -1;
	}

	XnTempInfo TempInfos[20];
	XnCommandGetTempList supportedTempList;
	supportedTempList.pTempInfos = TempInfos;
	supportedTempList.count = sizeof(TempInfos)/sizeof(TempInfos[0]);

	if (STATUS_OK != g_device.invoke(PS_COMMAND_GET_TEMP_LIST, supportedTempList))
	{
		printf("Failed getting Temperature list: %s\n\n", OpenNI::getExtendedError());
		return -2;
	}


	XnCommandTemperatureResponse response;
	if (xnOSStrCaseCmp(argv[1], "ALL") == 0)
	{
		for (XnUInt32 i = 0; i < supportedTempList.count; ++i)
		{
            response.id = supportedTempList.pTempInfos[i].id;
            nRetVal = g_device.invoke(PS_COMMAND_READ_TEMPERATURE, response);
            if(nRetVal != STATUS_OK)
            {
                printf("Failed getting Temperature data for id %d: %s\n\n", response.id, OpenNI::getExtendedError());
            }
            else
            {
                printf("%s \t Temperature: %f \n\n", supportedTempList.pTempInfos[i].name, response.temperature);
            }
		}
	}
	else
	{ 
        XnInt32 argInt= (argv[1][0] >= '0' && argv[1][0] <= '9') ? MyAtoi(argv[1]) : -1;   
        for (XnUInt32 i = 0; i < supportedTempList.count; ++i)
        {
            if ((xnOSStrCaseCmp(argv[1],supportedTempList.pTempInfos[i].name) == 0) 
                || (argInt == (XnInt32)supportedTempList.pTempInfos[i].id) )
            {
                response.id = supportedTempList.pTempInfos[i].id;
                nRetVal = g_device.invoke(PS_COMMAND_READ_TEMPERATURE, response);
                if(nRetVal != STATUS_OK)
                {
                    printf("Failed getting Temperature data for id %d: %s\n\n", response.id, OpenNI::getExtendedError());
                }
                else
                {
                    printf("%s \t Temperature: %f \n\n", supportedTempList.pTempInfos[i].name, response.temperature);
                }
                break;
            }
        }
	}

	return (XN_STATUS_OK);
}


int Quit(int /*argc*/, const char* /*argv*/[])
{
	g_continue = FALSE;
	return 0;
}

int UsbInterface(int argc, const char* argv[])
{
	Status nRetVal = STATUS_OK;

	if (argc == 1)
	{
		XnUsbInterfaceType type;
		nRetVal = g_device.getProperty(PS_PROPERTY_USB_INTERFACE, &type);
		if (nRetVal != STATUS_OK)
		{
			printf("Failed to get interface: %s\n\n", OpenNI::getExtendedError());
			return -3;
		}

		for (size_t i = 0; i < sizeof(g_usbInterfaceNames)/sizeof(g_usbInterfaceNames[0]); ++i)
		{
			if (g_usbInterfaceNames[i].type == type)
			{
				printf("Current USB alternative interface is %s (%d)\n\n", g_usbInterfaceNames[i].name, type);
				return 0;
			}
		}

		printf("Unknown USB interface: %d\n\n", type);
		return -4;
	}
	else if (argc == 2)
	{
		for (size_t i = 0; i < sizeof(g_usbInterfaceNames)/sizeof(g_usbInterfaceNames[0]); ++i)
		{
			if (xnOSStrCaseCmp(g_usbInterfaceNames[i].name, argv[1]) == 0)
			{
				nRetVal = g_device.setProperty(PS_PROPERTY_USB_INTERFACE, g_usbInterfaceNames[i].type);
				if (nRetVal != STATUS_OK)
				{
					printf("Failed to set interface: %s\n\n", OpenNI::getExtendedError());
					return -3;
				}
				else
				{
					return 0;
				}
			}
		}
	}
	else
	{
		printf("Usage: %s [ANY|ISO|BULK]\n\n", argv[0]);
		return -4;
	}

	return 0;
}

int FormatZone(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <Zone>\n", argv[0]);
		printf("Note - each parameter may be in hex, indicated by an '0x' prefix.\n\n");
		return -1;
	}

	XnUInt32 nZone = MyAtoi(argv[1]);
	XnCommandFormatZone formatZone;
	formatZone.zone = (uint8_t)nZone;
	Status nRetVal = g_device.invoke(PS_COMMAND_FORMAT_ZONE, formatZone);
	if (nRetVal == STATUS_OK)
	{
		printf("Successfully formatZone.\n\n");
	}
	else
	{
		printf("Failed to format Zone value: %s\n\n", OpenNI::getExtendedError());
	}

	return nRetVal;
}

int UsbTest(int argc, const char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (argc < 2)
	{
		printf("Usage: %s <seconds>\n\n", argv[0]);
		return -1;
	}

	XnUsbTestEndpointResult endpoints[10];
	XnCommandUsbTest args;
	args.seconds = MyAtoi(argv[1]);
	args.endpointCount = sizeof(endpoints)/sizeof(endpoints[0]);
	args.endpoints = endpoints;

	nRetVal = g_device.invoke(PS_COMMAND_USB_TEST, args);
	if (nRetVal != STATUS_OK)
	{
		printf("Failed to perform USB test: %s\n\n", OpenNI::getExtendedError());
		return -3;
	}

	printf("USB Test done:\n");
	for (XnUInt32 i = 0; i < args.endpointCount; ++i)
	{
		printf("\tEndpoint %u - Avg. Bandwidth: %.3f KB/s, Lost Packets: %u\n", i, args.endpoints[i].averageBytesPerSecond / 1000., args.endpoints[i].lostPackets);
	}

	return XN_STATUS_OK;
}

int TestAll(int /*argc*/, const char* /*argv*/[])
{
	XnChar strCommand[256];
	xnOSStrCopy(strCommand, "Bist ALL", sizeof(strCommand));
	RunCommand(strCommand);
	xnOSStrCopy(strCommand, "UsbTest 1", sizeof(strCommand));
	RunCommand(strCommand);

	return XN_STATUS_OK;
}

int Projector(int argc, const char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (argc > 1)
	{
        //Projector <on/off> command
        XnBool bProjectorActive = (xnOSStrCaseCmp(argv[1], "on") == 0);
        if((bProjectorActive || xnOSStrCaseCmp(argv[1], "off") == 0))
        {
            Status nRetVal = g_device.setProperty(LINK_PROP_PROJECTOR_ACTIVE, bProjectorActive);
            if (nRetVal == STATUS_OK)
            {
                printf("Projector is now %s.\n\n", argv[1]);
            }
            else
            {
                printf("Failed to set Projector %s: %s\n\n", argv[1], OpenNI::getExtendedError());
            }

            return nRetVal;
        }

		if (xnOSStrCaseCmp(argv[1], "power") == 0)
		{
			if (argc > 2)
			{
				// set power
				XnUInt16 power = (XnUInt16)MyAtoi(argv[2]);
				nRetVal = g_device.setProperty(LINK_PROP_PROJECTOR_POWER, power);
				if (nRetVal != XN_STATUS_OK)
				{
					printf("Failed to set projector power: %s\n\n", xnGetStatusString(nRetVal));
					return -2;
				}

				return 0;
			}
			else
			{
				// get power
				XnUInt16 power;
				nRetVal = g_device.getProperty(LINK_PROP_PROJECTOR_POWER, &power);
				if (nRetVal != XN_STATUS_OK)
				{
					printf("Failed to get projector power: %s\n\n", xnGetStatusString(nRetVal));
					return -3;
				}

				printf("Projector power is %u\n\n", power);
				return 0;
			}
		}
		else if (xnOSStrCaseCmp(argv[1], "pulse") == 0)
		{
			if (argc > 2)
			{
				if (xnOSStrCaseCmp(argv[2], "on") == 0 && argc > 5)
				{
					XnCommandSetProjectorPulse args;
					args.delay = (float)atof(argv[3]);
					args.cycle = (float)atof(argv[5]);
					args.width = (float)atof(argv[4]); //where both delay and width are in ms, and can be float

					nRetVal = g_device.invoke(LINK_COMMAND_SET_PROJECTOR_PULSE, args);
					if (nRetVal != XN_STATUS_OK)
					{
						printf("Failed to set projector pulse: %s\n\n", xnGetStatusString(nRetVal));
						return -3;
					}

					printf("Projector pulse set\n\n");
					return 0;
				}
				else if (xnOSStrCaseCmp(argv[2], "off") == 0)
				{
					nRetVal = g_device.invoke(LINK_COMMAND_DISABLE_PROJECTOR_PULSE, NULL, 0);
					if (nRetVal != XN_STATUS_OK)
					{
						printf("Failed to disable projector pulse: %s\n\n", xnGetStatusString(nRetVal));
						return -3;
					}

					printf("Projector pulse disabled\n\n");
					return 0;
				}
			}
		}
	}

	// if we got here, something was wrong with the arguments
	printf("Usage: \n");
    printf("\t%s <on/off>\n", argv[0]);
	printf("\t%s power [newVal]\n", argv[0]);
	printf("\t%s pulse on <start> <cycle> <DC>\n", argv[0]);
	printf("\t%s pulse off\n", argv[0]);
	printf("\n");
	return -1;
}

void RegisterCommands()
{
	RegisterCommand("Help", Help);
//	RegisterCommand("Versions", ComponentsVersions);
	RegisterCommand("BeginUpload", BeginUpload);
	RegisterCommand("Upload", Upload);
	RegisterCommand("EndUpload", EndUpload);
	RegisterCommand("Dir", Dir);
	RegisterCommand("Download", Download);
	RegisterCommand("FWVersion", PrintFirmwareVersion);
	RegisterCommand("BootStatus", PrintBootStatus);
	RegisterCommand("DumpStream", DumpStream);
	RegisterCommand("DumpEP", DumpEP);
	RegisterCommand("Enum", EnumerateStreams);
 	RegisterCommand("Create", CreateStream);
	RegisterCommand("Destroy", DestroyStream);
	RegisterCommand("Start", StartStream);
	RegisterCommand("Stop", StopStream);
 	RegisterCommand("Modes", PrintModes);
 	RegisterCommand("SetMode", SetMode);
 	RegisterCommand("GetMode", PrintMode);
	RegisterCommand("I2CList", PrintI2CList);
	RegisterCommand("WriteI2C", WriteI2C);
	RegisterCommand("ReadI2C", ReadI2C);
	RegisterCommand("WriteAHB", WriteAHB);
	RegisterCommand("ReadAHB", ReadAHB);
	RegisterCommand("SoftReset", SoftReset);
    RegisterCommand("HardReset", HardReset);
    RegisterCommand("ReadDebugData", ReadDebugData);
    RegisterCommand("Acc", Acc);
    RegisterCommand("VDD", VDD);
    RegisterCommand("PeriodBist", PeriodicBist);
	RegisterCommand("Log", Log);
	RegisterCommand("LogList", PrintLogFilesList);
	RegisterCommand("Script", Script);
	RegisterCommand("BistList", PrintBistList);
    RegisterCommand("Bist", RunBist);
    RegisterCommand("TempList", PrintTempList);
    RegisterCommand("Temp", ReadTemps);
	RegisterCommand("UsbInterface", UsbInterface);
	RegisterCommand("FormatZone", FormatZone);
	RegisterCommand("UsbTest", UsbTest);
	RegisterCommand("TestAll", TestAll);
	RegisterCommand("Projector", Projector);
	RegisterCommand("Quit", Quit);
	RegisterCommand("Bye", Quit);
	RegisterCommand("Exit", Quit);
}

int main(int argc, char* argv[])
{
	Status nRetVal = STATUS_OK;
	const XnChar* strScriptFile = NULL;
	XnUInt16 nProductID = 0;
	const char** commandArgv = NULL;
	int commandArgc = 0;

//	printf("PSLinkConsole version %s\n", XN_PS_VERSION_STRING);

	XnInt32 nArgIndex = 1;
	while (nArgIndex < argc)
	{
		if (argv[nArgIndex][0] == '-')
		{
			if (xnOSStrCaseCmp(argv[nArgIndex], "-product") == 0)
			{
				++nArgIndex;
				nProductID = (XnUInt16)MyAtoi(argv[nArgIndex++]);
			}
			else if (xnOSStrCaseCmp(argv[nArgIndex], "-script") == 0)
			{
				++nArgIndex;
				strScriptFile = argv[nArgIndex++];
			}
			else if (xnOSStrCaseCmp(argv[nArgIndex], "-help") == 0)
			{
				printf("USAGE\n");
				printf("\t%s [-product <PID>] [-script <fileName>] [-help] [command]\n", argv[0]);
				printf("OPTIONS\n");
				printf("\t-product <PID>\n");
				printf("\t\tOpen only devices with a specific product ID. By default, any device can be opened.\n");
				printf("\t-script <filename>\n");
				printf("\t\tRun a script file once connected.\n");
				printf("\t-help\n");
				printf("\t\tDisplay this information.\n");
				return 0;
			}
			else
			{
				printf("Unknown option: %s\n. Run %s -help for usage.\n", argv[nArgIndex], argv[0]);
				return -1;
			}
		}
		else
		{
			commandArgc = argc - nArgIndex;
			commandArgv = (const char**)(argv + nArgIndex);
			break;
		}
	}

	OpenNI::setLogConsoleOutput(true);
	OpenNI::setLogMinSeverity(0);

	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		printf("Failed to initialize OpenNI. Extended info: %s\n", OpenNI::getExtendedError());
		return -2;
	}

	XnUInt32 nWaitTimeRemaining = WAIT_FOR_DEVICE_TIMEOUT;

	Array<DeviceInfo> devices;
	OpenNI::enumerateDevices(&devices);

	const char* uri = NULL;

	while (uri == NULL && nWaitTimeRemaining > 0)
	{
		nWaitTimeRemaining = XN_MAX(0, (XnInt32)(nWaitTimeRemaining - WAIT_FOR_DEVICE_CHECK_INTERVAL_MS));
		
		// check if the requested device is connected
		for (int i = 0; i < devices.getSize(); ++i)
		{
			if (nProductID == 0 || devices[i].getUsbProductId() == nProductID)
			{
				uri = devices[i].getUri();
				break;
			}
		}

		if (uri == NULL)
		{
			xnOSSleep(WAIT_FOR_DEVICE_CHECK_INTERVAL_MS);
		}
	}

	if (uri == NULL)
	{
		printf("Device not found (after %u milliseconds)\n", WAIT_FOR_DEVICE_TIMEOUT);
		XN_ASSERT(FALSE);
		return -3;
	}

	printf("Device found, connecting...\n");

	nRetVal = g_device._openEx(uri, "lr");
	if (nRetVal != STATUS_OK)
	{
		printf("Failed to open device. Extended info: %s\n", OpenNI::getExtendedError());
		return -4;
	}

    //Prime Client is now connected :)

	RegisterCommands();

	if (strScriptFile != NULL)
	{
		if (XN_STATUS_OK != RunScript(strScriptFile))
		{
			// error is returned only if script could not be run, not if any command in it failed.
			return -5;
		}

        return 0;
	}
	else if (commandArgc != 0)
	{
		RunCommand(commandArgc, commandArgv);
		return 0;
	}
	else
	{
		ExecuteCommandsFromStream(stdin, TRUE);
	}

	g_device.close();
	OpenNI::shutdown();

	return 0;
}
