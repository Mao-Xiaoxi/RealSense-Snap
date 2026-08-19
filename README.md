# RealSense-Snap

## 任务

任务1：自由切换背景的照相软件
描述：借助RealSense摄像头，人站在摄像头前面拍照，可以自由切换人后面的背景，人的轮廓要清晰平滑

任务2：眉心与体温识别
描述：可以根据人的眼睛位置，使用摄像头，实时识别人的眉心位置，并检测出当前人的体温

## 开发进度

- 已完成 Qt Quick 主界面搭建，包含实时画面显示区、摄像头选择、设备刷新、深度叠加强度调节和明暗模式切换。
- 已完成 RealSense 采集线程框架，支持设备枚举、指定序列号切换相机、深度流和彩色流采集，以及异常状态回传。
- 已完成 `VideoItem` 自定义 QML 绘制组件，用于接收 C++ 传入的 `QImage` 并在界面中按比例显示。
- 已实现彩色图与深度伪彩的实时叠加预览，叠加强度可通过前端滑块调节。
- 已初步搭建背景替换模块，包含 `BackgroundProvider` 抽象接口和 `ImageBackgroundProvider` 图片背景实现，当前可加载默认背景图并按视频尺寸缓存缩放结果。
- 已在 `CameraWorker` 中接入基于深度阈值的背景替换雏形，当前仍需继续优化人物边缘、阈值调节和前端背景切换流程。
- 眉心定位与体温识别功能尚未开始实现。

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
