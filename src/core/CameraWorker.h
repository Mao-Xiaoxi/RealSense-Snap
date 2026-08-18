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

#include "BackgroundProvider.h"


// 继承
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

     void setBackgroundProvider(BackgroundProvider *provider);

signals:
    /**
     * @brief 透明度发生变化时发出。
     */
    void alphaChanged();

public slots:
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
    /**
     * @brief 读取并处理一帧相机数据。
     */
    void processFrame();
    bool m_pipelineStarted = false;
    float m_alpha=0.f;
    std::atomic<bool> m_running{false};

    mutable QMutex m_alphaMutex;
    QTimer *m_timer = nullptr;
    QString m_selectedSerial;

    BackgroundProvider *m_background = nullptr;

    rs2::pipeline pipe;
    rs2::align align_to_depth{RS2_STREAM_DEPTH};    // 尽量使用C++11列表处理化，避免语法歧义
    rs2::align align_to_color{RS2_STREAM_COLOR};
    rs2::colorizer colorizer;
    // 滤波器
    rs2::decimation_filter m_decimationFilter;
    rs2::spatial_filter m_spatialFilter;
    rs2::temporal_filter m_temporalFilter;
    rs2::hole_filling_filter m_holeFilter;
    rs2::disparity_transform m_depthToDisparity{true};
    rs2::disparity_transform m_disparityToDepth{false};

    // 私有函数
    /**
     * @brief 对深度帧应用降采样、空间和时间滤波。
     * @param depth 原始深度帧。
     * @return 滤波后的深度帧。
     */
    rs2::depth_frame applyDepthFilters(const rs2::depth_frame &depth);

    /**
     * @brief 根据深度信息处理彩色帧背景。
     * @param color 彩色帧。
     * @param depth 深度帧。
     * @return 处理后的彩色帧。
     */
    rs2::video_frame backgroundRemoval(rs2::video_frame &color, const rs2::depth_frame &depth);
};


#endif // CAMERAWORKER_H
