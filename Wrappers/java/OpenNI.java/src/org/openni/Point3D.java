package org.openni;

/**
 * The Point3D object encapsulate 3 Dimension point
 **/
public class Point3D<T> {
  /**
   * The Point3D constructor
   * 
   * @param x coordinate of the point
   * @param y coordinate of the point
   * @param z coordinate of the point
   **/
  public Point3D(T x, T y, T z) {
    this.mX = x;
    this.mY = y;
    this.mZ = z;
  }

  /**
   * Return X coordinate
   * 
   * @return X coordinate
   **/
  public T getX() {
    return mX;
  }

  /**
   * Return Y coordinate
   * 
   * @return Y coordinate
   **/
  public T getY() {
    return mY;
  }

  /**
   * Return Z coordinate
   * 
   * @return Z coordinate
   **/
  public T getZ() {
    return mZ;
  }

  private final T mX, mY, mZ;
}
