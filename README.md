# RealSense Snap

RealSense Snap 是一个基于 Qt Quick、Intel RealSense、OpenCV 和 ONNX Runtime 的实时拍照应用。当前主要实现摄像头采集、深度图处理、前景分割、背景替换、照片保存，并提供独立 demo 用于验证视觉模型。

## 功能

- 枚举并选择 Intel RealSense 摄像头。
- 实时采集彩色流和深度流，并进行画面对齐。
- 使用 RealSense SDK 和 OpenCV 处理深度图。
- 基于深度信息进行前景分割和背景替换。
- 支持调节 RGB 与深度伪彩叠加强度。
- 支持保存当前合成画面。
- 使用 ONNX Runtime 接入 YOLO 分割模型，并提供 demo 测试入口。

## 项目结构

```text
RealSense_Snap/
├── CMakeLists.txt
├── README.md
├── docs/                  # 需求与开发文档
├── qml/                   # Qt Quick 界面
├── resources/
│   ├── images/            # 默认背景图
│   └── models/            # 视觉模型文件
├── src/
│   ├── main.cpp
│   ├── background/        # 背景图片提供与缓存
│   ├── core/              # 相机采集、控制器、视频显示组件
│   ├── processing/        # 深度滤波与前景 mask 处理
│   └── utils/             # YOLO 分割等工具类
├── demos/
│   └── vision/            # 视觉模型 demo
└── importedcontent/       # Qt/Figma 导入内容占位
```

## 关键文件

- `src/core/CameraWorker.*`：RealSense 采集、帧处理、背景替换和拍照保存。
- `src/core/cameracontroller.*`：QML 与 C++ 后端之间的控制桥。
- `src/processing/filterprocessing.*`：深度图滤波、聚类和 mask 后处理。
- `src/background/imagebackgroundprovider.*`：背景图片加载与尺寸缓存。
- `src/utils/yoloseg.*`：YOLO 分割模型封装。
- `demos/vision/visionDemo.cpp`：视觉模型独立测试入口。
- `resources/models/yolo26n-seg.onnx`：当前使用的 ONNX 模型。

## 构建

项目默认从下面位置读取第三方依赖：

```text
~/Documents/Packages/librealsense
~/Documents/Packages/opencv/build
~/Documents/Packages/onnxruntime-osx-arm64-1.26.0
```

如果依赖路径不同，可以在配置时指定：

```bash
cmake -S . -B build/Qt_6_11_1_for_macOS_Debug \
  -DREALSENSE_ROOT=/path/to/librealsense \
  -DOpenCV_DIR=/path/to/opencv/build \
  -DONNXRUNTIME_ROOT=/path/to/onnxruntime

cmake --build build/Qt_6_11_1_for_macOS_Debug
```

## 使用

1. 连接 Intel RealSense 摄像头。
2. 启动应用。
3. 点击“刷新”加载设备列表。
4. 选择摄像头。
5. 可选：切换背景图片或调节深度叠加强度。
6. 点击“拍照”保存当前画面。

照片默认保存到：

```text
resources/photos/
```

视觉模型文件放在：

```text
resources/models/
```
