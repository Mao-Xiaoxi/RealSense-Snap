cmake_minimum_required(VERSION 3.16)

project(vision_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(_default_opencv_dir "$ENV{HOME}/Documents/Packages/opencv/build")
if(NOT OpenCV_DIR OR OpenCV_DIR MATCHES "-NOTFOUND$")
    set(OpenCV_DIR "${_default_opencv_dir}" CACHE PATH "OpenCV CMake package directory" FORCE)
endif()
find_package(OpenCV REQUIRED)

set(ONNXRUNTIME_ROOT "/Users/maoxiaoxi/Documents/Packages/onnxruntime-osx-arm64-1.26.0" CACHE PATH "ONNX Runtime root directory")
add_library(onnxruntime SHARED IMPORTED)
set_target_properties(onnxruntime PROPERTIES
    IMPORTED_LOCATION "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_ROOT}/include"
)


add_executable(visionDemo 
    visionDemo.cpp
    ../../src/utils/yoloseg.cpp
)
set_target_properties(visionDemo PROPERTIES
    BUILD_RPATH "${ONNXRUNTIME_ROOT}/lib"
)
target_include_directories(visionDemo 
    PRIVATE 
        ${OpenCV_INCLUDE_DIRS}
        ../../src/utils
)
target_link_libraries(visionDemo PRIVATE ${OpenCV_LIBS} onnxruntime)
