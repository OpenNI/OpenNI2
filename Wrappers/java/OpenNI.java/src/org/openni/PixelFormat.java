package org.openni;

import java.util.NoSuchElementException;

/**
 * Provides string names values for all pixel format type codes. <BR>
 * <BR>
 * 
 */
public enum PixelFormat {
  DEPTH_1_MM(100), DEPTH_100_UM(101), SHIFT_9_2(102), SHIFT_9_3(103),

  // Color
  RGB888(200), YUV422(201), GRAY8(202), GRAY16(203), JPEG(204), YUYV(205);
  private final int mValue;

  private PixelFormat(int value) {
    this.mValue = value;
  }

  public int toNative() {
    return this.mValue;
  }

  public static PixelFormat fromNative(int value) {
    for (PixelFormat type : PixelFormat.values()) {
      if (type.mValue == value) return type;
    }

    throw new NoSuchElementException(String.format("Unknown pixel format: %d", value));
  }



}
