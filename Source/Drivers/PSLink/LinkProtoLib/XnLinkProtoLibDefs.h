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

typedef struct XnLeanVersion
{
	XnUInt8 m_nMajor;
	XnUInt8 m_nMinor;
} XnLeanVersion;

typedef struct XnComponentVersion
{
	XnChar m_strName[XN_MAX_COMPONENT_NAME_LENGTH];
	XnChar m_strVersion[XN_MAX_VERSION_LENGTH];
} XnComponentVersion;

#endif // __XNLINKPROTOLIBDEFS_H__
