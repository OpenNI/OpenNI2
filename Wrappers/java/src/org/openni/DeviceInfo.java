package org.openni;

/**
 * <p>
 * The DeviceInfo class encapsulates info related to a specific device.
 * </p>
 * 
 * <p>
 * Applications will generally obtain objects of this type via calls to
 * {@link org.openni.OpenNI#enumerateDevices()} or {@link org.openni.Device#getDeviceInfo()}, and
 * then use the various accessor functions to obtain specific information on that device.
 * </p>
 * 
 * <p>
 * There should be no reason for application code to instantiate this object directly.
 * </p>
 */
public class DeviceInfo {
  public DeviceInfo(String uri, String vendor, String name, int usbVendorId, int usbProductId) {
    this.mUri = uri;
    this.mVendor = vendor;
    this.mName = name;
    this.mUsbVendorId = usbVendorId;
    this.mUsbProductId = usbProductId;
  }

  /**
   * Getter function for the device URI. URI can be used by {@link org.openni.Device#open(String)}
   * to open a specific device.
   * 
   * @return The URI string format is determined by the driver.
   */
  public final String getUri() {
    return mUri;
  }

  /**
   * Getter function for the device vendor name.
   * 
   * @return A the vendor name for this device.
   * */
  public final String getVendor() {
    return mVendor;
  }

  /**
   * Getter function for the device name.
   * 
   * @return The device name for this device.
   */
  public final String getName() {
    return mName;
  }

  /**
   * Getter function for the USB VID device code.
   * 
   * @return The USB VID code for this device.
   */
  public int getUsbVendorId() {
    return mUsbVendorId;
  }

  /**
   * Getter function for the USB PID device code.
   * 
   * @return The USB PID code for this device.
   */
  public int getUsbProductId() {
    return mUsbProductId;
  }

  private final String mVendor, mName, mUri;
  private final int mUsbVendorId, mUsbProductId;
}
