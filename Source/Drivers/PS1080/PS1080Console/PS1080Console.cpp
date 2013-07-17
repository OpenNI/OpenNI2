/*****************************************************************************
*                                                                            *
*  PrimeSense Sensor 5.x Alpha                                               *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of PrimeSense Sensor                                    *
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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <OpenNI.h>
#include <PS1080.h>
#include <XnOS.h>
#include <XnStreamParams.h>

using namespace std;

typedef bool (*cbfunc)(openni::Device& Device, vector<string>& Command);

map<string, cbfunc> cbs;
map<string, cbfunc> mnemonics;
map<string, string>  helps;

XnVersions g_DeviceVersion;
const char* g_openMode;
openni::VideoStream g_depthStream;
openni::VideoStream g_colorStream;

const XnUInt16 REQUIRE_MODE_PS				= 0x0001;
const XnUInt16 REQUIRE_MODE_MAINTENANCE		= 0x0002;
const XnUInt16 REQUIRE_MODE_ANY				= 0xFFFF;

#define SWITCH_MODE_WAIT	3000

bool atoi2(const char* str, int* pOut)
{
	int output = 0;
	int base = 10;
	int start = 0;

	if (strlen(str) > 1 && str[0] == '0' && str[1] == 'x')
	{
		start = 2;
		base = 16;
	}

	for (size_t i = start; i < strlen(str); i++)
	{
		output *= base;
		if (str[i] >= '0' && str[i] <= '9')
			output += str[i]-'0';
		else if (base == 16 && str[i] >= 'a' && str[i] <= 'f')
			output += 10+str[i]-'a';
		else if (base == 16 && str[i] >= 'A' && str[i] <= 'F')
			output += 10+str[i]-'A';
		else
			return false;
	}
	*pOut = output;
	return true;
}


void mainloop(openni::Device& Device, istream& istr, bool prompt)
{
	char buf[256];
	string str;

	vector<string> Command;

	while (istr.good())
	{
		if (prompt)
			cout << "> ";
		Command.clear();
		istr.getline(buf, 256);
		str = buf;
		size_t previous = 0, next = 0;

		while (1)
		{
			next = str.find(' ', previous);
			
			if (next != previous && previous != str.size())
				Command.push_back(str.substr(previous, next-previous));

			if (next == str.npos)
				break;
			
			previous = next+1;
		}

		if (Command.size() > 0)
		{
			if (Command[0][0] == ';')
				continue;

			for (unsigned int i = 0; i < Command[0].size(); i++)
				Command[0][i] = (char)tolower(Command[0][i]);

			if (cbs.find(Command[0]) != cbs.end())
			{
				if (!(*cbs[Command[0]])(Device, Command))
					return;
			}
			else if (mnemonics.find(Command[0]) != mnemonics.end())
			{
				if (!(*mnemonics[Command[0]])(Device, Command))
					return;
			}
			else
			{
				cout << "Unknown command \"" << Command[0] << "\"" << endl;
			}
		}
	}
}

bool byebye(openni::Device& /*Device*/, vector<string>& /*Command*/)
{
	cout << "Bye bye" << endl;
	return false;
}

bool Version(openni::Device& Device, vector<string>& /*Command*/)
{
	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_VERSION, &g_DeviceVersion);
	if (rc != openni::STATUS_OK)
	{
		printf("Error: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	XnChar strPlatformString[XN_DEVICE_MAX_STRING_LENGTH];
	int size = sizeof(strPlatformString);
	rc = Device.getProperty(XN_MODULE_PROPERTY_SENSOR_PLATFORM_STRING, strPlatformString, &size);
	if (rc != openni::STATUS_OK)
	{
		printf("Error: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	printf("Firmware: V%d.%d.%d", g_DeviceVersion.nMajor, g_DeviceVersion.nMinor, g_DeviceVersion.nBuild);

	if (strPlatformString[0] != '\0')
	{
		printf("-%s", strPlatformString);
	}

	printf("; SDK: V%d.%d.%d.%d; Chip: 0x%08x\nFPGA: 0x%x; System: 0x%x; ",
			g_DeviceVersion.SDK.nMajor, g_DeviceVersion.SDK.nMinor, g_DeviceVersion.SDK.nMaintenance, g_DeviceVersion.SDK.nBuild,
			g_DeviceVersion.nChip, g_DeviceVersion.nFPGA, g_DeviceVersion.nSystemVersion);

	printf ("FWBaseLine: ");
	if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_0_17)
	{
		printf ("V0.17");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_1_1)
	{
		printf ("V1.1");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_1_2)
	{
		printf ("V1.2");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_3_0)
	{
		printf ("V3.0");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_4_0)
	{
		printf ("V4.0");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_0)
	{
		printf ("V5.0");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_1)
	{
		printf ("V5.1");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_2)
	{
		printf ("V5.2");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_3)
	{
		printf ("V5.3");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_4)
	{
		printf ("V5.4");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_5)
	{
		printf ("V5.5");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_6)
	{
		printf ("V5.6");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_5_7)
	{
		printf ("V5.7");
	}
	else if (g_DeviceVersion.FWVer == XN_SENSOR_FW_VER_UNKNOWN)
	{
		printf ("Unknown");
	}

	printf ("; Board: ");
	if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_CDB_10)
	{
		printf ("CDB1.0");
	}
	else if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_FPDB_10)
	{
		printf ("FPDB1.0");
	}
	else if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_RD_3)
	{
		printf ("RD3.0");
	}
	else if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_RD_5)
	{
		printf ("RD5.0");
	}
	else if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_RD1081)
	{
		printf ("RD1081");
	}
	else if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_RD1082)
	{
		printf ("RD1082");
	}
	else if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_RD109)
	{
		printf ("RD109");
	}	
	else if (g_DeviceVersion.HWVer == XN_SENSOR_HW_VER_UNKNOWN)
	{
		printf ("Unknown");
	}

	printf ("; ChipType: ");
	if (g_DeviceVersion.ChipVer == XN_SENSOR_CHIP_VER_PS1000)
	{
		printf ("PS1000");
	}
	else if (g_DeviceVersion.ChipVer == XN_SENSOR_CHIP_VER_PS1080)
	{
		printf ("PS1080");
	}
	else if (g_DeviceVersion.ChipVer == XN_SENSOR_CHIP_VER_PS1080A6)
	{
		printf ("PS1080A6");
	}
	else if (g_DeviceVersion.ChipVer == XN_SENSOR_CHIP_VER_UNKNOWN)
	{
		printf ("Unknown");
	}

	printf ("\n");

	return true;
}

bool DeleteFile(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() != 2)
	{
		printf("Usage: %s <file id>\n", Command[0].c_str());
		return true;
	}

	int nId;
	if (!atoi2(Command[1].c_str(), &nId))
	{
		printf("Id  (%s) isn't a number\n", Command[1].c_str());
		return true;
	}
	printf("Deleting file id %d: ", nId);

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_DELETE_FILE, (XnUInt64)nId);
	if (rc != openni::STATUS_OK)
	{
		printf("Failed: %s\n", openni::OpenNI::getExtendedError());
	}

	printf("Done\n");

	return true;
}

bool Attrib(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 2)
	{
		printf("Usage: %s <id> [<+-><attributes> ...]\n", Command[0].c_str());
		printf("\t+r - Set read only. -r - Set not read only\n");
		return true;
	}
	int nId;
	if (!atoi2(Command[1].c_str(), &nId))
	{
		printf("Id (%s) isn't a number\n", Command[1].c_str());
		return true;
	}

	XnFlashFile Files[100];
	
	XnFlashFileList FileList;
	FileList.pFiles = (XnFlashFile*)Files;
	FileList.nFiles = 100;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_FILE_LIST, &FileList);
	if (rc != openni::STATUS_OK)
	{
		printf("Couldn't get file list: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	XnFlashFile* pEntry = NULL;

	for (int i = 0; i < FileList.nFiles; i++)
	{
		if (FileList.pFiles[i].nId == nId)
			pEntry = &FileList.pFiles[i];
	}
	if (pEntry == NULL)
	{
		printf("No such id %d\n", nId);
		return true;
	}

	if (Command.size() == 2)
	{
		// Get
		printf("0x%04x: ", pEntry->nAttributes);

		// All attributes
		printf("%cr ", (pEntry->nAttributes & XnFileAttributeReadOnly) ? '+':'-');
		// Done
		printf("\n");
		return true;
	}

	XnUInt16 nNewAttributes = pEntry->nAttributes;

	for (unsigned int i = 2; i < Command.size(); i++)
	{
		if (Command[i] == "+r")
			nNewAttributes |= XnFileAttributeReadOnly;
		else if (Command[i] == "-r")
			nNewAttributes &= ~XnFileAttributeReadOnly;
		else
		{
			printf("Unknown attribute: %s\n", Command[i].c_str());
			return true;
		}
	}

	if (nNewAttributes == pEntry->nAttributes)
	{
		printf("Target attributes is same as existing one!\n");
		return true;
	}

	XnFileAttributes Attribs;
	Attribs.nId = pEntry->nId;
	Attribs.nAttribs = nNewAttributes;

	rc = Device.setProperty(XN_MODULE_PROPERTY_FILE_ATTRIBUTES, Attribs);
	if (rc != openni::STATUS_OK)
	{
		printf("Couldn't change attributes: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	return true;
}

bool Connect(openni::Device& Device)
{
	openni::Status rc = openni::STATUS_OK;

	int nCounter = 10;
	while (nCounter-- != 0)
	{
		rc = Device._openEx(NULL, g_openMode);
		if (rc == openni::STATUS_OK || rc == openni::STATUS_NO_DEVICE)
			break;

		// wait and try again
		xnOSSleep(1000);
	}

	if (rc != openni::STATUS_OK)
	{
		printf("Open failed: %s\n", openni::OpenNI::getExtendedError());
		return false;
	}

	return true;
}

bool Reconnect(openni::Device& Device, vector<string>& /*Command*/)
{
	Device.close();

	if (!Connect(Device))
	{
		return false;
	}

	printf("Reconnected\n");
	
	return true;
}

bool Sleep(openni::Device& /*Device*/, vector<string>& Command)
{
	if (Command.size() != 2)
	{
		printf("Usage: %s <ms>\n", Command[0].c_str());
		return true;
	}

	XnInt32 nMilliSeconds;
	if (!atoi2(Command[1].c_str(), &nMilliSeconds))
	{
		printf("%s doesn't describe a time quantity in milliseconds\n", Command[1].c_str());
		return true;
	}

	xnOSSleep(nMilliSeconds);
	return true;
}

#define NUM_OF_FILE_TYPES 31
bool FileList(openni::Device& Device, vector<string>& /*Command*/)
{
	const char *FileTypes[NUM_OF_FILE_TYPES]= { "FILE_TABLE",
												"SCRATCH_FILE",
												"BOOT_SECTOR",
												"BOOT_MANAGER", 
												"CODE_DOWNLOADER",
												"MONITOR",
												"APPLICATION",
												"FIXED_PARAMS",
												"DESCRIPTORS",
												"DEFAULT_PARAMS",
												"IMAGE_CMOS",
												"DEPTH_CMOS",
												"ALGORITHM_PARAMS",
												"QVGA_REFERENCE",
												"VGA_REFERENCE",
												"MAINTENANCE",
												"DEBUG_PARAMS",
												"PRIME_PROCESSOR",
												"GAIN_CONTROL",
												"REG_PARAMS",
												"ID_PARAMS",
												"TEC_PARAMS",
												"APC_PARAMS",
												"SAFETY_PARAMS",
												"PRODUCTION_FILE",
												"UPGRADE_IN_PROGRESS",
												"WAVELENGTH_CORRECTION",
												"GMC_REF_OFFSET",
												"NESA_PARAMS",
												"SENSOR_FAULT",
                                                "VENDOR_DATA",
											  };

	XnFlashFile Files[100];

	
	XnFlashFileList FileList;
	FileList.pFiles = (XnFlashFile*)Files;
	FileList.nFiles = 100;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_FILE_LIST, &FileList);
	if (rc != openni::STATUS_OK)
	{
		printf("Error: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	printf("%-3s %-22s %-8s    %-8s   %-8s %-8s\n", "Id", "Type", "Version", "Offset", "SizeInWords", "Crc");
	for (int i = 0; i < FileList.nFiles; i++)
	{
		XnFlashFile *Entry = &FileList.pFiles[i];
		const char *TypeCaption = "Unknown Type";
		if ( Entry->nType < NUM_OF_FILE_TYPES ) 
			TypeCaption = FileTypes[Entry->nType];
		printf("%-3d %-22s %2x.%02x.%04x  0x%-8X %-8d    0x%-8X", Entry->nId, TypeCaption, Entry->nVersion>>24, (Entry->nVersion>>16)&0xff, Entry->nVersion&0xffff, Entry->nOffset, Entry->nSize, Entry->nCrc);
		if (Entry->nAttributes & 0x8000)
		{
			printf(" [R]");
		}
		printf("\n");
	}

	return true;
}

bool Reset(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() != 2)
	{
		printf("Usage: %s <type>\n", Command[0].c_str());
		printf("          power|soft\n");
		return true;
	}

	XnParamResetType Type;
	if (Command[1] == "power")
		Type = XN_RESET_TYPE_POWER;
	else if (Command[1] == "soft")
		Type = XN_RESET_TYPE_SOFT;
	else
	{
		printf("Unknown reset type (power|soft)\n");
		return true;
	}

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_RESET, (XnUInt64)Type);
	if (rc != openni::STATUS_OK)
	{
		printf("Error: %s\n", openni::OpenNI::getExtendedError());
	}

	return true;
}

bool Log(openni::Device& Device, vector<string>& /*Command*/)
{
	XnUChar LogBuffer[XN_MAX_LOG_SIZE] = {0};
	XnBool bAll = true;
	openni::Status rc;

	do
	{
		LogBuffer[0] = '\0';
		int size = sizeof(LogBuffer);
		rc = Device.getProperty(XN_MODULE_PROPERTY_FIRMWARE_LOG, LogBuffer, &size);
		if (rc != openni::STATUS_OK)
		{
			printf("Error: %s\n", openni::OpenNI::getExtendedError());
			break;
		}

		if (LogBuffer[0] != '\0')
		{
			printf("%s", LogBuffer);
		}
		else
		{
			bAll = false;
		}
	} while (bAll);

	return true;
}

bool Script(openni::Device& Device, vector<string>& Command)
{

	if (Command.size() != 2)
	{
		cout << "Usage: " << Command[0] << " <file>" << endl;
		return true;
	}
	string filename = "";

	for (unsigned int i = 1; i < Command.size(); i++)
	{
		if (i != 1)
			filename += " ";
		filename += Command[i];
	}

	ifstream ifs;
	ifs.open(filename.c_str());
	if (!ifs.good())
	{
		cout << "Bad file" << endl;
		return true;
	}
	mainloop(Device, ifs, false);
	ifs.close();
	return true;
}

bool Help(openni::Device& /*Device*/, vector<string>& /*Command*/)
{
	for (map<string, cbfunc>::iterator iter = cbs.begin(); iter != cbs.end(); ++iter)
	{
		cout << "\"" << iter->first << "\" - " << helps[iter->first] << endl;
	}

	return true;
}

bool SetGeneralParam(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() != 3)
	{
		cout << "Usage: " << Command[0] << " <param> <value>" << endl;
		return true;
	}

	int nParam, nValue;

	if (!atoi2(Command[1].c_str(), &nParam))
	{
		printf("Don't understand %s as a parameter\n", Command[1].c_str());
		return true;
	}
	if (!atoi2(Command[2].c_str(), &nValue))
	{
		printf("Don't understand %s as a value\n", Command[2].c_str());
		return true;
	}

	XnInnerParamData Param;
	Param.nParam = (unsigned short)nParam;
	Param.nValue = (unsigned short)nValue;

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_FIRMWARE_PARAM, Param);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}
	else
	{
		printf("Done\n");
	}

	return true;
}

bool GetGeneralParam(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() != 2)
	{
		cout << "Usage: " << Command[0] << " <param>" << endl;
		return true;
	}

	int nParam;
	if (!atoi2(Command[1].c_str(), &nParam))
	{
		printf("Don't understand %s as a parameter\n", Command[1].c_str());
		return true;
	}

	XnInnerParamData Param;
	Param.nParam = (unsigned short)nParam;
	Param.nValue = (unsigned short)0;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_FIRMWARE_PARAM, &Param);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}
	else
	{
		cout << "Param[" << Param.nParam << "] = " << Param.nValue << endl;
	}

	return true;
}

bool WriteI2C(openni::Device& Device, vector<string>& Command, XnControlProcessingData& I2C)
{
	if (Command.size() != 5)
	{
		cout << "Usage: " << Command[0] << " " << Command[1] << " <cmos> <register> <value>" << endl;
		return true;
	}

	int nRegister, nValue;
	if (!atoi2(Command[3].c_str(), &nRegister))
	{
		printf("Don't understand %s as a register\n", Command[3].c_str());
		return true;
	}
	if (!atoi2(Command[4].c_str(), &nValue))
	{
		printf("Don't understand %s as a value\n", Command[4].c_str());
		return true;
	}
	I2C.nRegister = (unsigned short)nRegister;
	I2C.nValue = (unsigned short)nValue;

	int nParam = 0;

	int command;
	if (!atoi2(Command[2].c_str(), &command))
	{
		printf("cmos should be 0 (depth) or 1 (image)\n");
		return true;
	}

	if (command == 1)
		nParam = XN_MODULE_PROPERTY_DEPTH_CONTROL;
	else if (command == 0)
		nParam = XN_MODULE_PROPERTY_IMAGE_CONTROL;
	else
	{
		cout << "cmos must be 0/1" << endl;
		return true;
	}

	openni::Status rc = Device.setProperty(nParam, I2C);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}

	return true;
}

bool ReadI2C(openni::Device& Device, vector<string>& Command, XnControlProcessingData& I2C)
{
	if (Command.size() != 4)
	{
		cout << "Usage: " << Command[0] << " " << Command[1] << " <cmos> <register>" << endl;
		return true;
	}

	int nRegister;
	if (!atoi2(Command[3].c_str(), &nRegister))
	{
		printf("Don't understand %s as a register\n", Command[3].c_str());
		return true;
	}
	I2C.nRegister = (unsigned short)nRegister;

	int nParam = 0;

	int command;
	if (!atoi2(Command[2].c_str(), &command))
	{
		cout << "cmos must be 0/1" << endl;
		return true;
	}

	if (command == 1)
		nParam = XN_MODULE_PROPERTY_DEPTH_CONTROL;
	else if (command == 0)
		nParam = XN_MODULE_PROPERTY_IMAGE_CONTROL;
	else
	{
		cout << "cmos must be 0/1" << endl;
		return true;
	}

	if (Device.getProperty(nParam, &I2C) != openni::STATUS_OK)
	{
		cout << "GetParam failed!" << endl;
		return true;
	}

	cout << "I2C(" << command << ")[0x" << hex << I2C.nRegister << "] = 0x" << hex << I2C.nValue << endl;

	return true;
}

bool GeneralI2C(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 2)
	{
		cout << "Usage: " << Command[0] << " <read/write> ..." << endl;
		return true;
	}
	XnControlProcessingData I2C;

	if (Command[1] == "read")
	{
		return ReadI2C(Device, Command, I2C);
	}
	else if (Command[1] == "write")
	{
		return WriteI2C(Device, Command, I2C);
	}

	cout << "Usage: " << Command[0] << " <read/write> ..." << endl;
	return true;
}

bool WriteAHB(openni::Device& Device, vector<string>& Command, XnAHBData& AHB)
{
	if (Command.size() != 5)
	{
		cout << "Usage: " << Command[0] << " " << Command[1] << " <register> <value> <mask>" << endl;
		return true;
	}

	int nRegister, nValue, nMask;

	if (!atoi2(Command[2].c_str(), &nRegister))
	{
		printf("Can't understand %s as register\n", Command[2].c_str());
		return true;
	}
	if (!atoi2(Command[3].c_str(), &nValue))
	{
		printf("Can't understand %s as value\n", Command[3].c_str());
		return true;
	}
	if (!atoi2(Command[4].c_str(), &nMask))
	{
		printf("Can't understand %s as mask\n", Command[4].c_str());
		return true;
	}


	AHB.nRegister = nRegister;
	AHB.nValue = nValue;
	AHB.nMask = nMask;

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_AHB, AHB);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}

	return true;
}

bool ReadAHB(openni::Device& Device, vector<string>& Command, XnAHBData& AHB)
{
	if (Command.size() != 3)
	{
		cout << "Usage: " << Command[0] << " " << Command[1] << " <register>" << endl;
		return true;
	}

	int nRegister;
	if (!atoi2(Command[2].c_str(), &nRegister))
	{
		printf("Can't understand %s as register\n", Command[2].c_str());
		return true;
	}

	AHB.nRegister = nRegister;
	AHB.nValue = 0;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_AHB, &AHB);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}
	else
	{
		cout << "AHB[0x" << hex << AHB.nRegister << "] = 0x" << hex << AHB.nValue << endl;
	}

	return true;
}

bool GeneralAHB(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 2)
	{
		cout << "Usage: " << Command[0] << " <read/write> ..." << endl;
		return true;
	}
	XnAHBData AHB;

	if (Command[1] == "read")
	{
		return ReadAHB(Device, Command, AHB);
	}
	else if (Command[1] == "write")
	{
		return WriteAHB(Device, Command, AHB);
	}

	cout << "Usage: " << Command[0] << " <read/write> ..." << endl;
	return true;
}

bool Upload(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 3)
	{
		cout << "Usage: " << Command[0] << " <offset> <file> [ro]" << endl;
		return true;
	}
	int nOffset;

	if (!atoi2(Command[1].c_str(), &nOffset))
	{
		printf("Can't understand %s as offset\n", Command[1].c_str());
		return true;
	}

	XnUInt16 nAttributes = 0;

	if (Command.size() >= 4)
	{
		if (Command[3] == "ro")
		{
			nAttributes |= XnFileAttributeReadOnly;
		}
	}

	XnParamFileData ParamFile;
	ParamFile.nOffset = nOffset;
	ParamFile.strFileName = Command[2].c_str();
	ParamFile.nAttributes = nAttributes;

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_FILE, ParamFile);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}

	return true;
}

bool Download(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 3)
	{
		cout << "Usage: " << Command[0] << " <id> <file>" << endl;
		return true;
	}
	int nId;

	if (!atoi2(Command[1].c_str(), &nId))
	{
		printf("Can't understand %s as id\n", Command[1].c_str());
		return true;
	}
	XnParamFileData ParamFile = { (uint32_t) nId, Command[2].c_str()};

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_FILE, &ParamFile);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}

	return true;
}

bool Filter(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 2)
	{
		cout << "Usage: " << Command[0] << " <get/set> ..." << endl;
		return true;
	}

	XnInt32 nFilter;
	openni::Status rc;

	if (Command[1] == "get")
	{
		XnUInt64 nValue;
		rc = Device.getProperty(XN_MODULE_PROPERTY_FIRMWARE_LOG_FILTER, &nValue);
		if (rc == openni::STATUS_OK)
		{
			nFilter = (XnInt32)nValue;
			printf("Filter 0x%04x\n", nFilter);
		}
		else
		{
			printf("Failed: %s\n", openni::OpenNI::getExtendedError());
		}
	}
	else if (Command[1] == "set")
	{
		if (Command.size() < 3)
		{
			cout << "Usage: " << Command[0] << " set <filter>" << endl;
			return true;
		}
		if (!atoi2(Command[2].c_str(), &nFilter))
		{
			printf("Can't understand %s as filter\n", Command[2].c_str());
			return true;
		}
		rc = Device.setProperty(XN_MODULE_PROPERTY_FIRMWARE_LOG_FILTER, (XnUInt64)nFilter);
		if(rc == openni::STATUS_OK)
		{
			printf("Done.\n");
		}
		else
		{
			printf("Failed: %s\n", openni::OpenNI::getExtendedError());
		}
	}
	else
	{
		cout << "Usage: " << Command[0] << " <get/set> ..." << endl;
	}

	return true;
	
}

bool Led(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 3)
	{
		cout << "Usage: " << Command[0] << " <id> <on|off>" << endl;
		return true;
	}

	int nLedId;
	if (!atoi2(Command[1].c_str(), &nLedId))
	{
		printf("Can't understand '%s' as LED id\n", Command[1].c_str());
		return true;
	}

	int nState;
	if (Command[2] == "on")
		nState = 1;
	else if (Command[2] == "off")
		nState = 0;
	else
	{
		printf("State must be 'on' or 'off'!\n");
		return true;
	}

	XnLedState ledState;
	ledState.nLedID = (uint16_t)nLedId;
	ledState.nState = (uint16_t)nState;

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_LED_STATE, ledState);
	if(rc == openni::STATUS_OK)
	{
		printf("Done.\n");
	}
	else
	{
		printf("Failed: %s\n", openni::OpenNI::getExtendedError());
	}
	return true;
}

bool ReadFixed(openni::Device& Device, vector<string>& Command)
{
	int nParam = -1;
	if (Command.size() > 1)
	{
		if (!atoi2(Command[1].c_str(), &nParam))
		{
			printf("Don't understand %s as a parameter\n", Command[1].c_str());
			return true;
		}
	}

	XnUInt32 anFixedParams[100];
	XnDynamicSizeBuffer buffer;
	buffer.nMaxSize = sizeof(anFixedParams);
	buffer.pData = anFixedParams;
	buffer.nDataSize = 0;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_FIXED_PARAMS, &buffer);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}
	else
	{
		if (nParam != -1)
		{
			if (buffer.nDataSize < nParam * sizeof(XnUInt32))
			{
				cout << "Invalid index! Last index is " << buffer.nDataSize / sizeof(XnUInt32) - 1 << endl;
			}
			else
			{
				cout << "Fixed Param [" << nParam << "] = " << anFixedParams[nParam] << endl;
			}
		}
		else
		{
			for (XnUInt32 i = 0; i < buffer.nDataSize / sizeof(XnUInt32); ++i)
			{
				cout << "Fixed Param [" << i << "] = " << anFixedParams[i] << endl;
			}
		}
	}

	return true;
}

bool Debug(openni::Device& /*Device*/, vector<string>& Command)
{
	for (unsigned int i = 0; i < Command.size(); i++)
		cout << i << ". \"" << Command[i] << "\"" << endl;

	return true;
}

void RegisterCB(string cmd, cbfunc func, const string& strHelp)
{
	for (unsigned int i = 0; i < cmd.size(); i++)
		cmd[i] = (char)tolower(cmd[i]);
	cbs[cmd] = func;
	helps[cmd] = strHelp;
}

void RegisterMnemonic(string strMnemonic, string strCommand)
{
	for (unsigned int i = 0; i < strCommand.size(); i++)
		strCommand[i] = (char)tolower(strCommand[i]);
	for (unsigned int i = 0; i < strMnemonic.size(); i++)
		strMnemonic[i] = (char)tolower(strMnemonic[i]);

	if (cbs.find(strCommand) != cbs.end())
	{
		mnemonics[strMnemonic] = cbs[strCommand];
	}
}

bool ReadFlash(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 4)
	{
		cout << "Usage: " << Command[0] << " <offset> <size> <file>" << endl;
		return true;
	}

	XnParamFlashData ParamFlash;

	if (!atoi2(Command[1].c_str(), (int*)&ParamFlash.nOffset))
	{
		printf("Can't understand %s as offset\n", Command[1].c_str());
		return true;
	}

	if (!atoi2(Command[2].c_str(), (int*)&ParamFlash.nSize))
	{
		printf("Can't understand %s as size\n", Command[1].c_str());
		return true;
	}

	int nSizeInBytes = ParamFlash.nSize * sizeof(XnUInt16);
	ParamFlash.pData = new XnUChar[nSizeInBytes];

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_FLASH_CHUNK, &ParamFlash);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}

	xnOSSaveFile(Command[3].c_str(), ParamFlash.pData, nSizeInBytes);

	return true;
}

bool RunBIST(openni::Device& Device, vector<string>& Command)
{
	string sBistTests[] = 
	{
		"ImageCmos",
		"IrCmos",
		"Potentiometer",
		"Flash",
		"FlashFull",
		"Projector",
		"Tec",
		"NESA",
		"NESAUnlimited"
	};

	int nBistTests = sizeof(sBistTests) / sizeof(string);

	string sBistErrors[] = 
	{
		"Ram",
		"Ir Cmos Control Bus",
		"Ir Cmos Data Bus",
		"Ir Cmos Bad Version",
		"Ir Cmos Reset",
		"Ir Cmos Trigger",
		"Ir Cmos Strobe",
		"Color Cmos Control Bus",
		"Color Cmos Data Bus",
		"Color Cmos Bad Version",
		"Color Cmos Reset",
		"Flash Write Line",
		"Flash Test",
		"Potentiometer Control Bus",
		"Potentiometer",
		"Audio Test",
		"Projector LD Failure",
		"Projector LD Failsafe Trigger",
		"Projector Failsafe High Path",
		"Projector Failsafe Low Path",
		"Heater Crossed",
		"Heater Disconnected",
		"Cooler Crossed",
		"Cooler Disconnected",
		"TEC still initializing",
		"TEC out of range",
		"NESA",
	};

	int nBistErrors = sizeof(sBistErrors) / sizeof(string);

	XnBist bist;
	bist.nTestsMask = 0;

	bool bShowUsage = false;

	if (Command.size() == 1)
	{
		bist.nTestsMask = (uint32_t)XN_BIST_ALL;
	}
	else if (Command.size() == 2 && Command[1] == "help")
	{
		bShowUsage = true;
	}
	else
	{
		for (XnUInt32 i = 1; i < Command.size(); ++i)
		{
			bool bFound = false;

			// search for this test
			for (int j = 0; j < nBistTests; ++j)
			{
				if (sBistTests[j] == Command[i])
				{
					bist.nTestsMask |= (1 << j);
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				if (Command[i] == "all")
				{
					bist.nTestsMask = (uint32_t)XN_BIST_ALL;
				}
				else
				{
					cout << "Unknown test: " << Command[i] << endl;
					bShowUsage = true;
					break;
				}
			}
		}
	}

	if (bShowUsage)
	{
		cout << "Usage: " << Command[0] << " [test_list]" << endl;
		cout << "where test_list can be one or more of:" << endl;
		for (int i = 0; i < nBistTests; ++i)
		{
			cout << "\t" << sBistTests[i] << endl;
		}
		return true;
	}

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_BIST, bist);
	if (rc != openni::STATUS_OK)
	{
		printf("Error: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	if (bist.nFailures == 0)
	{
		cout << "BIST passed." << endl;
	}
	else
	{
		cout << "BIST failed. Returning code: " << bist.nFailures << endl;
		cout << "The following tests have failed:" << endl;
		for (int i = 0; i < nBistErrors; ++i)
		{
			int nMask = 1 << i;
			if ((bist.nFailures & nMask) != 0)
			{
				cout << "\t" << sBistErrors[i] << endl;
			}
		}
	}

	return true;
}

bool CalibrateTec(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 3)
	{
		cout << "Usage: " << Command[0] << " " << Command[1] << " <set point>" << endl;
		return true;
	}

	int nSetPoint;

	if (!atoi2(Command[2].c_str(), (int*)&nSetPoint))
	{
		printf("Can't understand %s as set point\n", Command[2].c_str());
		return true;
	}

	if (nSetPoint > 0xFFFF)
	{
		printf("Set point can't fit in a 16-bit integer!\n");
		return true;
	}

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_TEC_SET_POINT, (XnUInt64)nSetPoint);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}

	return true;
}

bool GetTecData(openni::Device& Device, vector<string>& /*Command*/)
{
	XnTecData TecData;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_TEC_STATUS, &TecData);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}
	else
	{
		printf("SetPointVoltage: %hd\nCompensationVoltage: %hd\nDutyCycle: %hd\nHeatMode: %hd\nProportionalError: %d\nIntegralError: %d\nDeriativeError: %d\nScanMode: %hd\n",
			TecData.m_SetPointVoltage, TecData.m_CompensationVoltage, TecData.m_TecDutyCycle, 
			TecData.m_HeatMode, TecData.m_ProportionalError, TecData.m_IntegralError, 
			TecData.m_DerivativeError, TecData.m_ScanMode);
	}

	return true;
}

bool GetTecFastConvergenceData(openni::Device& Device, vector<string>& /*Command*/)
{
	XnTecFastConvergenceData TecData;
    XnFloat     SetPointTemperature;
    XnFloat     MeasuredTemperature;
    XnFloat     ErrorTemperature;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_TEC_FAST_CONVERGENCE_STATUS, &TecData);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}
	else
	{
        SetPointTemperature = TecData.m_SetPointTemperature;
        MeasuredTemperature = TecData.m_MeasuredTemperature;
        /* calculate error in temperature */
        ErrorTemperature =
            (XnFloat)TecData.m_SetPointTemperature - (XnFloat)TecData.m_MeasuredTemperature;

        /* scale back temperature values, as they are given scaled by factor
        of 100 (for precision) */
        SetPointTemperature = SetPointTemperature / 100;
        MeasuredTemperature = MeasuredTemperature / 100;
        ErrorTemperature = ErrorTemperature / 100;

        printf("SetPointTemperature: %f\nMeasuredTemperature: %f\nError: %f\nDutyCyclePercents: %hd\nHeatMode: %hd\nProportionalError: %d\nIntegralError: %d\nDeriativeError: %d\nScanMode: %hd\nTemperatureRange: %hd\n",
			SetPointTemperature, MeasuredTemperature, ErrorTemperature, TecData.m_TecDutyCycle, 
			TecData.m_HeatMode, TecData.m_ProportionalError, TecData.m_IntegralError, 
			TecData.m_DerivativeError, TecData.m_ScanMode, TecData.m_TemperatureRange);
	}

	return true;
}

bool Tec(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() > 1)
	{
		if (Command[1] == "calib")
		{
			return CalibrateTec(Device, Command);
		}
		else if (Command[1] == "get")
		{
			return GetTecData(Device, Command);
		}
	}

	cout << "Usage: " << Command[0] << " <calib/get> ..." << endl;
	return true;
}

bool TecFC(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() > 1)
	{
		if (Command[1] == "calib")
		{
			return CalibrateTec(Device, Command);
		}
		else if (Command[1] == "get")
		{
			return GetTecFastConvergenceData(Device, Command);
		}
	}

	cout << "Usage: " << Command[0] << " <calib/get> ..." << endl;
	return true;
}


bool CalibrateEmitter(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 3)
	{
		cout << "Usage: " << Command[0] << " " << Command[1] << " <set point>" << endl;
		return true;
	}

	int nSetPoint;

	if (!atoi2(Command[2].c_str(), (int*)&nSetPoint))
	{
		printf("Can't understand %s as set point\n", Command[2].c_str());
		return true;
	}

	if (nSetPoint > 0xFFFF)
	{
		printf("Set point can't fit in a 16-bit integer!\n");
		return true;
	}

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_EMITTER_SET_POINT, (XnUInt64)nSetPoint);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}

	return true;
}

bool GetEmitterData(openni::Device& Device, vector<string>& /*Command*/)
{
	XnEmitterData EmitterData;

	openni::Status rc = Device.getProperty(XN_MODULE_PROPERTY_EMITTER_STATUS, &EmitterData);
	if (rc != openni::STATUS_OK)
	{
		printf("%s\n", openni::OpenNI::getExtendedError());
	}
	else
	{
		printf("State: %hd\n", EmitterData.m_State);
		printf("SetPointVoltage: %hd\n", EmitterData.m_SetPointVoltage);
		printf("SetPointClocks: %hd\n", EmitterData.m_SetPointClocks);
		printf("PD Reading: %hd\n", EmitterData.m_PD_Reading);
		printf("EmitterSet: %hd\n", EmitterData.m_EmitterSet);
		printf("SettingLogic: %hd\n", EmitterData.m_EmitterSettingLogic);
		printf("LightMeasureLogic: %hd\n", EmitterData.m_LightMeasureLogic);
		printf("APC Enabled: %hd\n", EmitterData.m_IsAPCEnabled);
		printf("StepSize: %hd\n", EmitterData.m_EmitterSetStepSize);

		if (g_DeviceVersion.FWVer < XN_SENSOR_FW_VER_5_3)
		{
			printf("Tolerance: %hd\n", EmitterData.m_ApcTolerance);
		}
		else
		{
			printf("SubClocking: %hd\n", EmitterData.m_SubClocking);
			printf("Precision: %hd\n", EmitterData.m_Precision);
		}
	}

	return true;
}

bool Emitter(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() > 1)
	{
		if (Command[1] == "calib")
		{
			return CalibrateEmitter(Device, Command);
		}
		else if (Command[1] == "get")
		{
			return GetEmitterData(Device, Command);
		}
		else if (Command[1] == "on")
		{
			openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_EMITTER_STATE, (XnUInt64)TRUE);
			if (rc != openni::STATUS_OK)
			{
				printf("Error: %s\n", openni::OpenNI::getExtendedError());
			}
			return true;
		}
		else if (Command[1] == "off")
		{
			openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_EMITTER_STATE, (XnUInt64)FALSE);
			if (rc != openni::STATUS_OK)
			{
				printf("Error: %s\n", openni::OpenNI::getExtendedError());
			}
			return true;
		}
	}

	cout << "Usage: " << Command[0] << " <calib|get|on|off> ..." << endl;
	return true;
}

bool ProjectorFault(openni::Device& Device, vector<string>& Command)
{
	if (Command.size() < 3)
	{
		cout << "Usage: " << Command[0] << " <min> <max>" << endl;
		return true;
	}

	int nMin, nMax;
	if (!atoi2(Command[1].c_str(), (int*)&nMin))
	{
		printf("Can't understand %s as min\n", Command[1].c_str());
		return true;
	}

	if (!atoi2(Command[2].c_str(), (int*)&nMax))
	{
		printf("Can't understand %s as max\n", Command[2].c_str());
		return true;
	}

	XnProjectorFaultData ProjectorFaultData;
	ProjectorFaultData.nMinThreshold = (uint16_t)nMin;
	ProjectorFaultData.nMaxThreshold = (uint16_t)nMax;

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_PROJECTOR_FAULT, ProjectorFaultData);
	if (rc != openni::STATUS_OK)
	{
		printf("Error: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	if (ProjectorFaultData.bProjectorFaultEvent)
		printf("ProjectorFault event occurred!\n");
	else
		printf("ProjectorFault OK.\n");

	return true;
}

bool StartReadData(openni::Device& Device, vector<string>& Command)
{
	XnSensorUsbInterface usbInterface = XN_SENSOR_USB_INTERFACE_DEFAULT;

	if (Command.size() > 1)
	{
		if (xnOSStrCaseCmp(Command[1].c_str(), "bulk") == 0)
		{
			usbInterface = XN_SENSOR_USB_INTERFACE_BULK_ENDPOINTS;
		}
		else if (xnOSStrCaseCmp(Command[1].c_str(), "iso") == 0)
		{
			usbInterface = XN_SENSOR_USB_INTERFACE_ISO_ENDPOINTS;
		}
		else
		{
			cout << "Usage: " << Command[0] << " [bulk/iso]" << endl;
			return true;
		}
	}

	openni::Status rc = Device.setProperty(XN_MODULE_PROPERTY_USB_INTERFACE, (XnUInt64)usbInterface);
	if (rc != openni::STATUS_OK)
	{
		printf("Can't set USB interface: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	if (!g_depthStream.isValid())
	{
		// create stream
		rc = g_depthStream.create(Device, openni::SENSOR_DEPTH);
		if (rc != openni::STATUS_OK)
		{
			printf("Can't create depth stream: %s\n", openni::OpenNI::getExtendedError());
			return true;
		}
	}

	if (!g_colorStream.isValid())
	{
		rc = g_depthStream.create(Device, openni::SENSOR_COLOR);
		if (rc != openni::STATUS_OK)
		{
			printf("Can't create image stream: %s\n", openni::OpenNI::getExtendedError());
			return true;
		}
	}

	// and cause reading to take place (without start streaming)
	rc = g_depthStream.setProperty(XN_STREAM_PROPERTY_ACTUAL_READ_DATA, TRUE);
	if (rc != openni::STATUS_OK)
	{
		printf("Can't start depth endpoint: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	rc = g_colorStream.setProperty(XN_STREAM_PROPERTY_ACTUAL_READ_DATA, TRUE);
	if (rc != openni::STATUS_OK)
	{
		printf("Can't start image endpoint: %s\n", openni::OpenNI::getExtendedError());
		return true;
	}

	printf("Endpoints are now open.\n");

	return true;
}

int main(int argc, char **argv)
{
	// Open Device.
	openni::Status rc;

	// don't perform reset on startup
	g_openMode = "R";

	rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK)
	{
		printf("Init failed: %s\n", openni::OpenNI::getExtendedError());
		return 1;
	}

	if (argc > 1 && strcmp(argv[1], "-safe") == 0)
	{
		g_openMode = "RL";
		--argc;
		++argv;
	}

	openni::Device Device;
	if (!Connect(Device))
	{
		return 2;
	}

	vector<string> DummyCommand;
	Version(Device, DummyCommand);

	RegisterCB("exit", &byebye, "Exit interactive mode");
	RegisterMnemonic("bye", "exit");

	RegisterCB("set", &SetGeneralParam, "set inner parameters");
	RegisterCB("get", &GetGeneralParam, "get inner parameters");

	RegisterCB("ahb", &GeneralAHB, "read/write the AHB");

	RegisterCB("i2c", &GeneralI2C, "read/write the CMOS");

	RegisterCB("upload", &Upload, "Upload file to flash");
	RegisterCB("download", &Download, "Download file from flash");

	RegisterCB("help", &Help, "Get list of available commands");
	RegisterMnemonic("?", "help");

	RegisterCB("script", &Script, "Run in batch mode");
	RegisterMnemonic("-s", "script");

	RegisterCB("Version", &Version, "Get version");

	RegisterCB("FileList", &FileList, "Get File List");
	RegisterMnemonic("dir", "FileList");

	RegisterCB("Reset", &Reset, "Reset the device");

	RegisterCB("Delete", &DeleteFile, "Delete File");

	RegisterCB("Log", &Log, "Get Log");

	RegisterCB("Filter", &Filter, "Set/Get Log Filter");

	RegisterCB("Reconnect", &Reconnect, "Close+Open (Init+Shutdown if All)");

	RegisterCB("Sleep", &Sleep, "Sleep some milliseconds. Useful in scripts");

	RegisterCB("Attrib", &Attrib, "Get/Set file attributes");

	RegisterCB("BIST", &RunBIST, "Run BIST");

	RegisterCB("ReadFlash", &ReadFlash, "Reads a chunk of data from the flash");

	RegisterCB("tec", &Tec, "Calibrate/Read TEC");

	RegisterCB("tecfc", &TecFC, "Calibrate/Read TEC");

	RegisterCB("emitter", &Emitter, "Calibrate/Read Emitter");

	RegisterCB("projectorFault", &ProjectorFault, "Calibrate ProjectorFault");

	RegisterCB("readfixed", &ReadFixed, "Read Fixed Params");

	RegisterCB("led", &Led, "Set LED state");

	RegisterCB("startread", &StartReadData, "Starts data reading from endpoints");

	if (argc == 1)
	{
		mainloop(Device, cin, true);
	}
	else
	{
		vector<string> Command;
		for (int i = 1; i < argc; i++)
		{
			Command.push_back(argv[i]);
		}

		for (unsigned int i = 0; i < Command[0].size(); i++)
			Command[0][i] = (char)tolower(Command[0][i]);

		if (Command[0].size() == 0)
		{
			//
		}
		else if (cbs.find(Command[0]) != cbs.end())
		{
			(*cbs[Command[0]])(Device, Command);
		}
		else if (mnemonics.find(Command[0]) != mnemonics.end())
		{
			(*mnemonics[Command[0]])(Device, Command);
		}
		else
		{
			cout << "Unknown command \"" << Command[0] << "\"" << endl;
		}
	}

	g_depthStream.destroy();
	g_colorStream.destroy();
	Device.close();

	return 0;
}
