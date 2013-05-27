
#ifndef _XN_SHIFT_TO_DEPTH_H_
#define _XN_SHIFT_TO_DEPTH_H_

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <OniCTypes.h>
#include <XnPlatform.h>

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
typedef struct XnShiftToDepthConfig
{
	/** The zero plane distance in depth units. */
	OniDepthPixel nZeroPlaneDistance;
	/** The zero plane pixel size */
	XnFloat fZeroPlanePixelSize;
	/** The distance between the emitter and the Depth Cmos */
	XnFloat fEmitterDCmosDistance;
	/** The maximum possible shift value from this device. */
	XnUInt32 nDeviceMaxShiftValue;
	/** The maximum possible depth from this device (as opposed to a cut-off). */
	XnUInt32 nDeviceMaxDepthValue;

	XnUInt32 nConstShift;
	XnUInt32 nPixelSizeFactor;
	XnUInt32 nParamCoeff;
	XnUInt32 nShiftScale;

	XnDouble dDepthScale;
	OniDepthPixel nDepthMinCutOff;
	OniDepthPixel nDepthMaxCutOff;

} XnShiftToDepthConfig;

typedef struct XnShiftToDepthTables
{
	XnBool bIsInitialized;
	/** The shift-to-depth table. */
	OniDepthPixel* pShiftToDepthTable;
	/** The number of entries in the shift-to-depth table. */
	XnUInt32 nShiftsCount;
	/** The depth-to-shift table. */
	XnUInt16* pDepthToShiftTable;
	/** The number of entries in the depth-to-shift table. */
	XnUInt32 nDepthsCount;
} XnShiftToDepthTables;

//---------------------------------------------------------------------------
// Functions Declaration
//---------------------------------------------------------------------------
XnStatus XnShiftToDepthInit(XnShiftToDepthTables* pShiftToDepth, 
							const XnShiftToDepthConfig* pConfig);

XnStatus XnShiftToDepthUpdate(XnShiftToDepthTables* pShiftToDepth, 
							  const XnShiftToDepthConfig* pConfig);

XnStatus XnShiftToDepthConvert(const XnShiftToDepthTables* pShiftToDepth, 
							   const XnUInt16* pInput, 
							   XnUInt32 nInputSize, 
							   OniDepthPixel* pOutput);

XnStatus XnShiftToDepthFree(XnShiftToDepthTables* pShiftToDepth);

#endif //_XN_SHIFT_TO_DEPTH_H_
