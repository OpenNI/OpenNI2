cmake_minimum_required (VERSION 2.8)

project(libJPEG)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/.. ${CMAKE_MODULE_PATH})

include_directories(${CMAKE_CURRENT_LIST_DIR}/../PSCommon/XnLib/Include)

set (LIBJPEG_SRC
    ${CMAKE_CURRENT_LIST_DIR}/jcapimin.c
    ${CMAKE_CURRENT_LIST_DIR}/jcapistd.c
    ${CMAKE_CURRENT_LIST_DIR}/jccoefct.c
    ${CMAKE_CURRENT_LIST_DIR}/jccolor.c
    ${CMAKE_CURRENT_LIST_DIR}/jcdctmgr.c
    ${CMAKE_CURRENT_LIST_DIR}/jchuff.c
    ${CMAKE_CURRENT_LIST_DIR}/jcinit.c
    ${CMAKE_CURRENT_LIST_DIR}/jcmainct.c
    ${CMAKE_CURRENT_LIST_DIR}/jcmarker.c
    ${CMAKE_CURRENT_LIST_DIR}/jcmaster.c
    ${CMAKE_CURRENT_LIST_DIR}/jcomapi.c
    ${CMAKE_CURRENT_LIST_DIR}/jcparam.c
    ${CMAKE_CURRENT_LIST_DIR}/jcphuff.c
    ${CMAKE_CURRENT_LIST_DIR}/jcprepct.c
    ${CMAKE_CURRENT_LIST_DIR}/jcsample.c
    ${CMAKE_CURRENT_LIST_DIR}/jctrans.c
    ${CMAKE_CURRENT_LIST_DIR}/jdapimin.c
    ${CMAKE_CURRENT_LIST_DIR}/jdapistd.c
    ${CMAKE_CURRENT_LIST_DIR}/jdatadst.c
    ${CMAKE_CURRENT_LIST_DIR}/jdatasrc.c
    ${CMAKE_CURRENT_LIST_DIR}/jdcoefct.c
    ${CMAKE_CURRENT_LIST_DIR}/jdcolor.c
    ${CMAKE_CURRENT_LIST_DIR}/jddctmgr.c
    ${CMAKE_CURRENT_LIST_DIR}/jdhuff.c
    ${CMAKE_CURRENT_LIST_DIR}/jdinput.c
    ${CMAKE_CURRENT_LIST_DIR}/jdmainct.c
    ${CMAKE_CURRENT_LIST_DIR}/jdmarker.c
    ${CMAKE_CURRENT_LIST_DIR}/jdmaster.c
    ${CMAKE_CURRENT_LIST_DIR}/jdmerge.c
    ${CMAKE_CURRENT_LIST_DIR}/jdphuff.c
    ${CMAKE_CURRENT_LIST_DIR}/jdpostct.c
    ${CMAKE_CURRENT_LIST_DIR}/jdsample.c
    ${CMAKE_CURRENT_LIST_DIR}/jdtrans.c
    ${CMAKE_CURRENT_LIST_DIR}/jerror.c
    ${CMAKE_CURRENT_LIST_DIR}/jfdctflt.c
    ${CMAKE_CURRENT_LIST_DIR}/jfdctfst.c
    ${CMAKE_CURRENT_LIST_DIR}/jfdctint.c
    ${CMAKE_CURRENT_LIST_DIR}/jidctflt.c
    ${CMAKE_CURRENT_LIST_DIR}/jidctfst.c
    ${CMAKE_CURRENT_LIST_DIR}/jidctint.c
    ${CMAKE_CURRENT_LIST_DIR}/jidctred.c
    ${CMAKE_CURRENT_LIST_DIR}/jmemmgr.c
    ${CMAKE_CURRENT_LIST_DIR}/jmemnobs.c
    ${CMAKE_CURRENT_LIST_DIR}/jquant1.c
    ${CMAKE_CURRENT_LIST_DIR}/jquant2.c
    ${CMAKE_CURRENT_LIST_DIR}/jutils.c
    )

add_library(JPEG STATIC ${LIBJPEG_SRC})
