#include "CameraWorker.h"
#include <QDebug>
#include <QThread>
#include <opencv2/opencv.hpp>

CameraWorker::CameraWorker(QObject *parent)
    : QObject(parent)
    , align_to_depth(RS2_STREAM_DEPTH)   // 初始化对齐对象（昂贵操作只做一次）
    , align_to_color(RS2_STREAM_COLOR)
{
    // 构造函数中不要做耗时操作，此时 Worker 可能还在主线程

    m_spatialFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, 3);         // 滤波强度
    m_spatialFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.3f);   // 边缘保持参数
    m_spatialFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20);     // 边缘阈值
    m_spatialFilter.set_option(RS2_OPTION_HOLES_FILL, 1);               //填洞模式

    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.20f); // 当前帧权重，越低越平滑
    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 80);    // 允许更大的深度变化参与平滑
    m_temporalFilter.set_option(RS2_OPTION_HOLES_FILL, 3);              // 使用历史帧填补短暂空洞

    m_holeFilter.set_option(RS2_OPTION_HOLES_FILL, 1);                // 0：关闭 1:使用周围像素平均 2:使用梯度平均

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
        m_pipelineStarted = true;

        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &CameraWorker::processFrame);
        m_timer->start(0);

    }catch (const rs2::error &e) {
        qCritical() << "RealSense error:" << e.what();
        emit cameraError(QString::fromUtf8(e.what()));
        // 发生异常时，最好通知主线程发生错误（可以加一个 errorOccurred 信号）
        m_running = false;
        m_pipelineStarted = false;
    } catch (const std::exception &e) {
        qCritical() << "Standard exception:" << e.what();
        emit cameraError(QString::fromUtf8(e.what()));
        m_running = false;
        m_pipelineStarted = false;
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

    if (!m_pipelineStarted) {
        return;
    }

    try {
        pipe.stop();
    } catch (const std::exception &e) {
        qWarning() << "Pipeline stop failed:" << e.what();
    }

    m_pipelineStarted = false;
}

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

    cv::Mat color_rgb(cv::Size(color.get_width(),color.get_height()),CV_8UC3,(void*)color.get_data(),cv::Mat::AUTO_STEP);
    cv::Mat color_bgr;
    cv::cvtColor(color_rgb,color_bgr,cv::COLOR_RGB2BGR);

    cv::Mat filtered = applyDepthFilters(depth);
    if (filtered.empty()) {
        return;
    }

    // 对彩色图像背景进行替换
    color_bgr = backgroundRemoval(color_bgr,filtered);

    rs2::video_frame colorized_depth = colorizer.process(depth).as<rs2::video_frame>();

    cv::Mat depth_rgb(cv::Size(colorized_depth.get_width(),colorized_depth.get_height()),CV_8UC3,(void*)colorized_depth.get_data(), cv::Mat::AUTO_STEP);
    cv::Mat depth_bgr;
    cv::cvtColor(depth_rgb, depth_bgr, cv::COLOR_RGB2BGR);

    // 图片尺寸对齐
    if (depth_bgr.size() != color_bgr.size()) {
        cv::resize(depth_bgr, depth_bgr, color_bgr.size(), 0, 0, cv::INTER_NEAREST);
    }

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
}catch(const cv::Exception &e){
    if(m_running.load()){
        qCritical()<<"OpenCV frame processing error:"<<e.what();
    }
    m_running=false;
    if (m_timer) {
        m_timer->stop();
    }
}catch(const std::exception &e){
    if(m_running.load()){
        qCritical()<<"Frame processing exception:"<<e.what();
    }
    m_running=false;
    if (m_timer) {
        m_timer->stop();
    }
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

cv::Mat CameraWorker::applyDepthFilters(const rs2::depth_frame &depth)
{
    try{
        rs2::frame filtered = depth;

        /* RealSense Filter */

        //filtered = m_decimationFilter.process(filtered);
        filtered = m_depthToDisparity.process(filtered);
        filtered = m_spatialFilter.process(filtered);
        filtered = m_temporalFilter.process(filtered);
        //filtered = m_holeFilter.process(filtered);
        filtered = m_disparityToDepth.process(filtered);

        /* OpenCV Filter */

        rs2::depth_frame depth_frame = filtered.as<rs2::depth_frame>();

        int width = depth_frame.get_width();
        int height = depth_frame.get_height();
        const void* data = depth_frame.get_data();  // 只读数据

        cv::Mat filtered_gray(
            height, width, CV_16UC1,
            const_cast<void*>(data),  // 仅当不修改时才安全
            cv::Mat::AUTO_STEP
            );

        cv::Mat filtered_float;
        cv::Mat bilateral_float;
        filtered_gray.convertTo(filtered_float, CV_32F);
        cv::bilateralFilter(filtered_float, bilateral_float, 9, 75, 75);

        cv::Mat mask;
        cv::inRange(bilateral_float, cv::Scalar(500), cv::Scalar(1200), mask);
        cv::Mat kernel_1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::Mat kernel_2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel_1);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel_2);

        cv::Mat result_float;
        result_float = cv::Mat::zeros(bilateral_float.size(), bilateral_float.type());
        bilateral_float.copyTo(result_float, mask);

        cv::Mat result_depth;
        result_float.convertTo(result_depth, CV_16UC1);

        return result_depth;

    } catch(const rs2::error &e){
        qCritical()<<"Filter failed"<< e.what();
        emit cameraError(QString::fromUtf8(e.what()));
        return {};
    } catch(const cv::Exception &e){
        qCritical()<<"OpenCV filter failed"<< e.what();
        emit cameraError(QString::fromUtf8(e.what()));
        return {};
    } catch(const std::exception &e){
        qCritical()<<"Depth filter failed"<< e.what();
        emit cameraError(QString::fromUtf8(e.what()));
        return {};
    }
}

/*
 * 自动设置人体和背景的分割阈值，实现人体和背景的分离。
 * 使用 RealSense ID库，检测人脸区域，并以人脸蛇毒像素为基准，计算阈值。
 */
cv::Mat CameraWorker::backgroundRemoval(cv::Mat &color, const cv::Mat &depth) {
    // 1. 输入有效性检查
    if (!m_background || !m_background->isReady()) {
        return color;
    }
    if (color.empty() || depth.empty()) {
        return color;
    }
    if (color.type() != CV_8UC3 && color.type() != CV_8UC4) {
        // 仅支持8位彩色图，其他格式可能无法直接替换
        return color;
    }
    if (depth.type() != CV_16UC1) {
        // 深度图必须是16位无符号单通道（RealSense默认格式）
        return color;
    }

    int width = color.cols;
    int height = color.rows;
    int depthWidth = depth.cols;
    int depthHeight = depth.rows;

    // 取交集区域（防止尺寸不匹配）
    int processWidth = std::min(width, depthWidth);
    int processHeight = std::min(height, depthHeight);

    // 获取背景图像（应与color同尺寸同类型）
    cv::Mat background = m_background->backgroundForSize(cv::Size(width, height));
    if (background.empty() || background.type() != color.type()) {
        return color;
    }

    // 像素字节数（例如CV_8UC3为3，CV_8UC4为4）
    int elemSize = color.elemSize();

#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < processHeight; ++y) {
        // 获取三张图像的行指针
        uchar* colorRow = color.ptr<uchar>(y);
        const uchar* bgRow = background.ptr<const uchar>(y);
        const uint16_t* depthRow = depth.ptr<const uint16_t>(y);

        for (int x = 0; x < processWidth; ++x) {
            // 深度值（单位：毫米）
            uint16_t depthValue = depthRow[x];
            // 转换为米
            float distance = depthValue / 1000.0f;

            // 若距离无效（0）或超过阈值（1.2米），则用背景替换
            if (distance <= 0.0f || distance > 1.2f) {
                // 拷贝整个像素（elemSize个字节）
                int offset = x * elemSize;
                memcpy(&colorRow[offset], &bgRow[offset], elemSize);
            }
        }
    }

    return color;
}

void CameraWorker::setBackgroundProvider(BackgroundProvider *provider){
    m_background=provider;
}
