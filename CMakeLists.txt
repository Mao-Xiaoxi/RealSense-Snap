cmake_minimum_required(VERSION 3.16)

project(RealSense_Snap VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)   # Qt 标准设置，通常 qt_standard_project_setup 会默认开启

include(GNUInstallDirs)

set(THIRD_PARTY_ROOT "$ENV{HOME}/Documents/Packages" CACHE PATH "Third-party package root")
set(REALSENSE_ROOT "${THIRD_PARTY_ROOT}/librealsense" CACHE PATH "librealsense source/install root")
set(REALSENSE_INCLUDE_DIR "${REALSENSE_ROOT}/include" CACHE PATH "librealsense include directory")
set(REALSENSE_RUNTIME_DIR "${REALSENSE_ROOT}/build/Release" CACHE PATH "librealsense runtime library directory")

set(_default_opencv_dir "${THIRD_PARTY_ROOT}/opencv/build")
if(NOT OpenCV_DIR OR OpenCV_DIR MATCHES "-NOTFOUND$")
    set(OpenCV_DIR "${_default_opencv_dir}" CACHE PATH "OpenCV CMake package directory" FORCE)
endif()

set(ONNXRUNTIME_ROOT "${THIRD_PARTY_ROOT}/onnxruntime-osx-arm64-1.26.0" CACHE PATH "ONNX Runtime root directory")

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(OpenCV REQUIRED)
find_library(REALSENSE_LIB NAMES realsense2
    PATHS "${REALSENSE_RUNTIME_DIR}"
    NO_DEFAULT_PATH
)

add_library(onnxruntime SHARED IMPORTED)
set_target_properties(onnxruntime PROPERTIES
    IMPORTED_LOCATION "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_ROOT}/include"
)

if(NOT REALSENSE_LIB)
    message(FATAL_ERROR "Could not find librealsense2 library. Set REALSENSE_RUNTIME_DIR to the directory containing librealsense2.dylib.")
endif()

if(NOT EXISTS "${REALSENSE_INCLUDE_DIR}/librealsense2/rs.hpp")
    message(FATAL_ERROR "Could not find librealsense headers. Set REALSENSE_INCLUDE_DIR to the directory containing librealsense2/rs.hpp.")
endif()

if(NOT EXISTS "${ONNXRUNTIME_ROOT}/include/onnxruntime_cxx_api.h")
    message(FATAL_ERROR "Could not find ONNX Runtime headers. Set ONNXRUNTIME_ROOT to a valid ONNX Runtime package.")
endif()

if(NOT EXISTS "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.dylib")
    message(FATAL_ERROR "Could not find ONNX Runtime library. Set ONNXRUNTIME_ROOT to a valid ONNX Runtime package.")
endif()

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appRealSense_Snap)

qt_add_qml_module(appRealSense_Snap
    URI RealSense_Snap
    QML_FILES
        qml/Main.qml
        SOURCES src/core/videoitem.h src/core/videoitem.cpp
        SOURCES src/core/CameraWorker.h src/core/CameraWorker.cpp
        SOURCES src/core/cameracontroller.h src/core/cameracontroller.cpp
        SOURCES src/background/imagebackgroundprovider.h src/background/imagebackgroundprovider.cpp
        SOURCES src/background/BackgroundProvider.h
        SOURCES src/processing/filterprocessing.h src/processing/filterprocessing.cpp
        SOURCES src/utils/yoloseg.h src/utils/yoloseg.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appRealSense_Snap PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appRealSense_Snap
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE

    BUILD_RPATH "${REALSENSE_RUNTIME_DIR};${ONNXRUNTIME_ROOT}/lib"          # 编译目录下运行时找这里
    INSTALL_RPATH "@executable_path/../lib"
)

target_include_directories(appRealSense_Snap PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/background"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/processing"
    "${REALSENSE_INCLUDE_DIR}"
    ${OpenCV_INCLUDE_DIRS}
)

target_sources(appRealSense_Snap PRIVATE
    src/main.cpp
    src/core/CameraWorker.h
    src/core/CameraWorker.cpp
    src/core/videoitem.h
    src/core/videoitem.cpp
    # 注意：.h 文件不需要在这里列出，但为了 IDE 方便，可以加到工程里
)

target_link_libraries(appRealSense_Snap
    PRIVATE Qt6::Quick
    "${REALSENSE_LIB}"
    ${OpenCV_LIBS}
    onnxruntime
)

# OpenMP
option(REALSENSE_SNAP_ENABLE_OPENMP "Enable OpenMP acceleration" ON)

if(REALSENSE_SNAP_ENABLE_OPENMP)
    if(APPLE AND NOT OpenMP_ROOT AND EXISTS "/opt/homebrew/opt/libomp")
        set(OpenMP_ROOT "/opt/homebrew/opt/libomp" CACHE PATH "OpenMP root directory")
    endif()

    find_package(OpenMP QUIET)

    if(OpenMP_CXX_FOUND)
        target_link_libraries(appRealSense_Snap PRIVATE OpenMP::OpenMP_CXX)
        target_compile_definitions(appRealSense_Snap PRIVATE REALSENSE_SNAP_HAS_OPENMP=1)
        message(STATUS "OpenMP enabled")
    else()
        message(WARNING "OpenMP requested but not found. Build will continue without OpenMP.")
    endif()
endif()

# 资源路径
target_compile_definitions(appRealSense_Snap PRIVATE
    REALSENSE_SNAP_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

install(TARGETS appRealSense_Snap
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
