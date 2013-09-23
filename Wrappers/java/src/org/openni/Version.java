package org.openni;

/**
 * Holds an OpenNI version number, which consists of four separate numbers in the format:
 * major.minor.maintenance.build. For example: 2.0.0.20.
 */
public class Version {
  /**
   * This function return major value. Major version number, incremented for major API
   * restructuring.
   * 
   * @return major value
   * */
  public int getMajor() {
    return mMajor;
  }

  /**
   * This function return minor value. Minor version number, incremented when significant new
   * features added.
   * 
   * @return minor value
   * */
  public int getMinor() {
    return mMinor;
  }

  /**
   * This function return minor value. Maintenance build number, incremented for new releases that
   * primarily provide minor bug fixes.
   * 
   * @return maintenance value
   * */
  public int getMaintenance() {
    return mMaintenance;
  }

  /**
   * This function return build number. Incremented for each new API build. Generally not shown on
   * the installer and download site.
   * 
   * @return build value
   * */
  public int getBuild() {
    return mBuild;
  }

  private Version(int major, int minor, int maintenance, int build) {
    this.mMajor = major;
    this.mMinor = minor;
    this.mMaintenance = maintenance;
    this.mBuild = build;
  }

  private final int mMajor;
  private final int mMinor;
  private final int mMaintenance;
  private final int mBuild;
}
