#ifndef __LINKPROTOLIBVERSION_H__
#define __LINKPROTOLIBVERSION_H__

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define LINK_PROTO_LIB_MAJOR_VERSION 0
#define LINK_PROTO_LIB_MINOR_VERSION 25

#define LINK_PROTO_LIB_BRIEF_VERSION_STRING \
	XN_STRINGIFY(LINK_PROTO_LIB_MAJOR_VERSION) "." \
	XN_STRINGIFY(LINK_PROTO_LIB_MINOR_VERSION)

#define LINK_PROTO_LIB_VERSION (LINK_PROTO_LIB_MAJOR_VERSION * 100 + LINK_PROTO_LIB_MINOR_VERSION)

#define LINK_PROTO_LIB_VERSION_STRING \
	LINK_PROTO_LIB_BRIEF_VERSION_STRING  "-" \
	XN_PLATFORM_STRING " (" XN_TIMESTAMP ")"

#endif // __LINKPROTOLIBVERSION_H__
