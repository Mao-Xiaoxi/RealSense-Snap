#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

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



#endif // CAMERACONTROLLER_H
