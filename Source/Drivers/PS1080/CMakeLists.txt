cmake_minimum_required (VERSION 2.8)

project(PS1080)

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../Include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../ThirdParty/PSCommon/XnLib/Include)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/.. ${CMAKE_MODULE_PATH})

set (PS1080_SRC
    ${CMAKE_CURRENT_LIST_DIR}/Core/XnBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/XnCore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/XnCoreStatus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/XnIOFileStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Core/XnIONetworkStream.cpp

    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnActualGeneralProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnActualIntProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnActualPropertiesHash.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnActualRealProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnActualStringProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnAudioStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnCodecFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDDK.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDDKStatus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDepthStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDepthStream.h
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDeviceBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDeviceModule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDeviceModuleHolder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnDeviceStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnFrameBufferManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnFrameStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnGeneralProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnImageStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnIntProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnIntPropertySynchronizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnIRStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnPixelStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnPropertySet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnRealProperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnShiftToDepth.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnShiftToDepthStreamHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnStreamingStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDK/XnStringProperty.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnExportedOniDriver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnOniColorStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnOniDepthStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnOniDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnOniDriver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnOniIRStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnOniMapStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DriverImpl/XnOniStream.cpp

    ${CMAKE_CURRENT_LIST_DIR}/Formats/XnCodecs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Formats/XnFormats.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Formats/XnFormatsMirror.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Formats/XnFormatsStatus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Formats/XnStreamCompression.cpp

    ${CMAKE_CURRENT_LIST_DIR}/Sensor/GMC.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/LinAlg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/Registration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnGMCProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnUncompressedYUVImageProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnUncompressedYUVtoRGBImageProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/Bayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/Uncomp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnAudioProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnBayerImageProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnCmosInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnDataProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnDataProcessorHolder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnDepthProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnDeviceEnumeration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnDeviceSensorInit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnDeviceSensorIO.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnDeviceSensorProtocol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnFirmwareCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnFirmwareInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnFirmwareStreams.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnFrameStreamProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnGeneralDebugProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnGMCDebugProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnHostProtocol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnImageProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnIRProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnJpegImageProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnJpegToRGBImageProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnNesaDebugProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnPacked11DepthProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnPacked12DepthProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnPSCompressedDepthProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnPSCompressedImageProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorAudioStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorDepthStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorFirmware.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorFirmwareParams.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorFixedParams.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorFPS.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorImageStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorIRStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnSensorStreamHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnStreamProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnTecDebugProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnUncompressedBayerProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnUncompressedDepthProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnWavelengthCorrectionDebugProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/XnWholePacketProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Sensor/YUV.cpp
    )

link_directories(${XNLIB_LIBRARY_DIRS})

add_library(PS1080 SHARED ${PS1080_SRC})

# This prevents libraries that link against libusbx from also
# depending on libusbx's dependencies.
set_target_properties(PS1080
      PROPERTIES
      LINK_INTERFACE_LIBRARIES ""
      # OpenNI expects the PS1080 DLL to be named "libPS1080.dylib", so the "d"
      # postfix will break things here.
      DEBUG_POSTFIX ""
  )

target_link_libraries(PS1080 JPEG XnLib)
