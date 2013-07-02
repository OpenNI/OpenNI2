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

ORIG_PATH=`pwd`
cd `dirname $0`
SCRIPT_PATH=`pwd`
cd $ORIG_PATH

if [ "`uname -s`" != "Darwin" ]; then
    # Install UDEV rules for USB device
    cp ${SCRIPT_PATH}/primesense-usb.rules /etc/udev/rules.d/557-primesense-usb.rules 
fi

OUT_FILE="$SCRIPT_PATH/OpenNIDevEnvironment"

echo "export OPENNI2_INCLUDE=$SCRIPT_PATH/Include" > $OUT_FILE
echo "export OPENNI2_REDIST=$SCRIPT_PATH/Redist" >> $OUT_FILE
chmod a+r $OUT_FILE
