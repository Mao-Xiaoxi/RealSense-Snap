#include "cameracontroller.h"
#include <QDebug>
#include <QThread>
#include <opencv2/opencv.hpp>

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

void CameraController::setBackgroundImage(QString path){
    if(path.isEmpty()){
        setCameraStatus("背景图片路径为空");
            return;
    }
    emit backgroundImageRequested(path);
}

void CameraController::capturePhoto(){
    emit photoCaptureRequested();
}
