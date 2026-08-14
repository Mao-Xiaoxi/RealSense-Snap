#include "CameraWorker.h"
#include <QDebug>
#include <QThread>
#include <chrono>
#include <opencv2/opencv.hpp>

CameraWorker::CameraWorker(QObject *parent)
    : QObject(parent)
    , align_to_depth(RS2_STREAM_DEPTH)   // 初始化对齐对象（昂贵操作只做一次）
    , align_to_color(RS2_STREAM_COLOR)
{
    // 构造函数中不要做耗时操作，此时 Worker 可能还在主线程
}

CameraWorker::~CameraWorker(){
    stop();
}

void CameraWorker::start(){
    if(m_running.exchange(true)){
        qWarning()<<"CameraWorker already running";
        return;
    }
    qDebug()<<"CameraWorker starting in thread:"<<QThread::currentThread();

    try{
        rs2::config cfg;
        if(!m_selectedSerial.isEmpty()){
            cfg.enable_device(m_selectedSerial.toStdString());
        }
        cfg.enable_stream(RS2_STREAM_DEPTH);
        cfg.enable_stream(RS2_STREAM_COLOR);

        pipe.start(cfg);

        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &CameraWorker::processFrame);
        m_timer->start(0);

    }catch (const rs2::error &e) {
        qCritical() << "RealSense error:" << e.what();
        emit cameraError(QString::fromUtf8(e.what()));
        // 发生异常时，最好通知主线程发生错误（可以加一个 errorOccurred 信号）
        m_running = false;
        try{
            pipe.stop();
        } catch(const rs2::error &e){
            qCritical()<<"RealSense stop fail";
            emit cameraError(QString::fromUtf8(e.what()));
        }
    } catch (const std::exception &e) {
        qCritical() << "Standard exception:" << e.what();
        emit cameraError(QString::fromUtf8(e.what()));
        m_running = false;
        try{
            pipe.stop();
        } catch(const rs2::error &e){
            qCritical()<<"RealSense stop fail";
            emit cameraError(QString::fromUtf8(e.what()));
        }
    }
}

void CameraWorker::stop(){
    if(!m_running.exchange(false)){
        return;
    }
    qDebug()<<"CameraWorker stop requested";

    if(m_timer){
        m_timer->stop();
        m_timer->deleteLater();
        m_timer=nullptr;
    }
    try {
        pipe.stop();
    } catch (const std::exception &e) {
        qWarning() << "Pipeline stop failed:" << e.what();
    }
}

/**
 * @brief CameraWorker::refreshDevices
 */
void CameraWorker::refreshDevices() try{
    rs2::context ctx;
    auto devices = ctx.query_devices();
    QVariantList list;

    for(const auto &device : devices){  // 使用按值遍历会造成资源浪费，并且IDE会警告
        // bool hasColor = false;
        // bool hasDepth = false;

        // for(const auto &sensor : device.query_sensors()){
        //     for(const auto &profile : sensor.get_stream_profiles()){
        //         if(profile.stream_type()==RS2_STREAM_COLOR)
        //             hasColor = true;
        //         if(profile.stream_type()==RS2_STREAM_DEPTH)
        //             hasDepth = true;
        //     }
        // }
        bool hasColor = true;
        bool hasDepth = true;

        QVariantMap item;
        item["name"] = device.supports(RS2_CAMERA_INFO_NAME)
                           ? QString::fromStdString(device.get_info(RS2_CAMERA_INFO_NAME))
                           : "Unknown RealSense";
        item["serial"] = device.supports(RS2_CAMERA_INFO_SERIAL_NUMBER)
                             ? QString::fromStdString(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER))
                             : "";

        // item["valid"] = hasColor && hasDepth;
        // if (!hasColor)
        //     item["reason"] = "Missing color stream";
        // else if (!hasDepth)
        //     item["reason"] = "Missing depth stream";
        // else
        //     item["reason"] = "";

        item["valid"] = !item["serial"].toString().isEmpty();
        item["reason"] = item["valid"].toBool() ? "" : "Missing serial number";

        list.append(item);
    }

    emit deviceReady(list);
} catch(const rs2::error &e) {
    emit cameraError(QString::fromUtf8(e.what()));
}

void CameraWorker::selectCamera(QString serial){
    if(serial.isEmpty()){
        emit cameraError("Invalid camera serial");
        return;
    }

    if(serial == m_selectedSerial && m_running.load()){
        return;
    }

    bool wasRunning = m_running.load();

    if(wasRunning)
        stop();
    m_selectedSerial = serial;
    emit selectedCameraChanged(serial);
    start();
}

/**
 * @brief CameraWorker::processFrame
 */
void CameraWorker::processFrame() try
{
    if (!m_running.load(std::memory_order_acquire)) {
        return;
    }

    rs2::frameset frames;
    if(!pipe.try_wait_for_frames(&frames,5000)){
        qWarning()<<"No frame received";
        return;
    }
    if(false){
        frames=align_to_depth.process(frames);
    }else{
        frames=align_to_color.process(frames);
    }

    rs2::depth_frame depth=frames.get_depth_frame();
    rs2::video_frame color=frames.get_color_frame();
    if(!depth||!color){
        qWarning()<<"Incomplete frameset receive.d";
        return;
    }
    rs2::video_frame colorized_depth = colorizer.colorize(depth);

    cv::Mat color_rgb(cv::Size(color.get_width(),color.get_height()),CV_8UC3,(void*)color.get_data(),cv::Mat::AUTO_STEP);
    cv::Mat color_bgr;
    cv::cvtColor(color_rgb,color_bgr,cv::COLOR_RGB2BGR);

    cv::Mat depth_rgb(cv::Size(colorized_depth.get_width(),colorized_depth.get_height()),CV_8UC3,(void*)colorized_depth.get_data(), cv::Mat::AUTO_STEP);
    cv::Mat depth_bgr;
    cv::cvtColor(depth_rgb, depth_bgr, cv::COLOR_RGB2BGR);


    cv::Mat overlay;
    float currentAlpha;
    {
        QMutexLocker locker(&m_alphaMutex);
        currentAlpha = m_alpha;
    }
    cv::addWeighted(color_bgr, 1.0f - currentAlpha, depth_bgr, currentAlpha, 0, overlay);

    cv::Mat overlay_rgb;
    cv::cvtColor(overlay,overlay_rgb,cv::COLOR_BGR2RGB);

    QImage qimg(overlay_rgb.data,
                overlay_rgb.cols,
                overlay_rgb.rows,
                static_cast<int>(overlay_rgb.step),
                QImage::Format_RGB888);
    emit frameReady(qimg.copy());
}catch(const rs2::error &e){
    if(m_running.load()){
        qCritical()<<"Frame processing error:"<<e.what();
    }
    m_running=false;
    if (m_timer) {
        m_timer->stop();
    }
    // try{
    //     pipe.stop();
    // } catch(const rs2::error &e){
    //     qCritical()<<"RealSense stop fail";
    //     emit cameraError(QString::fromUtf8(e.what()));
    // }
}

float CameraWorker::alpha() const
{
    QMutexLocker locker(&m_alphaMutex);
    return m_alpha;
}

void CameraWorker::setAlpha(float a){
    if(a<0.0f||a>1.0f) return;
    {
        QMutexLocker locker(&m_alphaMutex);
        if(qFuzzyCompare(m_alpha,a)) return;
        m_alpha=a;
    }

    emit alphaChanged();    // 通知QML属性变化
}

QVariantList CameraController::cameras() const
{
    return m_cameras;
}

QString CameraController::cameraStatus() const
{
    return m_cameraStatus;
}

QString CameraController::selectedCameraSerial() const
{
    return m_selectedCameraSerial;
}

void CameraController::refreshDevices()
{
    emit refreshDevicesRequested();
}

void CameraController::setSelectedCameraSerial(QString serial)
{
    if (serial == m_selectedCameraSerial)
        return;

    m_selectedCameraSerial = serial;
    emit selectedCameraSerialChanged();
    emit cameraSelected(serial);
}

void CameraController::setDevices(QVariantList devices)
{
    m_cameras = devices;
    emit camerasChanged();

    m_cameraStatus = QString("检测到 %1 个设备").arg(devices.size());
    emit cameraStatusChanged();

}

void CameraController::setCameraStatus(QString message)
{
    if (message == m_cameraStatus)
        return;

    m_cameraStatus = message;
    emit cameraStatusChanged();
}

void CameraController::setSelectedCameraSerialFromWorker(QString serial)
{
    if (serial == m_selectedCameraSerial)
        return;

    m_selectedCameraSerial = serial;
    emit selectedCameraSerialChanged();
}



