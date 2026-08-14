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

// 继承
class CameraWorker : public QObject{
    Q_OBJECT
public:
    explicit CameraWorker(QObject *parent = nullptr);
    ~CameraWorker();

    float alpha() const;
    void setAlpha(float a);

signals:
    void alphaChanged();

public slots:
    void start();
    void stop();

    void refreshDevices();
    void selectCamera(QString serial);

signals:
    void frameReady(QImage image);

    void deviceReady(QVariantList devices);
    void cameraError(QString message);
    void selectedCameraChanged(QString serial);


private:
    void processFrame();
    QTimer *m_timer = nullptr;
    std::atomic<bool> m_running{false};
    bool m_pipelineStarted = false;
    mutable QMutex m_alphaMutex;
    float m_alpha=0.f;
    QString m_selectedSerial;

    rs2::pipeline pipe;
    rs2::align align_to_depth{RS2_STREAM_DEPTH};    // 尽量使用C++11列表处理化，避免语法歧义
    rs2::align align_to_color{RS2_STREAM_COLOR};
    rs2::colorizer colorizer;
};


#endif // CAMERAWORKER_H