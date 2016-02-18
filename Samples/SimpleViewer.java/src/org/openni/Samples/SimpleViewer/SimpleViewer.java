/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
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
package org.openni.Samples.SimpleViewer;

import java.awt.*;
import java.awt.image.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.openni.*;


public class SimpleViewer extends Component 
                          implements VideoStream.NewFrameListener {
    
    float mHistogram[];
    int[] mImagePixels;
    int mMaxGray16Value = 0;
    VideoStream mVideoStream;
    VideoFrameRef mLastFrame;
    BufferedImage mBufferedImage;

    public SimpleViewer() {
    }
    
    public void setStream(VideoStream videoStream) {
        if (mLastFrame != null) {
            mLastFrame.release();
            mLastFrame = null;
        }
        
        if (mVideoStream != null) {
            mVideoStream.removeNewFrameListener(this);
        }
        
        mVideoStream = videoStream;
        
        if (mVideoStream != null) {
            mVideoStream.addNewFrameListener(this);
        }
    }

    @Override
    public synchronized void paint(Graphics g) {
        if (mLastFrame == null) {
            return;
        }
        
        int width = mLastFrame.getWidth();
        int height = mLastFrame.getHeight();

        // make sure we have enough room
        if (mBufferedImage == null || mBufferedImage.getWidth() != width || mBufferedImage.getHeight() != height) {
            mBufferedImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
        }
        
        mBufferedImage.setRGB(0, 0, width, height, mImagePixels, 0, width);

        int framePosX = (getWidth() - width) / 2;
        int framePosY = (getHeight() - height) / 2;
        g.drawImage(mBufferedImage, framePosX, framePosY, null);
    }

    @Override
    public synchronized void onFrameReady(VideoStream stream) {
        if (mLastFrame != null) {
            mLastFrame.release();
            mLastFrame = null;
        }
        
        mLastFrame = mVideoStream.readFrame();
        ByteBuffer frameData = mLastFrame.getData().order(ByteOrder.LITTLE_ENDIAN);
        
        // make sure we have enough room
        if (mImagePixels == null || mImagePixels.length < mLastFrame.getWidth() * mLastFrame.getHeight()) {
            mImagePixels = new int[mLastFrame.getWidth() * mLastFrame.getHeight()];
        }
        
        switch (mLastFrame.getVideoMode().getPixelFormat())
        {
            case DEPTH_1_MM:
            case DEPTH_100_UM:
            case SHIFT_9_2:
            case SHIFT_9_3:
                calcHist(frameData);
                int pos = 0;
                while(frameData.remaining() > 0) {
                    int depth = (int)frameData.getShort() & 0xFFFF;
                    short pixel = (short)mHistogram[depth];
                    mImagePixels[pos] = 0xFF000000 | (pixel << 16) | (pixel << 8);
                    pos++;
                }
                break;
            case RGB888:
                pos = 0;
                while (frameData.remaining() > 0) {
                    int red = (int)frameData.get() & 0xFF;
                    int green = (int)frameData.get() & 0xFF;
                    int blue = (int)frameData.get() & 0xFF;
                    mImagePixels[pos] = 0xFF000000 | (red << 16) | (green << 8) | blue;
                    pos++;
                }
                break;
            case GRAY8:
                pos = 0;
                while (frameData.remaining() > 0) {
                    int pixel = (int)frameData.get() & 0xFF;
                    mImagePixels[pos] = 0xFF000000 | (pixel << 16) | (pixel << 8) | pixel;
                    pos++;
                }
                break;
            case GRAY16:
                calcMaxGray16Value(frameData);
                pos = 0;
                while (frameData.remaining() > 0) {
                    int pixel = (int)frameData.getShort() & 0xFFFF;
                    pixel = (int)(pixel * 255.0 / mMaxGray16Value);
                    mImagePixels[pos] = 0xFF000000 | (pixel << 16) | (pixel << 8) | pixel;
                    pos++;
                }
                break;
            default:
                // don't know how to draw
                mLastFrame.release();
                mLastFrame = null;
        }

        repaint();
    }

    private void calcHist(ByteBuffer depthBuffer) {
        // make sure we have enough room
        if (mHistogram == null || mHistogram.length < mVideoStream.getMaxPixelValue()) {
            mHistogram = new float[mVideoStream.getMaxPixelValue()];
        }
        
        // reset
        for (int i = 0; i < mHistogram.length; ++i)
            mHistogram[i] = 0;

        int points = 0;
        while (depthBuffer.remaining() > 0) {
            int depth = depthBuffer.getShort() & 0xFFFF;
            if (depth != 0) {
                mHistogram[depth]++;
                points++;
            }
        }

        for (int i = 1; i < mHistogram.length; i++) {
            mHistogram[i] += mHistogram[i - 1];
        }

        if (points > 0) {
            for (int i = 1; i < mHistogram.length; i++) {
                mHistogram[i] = (int) (256 * (1.0f - (mHistogram[i] / (float) points)));
            }
        }
        depthBuffer.rewind();
    }
    
    private void calcMaxGray16Value(ByteBuffer gray16Buffer) {
        while (gray16Buffer.remaining() > 0) {
            int pixel = (int)gray16Buffer.getShort() & 0xFFFF;
            if (pixel > mMaxGray16Value) {
                mMaxGray16Value = pixel;
            }
        }
        gray16Buffer.rewind();
    }
}
