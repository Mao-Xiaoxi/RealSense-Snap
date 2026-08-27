cmake_minimum_required(VERSION 3.16)
project(face_demo CXX)

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
    INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_ROOT}/include"   # 自动传递头文件路径
)

add_executable(faceDemo
    main.cpp  
    ../../src/utils/faciallandmark.cpp
)

# 运行时动态库搜索路径
set_target_properties(faceDemo PROPERTIES
    BUILD_RPATH "${ONNXRUNTIME_ROOT}/lib"
    INSTALL_RPATH "@executable_path/../lib"  
)

# 头文件路径（OpenCV 和你的项目内部头文件）
target_include_directories(faceDemo PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ../../src/utils
)

target_link_libraries(faceDemo PRIVATE
    ${OpenCV_LIBS}
    onnxruntime
)