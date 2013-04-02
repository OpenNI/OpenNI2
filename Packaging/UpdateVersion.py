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
#
import sys
import os
import re
import stat
from datetime import date

VERSION_MAJOR = 2
VERSION_MINOR = 2
VERSION_MAINTENANCE = 0
VERSION_BUILD = 8

class UpdateVersion:
    def main(self):
        self.version_major = VERSION_MAJOR
        self.version_minor = VERSION_MINOR
        self.version_maintenance = VERSION_MAINTENANCE
        self.version_build = VERSION_BUILD
        # ----------------------- MAIN -------------------------
        if len(sys.argv) == 5:
            self.version_major=int(sys.argv[1])
            self.version_minor=int(sys.argv[2])
            self.version_maintenance=int(sys.argv[3])
            self.version_build=int(sys.argv[4])

        if (self.version_major > 9):
            print( "Illegal major version")
            sys.exit()

        if (self.version_minor > 99):
            print ("Illegal minor version")
            sys.exit()

        if (self.version_maintenance > 99):
            print ("Illegal maintenance version")
            sys.exit()

        if (self.version_build > 9999):
            print ("Illegal build version")
            sys.exit()

        self.strVersion = str(self.version_major) + "." + str(self.version_minor) + "." + str(self.version_maintenance) + "." + str(self.version_build)

        print (("Going to update files to version: %d.%d.%d.%d" % (self.version_major, self.version_minor, self.version_maintenance, self.version_build)))

        self.update_self_defs("./UpdateVersion.py")
        self.update_src_ver_defs("../Include/OniVersion.h")
        self.update_wix_include("Install/Includes/Variables.wxi")
        self.update_wix_project("Install/Install.wixproj")
        self.update_doxygen("../Source/Documentation/Doxyfile")
        self.update_release_notes("../ReleaseNotes.txt")

        print ("\n*** Done ***")

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

    def update_self_defs (self,filePath):
        print(( "Updating self version defines: " + filePath))
        self.regx_replace("VERSION_MAJOR = \d+\n", "VERSION_MAJOR = " + str(self.version_major) + "\n", filePath)
        self.regx_replace("VERSION_MINOR = \d+\n", "VERSION_MINOR = " + str(self.version_minor) + "\n", filePath)
        self.regx_replace("VERSION_MAINTENANCE = \d+\n", "VERSION_MAINTENANCE = " + str(self.version_maintenance) + "\n", filePath)
        self.regx_replace("VERSION_BUILD = \d+\n", "VERSION_BUILD = " + str(self.version_build) + "\n", filePath)

    def update_src_ver_defs (self,filePath):
        print(( "Updating source version defines: " + filePath))
        self.regx_replace("#define ONI_VERSION_MAJOR[ \t](.*)", "#define ONI_VERSION_MAJOR\t" + str(self.version_major), filePath)
        self.regx_replace("#define ONI_VERSION_MINOR[ \t](.*)", "#define ONI_VERSION_MINOR\t" + str(self.version_minor), filePath)
        self.regx_replace("#define ONI_VERSION_MAINTENANCE[ \t](.*)", "#define ONI_VERSION_MAINTENANCE\t" + str(self.version_maintenance), filePath)
        self.regx_replace("#define ONI_VERSION_BUILD[ \t](.*)", "#define ONI_VERSION_BUILD\t" + str(self.version_build), filePath)

    def update_wix_include (self,filePath):
        print (("Updating wix include: " + filePath))
        self.regx_replace("define MajorVersion=(.*)", "define MajorVersion=" + str(self.version_major) + "?>", filePath)
        self.regx_replace("define MinorVersion=(.*)", "define MinorVersion=" + str(self.version_minor) + "?>", filePath)
        self.regx_replace("define MaintenanceVersion=(.*)", "define MaintenanceVersion=" + str(self.version_maintenance) + "?>", filePath)
        self.regx_replace("define BuildVersion=(.*)", "define BuildVersion=" + str(self.version_build) + "?>", filePath)

    def update_wix_project (self,filePath):
        print (("Updating wix project: " + filePath))
        self.regx_replace("<OutputName>(.*)</OutputName>", "<OutputName>OpenNI-Windows-$(Platform)-" + str(self.version_major) + "." + str(self.version_minor) + "." + str(self.version_maintenance) + "</OutputName>", filePath)

    def update_doxygen (self,filePath):
        print (("Updating doxygen: " + filePath))
        self.regx_replace("PROJECT_NAME\s*=\s*\"OpenNI (\d+)\.(\d+)\.(\d+)\"", "PROJECT_NAME = \"OpenNI " + str(self.version_major) + "." + str(self.version_minor) + "." + str(self.version_maintenance) + "\"", filePath)

    def update_release_notes (self,filePath):
        print (("Updating release notes: " + filePath))

        tempName = filePath + '~~~'
        os.system("attrib -r " + filePath)
        input = open(filePath)
        output = open(tempName, 'w')
        lines = input.readlines()
        input.close()
        today = date.today()

        lines[0] = 'OpenNI ' + str(self.version_major) + '.' + str(self.version_minor) + '.' + str(self.version_maintenance) + ' Build ' + str(self.version_build) + '\n'
        lines[1] = today.strftime('%B ') + str(today.day) + ' ' + str(today.year) + '\n'
        
        for s in lines:
            output.write(s)
        output.close()
        os.remove(filePath)
        os.rename(tempName,filePath)        
        
if __name__ == '__main__':
    UpdateVersion().main()
