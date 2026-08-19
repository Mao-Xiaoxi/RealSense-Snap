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
    /**
     * @brief 获取当前叠加透明度。
     * @return 透明度，范围为 0.0 到 1.0。
     */
    float alpha() const {return m_alpha;}

    /**
     * @brief 获取当前设备列表。
     * @return 相机设备信息列表。
     */
    QVariantList cameras() const;

    /**
     * @brief 获取当前相机状态文本。
     * @return 状态文本。
     */
    QString cameraStatus() const;

    /**
     * @brief 获取当前选中相机序列号。
     * @return 相机序列号。
     */
    QString selectedCameraSerial() const;

public slots:
    /**
     * @brief 设置叠加透明度并通知 Worker。
     * @param a 透明度，范围为 0.0 到 1.0。
     */
    void setAlpha(float a){
        if(a<0.0||a>1.0) return;
        if (qFuzzyCompare(m_alpha, a))
            return;
        m_alpha=a;
        emit alphaChanged();
        emit alphaRequested(a);
    }

    /**
     * @brief 请求刷新可用相机设备。
     */
    Q_INVOKABLE void refreshDevices();

    /**
     * @brief 设置当前选中相机序列号。
     * @param serial 相机序列号。
     */
    void setSelectedCameraSerial(QString serial);

    /**
     * @brief 更新设备列表。
     * @param devices 设备信息列表。
     */
    void setDevices(QVariantList devices);

    /**
     * @brief 更新相机状态文本。
     * @param message 状态或错误信息。
     */
    void setCameraStatus(QString message);

    /**
     * @brief 同步 Worker 返回的选中相机序列号。
     * @param serial 相机序列号。
     */
    void setSelectedCameraSerialFromWorker(QString serial);

    Q_INVOKABLE void setBackgroundImage(QString path);

signals:
    /**
     * @brief 透明度发生变化时发出。
     */
    void alphaChanged();

    /**
     * @brief 请求 Worker 更新透明度时发出。
     * @param a 透明度，范围为 0.0 到 1.0。
     */
    void alphaRequested(float a);

    // 通知UI更新
    /**
     * @brief 设备列表发生变化时发出。
     */
    void camerasChanged();

    /**
     * @brief 相机状态发生变化时发出。
     */
    void cameraStatusChanged();

    /**
     * @brief 当前选中相机发生变化时发出。
     */
    void selectedCameraSerialChanged();

    // 向Worker发送请求
    /**
     * @brief 请求 Worker 刷新设备列表时发出。
     */
    void refreshDevicesRequested();

    /**
     * @brief 请求 Worker 选择相机时发出。
     * @param serial 相机序列号。
     */
    void cameraSelected(QString serial);

    void backgroundImageRequested(QString path);


private:
    float m_alpha=0.0f;

    QVariantList m_cameras;
    QString m_cameraStatus;
    QString m_selectedCameraSerial;
};



#endif // CAMERACONTROLLER_H
