#!/usr/bin/python

import os
import sys
import platform
import shutil
import subprocess

def create_page(orig_path, page_name, page_header):
    orig = open(orig_path)
    dest = open("Temp/" + os.path.split(orig_path)[1] + ".txt", "w")
    dest.write("/** @page " + page_name + " " + page_header + "\n")
    dest.write(orig.read())
    dest.write("\n*/")
    orig.close()
    dest.close()
   
beforeDir = os.getcwd()
scriptDir = os.path.dirname(sys.argv[0])
os.chdir(scriptDir)

# Start with C++ reference
   
# create Legal page
if os.path.isdir("Temp"):
    shutil.rmtree("Temp")
os.mkdir("Temp")
create_page("../../NOTICE", "legal", "Legal Stuff & Acknowledgments")
create_page("../../ReleaseNotes.txt", "release_notes", "Release Notes")

errfile = "Temp/doxygen_error"
subprocess.check_call(["doxygen", "Doxyfile"], stdout=open(os.devnull,"w"), stderr=open(errfile,"w"))

# create Java
javaDocExe = ""
if platform.system() == 'Windows':
    javaDocExe = os.path.expandvars(os.path.join('$JAVA_HOME', 'bin', 'javadoc.exe'))
else:
    javaDocExe = 'javadoc'

javaSrc = os.path.join('..', '..', 'Wrappers', 'java', 'OpenNI.java', 'src', 'org', 'openni')

# workaround a strange linux behavior where you must pass the list of files
cmd = [javaDocExe, '-d', 'java']
for root, dirs, files in os.walk(javaSrc):
    for file in files:
        cmd.append(os.path.join(root, file))

errfile = "Temp/javadoc_error"
subprocess.check_call(cmd, stdout=open(os.devnull,"w"), stderr=open(errfile,"w"))

os.chdir(beforeDir)
