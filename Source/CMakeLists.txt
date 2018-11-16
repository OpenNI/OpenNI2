cmake_minimum_required (VERSION 2.8)

project(OpenNI2Core)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../Include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/PSCommon/XnLib/Include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/LibJPEG)
include_directories(${CMAKE_CURRENT_LIST_DIR}/Drivers/OniFile/Formats)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/.. ${CMAKE_MODULE_PATH})

set (CORE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniContext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniDataRecords.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniDeviceDriver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniDriverHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniRecorder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniStreamFrameHolder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OniSyncedStreamsFrameHolder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/OpenNI.cpp
    )

set (ONI_FILE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/OniFile/Formats/XnStreamCompression.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Drivers/OniFile/Formats/XnCodec.cpp
)

link_directories(${XNLIB_LIBRARY_DIRS})

add_library(OpenNI2 SHARED ${CORE_SRC} ${ONI_FILE_SRC})

# This prevents libraries that link against libusbx from also
# depending on libusbx's dependencies.
if(APPLE)
    set_target_properties(OpenNI2
        PROPERTIES
        LINK_INTERFACE_LIBRARIES ""
  )
endif(APPLE)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPENNI2_EXPORT")

target_link_libraries(OpenNI2 JPEG XnLib)

add_custom_command(TARGET OpenNI2 POST_BUILD
                   COMMAND ${CMAKE_COMMAND} ARGS -E copy 
                   "${CMAKE_CURRENT_SOURCE_DIR}/../Config/OpenNI.ini" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")
                                      
