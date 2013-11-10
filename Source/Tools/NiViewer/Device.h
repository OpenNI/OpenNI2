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
#ifndef DEVICE_H
#define DEVICE_H

// --------------------------------
// Includes
// --------------------------------
#include <OpenNI.h>

// --------------------------------
// Defines
// --------------------------------
#define MAX_STRINGS 20

// --------------------------------
// Types
// --------------------------------
typedef struct
{
	int nValuesCount;
	unsigned int pValues[MAX_STRINGS];
	const char* pValueToName[MAX_STRINGS];
} DeviceParameter;

typedef struct
{
	int nValuesCount;
	const char* pValues[MAX_STRINGS];
} DeviceStringProperty;

typedef enum SensorOpenType
{
	SENSOR_OFF,
	SENSOR_ON,
	SENSOR_TRY
} SensorOpenType;

typedef struct
{
	SensorOpenType openDepth;
	SensorOpenType openColor;
	SensorOpenType openIR;
} DeviceConfig;

// --------------------------------
// Global Variables
// --------------------------------
extern DeviceStringProperty g_PrimaryStream;
extern DeviceParameter g_Registration;

// --------------------------------
// Function Declarations
// --------------------------------
openni::Status openDevice(const char* uri, DeviceConfig config);
openni::Status openDeviceFromList(DeviceConfig config);
void closeDevice();
void readFrame();
void changeRegistration(int nValue);
void changePrimaryStream(int nValue);
void toggleMirror(int);
void seekFrame(int nDiff);
void seekFrameAbs(int frameId);
void toggleDepthState(int nDummy);
void toggleColorState(int nDummy);
void toggleIRState(int nDummy);
void toggleAudioState(int nDummy);
void getDepthFormats(const char** pNames, unsigned int* pValues, int* pCount);
void getImageFormats(const char** pNames, unsigned int* pValues, int* pCount);
void getAudioFormats(const char** pNames, unsigned int* pValues, int* pCount);
void getPrimaryStreams(const char** pNames, unsigned int* pValues, int* pCount);
bool isDepthOn();
bool isColorOn();
bool isIROn();
bool isAudioOn();
bool isPlayerOn();
const char* getFormatName(openni::PixelFormat format);

const openni::SensorInfo* getDepthSensorInfo();
const openni::SensorInfo* getColorSensorInfo();
const openni::SensorInfo* getIRSensorInfo();

void setDepthVideoMode(int mode);
void setColorVideoMode(int mode);
void setIRVideoMode(int mode);

void toggleDepthMirror(int);
void toggleColorMirror(int);
void toggleIRMirror(int);

void toggleImageAutoExposure(int);
void toggleImageAutoWhiteBalance(int);
void changeImageExposure(int);
void changeImageGain(int);

void toggleCloseRange(int);
void toggleEmitterState(int);
void toggleImageRegistration(int);

void setStreamCropping(openni::VideoStream& stream, int originX, int originY, int width, int height);
void resetDepthCropping(int);
void resetColorCropping(int);
void resetIRCropping(int);
void resetAllCropping(int);

openni::Device& getDevice();
openni::VideoStream&  getDepthStream();
openni::VideoStream&  getColorStream();
openni::VideoStream&  getIRStream();

openni::VideoFrameRef& getDepthFrame();
openni::VideoFrameRef& getColorFrame();
openni::VideoFrameRef& getIRFrame();
// const DepthMetaData* getDepthMetaData();
// const ImageMetaData* getImageMetaData();
// const IRMetaData* getIRMetaData();
// const AudioMetaData* getAudioMetaData();

void toggleFrameSync(int);

void toggleZoomCrop(int);

void togglePlaybackRepeat(int /*ignored*/);
openni::Status setPlaybackSpeed(float speed);
float getPlaybackSpeed();
void changePlaybackSpeed(int ratioDiff);

bool convertDepthPointToColor(int depthX, int depthY, openni::DepthPixel DepthZ, int* pColorX, int* pColorY);

#endif // DEVICE_H
