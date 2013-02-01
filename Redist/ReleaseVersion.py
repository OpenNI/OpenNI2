#!/usr/bin/python

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

import UpdateVersion

if len(sys.argv) < 2 or sys.argv[1] in ('-h','--help'):
    print "usage: " + sys.argv[0] + " <x86|x64|arm|android> [UpdateVersion]"
    sys.exit(1)
    
plat = sys.argv[1]
origDir = os.getcwd()

shouldUpdate = 0
if len(sys.argv) >= 3 and sys.argv[2] == 'UpdateVersion':
    shouldUpdate = 1

if shouldUpdate == 1:
    # Increase Build
    UpdateVersion.VERSION_BUILD += 1
    UpdateVersion.UpdateVersion().main()

def get_reg_values(reg_key, value_list):
    # open the reg key
    try:
        reg_key = win32api.RegOpenKeyEx(*reg_key)
    except pywintypes.error as e:
        raise Exception("Failed to open registry key!")
    # Get the values
    try:
        values = [(win32api.RegQueryValueEx(reg_key, name), data_type) for name, data_type in value_list]
        # values list of ((value, type), expected_type)
        for (value, data_type), expected in values:
            if data_type != expected:
                raise Exception("Bad registry value type! Expected %d, got %d instead." % (expected, data_type))
        # values okay, leave only values
        values = [value for ((value, data_type), expected) in values]
    except pywintypes.error as e:
        raise Exception("Failed to get registry value!")
    finally:
        try:
            win32api.RegCloseKey(reg_key)
        except pywintypes.error as e:
            # We don't care if reg key close failed...
            pass
    return tuple(values)

# Create installer
strVersion = str(UpdateVersion.VERSION_MAJOR) + "." + str(UpdateVersion.VERSION_MINOR) + "." + str(UpdateVersion.VERSION_MAINTENANCE)
finalDir = "Final"
if not os.path.isdir(finalDir):
    os.mkdir(finalDir)
    
if plat == 'android':
    if not 'NDK_ROOT' in os.environ:
        print 'Please define NDK_ROOT!'
        sys.exit(2)

    ndkDir = os.environ['NDK_ROOT']

    buildDir = 'AndroidBuild'
    if os.path.isdir(buildDir):
        shutil.rmtree(buildDir)

    outputDir = 'OpenNI-android-' + strVersion
    if os.path.isdir(outputDir):
        shutil.rmtree(outputDir)

    os.makedirs(buildDir + '/jni')
    os.symlink('../../../', buildDir + '/jni/OpenNI2')
    shutil.copy('../Android.mk', buildDir + '/jni')
    shutil.copy('../Application.mk', buildDir + '/jni')
    rc = subprocess.call([ ndkDir + '/ndk-build', '-C', buildDir, '-j8' ])
    if rc != 0:
        print 'Build failed!'
        sys.exit(3)

    outFile = 'Final/' + outputDir + '.tar'
    
    shutil.move(buildDir + '/libs/armeabi-v7a', outputDir)
    
    # add config files
    shutil.copy('../Config/OpenNI.ini', outputDir)
    shutil.copy('../Config/PS1080.ini', outputDir)

    print('Creating archive ' + outFile)

    rc = subprocess.call(['tar', '-cf', outFile, outputDir])
    if rc != 0:
        print 'Tar failed!'
        sys.exit(3)

elif platform.system() == 'Windows':
    import win32con,pywintypes,win32api,platform
    
    (bits,linkage) = platform.architecture()
    matchObject = re.search('64',bits)
    is_64_bit_machine = matchObject is not None

    if is_64_bit_machine:
        MSVC_KEY = (win32con.HKEY_LOCAL_MACHINE, r"SOFTWARE\Wow6432Node\Microsoft\VisualStudio\10.0")
    else:
        MSVC_KEY = (win32con.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\VisualStudio\10.0")
    
    MSVC_VALUES = [("InstallDir", win32con.REG_SZ)]
    VS_INST_DIR = get_reg_values(MSVC_KEY, MSVC_VALUES)[0]
    PROJECT_SLN = "..\OpenNI.sln"

    outfile = origDir+'/build.Release.'+plat+'.txt'
    devenv_cmd = '\"'+VS_INST_DIR + 'devenv\" '+PROJECT_SLN + ' /Project Install /Rebuild "Release|'+plat+'\" /out '+outfile
    print(devenv_cmd)
    rc = subprocess.call(devenv_cmd, close_fds=True)
    if rc == 0:
        os.remove(outfile)
    else:
        print "Error"

    shutil.move('Install/bin/' + plat + '/en-us/OpenNI-Windows-' + plat + '-' + strVersion + '.msi', finalDir)
        
elif platform.system() == 'Linux' or platform.system() == 'Darwin':
    import Redist
    config = Redist.Config()

    if platform.system() == 'Darwin':
        config.Platform = "MacOS"
    else:
        config.Platform = platform.system()

    if plat == 'x86':
        config.bits = '32'
    elif plat == 'x64':
        config.bits = '64'
    elif plat == 'arm':
        config.bits = 'arm'
    else:
        print 'Unknown platform: ', plat
        sys.exit(1)

    outFile = 'Final/OpenNI-' + config.Platform + '-' + config.getPlatforms()[0].getPlatformString() + '-' + strVersion + '.tar.bz2'
    dirName = 'OpenNI-' + strVersion + '-' + plat
        
    config.path = '..'
    config.output_dir = origDir + '/'+ dirName
    config.compile = 'Build'
    config.createDocs = True
    config.supplyTools = False
    config.machine = platform.machine()
    Redist.Redist(config)
    
    # Copy install script
    shutil.copy('Linux/install.sh', config.output_dir + '/install.sh')

    # Create archive
    print('Creating archive ' + outFile)

    archiveCmd = 'tar -cjf ' + outFile + ' ' + dirName + '/*'
    rc = os.system(archiveCmd)

    if rc != 0:
        print('Could not create archive' + outFile)
else:
    print "Unknown OS"
    sys.exit(2)
