#include "DepthUtils.h"
#include "DepthUtilsImpl.h"

struct _DepthUtils
{
	DepthUtilsImpl* pDepthUtils;
};

XN_C_API XnStatus DepthUtilsInitialize(DepthUtilsSensorCalibrationInfo* pDepthParameters, DepthUtilsHandle* handle)
{
	*handle = new _DepthUtils;
	(*handle)->pDepthUtils = new DepthUtilsImpl;
	XnStatus rc = (*handle)->pDepthUtils->Initialize(pDepthParameters);

	if (rc != XN_STATUS_OK)
	{
		DepthUtilsShutdown(handle);
	}

	return rc;
}
XN_C_API void DepthUtilsShutdown(DepthUtilsHandle* handle)
{
	if ((*handle) != NULL && (*handle)->pDepthUtils != NULL)
	{
		delete (*handle)->pDepthUtils;
		delete *handle;

		*handle = NULL;
	}
}

XN_C_API XnStatus DepthUtilsTranslatePixel(DepthUtilsHandle handle, unsigned int x, unsigned int y, unsigned short z, unsigned int* pX, unsigned int* pY)
{
	if (handle == NULL || handle->pDepthUtils == NULL)
	{
		return XN_STATUS_BAD_PARAM;
	}
	return handle->pDepthUtils->TranslateSinglePixel(x, y, z, *pX, *pY);
}
XN_C_API XnStatus DepthUtilsTranslateDepthMap(DepthUtilsHandle handle, unsigned short* depth)
{
	if (handle == NULL || handle->pDepthUtils == NULL)
	{
		return XN_STATUS_BAD_PARAM;
	}
	return handle->pDepthUtils->Apply(depth);
}

XN_C_API XnStatus DepthUtilsSetDepthConfiguration(DepthUtilsHandle handle, int xres, int yres, OniPixelFormat format, int isMirrored)
{
	if (handle == NULL || handle->pDepthUtils == NULL)
	{
		return XN_STATUS_BAD_PARAM;
	}
	return handle->pDepthUtils->SetDepthConfiguration(xres, yres, format, isMirrored == 1);
}
XN_C_API XnStatus DepthUtilsSetColorResolution(DepthUtilsHandle handle, int xres, int yres)
{
	if (handle == NULL || handle->pDepthUtils == NULL)
	{
		return XN_STATUS_BAD_PARAM;
	}
	return handle->pDepthUtils->SetColorResolution(xres, yres);
}
