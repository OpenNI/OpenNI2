#ifndef __XN_PRIME_CLIENT_PROPS_H__
#define __XN_PRIME_CLIENT_PROPS_H__

#include <PrimeSense.h>

enum
{
	/**** Device properties ****/

	/* XnDetailedVersion, get only */
	LINK_PROP_FW_VERSION = 0x12000001, // "FWVersion"
	/* Int, get only */
	LINK_PROP_VERSIONS_INFO_COUNT = 0x12000002, // "VersionsInfoCount"
	/* General - array - XnComponentVersion * count elements, get only */
	LINK_PROP_VERSIONS_INFO = 0x12000003, // "VersionsInfo"
	/* General. XnPropUploadFile. Set only. */
	LINK_PROP_UPLOAD_FILE	= 0x12000006, // "UploadFile"
	/* General. XnPropDownloadFile. Set only. */
	LINK_PROP_DOWNLOAD_FILE = 0x12000007, // "DownloadFile"
	/* Int - 0 means off, 1 means on. */
	LINK_PROP_EMITTER_ACTIVE = 0x12000008, // "EmitterActive"
	/* Int - 0 means off, 1 means on. */
	LINK_PROP_FW_LOG = 0x12000009, // "FWLog"
	/* String. Set only */
	LINK_PROP_PRESET_FILE = 0x1200000a, // "PresetFile"
	/* Int. Set only */
	LINK_PROP_FORMAT_ZONE = 0x1200000f, // "FormatZone"
	/* String, get only */
	LINK_PROP_SERIAL_NUMBER = 0x12000010, // "ID"

	/**** Stream properties ****/
	/* Int. 1 - Shifts 9.3, 2 - Grayscale16, 3 - YUV422, 4 - Bayer8 */
	LINK_PROP_PIXEL_FORMAT = 0x12001001, // "PixelFormat"
	/* Int. 0 - None, 1 - 8z, 2 - 16z, 3 - 24z, 4 - 6-bit, 5 - 10-bit, 6 - 11-bit, 7 - 12-bit */
	LINK_PROP_COMPRESSION = 0x12001002, // "Compression"

	/**** Depth Stream properties ****/
	/* Real, get only */
	LINK_PROP_DEPTH_SCALE = 0x1200000b, // "DepthScale"
	/* Int, get only */
	LINK_PROP_MAX_SHIFT = 0x12002001, // "MaxShift"
	/* Int, get only */
	LINK_PROP_ZERO_PLANE_DISTANCE = 0x12002002, // "ZPD"
	/* Int, get only */
	LINK_PROP_CONST_SHIFT = 0x12002003, // "ConstShift"
	/* Int, get only */
	LINK_PROP_PARAM_COEFF = 0x12002004, // "ParamCoeff"
	/* Int, get only */
	LINK_PROP_SHIFT_SCALE = 0x12002005, // "ShiftScale"
	/* Real, get only */
	LINK_PROP_ZERO_PLANE_PIXEL_SIZE = 0x12002006, // "ZPPS"
	/* Real, get only */
	LINK_PROP_ZERO_PLANE_OUTPUT_PIXEL_SIZE = 0x12002007, // "ZPOPS"
	/* Real, get only */
	LINK_PROP_EMITTER_DEPTH_CMOS_DISTANCE = 0x12002008, // "LDDIS"
	/*  General - array - MaxShift * XnDepthPixel elements, get only */
	LINK_PROP_SHIFT_TO_DEPTH_TABLE = 0x12002009, // "S2D"
	/* General - array - MaxDepth * uint16_t elements, get only */
	LINK_PROP_DEPTH_TO_SHIFT_TABLE = 0x1200200a, // "D2S"
};


#pragma pack (push, 1)

#define XN_MAX_VERSION_MODIFIER_LENGTH 16
typedef struct XnDetailedVersion
{
	uint8_t m_nMajor;
	uint8_t m_nMinor;
	uint16_t m_nMaintenance;
	uint32_t m_nBuild;
	char m_strModifier[XN_MAX_VERSION_MODIFIER_LENGTH];
} XnDetailedVersion;

typedef struct XnPropReadAHB
{
	uint32_t m_nAddress;
	uint32_t m_nBitOffset; //Offset in bits of value to read within address
	uint32_t m_nBitWidth; //Width in bits of value to read
	uint32_t m_nValue; //Output parameter - the value that was actually read
} XnPropReadAHB;

typedef struct XnPropWriteAHB
{
	uint32_t m_nAddress;
	uint32_t m_nValue;
	uint32_t m_nBitOffset; //Offset in bits of value to write within address
	uint32_t m_nBitWidth; //Width in bits of value to write
} XnPropWriteAHB;

typedef struct XnPropReadI2C
{
	uint32_t m_nDeviceID;
	uint32_t m_nAddressSize;
	uint32_t m_nValueSize;
	uint32_t m_nAddress;
	uint32_t m_nValue; //Output parameter - the value that was actually read
} XnPropReadI2C;

typedef struct XnPropWriteI2C
{
	uint32_t m_nDeviceID;
	uint32_t m_nAddressSize;
	uint32_t m_nValueSize;
	uint32_t m_nAddress; 
	uint32_t m_nValue; 
	uint32_t m_nMask;
} XnPropWriteI2C;

typedef struct XnPropUploadFile  
{
	char* m_nFileName;
	uint32_t m_nbOverrideFactorySettings;
} XnPropUploadFile;  

typedef struct XnPropDownloadFile  
{
	uint16_t m_nZone;
	char*  m_nStrFirmwareFileName;
	char*  m_nStrTargetFile;
} XnPropDownloadFile;  

typedef struct XnPropFormatZone 
{
	uint8_t m_nZone;
} XnPropFormatZone;  

#pragma pack (pop)

#endif //__XN_PRIME_CLIENT_PROPS_H__
