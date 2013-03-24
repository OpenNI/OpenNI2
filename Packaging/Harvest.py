#/****************************************************************************
#*                                                                           *
#*  OpenNI 2.x Alpha                                                         *
#*  Copyright (C) 2012 PrimeSense Ltd.                                       *
#*                                                                           *
#*  This file is part of OpenNI.                                             *
#*                                                                           *
#*  Licensed under the Apache License, Version 2.0 (the "License");          *
#*  you may not use this file except in compliance with the License.         *
#*  You may obtain a copy of the License at                                  *
#*                                                                           *
#*      http://www.apache.org/licenses/LICENSE-2.0                           *
#*                                                                           *
#*  Unless required by applicable law or agreed to in writing, software      *
#*  distributed under the License is distributed on an "AS IS" BASIS,        *
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
#*  See the License for the specific language governing permissions and      *
#*  limitations under the License.                                           *
#*                                                                           *
#****************************************************************************/
import os
import re
import sys
import shutil
import subprocess
import platform
import argparse
import stat
from commands import getoutput as gop

binDir = "../Bin/Win32-Release"
rootDir = "../"

def copySharedObject(sourceDir, name, targetDir):
    if platform.system() == 'Windows':
        shutil.copy(os.path.join(sourceDir, name + '.dll'), targetDir)
        shutil.copy(os.path.join(sourceDir, name + '.pdb'), targetDir)
    elif platform.system() == 'Linux':
        shutil.copy(os.path.join(sourceDir, 'lib' + name + '.so'), targetDir)
    elif platform.system() == 'Darwin':
        shutil.copy(os.path.join(sourceDir, 'lib' + name + '.dylib'), targetDir)
    else:
        raise 'Unsupported platform!'
        
def copyRedistFiles(targetDir):
    os.makedirs(targetDir)
    # start with OpenNI itself
    copySharedObject(binDir, 'OpenNI2', targetDir)
    copySharedObject(binDir, 'OpenNI2.jni', targetDir)
    shutil.copy(os.path.join(binDir, 'org.openni.jar'), targetDir)
    shutil.copy(os.path.join(rootDir, 'Config', 'OpenNI.ini'), targetDir)
    # and now all drivers
    binDriversDir = os.path.join(binDir, 'OpenNI2', 'Drivers')
    targetDriversDir = os.path.join(targetDir, 'OpenNI2', 'Drivers')
    os.makedirs(targetDriversDir)
    copySharedObject(binDriversDir, 'OniFile', targetDriversDir)
    copySharedObject(binDriversDir, 'PS1080', targetDriversDir)
    copySharedObject(binDriversDir, 'PSLink', targetDriversDir)
    copySharedObject(binDriversDir, 'Kinect', targetDriversDir)
    shutil.copy(os.path.join(rootDir, 'Config', 'OpenNI2', 'Drivers', 'PS1080.ini'), targetDriversDir)
    
def copySample(samplesDir, name, isLibrary = False):
    sampleTargetDir = os.path.join(samplesDir, name)
    sampleSourceDir = os.path.join(rootDir, 'Samples', name)
    shutil.copytree(sampleSourceDir, sampleTargetDir)

    # copy binaries
    splitName = os.path.splitext(name)
    if splitName[1] == '.java':
        # copy jar
        shutil.copy(os.path.join(binDir, 'org.openni.Samples.' + splitName[0] + '.jar'), os.path.join(samplesDir, 'Bin'))
        # and script
        if not isLibrary:
            if platform.system() == 'Windows':
                shutil.copy(os.path.join(binDir, 'org.openni.Samples.' + splitName[0] + '.bat'), os.path.join(samplesDir, 'Bin'))
            else:
                shutil.copy(os.path.join(binDir, 'org.openni.Samples.' + splitName[0]), os.path.join(samplesDir, 'Bin'))
    elif isLibrary:
        copySharedObject(binDir, name, os.path.join(samplesDir, 'Bin'))
    else:
        if platform.system() == 'Windows':
            shutil.copy(os.path.join(binDir, name + '.exe'), os.path.join(samplesDir, 'Bin'))
        else:
            shutil.copy(os.path.join(binDir, name), os.path.join(samplesDir, 'Bin'))
    
def harvest(targetRootDir):
    if os.path.exists(targetRootDir):
        shutil.rmtree(targetRootDir)
    
    copyRedistFiles(os.path.join(targetRootDir, 'Redist'))
    
    copyRedistFiles(os.path.join(targetRootDir, os.path.join('Samples', 'Bin')))
    copySample(os.path.join(targetRootDir, 'Samples'), 'SimpleRead')
    copySample(os.path.join(targetRootDir, 'Samples'), 'SimpleViewer')
    copySample(os.path.join(targetRootDir, 'Samples'), 'SimpleViewer.java')
    copySample(os.path.join(targetRootDir, 'Samples'), 'EventBasedRead')
    copySample(os.path.join(targetRootDir, 'Samples'), 'MultiDepthViewer')
    copySample(os.path.join(targetRootDir, 'Samples'), 'MultipleStreamRead')
    copySample(os.path.join(targetRootDir, 'Samples'), 'MWClosestPoint', True)
    copySample(os.path.join(targetRootDir, 'Samples'), 'MWClosestPointApp')
    copySample(os.path.join(targetRootDir, 'Samples'), 'ClosestPointViewer')
    
    copyRedistFiles(os.path.join(targetRootDir, 'Tools'))
        
def copyDocumentation(targetDir, sourceDir):
    if platform.system() == 'Windows':
        return [ [ os.path.join(targetDir, 'OpenNI.chm'), sourceDir ] ]
    elif platform.system() == 'Linux' or platform.system() == 'Darwin':
        result = [ [ os.path.join(targetDir, os.path.basename(file)), sourceDir ] for file in glob.glob(os.path.join(location, '*.html')) ]
        if len(result) == 0:
            raise 'no documentation files found!'
        return result
    else:
        raise 'Unsupported platform!'

harvest('Output_Harvest')
