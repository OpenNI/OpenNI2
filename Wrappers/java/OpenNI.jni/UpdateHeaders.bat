call Build.bat
"%JAVA_HOME%\bin\javah" -classpath ..\..\..\Bin\Intermediate\java org.openni.NativeMethods
CreateMethods.py