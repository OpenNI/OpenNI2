cmake_minimum_required (VERSION 2.8)

project(KinectDriver)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../Include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../ThirdParty/PSCommon/XnLib/Include)
include_directories($ENV{KINECTSDK10_DIR}/inc)

link_directories($ENV{KINECTSDK10_DIR}/lib/x86)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/.. ${CMAKE_MODULE_PATH})

set (KINECT_DRIVER_SRC
    ${CMAKE_CURRENT_LIST_DIR}/BaseKinectStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ColorKinectStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DepthKinectStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IRKinectStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/KinectDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/KinectDriver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/KinectStreamImpl.cpp
    )

add_library(KinectDriver SHARED ${KINECT_DRIVER_SRC})

target_link_libraries(KinectDriver XnLib Kinect10.lib)
