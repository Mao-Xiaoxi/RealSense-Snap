#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

#include <atomic>
#include <string>

#include "background/imagebackgroundprovider.h"
#include "./processing/filterprocessing.h"
#include "utils/yoloseg.h"

/**
 * @brief 相机采集与图像处理工作对象。
 *
 * 该类运行在独立 worker 线程中，负责 RealSense 设备枚举、相机采集、
 * 深度滤波、人像分割、背景替换、预览图生成和照片保存。
 * QML 不直接访问该类，而是通过 CameraController 发出请求。
 */
class CameraWorker : public QObject{
    Q_OBJECT
public:
    /**
     * @brief 创建相机工作对象。
     * @param parent Qt 父对象。
     */
    explicit CameraWorker(QObject *parent = nullptr);

    /**
     * @brief 停止采集并释放工作资源。
     */
    ~CameraWorker();

    /**
     * @brief 获取彩色图与深度图的叠加透明度。
     * @return 当前透明度，范围为 0.0 到 1.0。
     */
    float alpha() const;

    /**
     * @brief 设置彩色图与深度图的叠加透明度。
     * @param a 透明度，范围为 0.0 到 1.0。
     */
    void setAlpha(float a);

signals:
    /**
     * @brief 透明度发生变化时发出。
     */
    void alphaChanged();

public slots:

    /**
     * @brief 初始化 worker 线程内资源。
     *
     * 该函数用于加载默认背景图和 YOLO 模型。因为模型加载和图片读取
     * 属于可能耗时的操作，应在 moveToThread() 之后通过 queued connection 调用。
     */
    void initialize();

    /**
     * @brief 启动 RealSense 采集管线。
     */
    void start();

    /**
     * @brief 停止 RealSense 采集管线。
     */
    void stop();

    /**
     * @brief 刷新可用 RealSense 设备列表。
     */
    void refreshDevices();

    /**
     * @brief 选择指定序列号的相机。
     * @param serial 相机序列号。
     */
    void selectCamera(QString serial);

    /**
     * @brief 请求保存当前预览画面。
     *
     * 该函数只设置拍照标记，真正的保存动作在下一帧处理完成后执行。
     */
    void capturePhoto();

    /**
     * @brief 切换背景图片。
     * @param path 新背景图片路径。
     *
     * 背景图片由 worker 线程加载，避免 UI 线程和 worker 线程同时访问背景缓存。
     */
    void setBackgroundImage(const QString &path);

signals:
    /**
     * @brief 新视频帧准备完成时发出。
     * @param image 已转换为 QImage 的画面。
     */
    void frameReady(QImage image);

    /**
     * @brief 设备列表刷新完成时发出。
     * @param devices 设备信息列表。
     */
    void deviceReady(QVariantList devices);

    /**
     * @brief 相机发生错误时发出。
     * @param message 错误信息。
     */
    void cameraError(QString message);

    /**
     * @brief 已选相机发生变化时发出。
     * @param serial 当前相机序列号。
     */
    void selectedCameraChanged(QString serial);



private:

    // RealSense pipeline 是否已经 start，用于避免重复停止或重复启动。
    bool m_pipelineStarted = false;

    // RGB 与深度伪彩叠加的透明度，范围为 0.0 到 1.0。
    float m_alpha=0.f;

    // 采集循环运行标记，使用 atomic 便于跨线程安全地观察运行状态。
    std::atomic<bool> m_running{false};

    // 保护 m_alpha，避免 UI 请求修改透明度时和帧处理读取同时发生。
    mutable QMutex m_alphaMutex;

    // 驱动逐帧处理的定时器，归 CameraWorker 所在线程管理。
    QTimer *m_timer = nullptr;

    // 当前选中的 RealSense 设备序列号。
    QString m_selectedSerial;


    // RealSense 采集管线，负责从设备获取 depth/color frameset。
    rs2::pipeline pipe;

    // 对齐器：可将彩色图和深度图对齐到指定流。
    rs2::align align_to_depth{RS2_STREAM_DEPTH};    // 尽量使用C++11列表处理化，避免语法歧义
    rs2::align align_to_color{RS2_STREAM_COLOR};

    // RealSense 深度伪彩转换器，用于生成调试/预览叠加画面。
    rs2::colorizer colorizer;

    // 滤波器
    FilterProcessing filter_processing;

    // 是否在下一帧处理完成后保存照片。
    bool m_capture;

    // 默认背景图路径。
    QString m_backgroundPath;

    // 照片保存目录。
    QString m_save_path;

    // YOLO segmentation 模型路径。
    QString m_modelPath;

    // 图像分割模型封装。
    yoloSeg yolo26;

    // 背景图片提供器，负责加载图片和按当前画面尺寸缓存背景图。
    ImageBackgroundProvider m_backgroundProvider;

    // 是否执行本帧 YOLO 分割；可用于隔帧推理或关闭分割。
    bool m_if_segmentation;

    // 背景区域 mask，通常由人像 mask 取反得到。
    cv::Mat m_backgroundMask;

    // 私有函数
    /**
     * @brief 读取并处理一帧相机数据。
     */
    void processFrame();

    /**
     * @brief 对 RealSense 深度帧执行 SDK 滤波和 OpenCV 后处理。
     * @param depth RealSense 原始深度帧。
     * @return 处理后的深度图，通常为 CV_16UC1。
     */
    cv::Mat applyDepthFilters(const rs2::depth_frame &depth);

    /**
     * @brief 根据深度信息处理彩色帧背景。
     * @param color 彩色帧。
     * @param depth 深度帧。
     * @return 处理后的彩色帧。
     */
    cv::Mat backgroundRemoval(cv::Mat &color, const cv::Mat &depth);

    /**
     * @brief 生成彩色图与深度伪彩图的叠加预览。
     * @param depth RealSense 深度帧。
     * @param color_bgr 已完成背景替换的 BGR 彩色图。
     * @return RGB 格式的预览图，用于转换成 QImage 显示。
     */
    cv::Mat overlay_depth_color(rs2::depth_frame &depth, cv::Mat &color_bgr);

    /**
     * @brief 保存当前预览图到照片目录。
     * @param overlay RGB 格式的预览图。
     * @return 保存成功返回 true，失败返回 false。
     */
    bool save_photo(const cv::Mat &overlay);
};


#endif // CAMERAWORKER_H
