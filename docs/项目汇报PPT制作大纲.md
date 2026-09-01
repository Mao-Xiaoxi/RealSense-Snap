# RealSense Snap 项目汇报 PPT 制作大纲

## 汇报定位

本 PPT 面向公司答辩、项目验收或技术展示场景，目标是在 10-15 分钟内说明 RealSense Snap 的项目价值、系统架构、程序处理流程和关键技术实现。

建议页数：12-15 页。

建议风格：科技感、简洁、专业。主色可使用深灰、白色、蓝色，视觉元素以流程图、模块架构图、输入输出对比图为主，避免堆叠大段代码。

核心表达：

> RealSense Snap 通过 Qt Quick、Intel RealSense、OpenCV、ONNX Runtime 和 YOLO 模型，实现了从深度相机采集到 AI 分割、背景替换、深度叠加和拍照保存的完整实时图像处理链路。

## 第 1 页：标题页

标题：RealSense Snap 实时深度相机拍照系统

副标题：基于 Qt Quick、Intel RealSense、OpenCV 与 ONNX 模型的实时图像处理应用

建议内容：

- 项目名称
- 汇报人
- 日期
- 技术关键词：RealSense / Qt Quick / OpenCV / ONNX / YOLO / Background Replacement

视觉建议：

- 使用应用界面截图作为背景或右侧视觉主体。
- 标题放大，保持干净，不要堆太多文字。

## 第 2 页：项目背景与目标

标题：项目目标是把深度相机能力转化为可交互的实时拍照体验

讲述重点：

- 普通 RGB 摄像头只能获得二维图像，难以稳定区分前景和背景。
- RealSense 深度相机可以同时提供彩色图和深度图，为背景替换、前景识别、深度可视化提供基础。
- 项目目标是构建一个桌面端实时拍照应用，完成采集、处理、预览和保存的闭环。

建议内容：

- 实时采集 RealSense 彩色流和深度流。
- 对深度图进行滤波和前景提取。
- 结合 YOLO 人像分割提升前景判断稳定性。
- 支持背景替换、深度叠加、拍照保存。
- 扩展 YOLOv8 face 面部关键点检测能力。

视觉建议：

- 左侧放“问题”：背景复杂、边缘不稳定、传统 RGB 难分离。
- 右侧放“目标”：深度辅助、AI 分割、实时交互、照片输出。

## 第 3 页：功能概览

标题：系统已经打通从设备连接到图像输出的完整功能链路

讲述重点：

- 说明项目不是单一算法 demo，而是一个包含 UI、设备管理、图像处理和模型推理的完整应用。

建议内容：

- 摄像头设备刷新与选择。
- 彩色帧、深度帧实时采集。
- 深度图滤波与前景深度提取。
- YOLO 人像分割。
- 背景图片替换。
- RGB 与深度伪彩叠加。
- 实时预览显示。
- 拍照保存。
- 人脸框和 5 点面部关键点检测。

视觉建议：

- 使用一张横向功能链路图：
  `设备选择 -> 帧采集 -> 图像处理 -> AI 推理 -> 合成预览 -> 保存输出`

## 第 4 页：系统总体架构

标题：项目采用 Qt 前端、后台 Worker 和视觉工具类分层协作

讲述重点：

- 前端负责交互，后端负责采集和处理。
- `CameraWorker` 运行在独立线程，避免阻塞 UI。
- 图像处理能力被拆分到多个工具类中，便于独立维护和测试。

建议内容：

- QML 前端：`Main.qml`
- 控制桥：`CameraController`
- 后台工作对象：`CameraWorker`
- 视频显示：`VideoItem`
- 深度处理：`FilterProcessing`
- 背景管理：`ImageBackgroundProvider`
- 人像分割：`yoloSeg`
- 面部关键点：`facialLandmark`

视觉建议：

- 做三层架构图：
  - UI 层：QML / VideoItem
  - 调度层：CameraController / CameraWorker
  - 算法层：FilterProcessing / yoloSeg / facialLandmark / BackgroundProvider

## 第 5 页：核心程序流程

标题：每一帧图像都会经过采集、对齐、滤波、分割、合成和显示

讲述重点：

- 这一页是技术汇报重点，要把 `CameraWorker::processFrame()` 的逻辑讲清楚。

建议流程：

1. `QTimer` 周期触发 `processFrame()`。
2. RealSense pipeline 获取一组 frameset。
3. 将 depth frame 对齐到 color frame。
4. 提取深度帧和彩色帧。
5. 将 RealSense color frame 转换为 OpenCV BGR 图像。
6. 对深度帧执行 RealSense SDK 滤波和 OpenCV 后处理。
7. 隔帧执行 YOLO 人像分割，生成 person mask。
8. 使用 mask 和深度范围清理背景深度。
9. 根据深度前景区域进行背景替换和边缘羽化。
10. 使用 RealSense colorizer 生成深度伪彩图。
11. 按用户设置的 alpha 混合 RGB 画面和深度伪彩图。
12. 转换为 `QImage`，通过信号发送给 UI 显示。
13. 如果用户请求拍照，则保存当前画面。

视觉建议：

- 使用主流程图，建议横向排布。
- 用不同颜色区分：设备输入、图像处理、AI 推理、输出显示。

## 第 6 页：RealSense 采集与线程模型

标题：相机采集被放入 Worker 线程以保证界面响应

讲述重点：

- Qt GUI 线程负责 QML 渲染和用户交互。
- `CameraWorker` 被 `moveToThread()` 移到后台线程。
- 前端通过信号槽请求刷新设备、选择相机、切换背景和拍照。
- Worker 处理完成后通过 `frameReady(QImage)` 通知 UI 更新画面。

建议内容：

- `main.cpp` 中创建 `QThread workerThread`。
- `CameraWorker` 负责启动 RealSense pipeline。
- `CameraController` 作为 UI 与 worker 的解耦桥梁。
- 使用 `Qt::QueuedConnection` 跨线程调用，避免直接访问后台对象。

视觉建议：

- 做线程通信图：
  `QML/UI Thread -> CameraController -> queued signal -> CameraWorker Thread -> frameReady -> VideoItem`

## 第 7 页：深度图处理流程

标题：深度图滤波提升了前景判断的稳定性

讲述重点：

- RealSense 原始深度图存在噪声、空洞和边缘抖动。
- 项目先使用 RealSense SDK 滤波器，再使用 OpenCV 做阈值和形态学处理。

建议内容：

- RealSense SDK 滤波：
  - disparity transform
  - spatial filter
  - temporal filter
  - disparity to depth
- OpenCV 后处理：
  - 双边滤波
  - 无效深度清理
  - 深度范围阈值
  - 闭运算
  - 腐蚀
- 输出：只保留有效前景范围的深度图。

视觉建议：

- 用“原始深度 -> 滤波后深度 -> 前景 mask”的三段图说明。
- 如果有运行截图，可以放对比图。

## 第 8 页：YOLO 人像分割

标题：AI 分割用于补强深度阈值在复杂背景下的不足

讲述重点：

- 单靠深度阈值会受背景距离、空洞、边缘噪声影响。
- YOLO segmentation 模型可以从 RGB 图像中推理出人像区域。
- 项目将 person mask 取反后作为背景 mask，用于清理背景深度。

建议内容：

- 模型文件：`resources/models/yolo26n-seg.onnx`
- 推理框架：ONNX Runtime
- 输入：OpenCV BGR 图像
- 预处理：letterbox、RGB 转换、归一化、CHW 排列
- 后处理：置信度筛选、NMS、mask prototype 合成、sigmoid、还原到原图尺寸
- 性能策略：隔帧推理，降低实时处理压力。

视觉建议：

- 做一张“RGB 图 -> YOLO 推理 -> person mask -> 背景 mask”的流程图。

## 第 9 页：背景替换与边缘融合

标题：背景替换不仅是像素替换，还需要处理边缘过渡

讲述重点：

- 背景替换基于前景深度区域和背景图片。
- 对前景边缘进行羽化可以减少硬边和抖动。

建议内容：

- `ImageBackgroundProvider` 加载背景图片并按当前画面尺寸缓存。
- `backgroundRemoval()` 根据深度阈值判断前景/背景。
- 背景区域使用背景图片像素替换。
- 使用 `distanceTransform()` 计算前景边缘距离。
- 使用平滑插值对边缘区域进行 alpha 融合。

视觉建议：

- 展示“原始画面、mask、替换结果”的对比。
- 标出“边缘羽化”对视觉质量的影响。

## 第 10 页：深度伪彩叠加与实时预览

标题：深度叠加让用户能够直观看到空间信息

讲述重点：

- RealSense 深度帧本身是单通道距离信息，不适合直接显示。
- 项目通过 RealSense `colorizer` 将深度图转换为伪彩图。
- 用户可通过滑块调节 RGB 图与深度伪彩图的混合比例。

建议内容：

- `colorizer.process(depth)` 生成深度伪彩图。
- OpenCV `addWeighted()` 完成 RGB 与 depth color 的混合。
- `m_alpha` 控制混合比例。
- 输出 RGB 图像后转换为 `QImage`，传给 QML 显示。

视觉建议：

- 放一张透明度滑块截图。
- 用 `alpha = 0 / 0.5 / 1.0` 三种状态展示效果。

## 第 11 页：面部关键点检测扩展

标题：YOLOv8 face 为后续人脸增强功能提供基础能力

讲述重点：

- 项目新增 `facialLandmark` 工具类，替换原有 Haar + LBF 方案。
- 新方案使用 `yolov8n-face.onnx`，可同时输出人脸框和 5 个面部关键点。

建议内容：

- 模型文件：`resources/models/yolov8n-face.onnx`
- 推理框架：OpenCV DNN
- 检测输出：
  - 人脸矩形框
  - 置信度
  - 左眼
  - 右眼
  - 鼻尖
  - 左嘴角
  - 右嘴角
- 后处理要点：
  - letterbox 预处理
  - DFL 解码检测框
  - sigmoid 计算置信度
  - NMS 去除重复框
  - 坐标映射回原图

视觉建议：

- 使用一张人脸关键点检测结果图。
- 用 5 个点标注关键点含义。

## 第 12 页：前端交互设计

标题：Qt Quick 界面将复杂处理能力包装成清晰的用户操作

讲述重点：

- 前端不是只显示画面，还承担设备选择、参数调节、背景选择、拍照触发等交互。
- 当前界面采用左右布局，左侧为实时预览，右侧为控制面板。

建议内容：

- 视频预览区域：显示实时合成结果。
- 摄像头区域：刷新设备、选择设备、显示状态。
- 深度叠加强度：控制 RGB 与 depth 的混合比例。
- 背景切换：选择本地图片作为新背景。
- 拍照按钮：触发保存当前预览画面。
- 页面进入和按钮按压动画提升操作反馈。

视觉建议：

- 放应用界面截图。
- 用标注线指出关键控件。

## 第 13 页：工程实现亮点

标题：项目在实时性、模块化和可扩展性上做了工程化处理

讲述重点：

- 不只讲“用了哪些库”，更要讲为什么这样组织。

建议内容：

- UI 线程与相机处理线程分离，减少界面卡顿。
- 使用信号槽进行跨线程通信。
- 将深度滤波、背景图片、YOLO 分割、人脸关键点封装为独立工具类。
- 模型路径通过 `REALSENSE_SNAP_RESOURCE_DIR` 统一定位。
- 人像分割采用隔帧推理，兼顾实时性和效果。
- 背景图片按尺寸缓存，减少重复 resize 开销。
- 对 QML root object 和 `VideoItem` 增加加载检查，避免空对象崩溃。

视觉建议：

- 使用“技术亮点矩阵”，但每项文字要短。
- 可以配一张代码模块关系图。

## 第 14 页：问题排查与改进方向

标题：当前项目已经可运行，但相机稳定性和架构边界仍可继续优化

讲述重点：

- 展示你对工程问题的理解，而不是只展示结果。
- 说明 RealSense 在 macOS 上可能出现设备启动、USB 供电、权限或 profile 选择问题。

建议内容：

- 已遇到的问题：
  - QML 属性重复赋值导致 root object 加载失败。
  - `engine.rootObjects().first()` 在空列表时会触发崩溃。
  - RealSense 可能出现 `failed to set power state`。
  - `CameraWorker` 当前承担职责较多。
- 改进方向：
  - 显式指定 RealSense 流格式和分辨率，例如 `640x480 30fps`。
  - 完善 pipeline start/stop 状态机。
  - 将 `CameraWorker` 拆分为相机采集、帧处理流水线、背景合成、保存服务等模块。
  - 增加模型加载、图像处理和相机状态的错误提示。
  - 增加 demo 和单元测试，降低后续修改风险。

视觉建议：

- 左侧“已解决问题”，右侧“下一步优化”。
- 避免把错误日志整段贴上去，摘取关键词即可。

## 第 15 页：总结

标题：RealSense Snap 完成了实时深度拍照应用的核心闭环

讲述重点：

- 回扣项目目标：从 RealSense 设备输入，到 AI 和深度融合处理，再到用户可见的实时预览和照片保存。
- 强调技术综合性和工程实践价值。

建议内容：

- 项目完成了多源数据采集：RGB + Depth。
- 完成了传统视觉处理与深度学习模型的结合。
- 完成了 Qt Quick 前端和 C++ 后台 worker 的集成。
- 实现了实时预览、背景替换、深度叠加和拍照保存。
- 新增面部关键点检测，为后续人脸增强、美颜、姿态判断等功能留下扩展空间。

视觉建议：

- 用一张最终系统闭环图收尾：
  `RealSense 输入 -> 视觉处理 -> AI 推理 -> 图像合成 -> UI 交互 -> 照片输出`

## 可选附录页

如果汇报时间超过 15 分钟，可以增加以下附录：

### 附录 A：关键类职责说明

- `CameraWorker`：相机采集和处理调度。
- `CameraController`：QML 控制桥。
- `VideoItem`：视频画面绘制。
- `FilterProcessing`：深度滤波。
- `ImageBackgroundProvider`：背景图片管理。
- `yoloSeg`：人像分割模型推理。
- `facialLandmark`：人脸框和关键点检测。

### 附录 B：核心依赖说明

- Qt Quick：构建跨平台 GUI。
- Intel RealSense SDK：访问深度相机、对齐帧、深度伪彩。
- OpenCV：图像格式转换、滤波、形态学、背景融合、DNN 推理。
- ONNX Runtime：运行 YOLO segmentation 模型。
- OpenMP：加速部分像素级循环。

### 附录 C：面部关键点输出说明

`facialLandmark` 的第一张人脸关键点顺序建议说明为：

```text
landmark[0] = 左眼
landmark[1] = 右眼
landmark[2] = 鼻尖
landmark[3] = 左嘴角
landmark[4] = 右嘴角
```

## 推荐演讲节奏

- 第 1-3 页：项目背景和功能，约 2 分钟。
- 第 4-6 页：系统架构和主流程，约 4 分钟。
- 第 7-11 页：关键技术实现，约 6 分钟。
- 第 12-13 页：交互设计和工程亮点，约 2 分钟。
- 第 14-15 页：问题反思和总结，约 2 分钟。

## 制作建议

- 每页只突出一个观点，标题尽量写成结论句。
- 技术流程尽量用图，不要堆整段代码。
- 如果展示代码，只截取 8-15 行核心逻辑，并配一句解释。
- 汇报时重点讲数据流：彩色图、深度图、mask、背景图、最终预览图之间如何转换。
- 展示问题排查和改进方向，可以体现工程意识和技术深度。
