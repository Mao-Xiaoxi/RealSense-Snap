# RealSense Snap

RealSense Snap 是一个基于 Qt Quick、Intel RealSense、OpenCV 和 ONNX Runtime 的实时拍照应用。项目目前支持 RealSense 摄像头采集、深度图处理、人像分割、背景替换、深度伪彩叠加、照片保存，并提供独立 demo 用于验证视觉模型。

## 功能

- 枚举并选择 Intel RealSense 摄像头。
- 实时采集彩色流和深度流，并将深度帧对齐到彩色帧。
- 使用 RealSense SDK 和 OpenCV 对深度图进行滤波、阈值和形态学处理。
- 使用 YOLO segmentation 模型生成前景人像 mask。
- 基于深度和 mask 进行背景替换，并支持切换本地背景图片。
- 支持调节 RGB 画面与深度伪彩图的叠加强度。
- 支持保存当前合成画面到本地。
- 使用 YOLOv8 face ONNX 模型检测人脸框和 5 个面部关键点。
- 提供 `demos/face` 和 `demos/vision` 独立测试入口。

## 项目结构

```text
RealSense_Snap/
├── CMakeLists.txt
├── README.md
├── docs/                         # 需求与开发文档
├── qml/                          # Qt Quick 前端界面
│   └── Main.qml
├── resources/
│   ├── images/                   # 默认背景图
│   └── models/                   # ONNX 模型文件
├── src/
│   ├── main.cpp                  # 应用入口，初始化 QML、Worker 线程和信号连接
│   ├── background/               # 背景图片加载与尺寸缓存
│   ├── core/                     # CameraWorker、Controller、VideoItem
│   ├── processing/               # 深度滤波与前景 mask 处理
│   └── utils/                    # YOLO 分割和人脸关键点工具类
├── demos/
│   ├── face/                     # YOLOv8 face 关键点 demo
│   └── vision/                   # YOLO segmentation demo
└── importedcontent/              # Qt/Figma 导入内容占位
```

## 关键模块

- `src/core/CameraWorker.*`：核心 worker，负责 RealSense 采集、帧处理调度、背景替换、预览输出和拍照保存。
- `src/core/cameracontroller.*`：QML 和 worker 之间的控制桥，保存 UI 绑定状态并转发用户请求。
- `src/core/videoitem.*`：QML 中的视频显示组件。
- `src/processing/filterprocessing.*`：RealSense SDK 滤波、OpenCV 深度图后处理、异常闪烁检测。
- `src/background/imagebackgroundprovider.*`：背景图片加载、缓存和按视频尺寸缩放。
- `src/utils/yoloseg.*`：YOLO segmentation ONNX Runtime 推理封装。
- `src/utils/faciallandmark.*`：YOLOv8 face OpenCV DNN 推理封装，输出人脸框和 5 个关键点。

## 模型文件

当前模型放在：

```text
resources/models/
├── yolo26n-seg.onnx       # 人像分割模型
└── yolov8n-face.onnx      # 人脸检测和 5 点关键点模型
```

主工程通过 `REALSENSE_SNAP_RESOURCE_DIR` 编译宏定位 `resources` 目录。独立 demo 也会注入同样的资源路径宏，因此从 build 目录运行时也能找到模型。

## 依赖

项目默认从下面位置读取第三方依赖：

```text
~/Documents/Packages/librealsense
~/Documents/Packages/opencv/build
~/Documents/Packages/onnxruntime-osx-arm64-1.26.0
```

主程序依赖：

- Qt 6.10 或更高版本，当前构建目录使用 Qt 6.11.1
- Intel RealSense SDK / librealsense2
- OpenCV
- ONNX Runtime
- 可选 OpenMP，用于部分 OpenCV 像素循环加速

`demos/face` 只依赖 OpenCV，因为人脸关键点模型通过 OpenCV DNN 加载。

## 构建主程序

如果依赖路径使用默认布局：

```bash
cmake -S . -B build/Qt_6_11_1_for_macOS_Debug
cmake --build build/Qt_6_11_1_for_macOS_Debug
```

如果依赖路径不同，可以手动指定：

```bash
cmake -S . -B build/Qt_6_11_1_for_macOS_Debug \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/macos \
  -DREALSENSE_ROOT=/path/to/librealsense \
  -DREALSENSE_RUNTIME_DIR=/path/to/librealsense/build/Release \
  -DOpenCV_DIR=/path/to/opencv/build \
  -DONNXRUNTIME_ROOT=/path/to/onnxruntime

cmake --build build/Qt_6_11_1_for_macOS_Debug
```

运行：

```bash
./build/Qt_6_11_1_for_macOS_Debug/appRealSense_Snap.app/Contents/MacOS/appRealSense_Snap
```

## 使用主程序

1. 连接 Intel RealSense 摄像头。
2. 启动应用。
3. 点击“刷新”加载设备列表。
4. 在下拉框中选择可用摄像头。
5. 调节“深度叠加强度”观察 RGB 和深度伪彩混合效果。
6. 可选：点击“切换背景”选择新的背景图片。
7. 点击“拍照”保存当前合成画面。

照片默认保存到：

```text
resources/photos/
```

## Face Demo

`demos/face` 用于单独验证 `facialLandmark`，支持摄像头和单张图片。

构建：

```bash
cmake -S demos/face -B demos/face/build
cmake --build demos/face/build
```

运行默认摄像头：

```bash
demos/face/build/bin/faceDemo
```

指定摄像头：

```bash
demos/face/build/bin/faceDemo --camera 0
```

检测单张图片：

```bash
demos/face/build/bin/faceDemo --image path/to/input.jpg
```

检测并保存结果图：

```bash
demos/face/build/bin/faceDemo --image path/to/input.jpg --save path/to/output.jpg
```

指定模型路径：

```bash
demos/face/build/bin/faceDemo --model resources/models/yolov8n-face.onnx
```

## Vision Demo

`demos/vision` 用于验证 YOLO segmentation 模型。更多说明见：

```text
demos/vision/README.md
```

## 常见问题

### QML root object 加载失败

如果看到：

```text
QQmlApplicationEngine failed to load component
Failed to load QML root object.
```

优先查看前一行 QML 报错。常见原因是 QML 语法错误、属性重复赋值、模块导入失败或自定义组件未注册。

### RealSense failed to set power state

如果启动摄像头时报：

```text
RealSense error: failed to set power state
```

通常与 RealSense 设备、USB 供电/带宽、macOS USB 权限、设备被占用或 SDK 自动选择的流配置有关。建议：

- 先用 `realsense-viewer` 或 `rs-enumerate-devices` 验证设备是否能被官方工具打开。
- 尽量直连电脑 USB 口，避免扩展坞。
- 确认没有其他程序占用 RealSense。
- 优先使用较低分辨率和明确格式，例如 `640x480 30fps`。
- 避免在 pipeline 未成功 start 时继续调用 `try_wait_for_frames()`。

### Qt6Config.cmake 找不到

如果 CMake 报：

```text
Could not find Qt6Config.cmake
```

需要指定 Qt 安装目录：

```bash
cmake -S . -B build/Qt_6_11_1_for_macOS_Debug \
  -DCMAKE_PREFIX_PATH=/Users/maoxiaoxi/Qt/6.11.1/macos
```

## 当前工程状态

这个项目目前属于可运行原型阶段：功能链路已经打通，核心模块已经开始拆分，但 `CameraWorker` 仍承担较多职责。后续可以逐步将设备采集、帧转换、背景合成、预览渲染、照片保存和 AI 推理调度拆成更小的模块，让主 worker 更专注于线程内调度。
