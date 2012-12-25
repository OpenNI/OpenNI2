#!/usr/bin/python

import os
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
   
# create Legal page
if os.path.isdir("Temp"):
    shutil.rmtree("Temp")
os.mkdir("Temp")

create_page("../../NOTICE", "legal", "Legal Stuff & Acknowledgments")
create_page("../../ReleaseNotes.txt", "release_notes", "Release Notes")

errfile = "Temp/doxy_error"
rc = subprocess.call(["doxygen", "Doxyfile"], stdout=open(os.devnull,"w"), stderr=open(errfile,"w"))
if rc != 0:
    print "Failed running doxygen! stderr is in '%s'" % errfile
    sys.exit(1)
