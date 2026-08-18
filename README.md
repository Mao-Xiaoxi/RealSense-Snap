# RealSense-Snap

## 任务

任务1：自由切换背景的照相软件
描述：借助RealSense摄像头，人站在摄像头前面拍照，可以自由切换人后面的背景，人的轮廓要清晰平滑

任务2：眉心与体温识别
描述：可以根据人的眼睛位置，使用摄像头，实时识别人的眉心位置，并检测出当前人的体温

## 文件结构

```bash
RealSense_Snap/                         # 项目根目录
├── CMakeLists.txt                      # CMake 构建配置
├── README.md                           # 项目说明
├── docs/                               # 项目文档
│   ├── RealSense Snap 需求文档.md
│   └── RealSense开发.md
├── importedcontent/                    # Qt Creator 导入内容
│   └── README.md
├── qml/                                # QML 前端界面
│   └── Main.qml                        # 主界面
├── resources/                          # 静态资源
│   └── images/
│       └── background001.jpeg          # 默认背景图片
└── src/                                # C++ 源码
    ├── main.cpp                        # 程序入口与对象连接
    └── core/                           # 核心功能模块
        ├── BackgroundProvider.h        # 背景提供接口
        ├── imagebackgroundprovider.h   # 图片背景提供类声明
        ├── imagebackgroundprovider.cpp # 图片背景提供类实现
        ├── CameraWorker.h              # RealSense 采集与图像处理声明
        ├── CameraWorker.cpp            # RealSense 采集与图像处理实现
        ├── cameracontroller.h          # 相机控制器声明
        ├── cameracontroller.cpp        # 相机控制器实现
        ├── videoitem.h                 # QML 视频绘制项声明
        └── videoitem.cpp               # QML 视频绘制项实现
```
