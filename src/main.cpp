#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <QQmlContext>
#include "core/CameraWorker.h"
#include "core/videoitem.h"

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<VideoItem>("CustomComponents",1,0,"VideoItem"); // 注册，之后在qml进行相应的导入

    // 线程与Worker准备
    QThread workerThread;
    CameraWorker worker;
    CameraController controller;
    worker.moveToThread(&workerThread); // 线程暴露问题

    // 加载QML
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("cameraController", &controller);
    engine.loadFromModule("RealSense_Snap","Main");

    // 查找UI中的 VideoItem
    auto root = engine.rootObjects().first();
    // 模版参数是指针类型
    auto videoItem = root->findChild<VideoItem*>("liveView");

    // 这里可以理解为接口之间的对接
    QObject::connect(&worker, &CameraWorker::frameReady,
                     videoItem, &VideoItem::setImage,
                     Qt::QueuedConnection);
    QObject::connect(&controller, &CameraController::alphaRequested,
                     &worker, &CameraWorker::setAlpha,
                     Qt::QueuedConnection);

    // 启动线程并开始工作
    workerThread.start();
    QMetaObject::invokeMethod(&worker,&CameraWorker::start,Qt::QueuedConnection);

    //退出清理
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        QMetaObject::invokeMethod(
            &worker,
            &CameraWorker::stop,
            Qt::BlockingQueuedConnection
            );

        workerThread.quit();
        workerThread.wait();

    });

    return QGuiApplication::exec();
}
