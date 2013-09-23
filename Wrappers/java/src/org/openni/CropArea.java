package org.openni;

/**
 * The CropArea object encapsulate cropping information data
 **/
public class CropArea {
  /**
   * The CropArea constructor
   * 
   * @param originX X coordinate for the cropping
   * @param originY Y coordinate for the cropping
   * @param width width cropping value
   * @param height height cropping value
   **/
  public CropArea(int originX, int originY, int width, int height) {
    this.mOriginX = originX;
    this.mOriginY = originY;
    this.mWidth = width;
    this.mHeight = height;
  }

  /**
   * Return X cropping coordinate
   * 
   * @return X cropping coordinate
   **/
  public int getOriginX() {
    return mOriginX;
  }

  /**
   * Return Y cropping coordinate
   * 
   * @return Y cropping coordinate
   **/
  public int getOriginY() {
    return mOriginY;
  }

  /**
   * Return width cropping value
   * 
   * @return width cropping value
   **/
  public int getWidth() {
    return mWidth;
  }

  /**
   * Return height cropping value
   * 
   * @return height cropping value
   **/
  public int getHeight() {
    return mHeight;
  }

  private final int mOriginX, mOriginY, mWidth, mHeight;
}
