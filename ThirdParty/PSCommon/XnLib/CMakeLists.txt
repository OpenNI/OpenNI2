cmake_minimum_required (VERSION 2.8)

project(XnLib)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/.. ${CMAKE_MODULE_PATH})

include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

if(APPLE)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/Include/MacOSX)
elseif(WIN32)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/Include/Win32)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/Driver/Win32/Build/sys)
endif()

if(NOT WIN32)
    include(ExternalProject)

    ExternalProject_Add(
        openni2-libusb

        GIT_REPOSITORY "https://github.com/libusb/libusb.git"
        GIT_TAG "master"

        UPDATE_COMMAND  ${CMAKE_COMMAND} -E copy
                        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libusb/CMakeLists.txt
                        ${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/libusb/CMakeLists.txt
        PATCH_COMMAND ""

        SOURCE_DIR "ThirdParty/libusb/"
        CMAKE_ARGS -DCMAKE_CXX_STANDARD_LIBRARIES=${CMAKE_CXX_STANDARD_LIBRARIES}
                -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libusb_install

        TEST_COMMAND ""
    )

    include_directories(${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/libusb)

    set(XNLIB_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libusb_install/lib PARENT_SCOPE)
    link_directories(${XNLIB_LIBRARY_DIRS})
endif()

set (XNLIB_CORE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnErrorLogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnFPSCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnLib.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnOSMemoryProfiling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnProfiling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnScheduler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnStatus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnSytmmetricMatrix3x3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnUSB.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnVector3D.cpp
)

set (XNLIB_COMMON_OS_SRC
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnFiles.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnOS.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnStrings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnThreads.cpp
)

set (XNLIB_LOG_SRC
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnDump.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnDumpFileWriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnDumpFileWriter.h
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnLog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnLogConsoleWriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnLogConsoleWriter.h
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnLogFileWriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/XnLogFileWriter.h
    )

if(APPLE)
set (XNLIB_SRC_OS
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxCriticalSections.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxDebug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxEvents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxFiles.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxINI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxKeyboard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxMemory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxMutex.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxNetwork.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxPosixEvents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxPosixNamedEvents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxProcesses.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxSharedLibs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxSharedMemory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxStrings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxSysVNamedEvents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxThreads.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxTime.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxUSB.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Linux/XnLinuxUSBDevice.cpp
    )
elseif(WIN32)
set (XNLIB_SRC_OS
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32CriticalSection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnUSBWin32.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Events.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Files.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32INI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Internal.h
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Keyboard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Memory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Mutex.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Network.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32OS.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Processes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Semaphore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32SharedLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32SharedMemory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Strings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Threads.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/Win32/XnWin32Time.cpp
    )
endif()
add_library(XnLib STATIC ${XNLIB_CORE_SRC} ${XNLIB_LOG_SRC} ${XNLIB_COMMON_OS_SRC} ${XNLIB_SRC_OS})
if(WIN32)
    target_link_libraries(XnLib ws2_32.lib Setupapi.lib)
elseif(APPLE)
    target_link_libraries(XnLib
        usb
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif(WIN32)