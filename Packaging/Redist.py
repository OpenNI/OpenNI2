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

import UpdateVersion

origDir = os.getcwd()

if platform.system() == 'Windows':
    import win32con,pywintypes,win32api
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


class OS:
    #def __init__():
    def setConfiguration(self, config):
        self.config = config
    def getExportedDrivers(self):
        return ['PS1080', 'OniFile']
    def getExportedTools(self):
        return ['NiViewer']
    def getExportedSamples(self):
        return ['ClosestPointViewer', 'EventBasedRead', 'MultiDepthViewer', 'MultipleStreamRead', 'MWClosestPoint', 'MWClosestPointApp', 'SimpleRead', 'SimpleViewer']

    def cleanOutput(self):
        # Delete the redist directory and create it again
        if os.path.isdir(self.config.output_dir):
            shutil.rmtree(self.config.output_dir)
        os.mkdir(self.config.output_dir)
        return self.config.output_dir

    def compileAll(self):
        # Clean
        if os.path.isdir('./Bin') and self.config.compile == 'Rebuild':
            shutil.rmtree('Bin')
            
        for platform in self.config.getPlatforms():
            # Build
            if self.config.compile != 'None':
                rc = self.compile("Release", platform.getPlatformString(), self.config.compile)
                if rc != 0:
                    return rc
            
        return 0

    def copyOpenNI(self, where):
        # Copy config content to Redist
        for f in os.listdir('Config'):
            p = 'Config/' + f;
            if os.path.isdir(p):
                shutil.copytree(p, where+'/'+f);
            else:
                shutil.copy(p, where+'/'+f);

        return

    def createGeneralFiles(self):
        shutil.copy('LICENSE', self.config.output_dir)
        shutil.copy('NOTICE', self.config.output_dir)
        
    def createRedist(self):
        # Create Redist directory
        if not os.path.isdir(self.config.output_dir+'/Redist'):
            os.mkdir(self.config.output_dir+'/Redist')

        # Copy OpenNI to Redist
        self.copyOpenNI(self.config.output_dir+'/Redist')

    def createInclude(self):
        shutil.copytree('Include', self.config.output_dir+'/Include')

    def createLib(self):
        return

    def createTools(self):
        # Create Tools directory
        if not os.path.isdir(self.config.output_dir+'/Tools'):
            os.mkdir(self.config.output_dir+'/Tools')

        if not self.config.supplyTools:
            return False

        # Copy Source/Tools to Redist
        os.mkdir(self.config.output_dir+'/Source/')
        shutil.copytree('Source/Tools', self.config.output_dir+'/Source/Tools')
        for r, d, f in os.walk(self.config.output_dir+'/Source/Tools'):
            for file in f:
                if self.isIllegalSampleFile(file):
                    os.remove(r+'/'+file)
        # Copy ThirdParty/PSCommon/XnLib/Include to Redist
        os.makedirs(self.config.output_dir+'/ThirdParty/PSCommon/XnLib')
        shutil.copytree('ThirdParty/PSCommon/XnLib/Include', self.config.output_dir+'/ThirdParty/PSCommon/XnLib/Include')
        return True

    def createSamples(self):
        ## Copy entire Samples dir to Redist
        shutil.copytree('Samples', self.config.output_dir+'/Samples')
        
        ## Delete user files from samples dir on Redist
        for r, d, f in os.walk(self.config.output_dir+'/Samples'):
            for file in f:
                if self.isIllegalSampleFile(file):
                    os.remove(r+'/'+file)
        
        ## Copy ThirdParty/GL dir to Redist
        shutil.copytree('ThirdParty/GL', self.config.output_dir+'/Samples/GL')

    def createDocumentation(self):
        return 

    def getBinDir(self):
        for platform in self.config.getPlatforms():
            return 'Bin/'+platform.getBinDirString()+'-Release'
        return ""
    def isBaseDirValid(self, path):
        return False

    def regx_replace(self,findStr,repStr,filePath):
        "replaces all findStr by repStr in file filePath using regualr expression"
        findStrRegx = re.compile(findStr)
        tempName=filePath+'~~~'
        fileMode = os.stat(filePath).st_mode
        os.chmod(filePath, fileMode | stat.S_IWRITE)
        input = open(filePath)
        output = open(tempName,'w')
        for s in input:
            output.write(findStrRegx.sub(repStr,s))
        output.close()
        input.close()
        os.remove(filePath)
        os.rename(tempName,filePath)

class OSWin(OS):
    def __init__(self):
        self.supportsBoth = True
        MSVC_KEY = (win32con.HKEY_LOCAL_MACHINE, r"SOFTWARE\Wow6432Node\Microsoft\VisualStudio\10.0")
        MSVC_VALUES = [("InstallDir", win32con.REG_SZ)]
        self.VS_INST_DIR = get_reg_values(MSVC_KEY, MSVC_VALUES)[0]
        self.PROJECT_SLN = "OpenNI.sln"

    def getExportedDrivers(self):
        drivers = OS.getExportedDrivers(self)
        drivers.append('Kinect')
        return drivers

    def copyOpenNI(self, where):
        OS.copyOpenNI(self, where)

        # Copy the OpenNI binaries
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir):
            for file in f:
                if not self.isIllegalBinFile(file) and (file.startswith('OpenNI') and not file.endswith('.lib')):
                    shutil.copy(r+'/'+file, where)
        
        # Copy the OpenNI driver binaries
        if not os.path.isdir(where+'/OpenNI2/Drivers'):
            if not os.path.isdir(where+'/OpenNI2'):
                os.mkdir(where+'/OpenNI2')
            os.mkdir(where+'/OpenNI2/Drivers')
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir+'/OpenNI2/Drivers'):
            for file in f:
                if not self.isIllegalBinDriverFile(file) and not self.isIllegalBinFile(file):
                    shutil.copy(r+'/'+file, where+'/OpenNI2/Drivers')

    def createGeneralFiles(self):
        OS.createGeneralFiles(self)
        
        ## Copy Driver
        if not os.path.isdir(self.config.output_dir+'/Driver'):
            shutil.copytree('ThirdParty/PSCommon/XnLib/Driver/Win32/Bin', self.config.output_dir+'/Driver')

    def createLib(self):
        # Copy the libraries to the Libs directory
        if not os.path.isdir(self.config.output_dir+'/Lib'):
            os.mkdir(self.config.output_dir+'/Lib/')
        binDir = self.getBinDir()
        shutil.copy(binDir+'/OpenNI2.lib', self.config.output_dir+'/Lib/')

    def createTools(self):
        supplyTools = OS.createTools(self)

        # Copy tool files
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir):
            for file in f:
                if not self.isIllegalBinFile(file) and not self.isIllegalToolFile(file):
                    shutil.copy(r+'/'+file, self.config.output_dir+'/Tools/')

        # Copy OpenNI files
        self.copyOpenNI(self.config.output_dir+'/Tools/')

        # Copy GLUT .dll file
        shutil.copy('ThirdParty/GL/glut'+self.config.bits+'.dll', self.config.output_dir+'/Tools/')

        if not supplyTools:
            return

    def createSamples(self):
        OS.createSamples(self)
        
        # Create the samples bin directory
        if not os.path.isdir(self.config.output_dir+'/Samples/Bin'):
            os.mkdir(self.config.output_dir+'/Samples/Bin/')
        
        # Copy sample files
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir):
            for file in f:
                if not self.isIllegalBinFile(file) and not self.isIllegalSampleBinFile(file):
                    shutil.copy(r+'/'+file, self.config.output_dir+'/Samples/Bin/')

        # Copy OpenNI files
        self.copyOpenNI(self.config.output_dir+'/Samples/Bin/')
        
        # Copy GLUT .dll file
        shutil.copy('ThirdParty/GL/glut'+self.config.bits+'.dll', self.config.output_dir+'/Samples/Bin/')

        # replace location of OutDir in project files to relative path of Bin
        outDir = '<OutDir>\$\(SolutionDir\)Bin\\\\\$\(Platform\)-\$\(Configuration\)'
        newOutDir = '<OutDir>$(ProjectDir)..\\Bin'
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/ClosestPointViewer/ClosestPointViewer.vcxproj")
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/EventBasedRead/EventBasedRead.vcxproj")
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/MultiDepthViewer/MultiDepthViewer.vcxproj")
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/MultipleStreamRead/MultipleStreamRead.vcxproj")
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/MWClosestPoint/MWClosestPoint.vcxproj")
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/MWClosestPointApp/MWClosestPointApp.vcxproj")
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/SimpleRead/SimpleRead.vcxproj")
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/SimpleViewer/SimpleViewer.vcxproj")

        # replace location of IntDir in project files to relative path of Bin
        intDir = '<IntDir>\$\(SolutionDir\)Bin'
        newIntDir = '<IntDir>$(ProjectDir)..\\Bin'
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/ClosestPointViewer/ClosestPointViewer.vcxproj")
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/EventBasedRead/EventBasedRead.vcxproj")
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/MultiDepthViewer/MultiDepthViewer.vcxproj")
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/MultipleStreamRead/MultipleStreamRead.vcxproj")
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/MWClosestPoint/MWClosestPoint.vcxproj")
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/MWClosestPointApp/MWClosestPointApp.vcxproj")
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/SimpleRead/SimpleRead.vcxproj")
        self.regx_replace(intDir, newIntDir, self.config.output_dir + "/Samples/SimpleViewer/SimpleViewer.vcxproj")

        # replace location of OpenNI2.lib in project files to environment variable of OPENNI2_LIB[64]
        platform_suffix = ''
        if self.config.bits == '64':
            platform_suffix = '64'
        libDir = '<AdditionalLibraryDirectories>\$\(OutDir\)'
        newLibDir = '<AdditionalLibraryDirectories>$(OutDir);$(OPENNI2_LIB' + platform_suffix + ')'
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/ClosestPointViewer/ClosestPointViewer.vcxproj")
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/EventBasedRead/EventBasedRead.vcxproj")
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/MultiDepthViewer/MultiDepthViewer.vcxproj")
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/MultipleStreamRead/MultipleStreamRead.vcxproj")
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/MWClosestPoint/MWClosestPoint.vcxproj")
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/MWClosestPointApp/MWClosestPointApp.vcxproj")
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/SimpleRead/SimpleRead.vcxproj")
        self.regx_replace(libDir, newLibDir, self.config.output_dir + "/Samples/SimpleViewer/SimpleViewer.vcxproj")
        
        # replace location of OpenNI include path from ..\..\Include to environment variable OPENNI2_INCLUDE
        incDir = '..\\\\..\\\\Include'
        newIncDir = '$(OPENNI2_INCLUDE' + platform_suffix + ')'
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/ClosestPointViewer/ClosestPointViewer.vcxproj")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/EventBasedRead/EventBasedRead.vcxproj")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MultiDepthViewer/MultiDepthViewer.vcxproj")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MultipleStreamRead/MultipleStreamRead.vcxproj")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MWClosestPoint/MWClosestPoint.vcxproj")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MWClosestPointApp/MWClosestPointApp.vcxproj")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/SimpleRead/SimpleRead.vcxproj")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/SimpleViewer/SimpleViewer.vcxproj")

        # replace location GL files
        glDir = '..\\\\..\\\\ThirdParty\\\\GL'
        newGlDir = '..\\GL'
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/ClosestPointViewer/ClosestPointViewer.vcxproj")
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/EventBasedRead/EventBasedRead.vcxproj")
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/MultiDepthViewer/MultiDepthViewer.vcxproj")
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/MultipleStreamRead/MultipleStreamRead.vcxproj")
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/MWClosestPoint/MWClosestPoint.vcxproj")
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/MWClosestPointApp/MWClosestPointApp.vcxproj")
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/SimpleRead/SimpleRead.vcxproj")
        self.regx_replace(glDir, newGlDir, self.config.output_dir + "/Samples/SimpleViewer/SimpleViewer.vcxproj")

    def createDocumentation(self):
        if not self.config.createDocs:
            return
        ## Run doxygen
        beforeDir = os.getcwd()
        os.chdir('Source/Documentation')
        if os.path.isdir('index'):
            shutil.rmtree('index')
        subprocess.call(["python", "Runme.py"])
        
        ## Create documentation dir in redist
        os.chdir(beforeDir)
        if not os.path.isdir(self.config.output_dir+'/Documentation'):
            os.mkdir(self.config.output_dir+'/Documentation')
        
        ## copy doxygen output (html?) to doc dir in redist
        shutil.copy('Source/Documentation/html/OpenNI.chm', self.config.output_dir+'/Documentation/')

    def compile(self, configuration, platform, compilationMode):
        outfile = origDir+'/build.'+configuration+'.'+platform+'.txt'
        devenv_cmd = '\"'+self.VS_INST_DIR + 'devenv\" '+self.PROJECT_SLN + ' /' + compilationMode + ' \"'+configuration+'|'+platform+'\" /out '+outfile
        print(devenv_cmd)
        rc = subprocess.call(devenv_cmd, close_fds=True)
        print(compilationMode + ', RC: %d'%rc)
        if rc == 0:
            os.remove(outfile)
        
        return rc

    def isBaseDirValid(self, dir):
        if os.path.isdir(dir) and os.path.exists(dir+'/OpenNI.sln'):
            return True
        return False
    def isIllegalSampleFile(self, file):
        return file.endswith('.user') or file == 'Makefile' or file == 'Android.mk'
    def isIllegalBinFile(self, file):
        return not file.endswith('.exe') and not file.endswith('.dll') and not file.endswith('.pdb') and not file.endswith('.lib') or (not self.config.supplyTools and file == 'XnLib.lib')
    def isIllegalBinDriverFile(self, file):
        return not any(file.startswith(driver) for driver in self.getExportedDrivers()) or file.endswith('.lib')
    def isIllegalToolFile(self, file):
        return not any(file.startswith(tool) for tool in self.getExportedTools()) or file.endswith('.lib')
    def isIllegalSampleBinFile(self, file):
        return not any(file.startswith(sample) for sample in self.getExportedSamples())

class OSLinux(OS):
    def __init__(self):
        self.supportsBoth = False

    def copyOpenNI(self, where):
        OS.copyOpenNI(self, where)

        # Copy the OpenNI binaries
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir):
            for file in f:
                if not self.isIllegalBinFile(file) and file.startswith('libOpenNI'):
                    shutil.copy(r+'/'+file, where)
        
        # Copy the OpenNI driver binaries
        if not os.path.isdir(where+'/OpenNI2/Drivers'):
            if not os.path.isdir(where+'/OpenNI2'):
                os.mkdir(where+'/OpenNI2')
            os.mkdir(where+'/OpenNI2/Drivers')
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir+'/OpenNI2/Drivers'):
            for file in f:
                if not self.isIllegalBinDriverFile(file) and not self.isIllegalBinFile(file):
                    shutil.copy(r+'/'+file, where+'/OpenNI2/Drivers')

    def createTools(self):
        # Arm redist does not provide Tools:
        if isinstance(self.config.getPlatforms()[0], PlatformArm):
            return

        supplyTools = OS.createTools(self)

        # Copy NiViewer required files.
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir):
            for file in f:
                if not self.isIllegalBinFile(file) and not self.isIllegalToolFile(file):
                    shutil.copy(r+'/'+file, self.config.output_dir+'/Tools/')

        # Copy OpenNI files
        self.copyOpenNI(self.config.output_dir+'/Tools/')

        if not supplyTools:
            return

        #'Bin/'+platform.getBinDirString()+'-Release'
        #os.makedirs(self.config.output_dir+'/ThirdParty/PSCommon/XnLib/Bin')
        shutil.copytree('ThirdParty/PSCommon/XnLib/Bin/'+self.config.getPlatforms()[0].getBinDirString()+'-Release', self.config.output_dir+'/ThirdParty/PSCommon/XnLib/Bin')

    def createSamples(self):
        OS.createSamples(self)
        
        # Delete GLUT libs (they are windows libraries)
        if os.path.isfile(self.config.output_dir+'/Samples/GL/glut32.lib'):
            os.remove(self.config.output_dir+'/Samples/GL/glut32.lib')
        if os.path.isfile(self.config.output_dir+'/Samples/GL/glut64.lib'):
            os.remove(self.config.output_dir+'/Samples/GL/glut64.lib')
        
        # Create the samples bin directory
        if not os.path.isdir(self.config.output_dir+'/Samples/Bin'):
            os.mkdir(self.config.output_dir+'/Samples/Bin/')
        
        # Copy sample files
        binDir = self.getBinDir()
        for r, d, f in os.walk(binDir):
            for file in f:
                if not self.isIllegalBinFile(file) and not self.isIllegalSampleBinFile(file):
                    shutil.copy(r+'/'+file, self.config.output_dir+'/Samples/Bin/')

        # Copy OpenNI files
        self.copyOpenNI(self.config.output_dir+'/Samples/Bin/')

        ## Copy BuildSystem, which is needed to compile the samples - TEMPORARY
        shutil.copytree('ThirdParty/PSCommon/BuildSystem', self.config.output_dir+'/Samples/BuildSystem')

        # Change location of make file output directory
        outDir = 'OUT_DIR = \$\(BIN_DIR\)/\$\(PLATFORM\)-\$\(CFG\)'
        newOutDir = 'OUT_DIR = $(BIN_DIR)'
        self.regx_replace(outDir, newOutDir, self.config.output_dir + "/Samples/BuildSystem/CommonDefs.mak")

        # Change location of make files bin directory
        binDir = 'BIN_DIR = ../../Bin'
        newBinDir = 'BIN_DIR = ../Bin'
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/ClosestPointViewer/Makefile")
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/EventBasedRead/Makefile")
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/MultiDepthViewer/Makefile")
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/MultipleStreamRead/Makefile")
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/MWClosestPoint/Makefile")
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/MWClosestPointApp/Makefile")
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/SimpleRead/Makefile")
        self.regx_replace(binDir, newBinDir, self.config.output_dir + "/Samples/SimpleViewer/Makefile")

        # Change makefile build system reference
        incDir = '../../ThirdParty/PSCommon'
        newIncDir = '..'
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/ClosestPointViewer/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/EventBasedRead/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MultiDepthViewer/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MultipleStreamRead/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MWClosestPoint/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MWClosestPointApp/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/SimpleRead/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/SimpleViewer/Makefile")
        # Change GL include dir
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/ClosestPointViewer/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/EventBasedRead/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MultiDepthViewer/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MultipleStreamRead/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MWClosestPoint/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/MWClosestPointApp/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/SimpleRead/Makefile")
        self.regx_replace(incDir, newIncDir, self.config.output_dir + "/Samples/SimpleViewer/Makefile")

    def createDocumentation(self):
        if not self.config.createDocs:
            return
        ## Run doxygen
        beforeDir = os.getcwd()
        os.chdir('Source/Documentation')
        if os.path.isdir('index'):
            shutil.rmtree('index')
        if subprocess.call(["python", "Runme.py"]) != 0:
            print "Couldn't run doxygen!";
            os.chdir(beforeDir)
            sys.exit(3)

        ## Create documentation dir in redist
        os.chdir(beforeDir)
        if not os.path.isdir(self.config.output_dir+'/Documentation'):
            os.mkdir(self.config.output_dir+'/Documentation')
        
        ## copy doxygen output (html?) to doc dir in redist
        shutil.copytree('Source/Documentation/html', self.config.output_dir+'/Documentation/html')
        
        for r, d, f in os.walk(self.config.output_dir+'/Documentation/html/'):
            for file in f:
                if file.endswith('md5'):
                    os.remove(r+'/'+file)

    def compile(self, configuration, platform, compilationMode):
        def calc_jobs_number():
            cores = 1
            
            try:
                if isinstance(self, OSMac):
                    txt = gop('sysctl -n hw.physicalcpu')
                else:
                    txt = gop('grep "processor\W:" /proc/cpuinfo | wc -l')

                cores = int(txt)
            except:
                pass
               
            return str(cores * 2)

        # make sure platform is valid (linux compilation can only be done on platform machine's type).
        if self.config.machine == 'x86_64' and platform == 'x86':
            print('Error: Building x86 platform requires 32bit operating system')
            return 1
        outfile = origDir+'/build.'+configuration+'.'+platform+'.txt'
        
        compilation_cmd = "make -j" + calc_jobs_number() + " CFG=" + configuration + " PLATFORM=" + platform + " > " + outfile + " 2>&1"
        if compilationMode == 'Rebuild':
            compilation_cmd = "make CFG=" + configuration + " PLATFORM=" + platform + " clean > /dev/null && " + compilation_cmd
        
        print(compilation_cmd)
        rc = os.system(compilation_cmd)
        print('RC: %d'%rc)
        if rc == 0:
            os.remove(outfile)
        
        return rc

    def isBaseDirValid(self, dir):
        if os.path.isdir(dir) and os.path.exists(dir+'/Makefile'):
            return True
        return False
    def isIllegalBinFile(self, file):
        return False
    def isIllegalBinDriverFile(self, file):
        return not any(file=="lib"+driver+".so" for driver in self.getExportedDrivers())
    def isIllegalSampleFile(self, file):
        return any(file.endswith(ext) for ext in ['.vcxproj', '.vcxproj.filters', 'Android.mk'])
    def isIllegalToolFile(self, file):
        return not any(file.startswith(tool) for tool in self.getExportedTools())
    def isIllegalSampleBinFile(self, file):
        return not any((file.startswith(sample) or file.startswith('lib'+sample)) for sample in self.getExportedSamples())

class OSMac(OSLinux):
    def isIllegalBinDriverFile(self, file):
        return not any(file=="lib"+driver+".dylib" for driver in self.getExportedDrivers())

class Platform:
    def __init__(self):
        print "Bla"
    def getBinDirString(self):
        return self.platformString
    def getPlatformString(self):
        return self.generalPlatformString
    def getBits(self):
        return self.bits
        
class Platform32(Platform):
    def __init__(self):
        if platform.system() == 'Windows':
            self.platformString = 'Win32'
        else:
            self.platformString = 'x86'
        self.generalPlatformString = 'x86'
        self.bits = '32'

class Platform64(Platform):
    def __init__(self):
        self.platformString = 'x64'
        self.generalPlatformString = 'x64'
        self.bits = '64'

class PlatformArm(Platform):
    def __init__(self):
        self.platformString = 'Arm'
        self.generalPlatformString = 'Arm'
        self.bits = 'arm'

def boolean(string):
    string = string.lower()
    if string in ['0', 'f', 'false', 'no', 'off']:
        return False
    elif string in ['1', 't', 'true', 'yes', 'on']:
        return True
    raise ValueError()

class Config:
    def __init__(self):
        self.bits = '32'
        self.path = '..'
        self.output_dir = ''
        self.compile = 'Rebuild'
        self.createDocs = True
        self.supplyTools = False
        self.machine = platform.machine()
        
    def parseArgs(self, args):
        parser = argparse.ArgumentParser(prog=sys.argv[0])
        parser.add_argument('-path', default='..')
        parser.add_argument('-output', default='')
        parser.add_argument('-platform', default='32', choices=['32', '64', 'both', 'x86', 'x64', 'arm'])
        parser.add_argument('-compile', default='Rebuild', choices=['Rebuild', 'Build', 'None'])
        parser.add_argument('-docs', default = True, const=True, nargs='?', type=boolean)
        parser.add_argument('-tools', default = False, const=True, nargs='?', type=boolean)
        
        options = parser.parse_args(args)
        self.path = options.path
        self.compile = options.compile
        self.createDocs = options.docs
        self.supplyTools = options.tools
        
        self.bits = options.platform
        if self.bits == 'x86':
            self.bits = '32'
        elif self.bits == 'x64':
            self.bits = '64'
       
        if options.output != '':
            self.output_dir = options.output
        else:
            self.output_dir = 'Packaging/Output'
            if (len(self.getPlatforms()) == 1):
                self.output_dir = self.output_dir + self.getPlatforms()[0].getBits()
        
        return True
        
    def getPlatforms(self):
        platforms = []
        if self.bits == '32':
            platforms.append(Platform32())
        elif self.bits == '64':
            platforms.append(Platform64())
        elif self.bits == 'arm':
            platforms.append(PlatformArm())
        elif self.bits == 'both':
            platforms.append(Platform32())
            platforms.append(Platform64())
    
        return platforms
    
    def printState(self):
        print "Path: " + self.path
        print "Output: " + self.output_dir
        print "Platform: " + self.bits
        print "Compile: " + str(self.compile)
        print "Docs: " + str(self.createDocs)
        print "Tools: " + str(self.supplyTools)

def Redist(myConfig):
    # Check operating system.
    plat = platform.system()
    if plat == 'Windows':
        myOS = OSWin()
    elif plat == 'Linux':
        myOS = OSLinux()
    elif plat == 'Darwin':
         myOS = OSMac()
    else:
        print "Unsupported OS: " + platform.system()
        sys.exit(1)

    myConfig.printState()

    myOS.setConfiguration(myConfig)
    
    origDir = os.getcwd()

    if not myOS.isBaseDirValid(myConfig.path):
        print 'Directory '+myConfig.path+' not valid'
        sys.exit(1)

    os.chdir(myConfig.path)

    output_dir = myOS.cleanOutput()
    if myOS.compileAll() != 0:
        print 'Compilation failure'
        sys.exit(2)

    # Create file structure
    myOS.createGeneralFiles()
    myOS.createRedist()
    myOS.createInclude()
    myOS.createTools()
    myOS.createLib()
    myOS.createSamples()
    myOS.createDocumentation()

    ## Done
    os.chdir(origDir)
    
###### main
if __name__ == '__main__':
    # Parse configuration.
    myConfig = Config()
    if not myConfig.parseArgs(sys.argv[1:]):
        sys.exit(1)

    # Run
    Redist(myConfig)
