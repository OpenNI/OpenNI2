cmake_minimum_required (VERSION 2.8)

project(OpenNI2)

add_definitions(-UUNICODE)
add_definitions(-U_UNICODE)

if(APPLE)
    add_subdirectory(libusbx)
endif(APPLE)

add_subdirectory(ThirdParty/LibJPEG)
add_subdirectory(ThirdParty/PSCommon/XnLib)
add_subdirectory(Source)
add_subdirectory(Source/Drivers/PS1080)
