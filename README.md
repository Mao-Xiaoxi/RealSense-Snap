# RealSense Snap

RealSense Snap 是一个基于 Qt Quick、Intel RealSense 和 OpenCV 的实时拍照应用。项目当前聚焦于 RealSense 摄像头画面采集、深度图处理、前景分割、背景替换和照片保存。

## 当前功能

- 枚举并选择可用的 Intel RealSense 摄像头。
- 实时采集彩色流和深度流，并将深度帧对齐到彩色帧。
- 使用 RealSense 滤波器和 OpenCV 双边滤波处理深度图。
- 使用 K-means 根据深度自动生成前景 mask。
- 将前景与图片背景合成，实现实时背景替换。
- 支持选择本地图片作为背景。
- 支持调节 RGB 画面与深度伪彩画面的叠加强度。
- 支持将当前合成画面保存为 PNG 图片。

## 技术栈

- C++17
- Qt 6 / Qt Quick / QML
- Intel RealSense SDK 2.0
- OpenCV
- CMake

## 项目结构

```text
RealSense_Snap/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── RealSense Snap 需求文档.md
│   └── RealSense开发.md
├── qml/
│   └── Main.qml
├── resources/
│   ├── images/
│   │   ├── background001.jpeg
│   │   └── background002.jpeg
│   └── photos/
├── src/
│   ├── main.cpp
│   ├── background/
│   │   ├── BackgroundProvider.h
│   │   ├── imagebackgroundprovider.h
│   │   └── imagebackgroundprovider.cpp
│   ├── core/
│   │   ├── CameraWorker.h
│   │   ├── CameraWorker.cpp
│   │   ├── cameracontroller.h
│   │   ├── cameracontroller.cpp
│   │   ├── videoitem.h
│   │   └── videoitem.cpp
│   └── processing/
│       ├── filterprocessing.h
│       └── filterprocessing.cpp
└── importedcontent/
    └── README.md
```

## 核心模块

- `CameraWorker`：运行在工作线程中，负责 RealSense 采集、帧处理、背景替换和拍照保存。
- `CameraController`：作为 QML 与 C++ 后端之间的控制桥，处理设备刷新、相机选择、背景切换和拍照请求。
- `VideoItem`：自定义 QML 绘制组件，用于显示 C++ 传入的实时 `QImage`。
- `FilterProcessing`：负责 RealSense 深度滤波、OpenCV 深度平滑、K-means 前景 mask 生成和形态学处理。
- `ImageBackgroundProvider`：加载本地背景图片，并按当前视频尺寸缓存缩放后的背景。

## 深度帧处理流程

程序对连续视频深度帧的处理主要发生在 `CameraWorker::processFrame()` 和 `FilterProcessing` 中。每次收到一组 RealSense 帧后，会按照下面的流程处理：

1. 获取一组 RealSense 帧，包括彩色帧和深度帧。
2. 将深度帧对齐到彩色帧坐标系，保证后续 mask 可以和 RGB 图像对应。
3. 对深度帧执行 RealSense SDK 滤波：
   - `spatial_filter`：空间滤波，降低局部深度噪声。
   - `temporal_filter`：时间滤波，利用前后帧关系减少深度闪烁。
   - `hole_filling_filter`：对部分深度空洞进行填补。
4. 将 RealSense 深度帧转换为 OpenCV `cv::Mat`。
5. 将深度图转换为 `CV_32FC1`，方便后续滤波和聚类计算。
6. 使用 OpenCV 双边滤波平滑深度图，在降低噪声的同时尽量保留前景边缘。
7. 清除无效深度值，将深度小于等于 0 的像素重新置为无效区域。
8. 使用 K-means 对有效深度像素聚成两类：
   - 深度中心较小的一类表示离相机更近，作为前景候选。
   - 深度中心较大的一类表示更远区域，作为背景候选。
9. 根据聚类结果生成 `CV_8UC1` 前景 mask，前景为 `255`，背景为 `0`。
10. 对 mask 做形态学处理：
    - 闭运算用于填补前景内部的小孔洞。
    - 腐蚀用于收缩边缘，减少深度边界外溢。
11. 使用处理后的深度结果和背景图片进行背景替换。
12. 将合成后的 RGB 图像与深度伪彩图按滑块设置的透明度叠加，输出到 QML 界面。
13. 如果触发拍照，则将当前合成画面保存到 `resources/photos/`。

整体流程可以概括为：

```text
RealSense frames
  -> depth/color alignment
  -> RealSense spatial/temporal/hole-filling filters
  -> cv::Mat depth conversion
  -> OpenCV bilateral smoothing
  -> invalid depth cleanup
  -> K-means foreground/background clustering
  -> morphology cleanup
  -> background replacement
  -> RGB/depth overlay preview
  -> optional photo saving
```

## 构建说明

项目默认从用户目录下读取第三方依赖：

```text
~/Documents/Packages/librealsense
~/Documents/Packages/opencv/build
```

如依赖路径不同，可以在 CMake 配置时指定：

```bash
cmake -S . -B build/Qt_6_11_1_for_macOS_Debug \
  -DREALSENSE_ROOT=/path/to/librealsense \
  -DOpenCV_DIR=/path/to/opencv/build

cmake --build build/Qt_6_11_1_for_macOS_Debug
```

## 使用说明

1. 连接 Intel RealSense 摄像头。
2. 启动应用。
3. 点击“刷新”加载设备列表。
4. 在摄像头下拉框中选择设备。
5. 点击“切换背景”选择本地背景图片。
6. 调节“深度叠加强度”查看 RGB 与深度伪彩叠加效果。
7. 点击“拍照”保存当前画面。

照片默认保存到：

```text
resources/photos/
```

## 当前状态

项目已经具备实时预览、背景替换和拍照保存的基础流程。当前前景分割主要依赖深度图 K-means 聚类与深度范围约束，复杂边缘、遮挡、孔洞和多层背景场景仍需要继续优化。
