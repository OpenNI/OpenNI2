package org.openni;

import java.util.NoSuchElementException;

/**
 * Provides string names values for all sensors type codes. <BR>
 * <BR>
 * 
 */
public enum SensorType {
  IR(1), COLOR(2), DEPTH(3);

  public int toNative() {
    return this.mValue;
  }

  public static SensorType fromNative(int value) {
    for (SensorType type : SensorType.values()) {
      if (type.mValue == value) return type;
    }

    throw new NoSuchElementException();
  }

  private final int mValue;

  private SensorType(int value) {
    this.mValue = value;
  }

}
