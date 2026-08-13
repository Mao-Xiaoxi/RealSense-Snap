#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QTimer>

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

signals:
    void frameReady(QImage image);

private:
    void processFrame();
    QTimer *m_timer = nullptr;
    std::atomic<bool> m_running{false};
    mutable QMutex m_alphaMutex;
    float m_alpha=0.f;

    rs2::pipeline pipe;
    rs2::align align_to_depth{RS2_STREAM_DEPTH};    // 尽量使用C++11列表处理化，避免语法歧义
    rs2::align align_to_color{RS2_STREAM_COLOR};
    rs2::colorizer colorizer;
};


class CameraController : public QObject{
    Q_OBJECT
    Q_PROPERTY(float alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)   //调用内部的函数来实现接口通信
public:
    float alpha() const {return m_alpha;}

public slots:
    void setAlpha(float a){
        if(a<0.0||a>1.0) return;
        if (qFuzzyCompare(m_alpha, a))
            return;
        m_alpha=a;
        emit alphaChanged();
        emit alphaRequested(a);
    }
signals:
    void alphaChanged();
    void alphaRequested(float a);

private:
    float m_alpha=0.0f;
};


#endif // CAMERAWORKER_H