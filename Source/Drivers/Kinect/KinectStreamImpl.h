#ifndef _KINECT_STREAM_IMPL_H_
#define _KINECT_STREAM_IMPL_H_

#include "BaseKinectStream.h"
#include <Shlobj.h>
#include "NuiApi.h"
#include "XnList.h"

struct INuiSensor;

namespace kinect_device {
class KinectStreamImpl
{
public:
	KinectStreamImpl(INuiSensor * pNuiSensor, OniSensorType sensorType);
	
	virtual ~KinectStreamImpl();
	
	void addStream(BaseKinectStream* stream);
	
	void removeStream(BaseKinectStream* stream);	

	unsigned int getStreamCount();

	void setVideoMode(OniVideoMode* videoMode);
	
	OniStatus virtual start();

	void virtual stop();

	bool	isRunning() { return m_running; }

	OniSensorType getSensorType () { return m_sensorType; }

	void setSensorType(OniSensorType sensorType);
	
	void mainLoop();

	OniStatus setAutoWhiteBalance(BOOL val);

	OniStatus getAutoWhitBalance(BOOL *val);

	OniStatus setAutoExposure(BOOL val);

	OniStatus getAutoExposure(BOOL *val);
	
	OniImageRegistrationMode getImageRegistrationMode() const { return m_imageRegistrationMode; }

	void setImageRegistrationMode(OniImageRegistrationMode mode) { m_imageRegistrationMode = mode; }

	OniStatus convertDepthToColorCoordinates(oni::driver::StreamBase* colorStream, 
								int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY);

	static NUI_IMAGE_RESOLUTION getNuiImagResolution(int resolutionX, int resolutionY);

	INuiSensor* getNuiSensor() { return m_pNuiSensor; } // Need review: not sure if it is a good idea to expose this.
	
private:
		
	void setDefaultVideoMode();
	
	NUI_IMAGE_TYPE getNuiImageType();

	static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam);

	xnl::List<BaseKinectStream*> m_streamList;

	OniImageRegistrationMode m_imageRegistrationMode;

	// Thread
	INuiSensor* m_pNuiSensor;
	OniSensorType m_sensorType;
	bool		 m_running;
	HANDLE       m_hStreamHandle;
	HANDLE       m_hNextFrameEvent;
	OniVideoMode m_videoMode;
	
	XN_THREAD_HANDLE m_threadHandle;	
};

//
// Helper templates for frame copy
//

// This is the most standard pixel copy implementation.
// You can create your own pixel copy implementation if any conversion is necessary.
template<typename SourceType, typename DestType>
struct PixelCopier
{
	void operator()(const SourceType* const in, DestType* const out)
	{
		*out = *in;
	}
};

template<typename SourceType, typename DestType, typename PixelCopier, typename SourceMover>
struct LineCopier
{
	void operator()(const SourceType* in, DestType* out, const int count)
	{
		PixelCopier copyPixel;
		SourceMover moveSource;
		const SourceType* end = moveSource(in, count);
		while (in != end) {
			copyPixel(in, out);
			in = moveSource(in, 1);
			++out;
		}
	}
};

template<typename SourceType, typename DestType, typename PixelCopier, typename HorizontalSourceMover, typename VerticalSourceMover>
struct RectCopier
{
	void operator()(const SourceType* in, DestType* out, const int copyWidth, const int totalWidth, const int copyHeight)
	{
		LineCopier<SourceType, DestType, PixelCopier, HorizontalSourceMover> copyLine;
		VerticalSourceMover moveSource;
		const SourceType* end = moveSource(in, totalWidth * copyHeight);
		while (in != end) {
			copyLine(in, out, copyWidth);
			in = moveSource(in, totalWidth);
			out += copyWidth;
		}
	}
};

template<typename T>
struct ForwardMover {
	T* operator()(T* const p, const int offset) { return p+offset; }
	const T* operator()(const T* const p, const int offset) { return p+offset; }
};

template <typename T>
struct BackwardMover {
	T* operator()(T* const p, const int offset) { return p-offset; }
	const T* operator()(const T* const p, const int offset) { return p-offset; }
};

template<typename SourceType, typename DestType, typename ForwardLineCopier, typename ForwardRectCopier, typename MirrorRectCopier>
struct FrameCopier {
	void operator()(const SourceType* in, DestType* out, OniFrame* pFrame, const OniVideoMode& videoMode, const OniCropping& cropping, OniBool mirroring)
	{
		const int resX = videoMode.resolutionX;
		const int resY = videoMode.resolutionY;

		if (!cropping.enabled)
		{
			const int pixelCount = resX * resY;
			pFrame->dataSize = pixelCount * sizeof(DestType);
			pFrame->stride = resX * sizeof(DestType);

			if (!mirroring)
			{
				ForwardLineCopier copyLine;
				copyLine(in, out, pixelCount);
			} else {
				MirrorRectCopier copyRect;
				copyRect(in+resX-1, out, resX, resX, resY);
			}
		}
		else
		{
			const int pixelCount = cropping.height * cropping.width;
			pFrame->dataSize = pixelCount * sizeof(DestType);
			pFrame->stride = cropping.width * sizeof(DestType);

			if (!mirroring)
			{
				ForwardRectCopier copyRect;
				const SourceType* origin = in + cropping.originY * resX + cropping.originX;
				copyRect(origin, out, cropping.width, resX, cropping.height);
			}
			else
			{
				MirrorRectCopier copyRect;
				const SourceType* origin = in + (cropping.originY+1) * resX - cropping.originX - 1;
				copyRect(origin, out, cropping.width, resX, cropping.height);
			}
		}
	}
};

} // namespace kinect_device

#endif //_KINECT_STREAM_IMPL_H_
