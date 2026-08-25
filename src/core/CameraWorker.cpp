#include "CameraWorker.h"
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QStandardPaths>
#include <QThread>
#include <opencv2/opencv.hpp>

CameraWorker::CameraWorker(QObject *parent)
    : QObject(parent)
    , align_to_depth(RS2_STREAM_DEPTH)   // 初始化对齐对象（昂贵操作只做一次）
    , align_to_color(RS2_STREAM_COLOR)
{
    // 构造函数中不要做耗时操作，此时 Worker 可能还在主线程
    m_capture = false;
    m_save_path = "/Users/maoxiaoxi/Documents/code/C++/Qt/RealSense_Snap/resources/photos";
    const std::string model_path = "/Users/maoxiaoxi/Documents/code/C++/Qt/RealSense_Snap/resources/models/yolo26n-seg.onnx";
    if (!yolo26.loadModel(model_path)) {
        qWarning() << "Failed to load YOLO segmentation model:"
                   << QString::fromStdString(model_path);
    }
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
    const bool wasRunning = m_running.exchange(false);
    if(!wasRunning && !m_pipelineStarted && !m_timer){
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

        QVariantMap item;
        item["name"] = device.supports(RS2_CAMERA_INFO_NAME)
                           ? QString::fromStdString(device.get_info(RS2_CAMERA_INFO_NAME))
                           : "Unknown RealSense";
        item["serial"] = device.supports(RS2_CAMERA_INFO_SERIAL_NUMBER)
                             ? QString::fromStdString(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER))
                             : "";

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

void CameraWorker::capturePhoto(){
    m_capture = true;
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

    cv::Mat color_bgr;
    const auto colorFormat = color.get_profile().format();
    if (colorFormat == RS2_FORMAT_RGB8) {
        cv::Mat colorRgb(cv::Size(color.get_width(), color.get_height()),
                         CV_8UC3,
                         const_cast<void *>(color.get_data()),
                         cv::Mat::AUTO_STEP);
        cv::cvtColor(colorRgb, color_bgr, cv::COLOR_RGB2BGR);
    } else if (colorFormat == RS2_FORMAT_BGR8) {
        color_bgr = cv::Mat(cv::Size(color.get_width(), color.get_height()),
                            CV_8UC3,
                            const_cast<void *>(color.get_data()),
                            cv::Mat::AUTO_STEP).clone();
    } else {
        qWarning() << "Unsupported color frame format:" << colorFormat;
        return;
    }

    cv::Mat filtered = applyDepthFilters(depth);
    if (yolo26.isLoaded()) {
        cv::Mat personMask = yolo26.Segmatation(color_bgr);
        if (!personMask.empty()) {
            if (personMask.size() != filtered.size()) {
                cv::resize(personMask, personMask, filtered.size(), 0, 0, cv::INTER_NEAREST);
            }

            cv::Mat backgroundMask;
            cv::bitwise_not(personMask, backgroundMask);
            filtered.setTo(cv::Scalar(0), backgroundMask);
        }
    }

    // if (filtered.empty()) return;
    // cv::Mat flickerMask = filter_processing.flickerDetection(depth);
    // if (flickerMask.empty()) return;

    // // 确保掩码和深度图尺寸一致
    // if (flickerMask.size() != filtered.size()) {
    //     cv::resize(
    //         flickerMask,
    //         flickerMask,
    //         filtered.size(),
    //         0,
    //         0,
    //         cv::INTER_NEAREST);
    // }

    // // 将闪烁区域设置为无效深度
    // filtered.setTo(cv::Scalar(0), flickerMask);

    // 对彩色图像进行背景替换
    color_bgr = backgroundRemoval(color_bgr, filtered);

    rs2::frame colorized_frame = colorizer.process(depth);
    if (!colorized_frame) {
        qWarning() << "Colorized depth frame is empty";
        return;
    }
    rs2::video_frame colorized_depth = colorized_frame.as<rs2::video_frame>();

    if (colorized_depth.get_profile().format() != RS2_FORMAT_RGB8) {
        qWarning() << "Unsupported colorized depth format:";
        return;
    }

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

    // save overlay_rgb
    // may optimize with mediapipe
    if(m_capture)try{
        const QString savePath = QString::fromStdString(m_save_path);
        QDir saveDir(savePath);
        if (!saveDir.exists() && !saveDir.mkpath(".")) {
            qWarning() << "Failed to create photo directory:" << savePath;
            m_capture = false;
            return;
        }

        const QString baseName = QDateTime::currentDateTime()
                                     .toString("yyyy-MM-dd_hh-mm-ss-zzz");
        QString fullPath = saveDir.filePath(baseName + ".png");
        int suffix = 1;
        while (QFile::exists(fullPath)) {
            fullPath = saveDir.filePath(
                QString("%1_%2.png").arg(baseName).arg(suffix++));
        }

        if (!cv::imwrite(fullPath.toStdString(), overlay)) {
            qWarning() << "Failed to save photo:" << fullPath;
        } else {
            qDebug() << "Photo saved:" << fullPath;
            // 使用视觉模型进行细化分割
        }
        m_capture=false;
        }catch(const cv::Exception &e){
            qWarning()<<"Fail to save:"<<e.what();
            m_capture = false;
        }

    emit frameReady(qimg.copy());
}catch(const rs2::error &e){
    if(m_running.load()){
        qCritical()<<"Frame processing error:"<<e.what();
    }
    stop();
}catch(const cv::Exception &e){
    if(m_running.load()){
        qCritical()<<"OpenCV frame processing error:"<<e.what();
    }
    stop();
}catch(const std::exception &e){
    if(m_running.load()){
        qCritical()<<"Frame processing exception:"<<e.what();
    }
    stop();
}

cv::Mat CameraWorker::applyDepthFilters(const rs2::depth_frame &depth)
{
    const rs2::depth_frame rsFiltered = filter_processing.applyRsFilters(depth);
    return filter_processing.applyOpenCVFilters(rsFiltered);
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
    const float minDepth = filter_processing.minDepth();
    const float maxDepth = filter_processing.maxDepth();

    // 获取背景图像（应与color同尺寸同类型）
    cv::Mat background = m_background->backgroundForSize(cv::Size(width, height));
    if (background.empty() || background.type() != color.type()) {
        return color;
    }

    // 像素字节数（例如CV_8UC3为3，CV_8UC4为4）
    int elemSize = color.elemSize();
    cv::Mat foregroundMask = cv::Mat::zeros(height, width, CV_8UC1);

#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < processHeight; ++y) {
        // 获取三张图像的行指针
        uchar* colorRow = color.ptr<uchar>(y);
        const uchar* bgRow = background.ptr<const uchar>(y);
        const uint16_t* depthRow = depth.ptr<const uint16_t>(y);
        uchar* maskRow = foregroundMask.ptr<uchar>(y);

        for (int x = 0; x < processWidth; ++x) {
            // 深度值（单位：毫米）
            uint16_t depthValue = depthRow[x];

            // 深度无效或不在当前滤波范围内时，使用背景替换。
            if (depthValue >= minDepth && depthValue <= maxDepth) {
                maskRow[x] = 255;
            } else {
                // 拷贝整个像素（elemSize个字节）
                int offset = x * elemSize;
                memcpy(&colorRow[offset], &bgRow[offset], elemSize);
            }
        }
    }

    // 对图像边缘进行融合处理
    cv::Mat distanceMap;
    cv::distanceTransform(
        foregroundMask,
        distanceMap,
        cv::DIST_L2,
        cv::DIST_MASK_PRECISE);

    float featherRadius = 3.0f;

#pragma omp parallel for schedule(dynamic)
    for(int y=0; y<processHeight; ++y){
        uchar* colorRow = color.ptr<uchar>(y);
        const uchar* bfRow = background.ptr<uchar>(y);
        const float* distRow = distanceMap.ptr<float>(y);

        for(int x=0; x<processWidth; ++x){
            float dist = distRow[x];
            float alpha;
            if(dist>=featherRadius){
                alpha = 1.0f;
            }else{
                float t = dist / featherRadius;
                t = t * t * (3-2*t);
                alpha = 0.5f + 0.5f*t;
            }

            int idx = x* elemSize;
            for(int c = 0; c < elemSize; ++c){
                colorRow[idx + c] = static_cast<uchar>(
                    alpha * colorRow[idx + c] + (1.0f - alpha)*bfRow[idx + c]
                    );
            }
        }
    }

    return color;
}

void CameraWorker::setBackgroundProvider(BackgroundProvider *provider){
    m_background=provider;
}
