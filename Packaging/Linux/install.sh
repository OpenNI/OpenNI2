#!/bin/sh

#/***************************************************************************
#*                                                                          *
#*  OpenNI 2.0                                                              *
#*  Copyright (C) 2012 PrimeSense Ltd.                                      *
#*                                                                          *
#*  This file is part of OpenNI2 installation.                              *
#*                                                                          *
#*  OpenNI is free software: you can redistribute it and/or modify          *
#*  it under the terms of the GNU Lesser General Public License as published*
#*  by the Free Software Foundation, either version 3 of the License, or    *
#*  (at your option) any later version.                                     *
#*                                                                          *
#*  OpenNI is distributed in the hope that it will be useful,               *
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
#*  GNU Lesser General Public License for more details.                     *
#*                                                                          *
#*  You should have received a copy of the GNU Lesser General Public License*
#*  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.          *
#*                                                                          *
#***************************************************************************/
#

# Check if user is root/running with sudo
if [ `whoami` != root ]; then
    echo Please run this script with sudo
    exit
fi

if [ "`uname -s`" != "Darwin" ]; then
    # Install UDEV rules for USB device
    echo '# Make primesense device mount with writing permissions (default is read only for unknown devices)
    SUBSYSTEM=="usb", ATTR{idProduct}=="0200", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="0300", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="0401", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="0500", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="0600", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="0601", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="1280", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="2100", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="2200", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
    SUBSYSTEM=="usb", ATTR{idProduct}=="f9db", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"' > /etc/udev/rules.d/557-primesense-usb.rules 
fi

ORIG_PATH=`pwd`
cd `dirname $0`
SCRIPT_PATH=`pwd`
cd $ORIG_PATH

OUT_FILE="$SCRIPT_PATH/OpenNIDevEnvironment"

echo "export OPENNI2_INCLUDE=$SCRIPT_PATH/Include" > $OUT_FILE
echo "export OPENNI2_REDIST=$SCRIPT_PATH/Redist" >> $OUT_FILE
chmod a+r $OUT_FILE
