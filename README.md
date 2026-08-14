# RealSense-Snap

## 任务

任务1：自由切换背景的照相软件
描述：借助RealSense摄像头，人站在摄像头前面拍照，可以自由切换人后面的背景，人的轮廓要清晰平滑

任务2：眉心与体温识别
描述：可以根据人的眼睛位置，使用摄像头，实时识别人的眉心位置，并检测出当前人的体温

## 文件结构

```bash
RealSense_Snap/                           # 项目根目录
├── CMakeLists.txt                      
├── README.md                          
├── .gitignore                          
│
├── src/                          
│   ├── main.cpp                          # 程序入口
│   ├── core/                             # 核心功能模块（RealSense驱动、图像处理）
│   │   ├── CameraWorker.h
│   │   ├── CameraWoker.cpp
│   │   ├── CameraController.h
│   │   ├── CameraController.cpp
│   │   ├── videoitem.h
│   │   └── videoitem.cpp
│   ├── models/                           # 数据模型（结构体、枚举、数据传输对象）
│   └── utils/                            # 工具类（日志、数学、文件操作）
│
├── include/                          
│   └── app_export.h                      # 用于 DLL/动态库导出的宏定义
│
├── qml/                                  # QML 前端界面文件（全部放在此处）
│   └── Main.qml                          # 主界面
│
├── resources/                     
│   ├── images/
│   │   ├── logo.png
│   │   └── icon.svg
│   ├── fonts/
│   │   └── NotoSans.ttf
│   └── qml.qrc                   
│
├── importedcontent/             
│   └── CMakeLists.txt
│
└── build/                          
    ├── Qt_6_11_1_for_macOS_Debug/
    └── Release/
```

