// PSLinkConsole.cpp : Defines the entry point for the console application.
//

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "PrimeClient.h"
#include "LenaDevice.h"
#include "PS1200Device.h"
#include "XnLinkInputStream.h"
#include "XnLinkFrameInputStream.h"
#include "XnPsVersion.h"
#include "IConnectionFactory.h"
#include "XnClientUSBConnectionFactory.h"
#include "XnSocketConnectionFactory.h"
#include <XnStringsHash.h>
#include <XnList.h>
#include <XnArray.h>
#include <XnLog.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------
#define CHECK_RC(what, nRetVal) \
	if (nRetVal != XN_STATUS_OK) \
	{ \
		printf("Failed to %s: %s\n", what, xnGetStatusString(nRetVal)); \
		XN_ASSERT(FALSE); \
		return nRetVal; \
	}

#define CHECK_RC_NO_RET(what, nRetVal) \
	if (nRetVal != XN_STATUS_OK) \
	{ \
		printf("Failed to %s: %s\n", what, xnGetStatusString(nRetVal)); \
		XN_ASSERT(FALSE); \
	}

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define WAIT_FOR_DEVICE_TIMEOUT 10000
#define WAIT_FOR_DEVICE_CHECK_INTERVAL_MS 5000
#define XN_MASK_PRIME_CONSOLE "PSLinkConsole"
#define MAX_DEVICES_COUNT 10

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
typedef int (*CommandHandler)(int argc, const char* argv[]);

typedef struct  
{
	const XnChar* name;
	CommandHandler handler;
} Command;

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
static xn::PrimeClient* g_pPrimeClient = NULL;
static xn::PS1200Device* g_pPS1200Device = NULL;
static xnl::List<const XnChar*> g_commandsList;
static XnStringsHashT<Command> g_commands;
static XnBool g_continue = TRUE;
static XnConnectionString g_connectionStrings[MAX_DEVICES_COUNT];

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

//---------------------------------------------------------------------------
// Framework
//---------------------------------------------------------------------------
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
		printf("Failed checking for file existence: %s\n", xnGetStatusString(nRetVal));
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

int ComponentsVersions(int /*argc*/, const char* /*argv*/[])
{
	xnl::Array<XnComponentVersion> components;
	XnStatus nRetVal = g_pPrimeClient->GetComponentsVersions(components);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to get components versions: %s\n\n", xnGetStatusString(nRetVal));
		return nRetVal;
	}

	for (XnUInt32 i = 0; i < components.GetSize(); ++i)
	{
		printf("%s: %s\n", components[i].m_strName, components[i].m_strVersion);
	}

	return 0;
}

int BeginUpload(int /*argc*/, const char* /*argv*/[])
{
	XnStatus nRetVal = g_pPrimeClient->BeginUploadFileOnControlEP();
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Begin upload successful\n\n");
		return 0;
	}
	else
	{
		printf("Begin upload failed: %s\n\n", xnGetStatusString(nRetVal));
		return nRetVal;
	}
}

int EndUpload(int /*argc*/, const char* /*argv*/[])
{
	XnStatus nRetVal = g_pPrimeClient->EndUploadFileOnControlEP();
	if (nRetVal == XN_STATUS_OK)
	{
		printf("End upload successful\n\n");
		return 0;
	}
	else
	{
		printf("End upload failed: %s\n\n", xnGetStatusString(nRetVal));
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

	XnBool bOverrideFactorySettings = (argc > 2);

	const XnChar* strFileName = argv[1];
	printf("Uploading file '%s'...", strFileName);
	XnStatus nRetVal = g_pPrimeClient->UploadFileOnControlEP(strFileName, bOverrideFactorySettings);
	printf("\n");
	if (nRetVal == XN_STATUS_OK)
	{
		printf("File uploaded successfully\n\n");
		return 0;
	}
	else
	{
		printf("Failed to upload file '%s': %s\n\n", strFileName, xnGetStatusString(nRetVal));
		return nRetVal;
	}
}

int Dir(int /*argc*/, const char* /*argv*/[])
{
	xnl::Array<XnFileEntry> files;
	XnStatus nRetVal = g_pPrimeClient->GetFileList(files);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to get file list: %s\n\n", xnGetStatusString(nRetVal));
		return nRetVal;
	}
	else
	{
		// print list
		printf("\n");
		printf("%-4s  %-32s  %-8s  %-10s  %-6s  %-6s  %-15s\n", "ZONE", "NAME", "VERSION", "ADDRESS", "SIZE", "CRC", "FLAGS");
		printf("%-4s  %-32s  %-8s  %-10s  %-6s  %-6s  %-15s\n", "====", "====", "=======", "=======", "====", "===", "=====");

		for (XnUInt32 i = 0; i < files.GetSize(); ++i)
		{
			XnFileEntry& file = files[i];
			printf("%-4u  %-32s  %01u.%01u.%01u.%02u  0x%08x  %6u  0x%04x  ", 
				file.m_nZone, file.m_strName, 
				file.m_nVersion.m_nMajor, file.m_nVersion.m_nMinor, file.m_nVersion.m_nMaintenance, file.m_nVersion.m_nBuild,
				file.m_nAddress, file.m_nSize, file.m_nCRC);

			// flags
			if ((file.m_nFlags & XN_LINK_FILE_FLAG_BAD_CRC) != 0)
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

	XnUInt16 zone = (XnUInt16)MyAtoi(argv[1]);

	XnStatus nRetVal = g_pPrimeClient->DownloadFile(zone, argv[2], argv[3]);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to download file '%s' from zone %u: %s\n\n", argv[1], zone, xnGetStatusString(nRetVal));
	}

	return nRetVal;
}

int PrintFirmwareVersion(int /*argc*/, const char* /*argv*/[])
{
	const XnDetailedVersion& fwVersion = g_pPrimeClient->GetFWVersion();
	printf("FW version from device: %u.%u.%u.%u-%s\n\n", fwVersion.m_nMajor, fwVersion.m_nMinor, fwVersion.m_nMaintenance, fwVersion.m_nBuild, fwVersion.m_strModifier);
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
	XnChar strDumpName[XN_FILE_MAX_PATH] = "";
	xnLinkGetStreamDumpName(nStreamID, strDumpName, sizeof(strDumpName));
	xnDumpSetMaskState(strDumpName, bDumpOn);
	if (bDumpOn)
	{
		XnChar strCurrentDir[XN_FILE_MAX_PATH];
		XnStatus nRetVal = xnOSGetCurrentDir(strCurrentDir, sizeof(strCurrentDir));
		XN_REFERENCE_VARIABLE(nRetVal);
		XN_ASSERT(nRetVal == XN_STATUS_OK);
		printf("Dumping stream %u to directory '%s%sLog'\n\n", nStreamID, strCurrentDir, XN_FILE_DIR_SEP);
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

	XnUInt16 nEPID = (XnUInt16)MyAtoi(argv[1]);
	XnBool bDumpOn = (xnOSStrCaseCmp(argv[2], "on") == 0);
	XnChar strDumpName[XN_FILE_MAX_PATH] = "";
	xnLinkGetEPDumpName(nEPID, strDumpName, sizeof(strDumpName));
	xnDumpSetMaskState(strDumpName, bDumpOn);
	if (bDumpOn)
	{
		XnChar strCurrentDir[XN_FILE_MAX_PATH];
		XnStatus nRetVal = xnOSGetCurrentDir(strCurrentDir, sizeof(strCurrentDir));
		XN_REFERENCE_VARIABLE(nRetVal);
		XN_ASSERT(nRetVal == XN_STATUS_OK);
		printf("Dumping endpoint %u to directory '%s%sLog'\n\n", nEPID, strCurrentDir, XN_FILE_DIR_SEP);
	}
	else
	{
		printf("Endpoint %u dump is now off\n\n", nEPID);
	}

	return 0;
}

int EnumerateStreams(int argc, const char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;
	if (argc < 2)
	{
		printf("Usage: %s <stream type|ALL>\n", argv[0]);
		printf("Enumerate available stream of a certain type, where stream type is one of the following:\n");
		printf("\tDepth, IR, Log, Image, User, Hands, Gestures, DY\n\n");
		return -1;
	}

	xnl::Array<XnStreamInfo> streamInfos;
	if (strcmp(argv[1], "ALL") == 0)
	{
		nRetVal = g_pPrimeClient->EnumerateStreams(streamInfos);
	}
	else
	{
		XnStreamType streamType = xnLinkStreamTypeFromString(argv[1]);
		if (streamType == XN_LINK_STREAM_TYPE_INVALID)
		{
			printf("Bad stream type '%s'\n\n", argv[1]);
			return -2;
		}
		nRetVal = g_pPrimeClient->EnumerateStreams(streamType, streamInfos);
	}

	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully enumerated streams of type '%s'. Got %u results:\n", argv[1], streamInfos.GetSize());
		for (XnUInt32 i = 0; i < streamInfos.GetSize(); i++)
		{
			printf("\t[%u] stream type='%s', creationInfo='%s'\n", 
				i,
				xnLinkStreamTypeToString(streamInfos[i].m_nStreamType), 
				streamInfos[i].m_strCreationInfo);
		}
	}
	else
	{
		printf("Failed to enumerate streams: %s\n", xnGetStatusString(nRetVal));
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

	XnUInt16 nStreamID = 0;

	XnStreamType streamType = xnLinkStreamTypeFromString(argv[1]);
	if (streamType == XN_LINK_STREAM_TYPE_INVALID)
	{
		printf("Bad stream type '%s'.\n\n", argv[1]);
		return -2;
	}

	XnStatus nRetVal = g_pPrimeClient->CreateInputStream(streamType, argv[2], nStreamID);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully created stream of type %s with ID %u.\n\n", argv[1], nStreamID);
	}
	else
	{
		printf("Failed to create stream: %s\n\n", xnGetStatusString(nRetVal));
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

	XnUInt16 nStreamID = (XnUInt16)MyAtoi(argv[1]);
	XnStatus nRetVal = g_pPrimeClient->DestroyInputStream(nStreamID);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully destroyed stream %u\n\n", nStreamID);
	}
	else
	{
		printf("Failed to destroy stream: %s\n\n", xnGetStatusString(nRetVal));
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

	XnUInt16 nStreamID = (XnUInt16)MyAtoi(argv[1]);
	xn::LinkInputStream* pInputStream = g_pPrimeClient->GetInputStream(nStreamID);
	if (pInputStream == NULL)
	{
		printf("Stream %u was not created.\n\n", nStreamID);
		return -2;
	}

	XnStatus nRetVal = pInputStream->Start();
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully started stream %u.\n\n", nStreamID);
	}
	else
	{
		printf("Failed to start stream %u: %s\n\n", nStreamID, xnGetStatusString(nRetVal));
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

	XnUInt16 nStreamID = (XnUInt16)MyAtoi(argv[1]);
	xn::LinkInputStream* pInputStream = g_pPrimeClient->GetInputStream(nStreamID);
	if (pInputStream == NULL)
	{
		printf("Stream %u was not created.\n\n", nStreamID);
		return -2;
	}

	XnStatus nRetVal = pInputStream->Stop();
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully stopped stream %u.\n\n", nStreamID);
	}
	else
	{
		printf("Failed to stop stream %u: %s\n\n", nStreamID, xnGetStatusString(nRetVal));
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

	XnUInt16 nStreamID = (XnUInt16)MyAtoi(argv[1]);

	xn::LinkInputStream* pInputStream = g_pPrimeClient->GetInputStream(nStreamID);
	if (pInputStream == NULL)
	{
		printf("Input stream %u was not created.\n\n", nStreamID);
		return -2;
	}

	if (pInputStream->GetStreamFragLevel() != XN_LINK_STREAM_FRAG_LEVEL_FRAMES)
	{
		printf("Stream %u is not a frame stream.\n\n", nStreamID);
		return -3;
	}

	xn::LinkFrameInputStream* pFrameInputStream = (xn::LinkFrameInputStream*)pInputStream;

	const xnl::Array<XnStreamVideoMode>& supportedVideoModes = pFrameInputStream->GetSupportedVideoModes();
	printf("Got %u modes:\n", supportedVideoModes.GetSize());
	XnChar strVideoMode[100];
	for (XnUInt32 i = 0; i < supportedVideoModes.GetSize(); i++)
	{
		xnLinkVideoModeToString(supportedVideoModes[i], strVideoMode, sizeof(strVideoMode));
		printf("\t[%u] %s\n", i, strVideoMode);
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
		for (int i = 1; i <= XN_LINK_PIXEL_FORMAT_BAYER8; ++i)
			printf("%s, ", xnLinkPixelFormatToName((XnLinkPixelFormat)i));
		printf("\n");
		printf("Allowed compressions: ");
		for (int i = 0; i <= XN_LINK_COMPRESSION_12_BIT_PACKED; ++i)
			printf("%s, ", xnLinkCompressionToName((XnLinkCompressionType)i));
		printf("\n\n");
		return -1;
	}

	XnStreamVideoMode videoMode;
	XnUInt16 nStreamID = (XnUInt16)MyAtoi(argv[1]);
	videoMode.m_nXRes = MyAtoi(argv[2]);
	videoMode.m_nYRes = MyAtoi(argv[3]);
	videoMode.m_nFPS = MyAtoi(argv[4]);
	videoMode.m_nPixelFormat = xnLinkPixelFormatFromName(argv[5]);
	videoMode.m_nCompression = xnLinkCompressionFromName(argv[6]);

	xn::LinkInputStream* pInputStream = g_pPrimeClient->GetInputStream(nStreamID);
	if (pInputStream == NULL)
	{
		printf("Stream %u was not created.\n\n", nStreamID);
		return -2;
	}

	if (pInputStream->GetStreamFragLevel() != XN_LINK_STREAM_FRAG_LEVEL_FRAMES)
	{
		printf("Stream %u is not a frame stream.\n\n", nStreamID);
		return -3;
	}

	xn::LinkFrameInputStream* pFrameInputStream = (xn::LinkFrameInputStream*)pInputStream;

	XnStatus nRetVal = pFrameInputStream->SetVideoMode(videoMode);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully set video mode.\n\n");
	}
	else
	{
		printf("Failed to set video mode: %s\n\n", xnGetStatusString(nRetVal));
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

	XnUInt16 nStreamID = (XnUInt16)MyAtoi(argv[1]);
	xn::LinkInputStream* pInputStream = g_pPrimeClient->GetInputStream(nStreamID);
	if (pInputStream == NULL)
	{
		printf("Stream %u was not created.\n\n", nStreamID);
		return -2;
	}

	if (pInputStream->GetStreamFragLevel() != XN_LINK_STREAM_FRAG_LEVEL_FRAMES)
	{
		printf("Stream %u is not a frame stream.\n\n", nStreamID);
		return -3;
	}

	xn::LinkFrameInputStream* pFrameInputStream = (xn::LinkFrameInputStream*)pInputStream;

	const XnStreamVideoMode& videoMode = pFrameInputStream->GetVideoMode();
	XnChar strVideoMode[100];
	xnLinkVideoModeToString(videoMode, strVideoMode, sizeof(strVideoMode));
	printf("Video mode of stream %u: %s\n\n", nStreamID, strVideoMode);

	return 0;
}

XnStatus GetI2CDeviceIDFromName(const char* deviceName, XnUInt8* result)
{
	static xnl::Array<XnLinkI2CDevice> s_deviceList;
	XnStatus nRetVal = XN_STATUS_NO_MATCH;
	
	//Fetch device list if needed
	if(s_deviceList.GetSize() == 0)
	{
		nRetVal = g_pPrimeClient->GetSupportedI2CDevices(s_deviceList);
		XN_IS_STATUS_OK_LOG_ERROR("Get device list", nRetVal);

		for(XnUInt32 i=0; i<s_deviceList.GetSize(); i++)
			ToLower(s_deviceList[i].m_strName);
	}
	
	nRetVal = XN_STATUS_NO_MATCH;
	for(XnUInt32 i=0; i<s_deviceList.GetSize() && nRetVal==XN_STATUS_NO_MATCH; i++)
	{
		if(xnOSStrCaseCmp(s_deviceList[i].m_strName, deviceName) == 0)
		{
			nRetVal = XN_STATUS_OK;
			*result = (XnUInt8)s_deviceList[i].m_nID;
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

	//Try to get parse ID by name, and then by number
	XnUInt8 deviceID = 0;
	if(GetI2CDeviceIDFromName(argv[1], &deviceID) == XN_STATUS_NO_MATCH)
		deviceID = (XnUInt8)MyAtoi(argv[1]);

	XnUInt8 addressSize = (XnUInt8)MyAtoi(argv[2]);
	XnUInt32 address = (XnUInt32)MyAtoi(argv[3]);
	XnUInt8 valueSize = (XnUInt8)MyAtoi(argv[4]);
	XnUInt32 value = (XnUInt32)MyAtoi(argv[5]);
	XnUInt32 mask = (argc > 6) ? (XnUInt32)MyAtoi(argv[6]) : 0xFFFFFFFF;

	XnStatus nRetVal = g_pPrimeClient->WriteI2C(deviceID, addressSize, address, valueSize, value, mask);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully written I2C value.\n\n");
	}
	else
	{
		printf("Failed to write I2C value: %s\n\n", xnGetStatusString(nRetVal));
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

	//Try to get parse ID by name, and then by number
	XnUInt8 deviceID = 0;
	if(GetI2CDeviceIDFromName(argv[1], &deviceID) == XN_STATUS_NO_MATCH)
		deviceID = (XnUInt8)MyAtoi(argv[1]);

	XnUInt8 addressSize = (XnUInt8)MyAtoi(argv[2]);
	XnUInt32 address = (XnUInt32)MyAtoi(argv[3]);
	XnUInt8 valueSize = (XnUInt8)MyAtoi(argv[4]);

	XnUInt32 value = 0;
	XnStatus nRetVal = g_pPrimeClient->ReadI2C(deviceID, addressSize, address, valueSize, value);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully read I2C value: %u (0x%X)\n\n", value, value);
	}
	else
	{
		printf("Failed to read I2C value: %s\n\n", xnGetStatusString(nRetVal));
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
	XnStatus nRetVal = g_pPrimeClient->WriteAHB(MyAtoi(argv[1]), //Address
		MyAtoi(argv[2]), //Value
		(XnUInt8)MyAtoi(argv[3]), //BitOffset
		(XnUInt8)MyAtoi(argv[4])); //BitWidth
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully written AHB value.\n\n");
	}
	else
	{
		printf("Failed to write AHB value: %s\n\n", xnGetStatusString(nRetVal));
	}

	return nRetVal;
}

int ReadAHB(int argc, const char* argv[])
{
	XnUInt32 nValue = 0;
	if (argc < 4)
	{
		printf("Usage: %s <Address> <BitOffset> <BitWidth>\n", argv[0]);
		printf("Note - each parameter may be in hex, indicated by an '0x' prefix.\n\n");
		return -1;
	}
	XnStatus nRetVal = g_pPrimeClient->ReadAHB(MyAtoi(argv[1]), //Address
		(XnUInt8)MyAtoi(argv[2]), //Bit Offset
		(XnUInt8)MyAtoi(argv[3]), //Bit Width
		nValue); //Value
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully read AHB value: %u (0x%X)\n\n", nValue, nValue);
	}
	else
	{
		printf("Failed to read I2C value: %s\n\n", xnGetStatusString(nRetVal));
	}

	return nRetVal;
}

int SoftReset(int /*argc*/, const char* /*argv*/[])
{
	printf("Resetting device...");
	XnStatus nRetVal = g_pPrimeClient->SoftReset();
	printf("\n");
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully executed Soft Reset.\n\n");
	}
	else
	{
		printf("Failed to execute soft reset: %s\n\n", xnGetStatusString(nRetVal));
	}

	return nRetVal;
}

int HardReset(int /*argc*/, const char* /*argv*/[])
{
	printf("Resetting device...");
	XnStatus nRetVal = g_pPrimeClient->HardReset();
	printf("\n");
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully executed Hard Reset.\n\n");
		printf("**********************************************\n");
		printf("Warning: console must be restarted!\n");
		printf("**********************************************\n\n");
	}
	else
	{
		printf("Failed to execute hard reset: %s\n\n", xnGetStatusString(nRetVal));
	}

	return nRetVal;
}

int Emitter(int argc, const char* argv[])
{
	if ((argc < 2) || 
		((xnOSStrCaseCmp(argv[1], "on") != 0) && (xnOSStrCaseCmp(argv[1], "off") != 0)))
	{
		printf("Usage: %s <on or off>\n\n", argv[0]);
		return -1;
	}
	const XnChar* strEmitterActive = argv[1];
	XnBool bEmitterActive = (xnOSStrCaseCmp(strEmitterActive, "on") == 0);
	XnStatus nRetVal = g_pPrimeClient->SetEmitterActive(bEmitterActive);
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Emitter is now %s.\n\n", strEmitterActive);
	}
	else
	{
		printf("Failed to set emitter %s: %s\n\n", strEmitterActive, xnGetStatusString(nRetVal));
	}

	return nRetVal;
}

XnStatus GetLogIDFromName(const char* logName, XnUInt8* result) 
{
	static xnl::Array<XnLinkLogFile> s_fileList;
	XnStatus nRetVal = XN_STATUS_NO_MATCH;

	//Fetch files list if needed
	if(s_fileList.GetSize() == 0)
	{
		nRetVal = g_pPrimeClient->GetSupportedLogFiles(s_fileList);
		XN_IS_STATUS_OK_LOG_ERROR("Get log file list", nRetVal);

		for(XnUInt32 i=0; i<s_fileList.GetSize(); i++)
			ToLower(s_fileList[i].m_strName);
	}

	nRetVal = XN_STATUS_NO_MATCH;
	for(XnUInt32 i=0; i<s_fileList.GetSize() && nRetVal==XN_STATUS_NO_MATCH; i++)
	{
		if(xnOSStrCaseCmp(s_fileList[i].m_strName, logName) == 0)
		{
			nRetVal = XN_STATUS_OK;
			*result = s_fileList[i].m_nID;
		}
	}

	return nRetVal;
}

int Log(int argc, const char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;
	bool isOnOffCommnad = (argc == 2) && ((xnOSStrCaseCmp(argv[1], "on") == 0) || (xnOSStrCaseCmp(argv[1], "off") == 0));
	bool isStartStopCommnad = (argc == 3) && ((xnOSStrCaseCmp(argv[1], "open") == 0) || (xnOSStrCaseCmp(argv[1], "close") == 0));
	
	if(!isOnOffCommnad && !isStartStopCommnad)
	{
		printf("Usage: %s <on|off> or <open|close> <stream name or id>\n\n", argv[0]);
		return -1;
	}

	if(isStartStopCommnad)
	{
		//Try to get parse ID by name, and then by number
		XnUInt8 logID = 0;
		if(GetLogIDFromName(argv[1], &logID) == XN_STATUS_NO_MATCH){
			logID = (XnUInt8)MyAtoi(argv[2]);
		}

		//Start command
		if(xnOSStrCaseCmp(argv[1], "open") == 0){
			nRetVal = g_pPrimeClient->OpenFWLogFile(logID);
		}
		//Stop command
		else{
			nRetVal = g_pPrimeClient->CloseFWLogFile(logID);
		}

		if (nRetVal == XN_STATUS_OK){
			printf("Sent %s command for log #%d", argv[1], (int)logID);
		}else{
			printf("Failed to send %s command for log #%d: %s\n\n", argv[1], (int)logID, xnGetStatusString(nRetVal));
		}

	}
	//on/off command
	else
	{
		const XnChar* strLogOn = argv[1];
		XnBool bLogOn = (xnOSStrCaseCmp(strLogOn, "on") == 0);
		if (bLogOn)
		{
			nRetVal = g_pPrimeClient->StartFWLog();
		}
		else
		{
			nRetVal = g_pPrimeClient->StopFWLog();
		}

		if (nRetVal == XN_STATUS_OK)
		{
			if (bLogOn)
			{
				XnChar strCurrentDir[XN_FILE_MAX_PATH];
				nRetVal = xnOSGetCurrentDir(strCurrentDir, sizeof(strCurrentDir));
				XN_ASSERT(nRetVal == XN_STATUS_OK);
				printf("Saving firmware log to '%s%sLog'\n\n", strCurrentDir, XN_FILE_DIR_SEP);
			}
			else
			{
				printf("Firmware log is now off\n\n");
			}
		}
		else
		{
			printf("Failed to set log %s: %s\n\n", strLogOn, xnGetStatusString(nRetVal));
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
	XnStatus nRetVal = g_pPrimeClient->GetBootStatus(bootStatus);
	
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to get boot status: %s\n\n", xnGetStatusString(nRetVal));
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
		switch(bootStatus.m_nZone){
			CASE_ENUM_TOSTRING(XN_LINK_BOOT_FACTORY_ZONE, "FACTORY_ZONE", bootZoneStr)
			CASE_ENUM_TOSTRING(XN_LINK_BOOT_UPDATE_ZONE, "UPDATE_ZONE", bootZoneStr)
			default:
				sprintf(bootZoneStr, "Unexpected - %d",bootStatus.m_nZone);
		}

		//Get XnLinkBootErrorCode string
		switch(bootStatus.m_nErrorCode){
			CASE_ENUM_TOSTRING(XN_LINK_BOOT_OK, "BOOT_OK", bootZoneErr)
			CASE_ENUM_TOSTRING(XN_LINK_BOOT_BAD_CRC, "BAD_CRC", bootZoneErr)
			CASE_ENUM_TOSTRING(XN_LINK_BOOT_UPLOAD_IN_PROGRESS, "UPLOAD_IN_PROGRESS", bootZoneErr)
			default:
				sprintf(bootZoneErr, "Unexpected - %d",bootStatus.m_nErrorCode);
		}

		printf("Zone: %s\nError code: %s\n\n", bootZoneStr, bootZoneErr);

		return nRetVal;
	}
}


int PrintLogFilesList(int /*argc*/, const char* /*argv*/[])
{
	xnl::Array<XnLinkLogFile> fileList;
	XnStatus nRetVal = g_pPrimeClient->GetSupportedLogFiles(fileList);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to get log files list: %s\n\n", xnGetStatusString(nRetVal));
	}
	else
	{
		for (XnUInt32 i = 0; i < fileList.GetSize(); ++i)
		{
			printf("%4u %s\n", (unsigned int)fileList[i].m_nID, fileList[i].m_strName);
		}

		printf("\n");
	}

	return nRetVal;
}

int PrintI2CList(int /*argc*/, const char* /*argv*/[])
{
	xnl::Array<XnLinkI2CDevice> deviceList;
	XnStatus nRetVal = g_pPrimeClient->GetSupportedI2CDevices(deviceList);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to get device list: %s\n\n", xnGetStatusString(nRetVal));
	}
	else
	{
		for (XnUInt32 i = 0; i < deviceList.GetSize(); ++i)
		{
			printf("%4u %s\n", deviceList[i].m_nID, deviceList[i].m_strName);
		}

		printf("\n");
	}

	return nRetVal;
}

int PrintBistList(int /*argc*/, const char* /*argv*/[])
{
	xnl::Array<XnBistTest> testsList;
	XnStatus nRetVal = g_pPrimeClient->GetSupportedBistTests(testsList);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to get tests list: %s\n\n", xnGetStatusString(nRetVal));
	}
	else
	{
		for (XnUInt32 i = 0; i < testsList.GetSize(); ++i)
		{
			printf("%4u %s\n", testsList[i].m_nID, testsList[i].m_strName);
		}

		printf("\n");
	}

	return nRetVal;
}

int RunBist(int argc, const char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (argc < 2)
	{
		printf("Usage: %s ALL | <Test>...\n\n", argv[0]);
		return -1;
	}

	xnl::BitSet requestedTests;

	if (xnOSStrCaseCmp(argv[1], "ALL") == 0)
	{
		xnl::Array<XnBistTest> supportedTests;
		nRetVal = g_pPrimeClient->GetSupportedBistTests(supportedTests);
		if (nRetVal != XN_STATUS_OK)
		{
			printf("Failed to get supported tests list: %s\n\n", xnGetStatusString(nRetVal));
			return nRetVal;
		}

		for (XnUInt32 i = 0; i < supportedTests.GetSize(); ++i)
		{
			requestedTests.Set(supportedTests[i].m_nID, TRUE);
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
	XnBistTestResponse* pBistResponse = reinterpret_cast<XnBistTestResponse*>(response);

	for (XnUInt32 i = 0; i < requestedTests.GetSize(); ++i)
	{
		if (!requestedTests.IsSet(i))
		{
			continue;
		}

		printf("Executing test %u...\n", i);
		nRetVal = g_pPrimeClient->ExecuteBist(i, pBistResponse, sizeof(response));
		if (nRetVal != XN_STATUS_OK)
		{
			printf("\nFailed to execute: %s\n\n", xnGetStatusString(nRetVal));
			return nRetVal;
		}

		printf("Test %u ", i);

		if (pBistResponse->m_nErrorCode != 0)
		{
			printf("Failed (error code 0x%04X).", pBistResponse->m_nErrorCode);
		}
		else
		{
			printf("Passed.");
		}

		printf("\n");

		// extra data
		if (pBistResponse->m_nExtraDataSize > 0)
		{
			printf("Extra Data: ");
			for (XnUInt32 j = 0; j < pBistResponse->m_nExtraDataSize; ++j)
			{
				printf("%02X ", pBistResponse->m_extraData[j]);
			}
			printf("\n");
		}
	}

	printf("\n");

	return (XN_STATUS_OK);
}

int Quit(int /*argc*/, const char* /*argv*/[])
{
	g_continue = FALSE;
	return 0;
}

int UsbInterface(int argc, const char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;

	if (g_pPS1200Device == NULL)
	{
		printf("Device does not support setting the usb interface!\n\n");
		return -2;
	}

	if (argc == 1)
	{
		XnUInt8 altInterface;
		nRetVal = g_pPS1200Device->GetUsbAltInterface(altInterface);
		if (nRetVal != XN_STATUS_OK)
		{
			printf("Failed to get interface: %s\n\n", xnGetStatusString(nRetVal));
			return -3;
		}
		printf("Current USB alternative interface is %u\n\n", (XnUInt32)altInterface);
	}
	else if (argc == 2)
	{
		XnUInt32 altInterface = MyAtoi(argv[1]);
		nRetVal = g_pPS1200Device->SetUsbAltInterface((XnUInt8)altInterface);
		if (nRetVal != XN_STATUS_OK)
		{
			printf("Failed to set interface: %s\n\n", xnGetStatusString(nRetVal));
			return -3;
		}
	}
	else
	{
		printf("Usage: %s [num]\n\n", argv[0]);
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
	XnStatus nRetVal = g_pPrimeClient->FormatZone((XnUInt8)nZone); //Zone
	if (nRetVal == XN_STATUS_OK)
	{
		printf("Successfully formatZone.\n\n");
	}
	else
	{
		printf("Failed to format Zone value: %s\n\n", xnGetStatusString(nRetVal));
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

	XnUInt32 nSeconds = MyAtoi(argv[1]);

	if (g_pPS1200Device == NULL)
	{
		printf("Device does not support setting the usb interface!\n\n");
		return -2;
	}

	xn::UsbTestResults results;
	nRetVal = g_pPS1200Device->UsbTest(nSeconds, &results);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("Failed to perform USB test: %s\n\n", xnGetStatusString(nRetVal));
		return -3;
	}

	printf("USB Test done:\n");
	for (XnUInt32 i = 0; i < results.nNumEndpoints; ++i)
	{
		printf("\tEndpoint %u - Avg. Bandwidth: %.3f KB/s, Lost Packets: %u\n", i, results.aEndpoints[i].nAverageBytesPerSecond / 1000., results.aEndpoints[i].nLostPackets);
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

void RegisterCommands()
{
	RegisterCommand("Help", Help);
	RegisterCommand("Versions", ComponentsVersions);
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
	RegisterCommand("Emitter", Emitter);
	RegisterCommand("Log", Log);
	RegisterCommand("LogList", PrintLogFilesList);
	RegisterCommand("Script", Script);
	RegisterCommand("BistList", PrintBistList);
	RegisterCommand("Bist", RunBist);
	RegisterCommand("UsbInterface", UsbInterface);
	RegisterCommand("FormatZone", FormatZone);
	RegisterCommand("UsbTest", UsbTest);
	RegisterCommand("TestAll", TestAll);
	RegisterCommand("Quit", Quit);
	RegisterCommand("Bye", Quit);
	RegisterCommand("Exit", Quit);
}

XnStatus EnumerateAllUsbDevices(XnUInt16 /*nProductID*/, XnConnectionString*& astrConnStrings, XnUInt32& nCount)
{
	XnStatus nRetVal = XN_STATUS_OK;
	XnUInt32 nTotalCount = 0;

	XnUInt16 productIDs[] = 
	{
		XN_PRODUCT_ID_PS1250,
		XN_PRODUCT_ID_PS1260,
		XN_PRODUCT_ID_PS1270,
		XN_PRODUCT_ID_PS1290,
	};

	int nProductIDs = sizeof(productIDs)/sizeof(productIDs[0]);

	for (int i = 0; i < nProductIDs; ++i)
	{
		XnUInt32 nCount = 0;
		nRetVal = xn::ClientUSBConnectionFactory::EnumerateConnStrings(productIDs[i], astrConnStrings, nCount);
		CHECK_RC("Enumerate connection strings", nRetVal);
		xnOSMemCopy(g_connectionStrings[nTotalCount], astrConnStrings, sizeof(astrConnStrings[0])*nCount);
		xn::ClientUSBConnectionFactory::FreeConnStringsList(astrConnStrings);
		nTotalCount += nCount;
	}

	astrConnStrings = g_connectionStrings;
	nCount = nTotalCount;
	return XN_STATUS_OK;
}

typedef XnStatus (*EnumerateFunc)(XnUInt16 nProductID, XnConnectionString*& astrConnStrings, XnUInt32& nCount);

int main(int argc, char* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;
	const XnChar* strScriptFile = NULL;
	XnTransportType transportType = XN_TRANSPORT_TYPE_USB;
	XnUInt16 nProductID = 0;
    XnBool bQuit = FALSE;
	EnumerateFunc pEnumerateConnStringsFunc = EnumerateAllUsbDevices;

	printf("PSLinkConsole version %s\n", XN_PS_VERSION_STRING);

	XnInt32 nArgIndex = 1;
	while (nArgIndex < argc)
	{
		if (argv[nArgIndex][0] == '-')
		{
			if (xnOSStrCaseCmp(argv[nArgIndex], "-transport") == 0)
			{
				++nArgIndex;
				if (xnOSStrCaseCmp(argv[nArgIndex], "usb") == 0)
				{
					transportType = XN_TRANSPORT_TYPE_USB;
					pEnumerateConnStringsFunc = EnumerateAllUsbDevices;
				}
				else
				{
					transportType = XN_TRANSPORT_TYPE_SOCKETS;
					xn::SocketConnectionFactory::AddEnumerationTarget(argv[nArgIndex]);
					pEnumerateConnStringsFunc = &xn::SocketConnectionFactory::EnumerateConnStrings;
				}
				++nArgIndex;
			}
			else if (xnOSStrCaseCmp(argv[nArgIndex], "-product") == 0)
			{
				++nArgIndex;
				nProductID = (XnUInt16)MyAtoi(argv[nArgIndex++]);
				pEnumerateConnStringsFunc = &(xn::ClientUSBConnectionFactory::EnumerateConnStrings);
			}
			else if (xnOSStrCaseCmp(argv[nArgIndex], "-script") == 0)
			{
				++nArgIndex;
				strScriptFile = argv[nArgIndex++];
			}
			else if (xnOSStrCaseCmp(argv[nArgIndex], "-help") == 0)
			{
				printf("USAGE\n");
				printf("\t%s [-transport <usb|ip:port>] [-product <PID>] [-script <fileName>] [-help]\n", argv[0]);
				printf("OPTIONS\n");
				printf("\t-transport <usb|ip:port>\n");
				printf("\t\tOpen a device from a specific transport, either USB or from a specific IP and port number.\n");
				printf("\t\tWhen omitted, USB will be used.\n");
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
			printf("Unknown option: %s\n. Run %s -help for usage.\n", argv[nArgIndex], argv[0]);
			return -1;
		}
	}

	xnLogSetConsoleOutput(TRUE);
	xnLogSetMaskMinSeverity(XN_LOG_MASK_ALL, XN_LOG_VERBOSE);

	if (transportType == XN_TRANSPORT_TYPE_USB)
	{
		g_pPS1200Device = XN_NEW(xn::PS1200Device);
		g_pPrimeClient = g_pPS1200Device; 
	}
	else
	{
		g_pPrimeClient = XN_NEW(xn::LenaDevice);
	}

	if (g_pPrimeClient == NULL)
	{
		printf("Failed to create prime client :(\n");
		return -3;
	}

	/* Wait for EE Device to become available */
	XnUInt32 nWaitTimeRemaining = WAIT_FOR_DEVICE_TIMEOUT;

	XnConnectionString* pastrConnStrings = NULL;
	XnUInt32 nConnStrings = 0;
	nRetVal = pEnumerateConnStringsFunc(nProductID, pastrConnStrings, nConnStrings);
	CHECK_RC("Enumerate connection strings", nRetVal);
	while ((nConnStrings == 0) && (nWaitTimeRemaining > 0))
	{
		nRetVal = pEnumerateConnStringsFunc(nProductID, pastrConnStrings, nConnStrings);
		CHECK_RC("Enumerate connection strings", nRetVal);
		nWaitTimeRemaining = XN_MAX(0, (XnInt32)(nWaitTimeRemaining - WAIT_FOR_DEVICE_CHECK_INTERVAL_MS));
		if (nConnStrings == 0)
		{
			xnOSSleep(WAIT_FOR_DEVICE_CHECK_INTERVAL_MS);
		}
	}

	if (nConnStrings == 0)
	{
		xnLogError(XN_MASK_PRIME_CONSOLE, "Device not found (after %u milliseconds)", WAIT_FOR_DEVICE_TIMEOUT);
		XN_ASSERT(FALSE);
		return XN_STATUS_DEVICE_NOT_CONNECTED;
	}

	printf("Device found, connecting...\n");
	
	/* Initialize Prime Client with first connection string */
	nRetVal = g_pPrimeClient->Init(pastrConnStrings[0], transportType);
	XN_IS_STATUS_OK_LOG_ERROR("Init EE Device", nRetVal);

	/* Connect PrimeClient */
	nRetVal = g_pPrimeClient->Connect();
	CHECK_RC("Connect Prime Client", nRetVal);

    //Prime Client is now connected :)

	RegisterCommands();

	if (strScriptFile != NULL)
	{
		nRetVal = RunScript(strScriptFile);
		if (nRetVal != XN_STATUS_OK)
		{
			// error is returned only if script could not be run, not if any command in it failed.
			return -3;
		}

        if (bQuit)
        {
            return 0;
        }
	}

	ExecuteCommandsFromStream(stdin, TRUE);

	g_pPrimeClient->Shutdown();

	return 0;
}
