cmake_minimum_required(VERSION 3.16)

project(RealSense_Snap VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)   # Qt 标准设置，通常 qt_standard_project_setup 会默认开启

set(REALSENSE_ROOT "/Users/maoxiaoxi/Documents/Packages/librealsense")
set(REALSENSE_INCLUDE_DIR "${REALSENSE_ROOT}/include")
set(REALSENSE_BUILD_DIR "${REALSENSE_ROOT}/build/Release")

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(OpenCV REQUIRED)

find_library(REALSENSE_LIB NAMES realsense2
    PATHS "${REALSENSE_BUILD_DIR}"
    NO_DEFAULT_PATH
)

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

    BUILD_RPATH "${REALSENSE_BUILD_DIR}"          # 编译目录下运行时找这里
    INSTALL_RPATH "@executable_path/../lib"
)

target_include_directories(appRealSense_Snap PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
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
)

install(TARGETS appRealSense_Snap
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
