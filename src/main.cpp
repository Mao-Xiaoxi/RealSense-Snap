#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <QQmlContext>
#include <QDebug>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

#include "CameraWorker.h"
#include "videoitem.h"
#include "cameracontroller.h"


/**
 * @brief 应用程序入口，初始化 QML、相机线程和信号连接。
 * @param argc 命令行参数数量。
 * @param argv 命令行参数数组。
 * @return 应用程序退出码。
 */
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
    QList<QObject*> rootObjs = engine.rootObjects();
    if (rootObjs.isEmpty()) {
        qCritical() << "Failed to load QML root object.";
        return -1;
    }

    auto root = rootObjs.first();
    // 模版参数是指针类型
    auto videoItem = root->findChild<VideoItem*>("liveView");
    if (!videoItem) {
        qCritical() << "Failed to find VideoItem object named liveView.";
        return -1;
    }

    // 信号与槽之间的对接，这里可以理解为接口之间的对接
    QObject::connect(&worker, &CameraWorker::frameReady,
                     videoItem, &VideoItem::setImage,
                     Qt::QueuedConnection);
    QObject::connect(&controller, &CameraController::alphaRequested,
                     &worker, &CameraWorker::setAlpha,
                     Qt::QueuedConnection);

    // controller与worker的对接
    QObject::connect(&controller, &CameraController::refreshDevicesRequested,
                     &worker, &CameraWorker::refreshDevices,
                     Qt::QueuedConnection);
    QObject::connect(&controller,&CameraController::cameraSelected,
                     &worker, &CameraWorker::selectCamera,
                     Qt::QueuedConnection);
    QObject::connect(&controller,&CameraController::photoCaptureRequested,
                     &worker, &CameraWorker::capturePhoto,
                     Qt::QueuedConnection);
    QObject::connect(&controller, &CameraController::backgroundImageRequested,
                     &worker, &CameraWorker::setBackgroundImage,
                     Qt::QueuedConnection);
    QObject::connect(&worker,&CameraWorker::deviceReady,
                     &controller,&CameraController::setDevices,
                     Qt::QueuedConnection);
    QObject::connect(&worker,&CameraWorker::cameraError,
                     &controller,&CameraController::setCameraStatus,
                     Qt::QueuedConnection);
    QObject::connect(&worker,&CameraWorker::selectedCameraChanged,
                     &controller,&CameraController::setSelectedCameraSerialFromWorker,
                     Qt::QueuedConnection);

    // 启动线程并开始工作
    workerThread.start();

    // 分开加载，防止卡死
    QMetaObject::invokeMethod(
        &worker,
        &CameraWorker::initialize,
        Qt::QueuedConnection
        );

    QMetaObject::invokeMethod(
        &worker,
        &CameraWorker::refreshDevices,
        Qt::QueuedConnection
        );

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
