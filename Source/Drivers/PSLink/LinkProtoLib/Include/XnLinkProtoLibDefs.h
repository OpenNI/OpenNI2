#ifndef __XNLINKPROTOLIBDEFS_H__
#define __XNLINKPROTOLIBDEFS_H__

#include <XnPlatform.h>
#include <XnLinkDefs.h>

#define XN_VENDOR_ID          0x1D27
#define XN_VENDOR_PRIMESENSE  "PrimeSense"

//Max sizes
#define XN_EE_MAX_DEVICE_NAME                  200
#define XN_EE_MAX_STREAM_CREATION_INFO_LENGTH  80
#define XN_EE_MAX_JOINTS                       25
#define XN_EE_MAX_BIST_TEST_NAME_LENGTH        32
#define XN_EE_MAX_FILE_NAME_LENGTH             32
#define XN_MAX_COMPONENT_NAME_LENGTH           32
#define XN_MAX_VERSION_LENGTH                  32

//Product IDs
#define XN_PRODUCT_ID_PS1250 0x1250
#define XN_PRODUCT_ID_PS1260 0x1260
#define XN_PRODUCT_ID_PS1270 0x1270
#define XN_PRODUCT_ID_PS1290 0x1290
#define XN_PRODUCT_ID_LENA   0x1280

//USB endpoint numbers
#define XN_EE_DEVICE_IN_DATA_BASE_ENDPOINT 0x81

//Control port numbers
#define XN_CONTROL_PORT_PS1200  20000
#define XN_CONTROL_PORT_LENA    30000

#define XN_SERIAL_NUMBER_SIZE   32

typedef XnChar XnConnectionString[XN_FILE_MAX_PATH];

typedef struct XnStreamInfo
{
	XnUInt32 m_nStreamType;
	XnChar m_strCreationInfo[XN_EE_MAX_STREAM_CREATION_INFO_LENGTH];
} XnStreamInfo;

typedef enum XnTransportType
{
	XN_TRANSPORT_TYPE_NONE = 0,
	XN_TRANSPORT_TYPE_USB = 1,
	XN_TRANSPORT_TYPE_SOCKETS = 2
} XnTransportType;

#define XN_FORMAT_PASS_THROUGH_UNPACK  (OniPixelFormat)0
#define XN_FORMAT_PASS_THROUGH_RAW     (OniPixelFormat)1

typedef XnUInt32 XnStreamFragLevel;
typedef XnUInt32 XnStreamType;

typedef struct XnAvailableGesture
{
	const XnChar* m_strGesture;
	XnBool m_bProgressSupported;
	XnBool m_bCurrentlyActive;
} XnAvailableGesture;

typedef struct XnBistTest
{
	XnUInt32 m_nID;
	XnChar m_strName[XN_EE_MAX_BIST_TEST_NAME_LENGTH];
} XnBistTest;

typedef struct XnBistTestResponse
{
	XnUInt32 m_nErrorCode;
	XnUInt32 m_nExtraDataSize;
	XnUInt8 m_extraData[1];
} XnBistTestResponse;

typedef struct XnLeanVersion
{
	XnUInt8 m_nMajor;
	XnUInt8 m_nMinor;
} XnLeanVersion;

typedef struct XnFileVersion
{
	XnUInt8 m_nMajor;
	XnUInt8 m_nMinor;
	XnUInt8 m_nMaintenance;
	XnUInt8 m_nBuild;
} XnFileVersion;

typedef struct XnComponentVersion
{
	XnChar m_strName[XN_MAX_COMPONENT_NAME_LENGTH];
	XnChar m_strVersion[XN_MAX_VERSION_LENGTH];
} XnComponentVersion;

typedef enum XnFileFlags
{
	XN_FILE_FLAG_BAD_CRC					= 0x0001,
} XnFileFlags;

typedef struct XnFileEntry
{
	XnChar m_strName[XN_EE_MAX_FILE_NAME_LENGTH];
	XnFileVersion m_nVersion;
	XnUInt32 m_nAddress;
	XnUInt32 m_nSize;
	XnUInt16 m_nCRC;
	XnUInt16 m_nZone;
	XnUInt8 m_nFlags; // bitmap of values from XnLinkFileFlags
} XnFileEntry;

typedef struct XnStreamVideoMode
{
	XnUInt32 m_nXRes;
	XnUInt32 m_nYRes;
	XnUInt32 m_nFPS;
	XnLinkPixelFormat m_nPixelFormat;
	XnLinkCompressionType m_nCompression;
} XnStreamVideoMode;

typedef struct XnBootStatus
{
	XnLinkBootZone m_nZone;
	XnLinkBootErrorCode m_nErrorCode;
} XnBootStatus;

#endif // __XNLINKPROTOLIBDEFS_H__
