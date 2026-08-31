cmake_minimum_required(VERSION 3.16)

project(face_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(REALSENSE_SNAP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(THIRD_PARTY_ROOT "$ENV{HOME}/Documents/Packages" CACHE PATH "Third-party package root")
set(_default_opencv_dir "${THIRD_PARTY_ROOT}/opencv/build")
if(NOT OpenCV_DIR OR OpenCV_DIR MATCHES "-NOTFOUND$")
    set(OpenCV_DIR "${_default_opencv_dir}" CACHE PATH "OpenCV CMake package directory" FORCE)
endif()

find_package(OpenCV REQUIRED)

add_executable(faceDemo
    main.cpp
    "${REALSENSE_SNAP_ROOT}/src/utils/faciallandmark.cpp"
)

target_include_directories(faceDemo PRIVATE
    "${REALSENSE_SNAP_ROOT}/src/utils"
    ${OpenCV_INCLUDE_DIRS}
)

target_compile_definitions(faceDemo PRIVATE
    REALSENSE_SNAP_RESOURCE_DIR="${REALSENSE_SNAP_ROOT}/resources"
)

target_link_libraries(faceDemo PRIVATE
    ${OpenCV_LIBS}
)

set_target_properties(faceDemo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
