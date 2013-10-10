/*****************************************************************************
*                                                                            *
*  OpenNI 1.x Alpha                                                          *
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

#include "org_openni_android_OpenNIView.h"
#include <OpenNI.h>

#if (ONI_PLATFORM == ONI_PLATFORM_ANDROID_ARM)

#define  GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>

#define DEBUG 1

#if DEBUG && defined(ANDROID)
#include <android/log.h>
#  define  LOGD(x...)  __android_log_print(ANDROID_LOG_INFO,"OpenNI-JNI",x)
#  define  LOGE(x...)  __android_log_print(ANDROID_LOG_ERROR,"OpenNI-JNI",x)
#else
#  define  LOGD(...)  do {} while (0)
#  define  LOGE(...)  do {} while (0)
#endif

using namespace openni;

static void
throwRuntimeException(JNIEnv *env, const char *message)
{
	jclass exClass;
	const char *className = "java/lang/RuntimeException";

	exClass = env->FindClass(className);
	if (!exClass)
		return;

	env->ThrowNew(exClass, message);
}

class OpenNIView
{
public:
	OpenNIView();
	~OpenNIView();
	
	void onSurfaceChanged(int w, int h);
	void onSurfaceCreated();
	void update(VideoFrameRef& frame);
	void clear();
	void onDraw();
	void setAlpha(unsigned char alpha) { m_alpha = alpha; }
	unsigned char getAlpha() const { return m_alpha; }

private:
	void calcDepthHist(VideoFrameRef& frame);
	int getClosestPowerOfTwo(int n);
	
	int m_textureWidth;
	int m_textureHeight;
	GLuint m_textureId;
	unsigned char* m_texture;
	unsigned char m_alpha;
	int m_viewWidth;
	int m_viewHeight;
	int *m_histogram;
	int m_maxGrayVal;
	int m_xres;
	int m_yres;
	
	enum { HISTSIZE = 0xFFFF, };
};

OpenNIView::OpenNIView() :
	m_textureWidth(0), m_textureHeight(0), m_textureId(0), m_texture(NULL), m_alpha(255), m_viewWidth(0), 
	m_viewHeight(0), m_histogram(NULL), m_maxGrayVal(0)
{
}

OpenNIView::~OpenNIView()
{
	free(m_histogram);
	free(m_texture);
}
	
int OpenNIView::getClosestPowerOfTwo(int n)
{
	int m = 2;
	while (m < n)
	{
		m <<= 1;
	}
	return m;
}

void YUVtoRGB888(XnUInt8 y, XnUInt8 u, XnUInt8 v, XnUInt8& r, XnUInt8& g, XnUInt8& b)
{
	XnInt32 nC = y - 16;
	XnInt16 nD = u - 128;
	XnInt16 nE = v - 128;

	nC = nC * 298 + 128;

	r = (XnUInt8)XN_MIN(XN_MAX((nC            + 409 * nE) >> 8, 0), 255);
	g = (XnUInt8)XN_MIN(XN_MAX((nC - 100 * nD - 208 * nE) >> 8, 0), 255);
	b = (XnUInt8)XN_MIN(XN_MAX((nC + 516 * nD           ) >> 8, 0), 255);
}

void OpenNIView::update(VideoFrameRef& frame)
{
	int xres = frame.getVideoMode().getResolutionX();
	int yres = frame.getVideoMode().getResolutionY();
	
	if (m_textureWidth < xres ||
		m_textureHeight < yres)
	{
		// need to reallocate texture
		free(m_texture);
		m_textureWidth = getClosestPowerOfTwo(xres);
		m_textureHeight = getClosestPowerOfTwo(yres);
		int bufSize = 4 * m_textureWidth * m_textureHeight;
		m_texture = (unsigned char*)malloc(bufSize);
	}
	
	switch (frame.getVideoMode().getPixelFormat())
	{
	case PIXEL_FORMAT_DEPTH_1_MM:
	case PIXEL_FORMAT_DEPTH_100_UM:
		{
			calcDepthHist(frame);
			
			for (int y = 0; y < yres; ++y)
			{
				unsigned char* texture = m_texture + y * m_textureWidth * 4;
				const OniDepthPixel* pDepth = (const OniDepthPixel*)frame.getData() + y * xres;
				for (int x = 0; x < xres; ++x, ++pDepth)
				{
					int val = m_histogram[*pDepth];
					texture[0] = val;
					texture[1] = val;
					texture[2] = 0;
					texture[3] = m_alpha;
					texture += 4;
				}
			}
		}
		break;
		
	case PIXEL_FORMAT_GRAY16:
		{
			const Grayscale16Pixel* pData = (const Grayscale16Pixel*)frame.getData();
			
			// find max val
			for (int i = 0; i < frame.getDataSize()/sizeof(Grayscale16Pixel); ++i, ++pData)
			{
				if (*pData > m_maxGrayVal)
				{
					m_maxGrayVal = *pData;
				}
			}
			
			// and update texture
			for (int y = 0; y < yres; ++y)
			{
				unsigned char* texture = m_texture + y * m_textureWidth * 4;
				const Grayscale16Pixel* pData = (const Grayscale16Pixel*)frame.getData() + y * xres;
				
				for (int x = 0; x < xres; ++x, ++pData)
				{
					texture[0] = texture[1] = texture[2] = 255.0 * (*pData) / m_maxGrayVal;
					texture[3] = m_alpha;
					texture += 4;
				}
			}
		}
		break;
		
	case PIXEL_FORMAT_GRAY8:
		{
			for (int y = 0; y < yres; ++y)
			{
				unsigned char* texture = m_texture + y * m_textureWidth * 4;
				const unsigned char* pData = (const unsigned char*)frame.getData() + y * xres;
				
				for (int x = 0; x < xres; ++x, ++pData)
				{
					texture[0] = texture[1] = texture[2] = *pData;
					texture[3] = m_alpha;
					texture += 4;
				}
			}
		}
		break;
		
	case PIXEL_FORMAT_RGB888:
		{
			for (int y = 0; y < yres; ++y)
			{
				unsigned char* texture = m_texture + y * m_textureWidth * 4;
				const RGB888Pixel* pData = (const RGB888Pixel*)frame.getData() + y * xres;
				
				for (int x = 0; x < xres; ++x, ++pData)
				{
					texture[0] = pData->r;
					texture[1] = pData->g;
					texture[2] = pData->b;
					texture[3] = m_alpha;
					texture += 4;
				}
			}
		}
		break;

	case PIXEL_FORMAT_YUV422:
		{
			for (int y = 0; y < yres; ++y)
			{
				unsigned char* texture = m_texture + y * m_textureWidth * 4;
				const YUV422DoublePixel* pData = (const YUV422DoublePixel*)frame.getData() + y * xres/2;

				for (int x = 0; x < xres/2; ++x, ++pData)
				{
					YUVtoRGB888(pData->y1, pData->u, pData->v, texture[0], texture[1], texture[2]);
					texture[3] = m_alpha;
					texture += 4;
					YUVtoRGB888(pData->y2, pData->u, pData->v, texture[0], texture[1], texture[2]);
					texture[3] = m_alpha;
					texture += 4;
				}
			}
		}
		break;

	case PIXEL_FORMAT_YUYV:
		{
			for (int y = 0; y < yres; ++y)
			{
				unsigned char* texture = m_texture + y * m_textureWidth * 4;
				const YUYVDoublePixel* pData = (const YUYVDoublePixel*)frame.getData() + y * xres/2;

				for (int x = 0; x < xres/2; ++x, ++pData)
				{
					YUVtoRGB888(pData->y1, pData->u, pData->v, texture[0], texture[1], texture[2]);
					texture[3] = m_alpha;
					texture += 4;
					YUVtoRGB888(pData->y2, pData->u, pData->v, texture[0], texture[1], texture[2]);
					texture[3] = m_alpha;
					texture += 4;
				}
			}
		}
		break;

	default:
		LOGE("Non-supported pixel format: %d", frame.getVideoMode().getPixelFormat());
	}
	
	m_xres = xres;
	m_yres = yres;
}

void OpenNIView::clear()
{
	memset(m_texture, 0, m_textureWidth * m_textureHeight * 4);
}

void OpenNIView::calcDepthHist(VideoFrameRef& frame)
{
	unsigned int value = 0;
	unsigned int index = 0;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int numberOfPoints = 0;
	const OniDepthPixel* pDepth = (const OniDepthPixel*)frame.getData();

	if (m_histogram == NULL) 
	{
		m_histogram = (int*)malloc(HISTSIZE * sizeof(int));
	}

	// Calculate the accumulative histogram
	memset(m_histogram, 0, HISTSIZE*sizeof(int));
	
	for (int i = 0; i < frame.getDataSize() / sizeof(DepthPixel); ++i, ++pDepth)
	{
		value = *pDepth;

		if (value != 0)
		{
			m_histogram[value]++;
			numberOfPoints++;
		}
	}

	for (index = 1; index < HISTSIZE; index++)
	{
		m_histogram[index] += m_histogram[index - 1];
	}
	
	if (numberOfPoints != 0)
	{
		for (index = 1; index < HISTSIZE; index++)
		{
			m_histogram[index] = (unsigned int)(256 * (1.0f - ((float)m_histogram[index] / numberOfPoints)));
		}
	}
}

void OpenNIView::onDraw()
{
	int startX = 0;
	int startY = 0;
	int viewAreaWidth = m_viewWidth;
	int viewAreaHeight = m_viewHeight;

	// skip if no frame yet
	if (m_xres == 0 || m_yres == 0)
		return;

	// if view ratio is larger than frame ratio, make width smaller. Otherwise, make height smaller
	if (m_xres * viewAreaHeight > m_yres * viewAreaWidth)
	{
		viewAreaHeight = m_yres * viewAreaWidth / m_xres;
		startY = (m_viewHeight - viewAreaHeight) / 2;
	}
	else
	{
		viewAreaWidth = m_xres * viewAreaHeight / m_yres;
		startX = (m_viewWidth - viewAreaWidth) / 2;
	}

	glBindTexture(GL_TEXTURE_2D, m_textureId);
	int rect[4] = {0, m_yres, m_xres, -m_yres};
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);

	glClear(GL_COLOR_BUFFER_BIT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, m_texture);

	glDrawTexiOES(startX, startY, 0, viewAreaWidth, viewAreaHeight);
}

void OpenNIView::onSurfaceCreated()
{
	/* Disable these capabilities. */
	static GLuint gCapbilitiesToDisable[] = {
		GL_FOG,
		GL_LIGHTING,
		GL_CULL_FACE,
		GL_ALPHA_TEST,
		GL_BLEND,
		GL_COLOR_LOGIC_OP,
		GL_DITHER,
		GL_STENCIL_TEST,
		GL_DEPTH_TEST,
		GL_COLOR_MATERIAL,
		0
	};

	for (GLuint *capability = gCapbilitiesToDisable; *capability; capability++)
	{
		glDisable(*capability);
	}

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &m_textureId);
	glBindTexture(GL_TEXTURE_2D, m_textureId);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glShadeModel(GL_FLAT);
}

void OpenNIView::onSurfaceChanged(int w, int h)
{
	m_viewWidth = w;
	m_viewHeight = h;
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL Java_org_openni_android_OpenNIView_nativeCreate(JNIEnv *, jclass)
{
	return (jlong)new OpenNIView();
}

JNIEXPORT void JNICALL Java_org_openni_android_OpenNIView_nativeDestroy(JNIEnv *, jclass, jlong nativePtr)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	delete pView;
}

JNIEXPORT void JNICALL Java_org_openni_android_OpenNIView_nativeSetAlphaValue(JNIEnv *, jclass, jlong nativePtr, jint alpha)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	pView->setAlpha(alpha);
}

JNIEXPORT jint JNICALL Java_org_openni_android_OpenNIView_nativeGetAlphaValue(JNIEnv *, jclass, jlong nativePtr)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	return pView->getAlpha();
}

JNIEXPORT void JNICALL Java_org_openni_android_OpenNIView_nativeOnSurfaceCreated(JNIEnv *, jclass, jlong nativePtr)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	pView->onSurfaceCreated();
}

JNIEXPORT void JNICALL Java_org_openni_android_OpenNIView_nativeOnSurfaceChanged(JNIEnv *, jclass, jlong nativePtr, jint w, jint h)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	pView->onSurfaceChanged(w, h);
}

JNIEXPORT void JNICALL Java_org_openni_android_OpenNIView_nativeUpdate(JNIEnv *, jclass, jlong nativePtr, jlong frameHandle)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	VideoFrameRef frame;
	frame._setFrame((OniFrame*)frameHandle);
	pView->update(frame);
}

JNIEXPORT void JNICALL Java_org_openni_android_OpenNIView_nativeClear(JNIEnv *, jclass, jlong nativePtr)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	pView->clear();
}

JNIEXPORT void JNICALL Java_org_openni_android_OpenNIView_nativeOnDraw(JNIEnv *, jclass, jlong nativePtr)
{
	OpenNIView* pView = (OpenNIView*)nativePtr;
	pView->onDraw();
}

#ifdef __cplusplus
}
#endif

#endif // if ANDROID