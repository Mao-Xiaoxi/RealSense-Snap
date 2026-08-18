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

    // 按照执行顺序依次加入算子。
    rs2::decimation_filter m_decimationFilter;

    rs2::spatial_filter m_spatialFilter;
    m_spatialFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, 3);         // 滤波强度
    m_spatialFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.3f);   // 边缘保持参数
    m_spatialFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20);     // 边缘阈值
    m_spatialFilter.set_option(RS2_OPTION_HOLES_FILL, 1);               //填洞模式

    rs2::temporal_filter m_temporalFilter;

    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.1f);  // 历史帧权重
    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 50);    //变化阈值
    m_temporalFilter.set_option(RS2_OPTION_HOLES_FILL, 0);              //填洞模式

    rs2::hole_filling_filter m_holeFilter;
    m_holeFilter.set_option(RS2_OPTION_HOLES_FILL, 1);                // 0：关闭 1:使用周围像素平均 2:使用梯度平均

    rs2::disparity_transform m_depthToDisparity{true};  // 深度转视差
    rs2::disparity_transform m_disparityToDepth{false};

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

    // 多线程处理
    rs2::depth_frame depth=frames.get_depth_frame();
    rs2::video_frame color=frames.get_color_frame();
    if(!depth||!color){
        qWarning()<<"Incomplete frameset receive.d";
        return;
    }
    rs2::depth_frame filtered = applyDepthFilters(depth);
    rs2::video_frame colorized_depth = colorizer.colorize(filtered);

    // 对彩色图像背景进行替换
    color=backgroundRemoval(color,filtered);

    cv::Mat color_rgb(cv::Size(color.get_width(),color.get_height()),CV_8UC3,(void*)color.get_data(),cv::Mat::AUTO_STEP);
    cv::Mat color_bgr;
    cv::cvtColor(color_rgb,color_bgr,cv::COLOR_RGB2BGR);

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


rs2::depth_frame CameraWorker::applyDepthFilters(const rs2::depth_frame &depth)
{
    try{
        rs2::frame filtered = depth;

        //filtered = m_decimationFilter.process(filtered);
        filtered = m_depthToDisparity.process(filtered);
        filtered = m_spatialFilter.process(filtered);
        filtered = m_temporalFilter.process(filtered);
        //filtered = m_holeFilter.process(filtered); // 重点是保证人物被分割出来，空洞填充会导致身体周围阴影与身体混在一起。
        filtered = m_disparityToDepth.process(filtered);

        return filtered.as<rs2::depth_frame>();
    } catch(const rs2::error &e){
        qCritical()<<"Filter failed"<< e.what();
        emit cameraError(QString::fromUtf8(e.what()));

        return depth;
    }
}

/*
 * 自动设置人体和背景的分割阈值，实现人体和背景的分离。
 * 使用 RealSense ID库，检测人脸区域，并以人脸蛇毒像素为基准，计算阈值。
 */
rs2::video_frame CameraWorker::backgroundRemoval(rs2::video_frame &color, const rs2::depth_frame &depth) {

    if (!m_background || !m_background->isReady()) {
        return color;
    }

    uint8_t* p_color_frame =reinterpret_cast<uint8_t*>(const_cast<void*>(color.get_data()));

    int width=color.get_width();
    int height=color.get_height();
    int depthWidth = depth.get_width();
    int depthHeight = depth.get_height();
    int processWidth = qMin(width, depthWidth);
    int processHeight = qMin(height, depthHeight);
    int color_bpp=color.get_bytes_per_pixel();    // 自动获取帧像素字节位数

    cv::Mat background=m_background->backgroundForSize(cv::Size(width, height));

    const uint8_t *p_background = background.data;

    #pragma omp parallel for schedule(dynamic)
    for(int y=0;y<processHeight;y++){
        auto color_pixel_index=y*width;

        for(int x=0;x<processWidth;x++,++color_pixel_index){
            float distance = depth.get_distance(x, y);

            if (distance <= 0.0f || distance > 1.2f){
                auto offset=color_pixel_index * color_bpp;
                std::memcpy(
                    &p_color_frame[offset],
                    &p_background[offset],
                    color_bpp
                    );
            }
        }
    }
    return color;
}

void CameraWorker::setBackgroundProvider(BackgroundProvider *provider){
    m_background=provider;
}
