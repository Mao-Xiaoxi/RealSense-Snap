# RealSense开发

## 任务

关于图像处理部分，请先学习以下内容，然后完成对应的任务。

学习内容：
学习RealSense: https://www.realsenseai.com/developers/

]
Developers - RealSense
www.realsenseai.com
Explore how developers can leverage the RealSense SDK 2.0 to build innovative applications across various platforms.

任务：
任务1：自由切换背景的照相软件
描述：借助RealSense摄像头，人站在摄像头前面拍照，可以自由切换人后面的背景，人的轮廓要清晰平滑

任务2：眉心与体温识别
描述：可以根据人的眼睛位置，使用摄像头，实时识别人的眉心位置，并检测出当前人的体温
.

## SDK安装

按照官网Mac OS教程下载并编译RealSense。

可能出现进程占用和权限问题，可尝试使用下指令进行解决。

```bash
// 杀死后台占用相机进程
sudo killall VDCAssistant AppleCameraAssistant

sudo ./Release/rs-enumerate-devices
```

**对于C++库的安装**

C++外部库的使用的一种方式为：自行下载编译源码，并安装在/usr/local中，之后通过CMake文件进行调用。

对于最终安装的相关细节，在cmake阶段指定。*尽量保存build/CMakeFiles相关文件，用于未来安全卸载和更新*

```bash
# 生成编译脚本，供后续使用
cmake ..

# 多核编译源码
make -j$(sysctl -n hw.cpu)

# 安装
sudo make install
```

一般来说，会使用pkg-config来对已经安装的库进行管理。

## RealSense示例

### 3D点云图的绘制

对于这个示例，官方提供了相应的example.hpp头文件，对于OpenGL窗口创建、管理和渲染循环，以及3D视角交互状态的glfw_state结构体。

实际开发过程中可能会使用其它专业的库：PCL/Open3D专业点云库，OpenGL/Vulkan图形框架

### 相机参数校准

RealSense提供自行校准相机参数的接口，可以自行校准相机参数并持久保存。

## 开发

### CMake基本格式

```cmake
# cmake版本控制：
make_mininum_required( VERSION x.xx )
# 项目名称语言
preoject(MyApp LANGUAGES CXX)

# 变量声明
set(ROOT "/User/../")
# 调用包和库
find_package(OpenCV REQUIRED)
find_library(REALSENSE_LIB NAMES realsense2
	PATHS "$(...)"
	NO_DEFAULT_PATH
)

# 主程序
add_executable(myApp myApp.cpp)
# 包和库的链接
tatget_include_directories(myApp PRIVATE ...)
target_link_libraries(myApp PRIVATE ...)

set_target_properties(myApp PROPERTIES ...)
```

### RealSence数据流

| 数据流类型     | `RS2_STREAM_*` 常量     | 数据格式 (`RS2_FORMAT_*`)                               | 分辨率 (宽x高)               | 帧率 (FPS)        | 主要作用与典型应用                                           |
| :------------- | :---------------------- | :------------------------------------------------------ | :--------------------------- | :---------------- | :----------------------------------------------------------- |
| **深度流**     | `RS2_STREAM_DEPTH`      | `RS2_FORMAT_Z16`                                        | 1280x720, 848x480, 640x480   | 90, 60, 30, 15, 6 | **核心流**，每个像素记录距离（mm）。用于3D扫描、避障、测量等。 |
| **彩色流**     | `RS2_STREAM_COLOR`      | `RS2_FORMAT_BGR8`, `RS2_FORMAT_RGB8`, `RS2_FORMAT_YUYV` | 1920x1080, 1280x720, 640x480 | 30, 60            | 提供视觉图像。用于显示、为点云提供颜色、物体识别。           |
| **红外流**     | `RS2_STREAM_INFRARED`   | `RS2_FORMAT_Y8`, `RS2_FORMAT_Y16`                       | 1280x720, 848x480, 640x480   | 90, 60, 30        | 获取红外光强度图像。辅助深度计算、低光照夜视。               |
| **鱼眼流**     | `RS2_STREAM_FISHEYE`    | `RS2_FORMAT_RAW8`, `RS2_FORMAT_Y8`                      | 640x480, 480x270             | 30, 60            | **特定型号（如T265）**。超广角视野，用于VIO、VR追踪等。      |
| **陀螺仪流**   | `RS2_STREAM_GYRO`       | `RS2_FORMAT_MOTION_XYZ32F`                              | N/A (Motion数据)             | 200, 400          | 测量旋转角速度。用于姿态估计、图像防抖。                     |
| **加速度计流** | `RS2_STREAM_ACCEL`      | `RS2_FORMAT_MOTION_XYZ32F`                              | N/A (Motion数据)             | 200, 250, 400     | 测量线性加速度。用于倾斜检测、运动感知。                     |
| **位姿流**     | `RS2_STREAM_POSE`       | `RS2_FORMAT_6DOF`                                       | N/A (6DoF数据)               | 200, 100          | **特定型号（如T265）**。输出6自由度位姿，用于SLAM和AR。      |
| **置信度流**   | `RS2_STREAM_CONFIDENCE` | `RS2_FORMAT_RAW8`                                       | 与深度流一致                 | 与深度流一致      | 深度图置信度分数（0-3），评估每个深度点的可靠度。            |
| **GPIO流**     | `RS2_STREAM_GPIO`       | `RS2_FORMAT_GPIO`                                       | N/A                          | 可配置            | 处理外部输入/输出信号，用于硬件同步。                        |

### 任务解决路径

| 维度         | 图像分割                         | 红外深度传感器                                             |
| ------------ | -------------------------------- | ---------------------------------------------------------- |
| 精度         | 高                               | 一般，边缘粗糙                                             |
| 鲁棒性       | 依赖训练数据；依赖环境光照条件。 | 暗光条件下稳定，易于受强阳光干扰；对黑色、反光物体可能失效 |
| 实时性与性能 | 消耗与精度正相关                 | 速度快、开销低。                                           |

### Qt

Qt使用一些列函数来实现数据的对接

```bash
QObject::connect(&controller, &CameraController::alphaRequested,
                 &worker, &CameraWorker::setAlpha,
                 Qt::QueuedConnection);

// Qt属性系统，使用自己函数，将属性暴露出来
Q_PROPERTY(float alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)

// 之后在函数实现中，C++代码函数中主动使用emit来发出数据传输请求。
emit alphaRequested(a);
// QML直接赋值实现数据传输。
cameraController.alpha = value
```

## 注意

* 由于macOS的权限原因，终端往往不能直接访问摄像头，因此在例如遍历摄像头设备的操作时，可能会出现如下报错，导致程序直接失效。

```bash
cannot access depth sensor
```

* 同时如果硬性设置摄像头功率，也可能设置失败，导致程序退出运行。因此在实际开发中建议设置失败时弹出警告，程序放弃设置继续运行。
