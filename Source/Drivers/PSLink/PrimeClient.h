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
#ifndef PRIMECLIENT_H
#define PRIMECLIENT_H

#include "XnLinkProtoLibDefs.h"
#include "XnLinkControlEndpoint.h"
#include "XnLinkInputDataEndpoint.h"
#include "XnLinkOutputDataEndpoint.h"
#include "XnLinkInputStreamsMgr.h"
#include "XnLinkOutputStreamsMgr.h"
#include "PrimeClientDefs.h"
#include "XnShiftToDepth.h"
#include <PSLink.h>
#include <XnPlatform.h>
#include <XnStatus.h>
#include <XnArray.h>

namespace xn
{

class ISyncIOConnection;
class IOutputConnection;
class IConnectionFactory;

class PrimeClient : 
	virtual public ILinkDataEndpointNotifications
{
public:
	PrimeClient();
	virtual ~PrimeClient();

	/* Initialization and shutdown */
	virtual XnStatus Init(const XnChar* strConnString, XnTransportType transportType);
	virtual void Shutdown();
	virtual XnBool IsInitialized() const;
	virtual XnStatus Connect();
    virtual XnBool IsConnected() const;
	virtual void Disconnect();

	/* Global Properties */
    virtual const XnDetailedVersion& GetFWVersion() const;
    virtual const XnUInt32 GetHWVersion() const;
    virtual const XnLeanVersion& GetDeviceProtocolVersion() const;
    virtual const XnChar* GetSerialNumber() const;
	virtual const XnStatus GetComponentsVersions(xnl::Array<XnComponentVersion>& componentVersions);
	const XnChar* GetConnectionString() const { return m_strConnectionString; }

    /* Global Device Commands */
	virtual	XnStatus GetBootStatus(XnBootStatus& bootStatus);
	virtual	XnStatus GetSupportedI2CDevices(xnl::Array<XnLinkI2CDevice>& supportedDevices);
    virtual XnStatus WriteI2C(XnUInt8 nDeviceID, XnUInt8 nAddressSize, XnUInt32 nAddress, XnUInt8 nValueSize, XnUInt32 nValue, XnUInt32 nMask);
    virtual XnStatus ReadI2C(XnUInt8 nDeviceID, XnUInt8 nAddressSize, XnUInt32 nAddress, XnUInt8 nValueSize, XnUInt32& nValue);
    virtual XnStatus WriteAHB(XnUInt32 nAddress, XnUInt32 nValue, XnUInt8 nBitOffset, XnUInt8 nBitWidth);
    virtual XnStatus ReadAHB(XnUInt32 nAddress, XnUInt8 nBitOffset, XnUInt8 nBitWidth, XnUInt32& nValue);
	virtual XnStatus SoftReset();
	virtual XnStatus HardReset();
    virtual XnStatus ReadDebugData(XnCommandDebugData& commandDebugData);
    virtual XnStatus SetProjectorActive(XnBool bActive);
    virtual XnStatus SetAccActive(XnBool bActive);
    virtual XnStatus GetAccActive(XnBool& bActive); 
    virtual XnStatus SetVDDActive(XnBool bActive);  
    virtual XnStatus GetVDDActive(XnBool& bActive); 
    virtual XnStatus SetPeriodicBistActive(XnBool bActive);  
    virtual XnStatus GetPeriodicBistActive(XnBool& bActive);   
    virtual XnStatus StartFWLog();
    virtual XnStatus StopFWLog();
	virtual XnStatus OpenFWLogFile(XnUInt8 logID);
	virtual	XnStatus CloseFWLogFile(XnUInt8 logID);
	virtual XnStatus GetSupportedLogFiles(xnl::Array<XnLinkLogFile>& supportedFiles);

	virtual XnStatus RunPresetFile(const XnChar* strFileName);
	virtual XnStatus GetSupportedBistTests(xnl::Array<XnBistInfo>& supportedTests);
    virtual XnStatus GetSupportedTempList(xnl::Array<XnTempInfo>& supportedTempList);
    virtual XnStatus GetTemperature(XnCommandTemperatureResponse& temp);
	virtual XnStatus ExecuteBist(XnUInt32 nID, uint32_t& errorCode, uint32_t& extraDataSize, uint8_t* extraData);
	virtual XnStatus FormatZone(XnUInt8 nZone);
    //TODO: Implement Get emitter active

    /* Stream Management */
    virtual XnStatus EnumerateStreams(xnl::Array<XnFwStreamInfo>& aStreamInfos);
    virtual XnStatus EnumerateStreams(XnStreamType streamType, xnl::Array<XnFwStreamInfo>& aStreamInfos);
    virtual XnStatus CreateInputStream(XnStreamType nodeType, const XnChar* strCreationInfo, XnUInt16& nStreamID);

    virtual XnStatus DestroyInputStream(XnUInt16 nStreamID);
    virtual LinkInputStream* GetInputStream(XnUInt16 nStreamID);
    virtual const LinkInputStream* GetInputStream(XnUInt16 nStreamID) const;

    virtual XnStatus InitOutputStream(XnUInt16 nStreamID, 
        XnUInt32 nMaxMsgSize, 
        XnUInt16 nMaxPacketSize,
        XnLinkCompressionType compression, 
        XnStreamFragLevel streamFragLevel);

    virtual void ShutdownOutputStream(XnUInt16 nStreamID);

	virtual	XnStatus BeginUploadFileOnControlEP();
	virtual	XnStatus EndUploadFileOnControlEP();
	virtual XnStatus UploadFileOnControlEP(const XnChar* strFileName, XnBool bOverrideFactorySettings);
	virtual XnStatus GetFileList(xnl::Array<XnFwFileEntry>& files);
	virtual XnStatus DownloadFile(XnUInt16 zone, const XnChar* strFirmwareFileName, const XnChar* strTargetFile);
	
	virtual XnStatus EnableProjectorPulse(XnFloat delay, XnFloat width, XnFloat cycle);
	virtual XnStatus DisableProjectorPulse();
	virtual XnStatus GetProjectorPulse(XnBool& enabled, XnFloat& delay, XnFloat& width, XnFloat& framesToskip);
	virtual XnStatus SetProjectorPower(XnUInt16 power);
	virtual XnStatus GetProjectorPower(XnUInt16& power);

	virtual void HandleLinkDataEndpointDisconnection(XnUInt16 nEndpointID);

protected:
    virtual XnStatus ConnectOutputDataEndpoint();
	virtual IConnectionFactory* CreateConnectionFactory(XnTransportType transportType) = 0;
//	XnStatus GetStreamBufferSize(XnUInt16 nStreamID, XnProductionNodeType nodeType, XnStreamFragLevel streamFragLevel, XnUInt32& nBufferSize);
    void LogVersions();
    XnStatus CreateInputStreamImpl(XnLinkStreamType streamType, const XnChar* strCreationInfo, XnUInt16& nStreamID, XnUInt16& nEndpointID);
	XnBool IsPropertySupported(XnUInt16 propID);
	LinkControlEndpoint m_linkControlEndpoint;
	LinkOutputDataEndpoint m_outputDataEndpoint;
	IConnectionFactory* m_pConnectionFactory;
	LinkInputStreamsMgr m_linkInputStreamsMgr;
	LinkOutputStreamsMgr m_linkOutputStreamsMgr;

private:
	static const XnUInt32 MAX_COMMAND_SIZE;
	static const XnUInt32 CONT_STREAM_PREDEFINED_BUFFER_SIZE;

	XnBool m_bInitialized;
    volatile XnBool m_bConnected;
	xnl::Array<LinkInputDataEndpoint> m_inputDataEndpoints;
	XnBool m_bAnyDataEndpointConnected;
    XnUInt16 m_nFWLogStreamID;
	XnChar m_strConnectionString[XN_FILE_MAX_PATH];

	xnl::Array<xnl::BitSet> m_supportedProps;
	XnDetailedVersion m_fwVersion;
	XnLeanVersion m_protocolVersion;
	XnUInt32 m_nHWVersion;
	XnChar m_strSerialNumber[XN_SERIAL_NUMBER_SIZE];
};

}

#endif // PRIMECLIENT_H
