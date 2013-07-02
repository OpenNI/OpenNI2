./Build.sh
javah -classpath ../../../Bin/Intermediate/java org.openni.NativeMethods
python CreateMethods.py
