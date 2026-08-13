#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QImage>
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <atomic>

// 继承
class CameraWorker : public QObject{
    Q_OBJECT
public:
    explicit CameraWorker(QObject *parent = nullptr);
    ~CameraWorker();

public slots:
    void start();
    void stop();

signals:
    void frameReady(QImage image);

private:
    void processFrame();
    std::atomic<bool> m_running{false};
    rs2::pipeline pipe;
    rs2::align align_to_depth{RS2_STREAM_DEPTH};    // 尽量使用C++11列表处理化，避免语法歧义
    rs2::align align_to_color{RS2_STREAM_COLOR};
    rs2::colorizer colorizer;
};

#endif // CAMERAWORKER_H