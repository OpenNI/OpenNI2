package org.openni;

import java.util.NoSuchElementException;

/**
 * Provides string names values for all image registration type codes. <BR>
 * <BR>
 * 
 */
public enum ImageRegistrationMode {
  OFF(0), DEPTH_TO_COLOR(1);

  public int toNative() {
    return this.mValue;
  }

  public static ImageRegistrationMode fromNative(int value) {
    for (ImageRegistrationMode type : ImageRegistrationMode.values()) {
      if (type.mValue == value) return type;
    }

    throw new NoSuchElementException();
  }

  private final int mValue;

  private ImageRegistrationMode(int value) {
    this.mValue = value;
  }
}
