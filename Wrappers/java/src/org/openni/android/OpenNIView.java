package org.openni.android;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import org.openni.VideoFrameRef;

/**
 * A View that displays OpenNI streams.
 */
public class OpenNIView extends GLSurfaceView {

	private long mNativePtr = 0;
	private Renderer mRenderer;
	private int mPrevFrameWidth = 0;
	private int mPrevFrameHeight = 0;
	
	public OpenNIView(Context context) {
		super(context);
	}

	public OpenNIView(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}
	
	private void init() {
		if(!isInEditMode()) {
			mNativePtr = nativeCreate();
		}

		mRenderer = new Renderer();
		setRenderer(mRenderer);
		setRenderMode(RENDERMODE_WHEN_DIRTY);
	}

	@Override
	protected void finalize() throws Throwable {
		if (mNativePtr != 0) {
			nativeDestroy(mNativePtr);
			mNativePtr = 0;
		}
		super.finalize();
	}
	
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int frameWidth = mPrevFrameWidth;
		int frameHeight = mPrevFrameHeight;
		
		int width = getDefaultSize(frameWidth, widthMeasureSpec);
		int height = getDefaultSize(frameHeight, heightMeasureSpec);
		
		if (frameWidth > 0 && frameHeight > 0) {
			if (frameWidth * height > width * frameHeight) {
				height = width * frameHeight / frameWidth;
			} else if (frameWidth * height < width * frameHeight) {
				width = height * frameWidth / frameHeight;
			}
		}
		setMeasuredDimension(width, height);
	}
	
	public void setAlphaValue(int alpha) {
		nativeSetAlphaValue(mNativePtr, alpha);
		requestRender();
	}
	
	public int getAlphaValue() {
		return nativeGetAlphaValue(mNativePtr);
	}
	
	/**
	 * Requests update of the view with an OpenNI frame.
	 * @param frame The frame to be drawn
	 */
	synchronized public void update(VideoFrameRef frame) {
		nativeUpdate(mNativePtr, frame.getHandle());
		if (mPrevFrameWidth != frame.getVideoMode().getResolutionX() || mPrevFrameHeight != frame.getVideoMode().getResolutionY()) {
			mPrevFrameWidth = frame.getVideoMode().getResolutionX();
			mPrevFrameHeight = frame.getVideoMode().getResolutionY();
			post(new Runnable() {
		        public void run() {
		        	requestLayout();
		        }
		    });
		}
		requestRender();
	}
	
	synchronized public void clear() {
		nativeClear(mNativePtr);
		requestRender();
	}

	class Renderer implements GLSurfaceView.Renderer {
		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig c) {
			nativeOnSurfaceCreated(mNativePtr);
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int w, int h) {
			nativeOnSurfaceChanged(mNativePtr, w, h);
		}

		@Override
		public void onDrawFrame(GL10 gl) {
			synchronized (OpenNIView.this) {
				nativeOnDraw(mNativePtr);
			}
		}
	}

	private static native long nativeCreate();
	private static native void nativeDestroy(long nativePtr);
	private static native void nativeSetAlphaValue(long nativePtr, int alpha);
	private static native int nativeGetAlphaValue(long nativePtr);
	private static native void nativeOnSurfaceCreated(long nativePtr);
	private static native void nativeOnSurfaceChanged(long nativePtr, int w, int h);
	private static native void nativeUpdate(long nativePtr, long frameRef);
	private static native void nativeClear(long nativePtr);
	private static native void nativeOnDraw(long nativePtr);
}