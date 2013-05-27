#ifndef _XN_PS_VERSION_H_
#define _XN_PS_VERSION_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <XnPlatform.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
/** Xiron major version. */ 
#define XN_PS_MAJOR_VERSION 6
/** Xiron minor version. */ 
#define XN_PS_MINOR_VERSION 2
/** Xiron maintenance version. */ 
#define XN_PS_MAINTENANCE_VERSION 2
/** Xiron build version. */ 
#define XN_PS_BUILD_VERSION 10

/** Xiron version (in brief string format): "Major.Minor.Maintenance (Build)" */ 
#define XN_PS_BRIEF_VERSION_STRING \
	XN_STRINGIFY(XN_PS_MAJOR_VERSION) "." \
	XN_STRINGIFY(XN_PS_MINOR_VERSION) "." \
	XN_STRINGIFY(XN_PS_MAINTENANCE_VERSION) \
	" (Build " XN_STRINGIFY(XN_PS_BUILD_VERSION) ")"

/** Xiron version (in numeric format): (Xiron major version * 100000000 + Xiron minor version * 1000000 + Xiron maintenance version * 10000 + Xiron build version). */
#define XN_PS_VERSION (XN_PS_MAJOR_VERSION*100000000 + XN_PS_MINOR_VERSION*1000000 + XN_PS_MAINTENANCE_VERSION*10000 + XN_PS_BUILD_VERSION)

/** Xiron version (in string format): "Major.Minor.Maintenance.Build-Platform (MMM DD YYYY HH:MM:SS)". */ 
#define XN_PS_VERSION_STRING \
		XN_PS_BRIEF_VERSION_STRING  "-" \
		XN_PLATFORM_STRING " (" XN_TIMESTAMP ")"

#endif //_XN_VERSION_H_
