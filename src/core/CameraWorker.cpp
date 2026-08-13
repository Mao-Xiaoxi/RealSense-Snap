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
    qDebug() << "Starting pipeline...";
    try{
        rs2::config cfg;
        cfg.enable_stream(RS2_STREAM_DEPTH);
        cfg.enable_stream(RS2_STREAM_COLOR);

        pipe.start(cfg);

        while(m_running.load(std::memory_order_acquire)){
            processFrame();
        }

        pipe.stop();
        qDebug()<<"CameraWorker stopped successfully.";
    }catch (const rs2::error &e) {
        qCritical() << "RealSense error:" << e.what();
        // 发生异常时，最好通知主线程发生错误（可以加一个 errorOccurred 信号）
        m_running = false;
        pipe.stop();
    } catch (const std::exception &e) {
        qCritical() << "Standard exception:" << e.what();
        m_running = false;
        pipe.stop();
    }
}

void CameraWorker::stop(){
    bool expected = true;
    if(m_running.compare_exchange_strong(expected,false)){
        qDebug()<<"CameraWorker stop requested";
    }
}

void CameraWorker::processFrame() try
{
    rs2::frameset frames=pipe.wait_for_frames();
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
    cv::addWeighted(depth_bgr,0.3,color_bgr,0.7,0,overlay);

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
}