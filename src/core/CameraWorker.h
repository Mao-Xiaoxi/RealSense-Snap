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
    mutable QMutex m_alphaMutex;
    float m_alpha=0.f;
    QString m_selectedSerial;

    rs2::pipeline pipe;
    rs2::align align_to_depth{RS2_STREAM_DEPTH};    // 尽量使用C++11列表处理化，避免语法歧义
    rs2::align align_to_color{RS2_STREAM_COLOR};
    rs2::colorizer colorizer;
};

// 摄像头选择与叠加参数调节。
class CameraController : public QObject{
    Q_OBJECT
    Q_PROPERTY(float alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)   //调用内部的函数来实现接口通信
    Q_PROPERTY(QVariantList cameras READ cameras NOTIFY camerasChanged)
    Q_PROPERTY(QString cameraStatus READ cameraStatus NOTIFY cameraStatusChanged)
    Q_PROPERTY(QString selectedCameraSerial READ selectedCameraSerial WRITE setSelectedCameraSerial NOTIFY selectedCameraSerialChanged)

public:
    float alpha() const {return m_alpha;}
    QVariantList cameras() const;
    QString cameraStatus() const;
    QString selectedCameraSerial() const;

public slots:
    void setAlpha(float a){
        if(a<0.0||a>1.0) return;
        if (qFuzzyCompare(m_alpha, a))
            return;
        m_alpha=a;
        emit alphaChanged();
        emit alphaRequested(a);
    }

    Q_INVOKABLE void refreshDevices();
    void setSelectedCameraSerial(QString serial);
    void setDevices(QVariantList devices);
    void setCameraStatus(QString message);
    void setSelectedCameraSerialFromWorker(QString serial);

signals:
    void alphaChanged();
    void alphaRequested(float a);

    // 通知UI更新
    void camerasChanged();
    void cameraStatusChanged();
    void selectedCameraSerialChanged();

    // 向Worker发送请求
    void refreshDevicesRequested();
    void cameraSelected(QString serial);


private:
    float m_alpha=0.0f;

    QVariantList m_cameras;
    QString m_cameraStatus;
    QString m_selectedCameraSerial;
};


#endif // CAMERAWORKER_H