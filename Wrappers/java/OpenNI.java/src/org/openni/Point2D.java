package org.openni;

/**
 * The Point2D object encapsulate 2 Dimension point
 **/
public class Point2D<T> {
  /**
   * The Point2D constructor
   * 
   * @param x coordinate of the point
   * @param y coordinate of the point
   **/
  public Point2D(T x, T y) {
    this.mX = x;
    this.mY = y;
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
   * Return X coordinate
   * 
   * @return X coordinate
   **/
  public T getY() {
    return mY;
  }

  private final T mX, mY;
}
