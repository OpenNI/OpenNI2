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
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "XnDeviceSensorProtocol.h"
#include "XnDeviceSensorIO.h"
#include "Uncomp.h"
#include "XnHostProtocol.h"
#include <XnLog.h>
#include <XnProfiling.h>
#include "XnStreamProcessor.h"
#include "XnSensor.h"
#include <XnOS.h>

FILE* g_fUSBDump;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
XnBool XN_CALLBACK_TYPE XnDeviceSensorProtocolUsbEpCb(XnUChar* pBuffer, XnUInt32 nBufferSize, void* pCallbackData)
{
	XN_PROFILING_START_MT_SECTION("XnDeviceSensorProtocolUsbEpCb");

	XnUInt32 nReadBytes;
	XnUInt16 nMagic;

	XnSpecificUsbDevice* pDevice = (XnSpecificUsbDevice*)pCallbackData;
	XnDevicePrivateData* pDevicePrivateData = pDevice->pDevicePrivateData;
	XnUChar* pBufEnd = pBuffer + nBufferSize;

	XnSpecificUsbDeviceState* pCurrState = &pDevice->CurrState;

	while (pBuffer < pBufEnd)
	{
		switch (pCurrState->State)
		{
		case XN_WAITING_FOR_CONFIGURATION:
			pCurrState->State = XN_IGNORING_GARBAGE;
			pCurrState->nMissingBytesInState = pDevice->nIgnoreBytes;
			break;

		case XN_IGNORING_GARBAGE:
			// ignore first bytes on this endpoint. NOTE: due to a bug in the firmware, the first data received
			// on each endpoint is corrupt, causing wrong timestamp calculation, causing future (true) timestamps
			// to be calculated wrongly. By ignoring the first data received on each endpoint we hope to get
			// only valid data.
			nReadBytes = XN_MIN((XnUInt32)(pBufEnd - pBuffer), pCurrState->nMissingBytesInState);

			if (nReadBytes > 0)
			{
				xnLogVerbose(XN_MASK_SENSOR_PROTOCOL, "ignoring %d bytes - ignore garbage phase!", nReadBytes);
				pCurrState->nMissingBytesInState -= nReadBytes;
				pBuffer += nReadBytes;
			}

			if (pCurrState->nMissingBytesInState == 0)
			{
				pCurrState->State = XN_LOOKING_FOR_MAGIC;
				pCurrState->nMissingBytesInState = sizeof(XnUInt16);
			}
			break;

		case XN_LOOKING_FOR_MAGIC:
			nMagic = XN_PREPARE_VAR16_IN_BUFFER(pDevicePrivateData->FWInfo.nFWMagic);

			if (pCurrState->nMissingBytesInState == sizeof(XnUInt8) && // first byte already found
				pBuffer[0] == ((XnUInt8*)&nMagic)[1])	// we have here second byte
			{
				// move to next byte
				pBuffer++;

				// move to next state
				pCurrState->CurrHeader.nMagic = nMagic;
				pCurrState->State = XN_PACKET_HEADER;
				pCurrState->nMissingBytesInState = sizeof(XnSensorProtocolResponseHeader);
				break;
			}

			while (pBuffer < pBufEnd)
			{
				if ((pBuffer + sizeof(XnUInt16) <= pBufEnd) && 
					nMagic == *(XnUInt16*)(pBuffer))
				{
					pCurrState->CurrHeader.nMagic = nMagic;
					pCurrState->State = XN_PACKET_HEADER;
					pCurrState->nMissingBytesInState = sizeof(XnSensorProtocolResponseHeader);
					break;
				}
				else
				{
					pBuffer++;
				}
			}

			if (pBuffer == pBufEnd &&					// magic wasn't found
				pBuffer[-1] == ((XnUInt8*)&nMagic)[0])	// last byte in buffer is first in magic
			{
				// mark that we found first one
				pCurrState->nMissingBytesInState--;
			}

			break;

		case XN_PACKET_HEADER:
			nReadBytes = XN_MIN((XnUInt32)(pBufEnd - pBuffer), pCurrState->nMissingBytesInState);
			xnOSMemCopy((XnUChar*)&pCurrState->CurrHeader + sizeof(XnSensorProtocolResponseHeader) - pCurrState->nMissingBytesInState, 
				pBuffer, nReadBytes);
			pCurrState->nMissingBytesInState -= nReadBytes;
			pBuffer += nReadBytes;

			if (pCurrState->nMissingBytesInState == 0)
			{
				// we have entire header. Fix it
				pCurrState->CurrHeader.nBufSize = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nBufSize);
				pCurrState->CurrHeader.nMagic = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nMagic);
				pCurrState->CurrHeader.nPacketID = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nPacketID);
				pCurrState->CurrHeader.nTimeStamp = XN_PREPARE_VAR32_IN_BUFFER(pCurrState->CurrHeader.nTimeStamp);
				pCurrState->CurrHeader.nType = XN_PREPARE_VAR16_IN_BUFFER(pCurrState->CurrHeader.nType);
				pCurrState->CurrHeader.nBufSize = xnOSEndianSwapUINT16(pCurrState->CurrHeader.nBufSize);
				pCurrState->CurrHeader.nBufSize -= sizeof(XnSensorProtocolResponseHeader);

				pCurrState->State = XN_PACKET_DATA;
				pCurrState->nMissingBytesInState = pCurrState->CurrHeader.nBufSize;
			}
			break;

		case XN_PACKET_DATA:
			nReadBytes = XN_MIN((XnUInt32)(pBufEnd - pBuffer), pCurrState->nMissingBytesInState);
			pDevicePrivateData->pSensor->GetFirmware()->GetStreams()->ProcessPacketChunk(&pCurrState->CurrHeader, pBuffer, pCurrState->CurrHeader.nBufSize - pCurrState->nMissingBytesInState, nReadBytes);
			pBuffer += nReadBytes;
			pCurrState->nMissingBytesInState -= nReadBytes;

			if (pCurrState->nMissingBytesInState == 0)
			{
				pCurrState->State = XN_LOOKING_FOR_MAGIC;
				pCurrState->nMissingBytesInState = sizeof(XnUInt16);
			}
			break;
		}
	}

	XN_PROFILING_END_SECTION;

	return TRUE;
}

XnStatus XnDeviceSensorProtocolFindStreamOfType(XnDevicePrivateData* pDevicePrivateData, const XnChar* strType, const XnChar** ppStreamName)
{
	XnStatus nRetVal = XN_STATUS_OK;
	
	const XnChar* strNames[100];
	XnUInt32 nCount = 100;

	nRetVal = pDevicePrivateData->pSensor->GetStreamNames(strNames, &nCount);
	XN_IS_STATUS_OK(nRetVal);

	for (XnUInt32 i = 0; i < nCount; ++i)
	{
		XnChar strCurType[XN_DEVICE_MAX_STRING_LENGTH];
		nRetVal = pDevicePrivateData->pSensor->GetProperty(strNames[i], XN_STREAM_PROPERTY_TYPE, strCurType);
		XN_IS_STATUS_OK(nRetVal);

		if (strcmp(strType, strCurType) == 0)
		{
			*ppStreamName = strNames[i];
			return (XN_STATUS_OK);
		}
	}

	*ppStreamName = NULL;
	return (XN_STATUS_NO_MATCH);
}

XN_THREAD_PROC XnDeviceSensorProtocolScriptThread(XN_THREAD_PARAM pThreadParam)
{
	// Local function variables	
	XnDevicePrivateData* pDevicePrivateData = (XnDevicePrivateData*)pThreadParam;
	XnSensor* pSensor = pDevicePrivateData->pSensor;
	
	pDevicePrivateData->LogThread.bThreadAlive = TRUE;
	pDevicePrivateData->LogThread.bKillThread = FALSE;

	XnStatus rc = XN_STATUS_OK;
	
	XnUInt32 nVal;
	XnUInt32 nLogEvery = 0;
	XnStatus ret;
	XnI2CWriteData D1;
	XnI2CReadData D2;
	XnUInt32 i=0;

//	if (FALSE /* pDevicePrivateData->bConfigure*/)
	{
		XnChar which;
		XnUInt32 address, value, mask;

		const XnChar* csFileName = "commands.txt";

		XnUInt64 nFileSize = 0;
		rc = xnOSGetFileSize64(csFileName, &nFileSize);
		if (rc != XN_STATUS_OK)
		{
			XN_THREAD_PROC_RETURN(0);
		}

		// read file
		XnChar* pcsCommandsFile = (XnChar*)xnOSCalloc((XnSizeT)(nFileSize + 1), sizeof(XnChar));
		rc = xnOSLoadFile(csFileName, pcsCommandsFile, (XnUInt32)nFileSize);
		if (rc == XN_STATUS_OK)
		{
			xnOSSleep(7000);

			XnChar* pFile = pcsCommandsFile;
			XnInt32 nRead = 0;

			// Parse commands file
			while (*pFile != '\0' && !pDevicePrivateData->LogThread.bKillThread)
			{
				which = *pFile++;

//				printf("Processing %c\n", which);
				switch (which)
				{
				case ';':
				case '#':
					// Comment line
					while (which != '\n' && *pFile != '\0')
						which = *pFile++;
					break;
				case 'V':
					{
						XnVersions Version;
	
						printf("* Requesting Version\n");
						rc = XnHostProtocolGetVersion(pDevicePrivateData, Version);
						if (rc == XN_STATUS_OK)
						{
							printf("** Firmware: V%d.%d.%d; SDK: V%d.%d.%d.%d; Chip: 0x%08x; FPGA: 0x%x; System: 0x%x\n",
								Version.nMajor, Version.nMinor, Version.nBuild,
								Version.SDK.nMajor, Version.SDK.nMinor, Version.SDK.nMaintenance, Version.SDK.nBuild,
								Version.nChip, Version.nFPGA, Version.nSystemVersion);
						}
						else
						{
							printf("** GetVersion failed\n");
						}
	
						while (which != '\n' && *pFile != '\0')
							which = *pFile++;
						break;
					}
				case 'L':
					{
						XnChar LogBuffer[XN_MAX_LOG_SIZE];
						XnHostProtocolGetLog(pDevicePrivateData, LogBuffer, XN_MAX_LOG_SIZE);
						printf("%s", (XnChar*)LogBuffer);
						break;
					}
				case 'F':
					{
						XnUInt32 nFilter;
						sscanf(pFile, "%x\n%n", &nFilter, &nRead);
						pFile += nRead;

						printf("* LogFilter: 0x%x\n", nFilter);
						rc = XnHostProtocolSetParam(pDevicePrivateData, PARAM_MISC_LOG_FILTER, (XnUInt16)nFilter);
						if (rc != XN_STATUS_OK)
						{
							printf("** Set Log Filter failed\n");
						}
						else
						{
							printf("** Done\n");
						}
						break;
					}
				case 'E':
						sscanf(pFile, "%u\n%n", &nLogEvery, &nRead);
						pFile += nRead;

						if (nLogEvery != 0)
							printf("* Will log every %u milliseconds\n", nLogEvery);

						rc = pSensor->SetProperty(XN_MODULE_NAME_DEVICE, XN_MODULE_PROPERTY_FIRMWARE_LOG_INTERVAL, (XnUInt64)nLogEvery);
						if (rc != XN_STATUS_OK)
						{
							printf("** Set log interval failed: %s\n", xnGetStatusString(rc));
						}

						break;
				case 'C':
					{
						XnUInt16 nCMOS;
						sscanf(pFile, "%hx %x %x\n%n", &nCMOS, &address, &value, &nRead);
						pFile += nRead;

						printf("* Processing '%c 0x%x 0x%x 0x%x'\n", which, nCMOS, address, value);

						if (pDevicePrivateData->FWInfo.nFWVer >= XN_SENSOR_FW_VER_3_0)
						{
							rc = XnHostProtocolSetCMOSRegisterI2C(pDevicePrivateData, (XnCMOSType)nCMOS, (XnUInt16)address, (XnUInt16)value);
						}
						else
						{
							rc = XnHostProtocolSetCMOSRegister(pDevicePrivateData, (XnCMOSType)nCMOS, (XnUInt16)address, (XnUInt16)value);
						}
		
						if (rc != XN_STATUS_OK)
						{
							printf("** Set CMOS Register failed\n");
						}
						else
						{
							printf("** Done\n");
						}
						break;
					}
				case 'W':
					mask = 0;
					sscanf(pFile, "%x %x %x\n%n", &address, &value, &mask, &nRead);
					pFile += nRead;

					printf("* Processing '%c 0x%x 0x%x 0x%x'\n", which, address, value, mask);

					if (mask == 0)
					{
						printf("** Bad format?\n");
						continue;
					}

					printf("** Sending WriteAHB\n");
					rc = XnHostProtocolWriteAHB(pDevicePrivateData, address, value, mask);
					if (rc != XN_STATUS_OK)
					{
						printf("** Write failed\n");
					}
					else
					{
						printf("** Done\n");
					}
					break;
				case 'R':
					sscanf(pFile, "%x\n%n", &address, &nRead);
					pFile += nRead;

					printf("* Processing '%c 0x%x'\n", which , address);
					rc = XnHostProtocolReadAHB(pDevicePrivateData, address, value);
					if (rc != XN_STATUS_OK)
					{
						printf("** Read failed\n");
					}
					else
					{
						printf("** AHB[0x%x] = 0x%x\n", address, value);
					}
					break;
				case 'P':
					{
						XnUInt32 param;
						sscanf(pFile, "%u %u\n%n", &param, &value, &nRead);
						pFile += nRead;

						printf("* Processing '%c %u %u'\n", which , param, value);
						rc = XnHostProtocolSetParam(pDevicePrivateData, (XnUInt16)param, (XnUInt16)value);
						if (rc != XN_STATUS_OK)
						{
							printf("** Set Param failed\n");
						}
						else
						{
							printf("** Done\n");
						}
					}
					break;
				case 'U':
					{
						XnChar filename[80] = {0};
						XnUInt32 nType;
						sscanf(pFile, "%u %s\n%n", &nType, filename, &nRead);
						pFile += nRead;

						printf("* Processing '%c %u %s'\n", which , nType, filename);
						rc = XnHostProtocolFileUpload(pDevicePrivateData, nType, filename, 0);
						if (rc != XN_STATUS_OK)
						{
							printf("** Upload failed\n");
						}
						else
						{
							printf("** Done\n");
						}
					}
					break;
				case 'D':
					{
						XnChar filename[80] = {0};
						XnUInt32 nType;
						sscanf(pFile, "%u %s\n%n", &nType, filename, &nRead);
						pFile += nRead;

						printf("* Processing '%c %u %s'\n", which , nType, filename);
						rc = XnHostProtocolFileDownload(pDevicePrivateData, (XnUInt16)nType, filename);
						if (rc != XN_STATUS_OK)
						{
							printf("** Download failed\n");
						}
						else
						{
							printf("** Done\n");
						}
					}
					break;
				case 'S':
					sscanf(pFile, "%u\n%n", &nVal, &nRead);
					pFile += nRead;

					xnOSSleep(nVal);
					break;
				case 'N':				
					// Generic I2C Read
					sscanf(pFile, "%hx %hx %hx %hx%n", &D2.nBus, &D2.nSlaveAddress, &D2.nReadSize, &D2.nWriteSize, &nRead);
					pFile += nRead;

					printf ("* I2C Read: Bus: 0x%hx SlaveAddress: 0x%hx ReadSize: 0x%hx WriteSize: 0x%hx\n", D2.nBus, D2.nSlaveAddress, D2.nReadSize, D2.nWriteSize);

					for (i=0; i<D2.nWriteSize; i++)
					{
						sscanf(pFile, "%hx%n", &D2.cpWriteBuffer[i], &nRead);
						pFile += nRead;

						printf ("	WriteData[%ud] = 0x%hx\n", i, D2.cpWriteBuffer[i]);
					}

					ret = XnHostProtocolReadI2C(pDevicePrivateData, &D2);
					printf ("** I2C Read Status: %u\n", ret);

					for (i=0; i<D2.nReadSize; i++)
					{
						printf ("	ReadData[%ud] = 0x%hx\n", i, D2.cpReadBuffer[i]);
					}

					break;
				case 'M':				
					// Generic I2C Write
					sscanf(pFile, "%hx %hx %hx%n", &D1.nBus, &D1.nSlaveAddress, &D1.nWriteSize, &nRead);
					pFile += nRead;

					printf ("* I2C Write: Bus: 0x%hx SlaveAddress: 0x%hx WriteSize: 0x%hx\n", D1.nBus, D1.nSlaveAddress, D1.nWriteSize);

					for (i=0; i<D1.nWriteSize; i++)
					{
						sscanf(pFile, "%hx%n", &D1.cpWriteBuffer[i], &nRead);
						pFile += nRead;

						printf ("	WriteData[%ud] = 0x%hx\n", i, D1.cpWriteBuffer[i]);
					}

					ret = XnHostProtocolWriteI2C(pDevicePrivateData, &D1);
					printf ("** I2C Write Status: %u\n", ret);				

					break;
				case 'X':
					printf ("* Exiting...\n");
					exit(0);
				case 'B':
					{
						printf("* Running BIST...\n");

						XnUInt32 nFailures;
						rc = XnHostProtocolRunBIST(pDevicePrivateData, (XnUInt32)XN_BIST_ALL, &nFailures);
						if (rc != XN_STATUS_OK)
							printf("** Failed to run BIST: %s", xnGetStatusString(rc));
						else
							printf("** BIST %s\n", (nFailures == 0) ? "Passed" : "Failed");
						break;
					}
				case 'Z':
					{
						XnUInt16 nSetPoint = 0;
						sscanf(pFile, "%hu%n", &nSetPoint, &nRead);
						pFile += nRead;

						printf("* TEC: Setting SetPoint to %d\n", nSetPoint);
						rc = XnHostProtocolCalibrateTec(pDevicePrivateData, nSetPoint);
						if (rc != XN_STATUS_OK)
						{
							printf("** Set TEC set point failed: %s\n", xnGetStatusString(rc));
						}
						else
						{
							printf("** Done\n");
						}

						break;
					}
				case 'A':
					{
						XnUInt16 nSetPoint = 0;
						sscanf(pFile, "%hu%n", &nSetPoint, &nRead);
						pFile += nRead;

						printf("* Emitter: Setting SetPoint to %d\n", nSetPoint);
						rc = XnHostProtocolCalibrateEmitter(pDevicePrivateData, nSetPoint);
						if (rc != XN_STATUS_OK)
						{
							printf("** Set Emitter set point failed: %s\n", xnGetStatusString(rc));
						}
						else
						{
							printf("** Done\n");
						}

						break;
					}
				case 'G':
					{
						XnUInt16 nMin, nMax = 0;
						sscanf(pFile, "%hu %hu%n", &nMin, &nMax, &nRead);

						printf("* Testing Projector Fault. Min: %hu, Max: %hu\n", nMin, nMax);

						XnBool bProjectorFaultEvent;
						rc = XnHostProtocolCalibrateProjectorFault(pDevicePrivateData, nMin, nMax, &bProjectorFaultEvent);
						if (rc != XN_STATUS_OK)
						{
							printf("** Testing Projector Fault failed: %s\n", xnGetStatusString(rc));
						}
						else if (bProjectorFaultEvent)
						{
							printf("** Done. Projector Fault event occurred!\n");
						}
						else
						{
							printf("** Done. Projector Fault is OK.\n");
						}

						break;
					}
				default:
					while (which != '\n' && *pFile != '\0')
						which = *pFile++;
					break;
				}
			} // while
		} // file read

		xnOSFree(pcsCommandsFile);

	} // if (configure)

	pDevicePrivateData->LogThread.bThreadAlive = FALSE;
	XN_THREAD_PROC_RETURN (XN_STATUS_OK);
}

