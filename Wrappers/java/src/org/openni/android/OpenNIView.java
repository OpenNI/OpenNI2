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
	private int mDrawX = 0;
	private int mDrawY = 0;
	private int mDrawWidth = 0;
	private int mDrawHeight = 0;
	private int mCurrFrameWidth = 0;
	private int mCurrFrameHeight = 0;
	private int mSurfaceWidth = 0;
	private int mSurfaceHeight = 0;
	
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

		setRenderer(new Renderer() {
			
			@Override
			public void onSurfaceCreated(GL10 gl, EGLConfig c) {
				nativeOnSurfaceCreated(mNativePtr);
			}

			@Override
			public void onSurfaceChanged(GL10 gl, int w, int h) {
				synchronized (OpenNIView.this) {
					mSurfaceWidth = w;
					mSurfaceHeight = h;
					calcDrawArea();
				}
			}

			@Override
			public void onDrawFrame(GL10 gl) {
				synchronized (OpenNIView.this) {
					draw(gl);
				}
			}
		});
		
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
		mCurrFrameWidth = frame.getVideoMode().getResolutionX();
		mCurrFrameHeight = frame.getVideoMode().getResolutionY();
		calcDrawArea();
		requestRender();
	}
	
	synchronized public void clear() {
		nativeClear(mNativePtr);
		requestRender();
	}
	
	protected void draw(GL10 gl) {
		nativeOnDraw(mNativePtr, mDrawX, mDrawY, mDrawWidth, mDrawHeight);
	}
	
	private void calcDrawArea() {
		if (mCurrFrameWidth == 0 || mCurrFrameHeight == 0 || mSurfaceWidth == 0 || mSurfaceHeight == 0) {
			mDrawX = mDrawY = mDrawWidth = mDrawHeight = 0;
			return;
		}
		
		// start with the entire surface
		mDrawX = 0;
		mDrawY = 0;
		mDrawWidth = mSurfaceWidth;
		mDrawHeight = mSurfaceHeight;

		// if view ratio is larger than frame ratio, make width smaller. Otherwise, make height smaller
		if (mCurrFrameWidth * mDrawHeight > mCurrFrameHeight * mDrawWidth)
		{
			mDrawHeight = mCurrFrameHeight * mDrawWidth / mCurrFrameWidth;
			mDrawY = (mSurfaceHeight - mDrawHeight) / 2;
		}
		else
		{
			mDrawWidth = mCurrFrameWidth * mDrawHeight / mCurrFrameHeight;
			mDrawX = (mSurfaceWidth - mDrawWidth) / 2;
		}
	}

	private static native long nativeCreate();
	private static native void nativeDestroy(long nativePtr);
	private static native void nativeSetAlphaValue(long nativePtr, int alpha);
	private static native int nativeGetAlphaValue(long nativePtr);
	private static native void nativeOnSurfaceCreated(long nativePtr);
	private static native void nativeUpdate(long nativePtr, long frameRef);
	private static native void nativeClear(long nativePtr);
	private static native void nativeOnDraw(long nativePtr, int x, int y, int width, int height);
}