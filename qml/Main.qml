import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic

import CustomComponents 1.0
import QtQuick.Controls 2.15


ApplicationWindow {
    id: window
    width: 640
    height: 480
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("RealSense Snap")
    property bool lightMode: Application.styleHints.colorScheme === Qt.Light
    property color reallyDark: "#1f1f1f"
    property color dark: "#262626"
    property color reallyLight: "#e7e7e7"
    property color light: "#e0e0e0"
    property color panelBackground: window.lightMode ? "#f2f3f5" : "#242629"
    property color panelSection: window.lightMode ? "#ffffff" : "#303236"
    property color primaryText: window.lightMode ? "#202124" : "#f4f4f4"
    property color secondaryText: window.lightMode ? "#62666d" : "#b8bcc4"
    property color borderColor: window.lightMode ? "#d8dbe0" : "#444850"
    property color accentColor: "#2f7de1"

    // Component.onCompleted: {
    //     cameraController.refreshDevices()
    // }

    GridLayout {
        id: grid
        columns: width < 400 ? 1 : 2
        rowSpacing: 0
        columnSpacing: 0
        anchors.fill: parent

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 320
            color: "black"

            VideoItem {
                id: liveView
                objectName: "liveView"
                anchors.fill: parent
            }
        }

        Rectangle {
            id: rectangle2
            color: window.panelBackground
            Layout.fillHeight: true
            Layout.preferredWidth: grid.columns === 1 ? grid.width : 280
            Layout.minimumWidth: 260
            Layout.maximumWidth: grid.columns === 1 ? grid.width : 340

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Label {
                        text: "RealSense Snap"
                        color: window.primaryText
                        font.pixelSize: 22
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Label {
                        text: cameraController.selectedCameraSerial.length > 0
                              ? "当前设备: " + cameraController.selectedCameraSerial
                              : "请选择可用摄像头"
                        color: window.secondaryText
                        font.pixelSize: 12
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: cameraSection.implicitHeight + 28
                    radius: 8
                    color: window.panelSection
                    border.color: window.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: cameraSection
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 14
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: "摄像头"
                                color: window.primaryText
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            Button {
                                text: "刷新"
                                implicitWidth: 64
                                implicitHeight: 32
                                onClicked: cameraController.refreshDevices()
                            }
                        }

                        ComboBox {
                            id: cameraCombo
                            Layout.fillWidth: true
                            implicitHeight: 36
                            model: cameraController.cameras
                            textRole: "name"

                            delegate: ItemDelegate {
                                width: cameraCombo.width
                                text: modelData.name + (modelData.valid ? "" : " - " + modelData.reason)
                                enabled: modelData.valid
                            }

                            onActivated: {
                                const camera = cameraController.cameras[index]
                                if (camera.valid) {
                                    cameraController.selectedCameraSerial = camera.serial
                                }
                            }
                        }

                        Label {
                            text: cameraController.cameraStatus.length > 0
                                  ? cameraController.cameraStatus
                                  : "已检测到 " + cameraController.cameras.length + " 个设备"
                            color: cameraController.cameraStatus.length > 0 ? "#d45c48" : window.secondaryText
                            font.pixelSize: 12
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: alphaSection.implicitHeight + 28
                    radius: 8
                    color: window.panelSection
                    border.color: window.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: alphaSection
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 14
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "深度叠加强度"
                                color: window.primaryText
                                font.pixelSize: 15
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            Label {
                                text: alphaSlider.value.toFixed(2)
                                color: window.secondaryText
                                font.pixelSize: 13
                            }
                        }

                        Slider {
                            id: alphaSlider
                            Layout.fillWidth: true
                            from: 0.0
                            to: 1.0
                            stepSize: 0.01
                            value: cameraController.alpha
                            onValueChanged: cameraController.alpha = value
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "RGB"
                                color: window.secondaryText
                                font.pixelSize: 12
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Label {
                                text: "Depth"
                                color: window.secondaryText
                                font.pixelSize: 12
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                Button {
                    id: button1
                    text: window.lightMode ? qsTr("切换深色模式")
                                           : qsTr("切换浅色模式")
                    Layout.fillWidth: true
                    implicitHeight: 38

                    contentItem: Text {
                        text: button1.text
                        color: window.lightMode ? window.light : window.dark
                        font: button1.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        radius: 8
                        color: window.lightMode ? window.dark : window.light
                    }

                    onClicked: window.lightMode = !window.lightMode
                }
            }
        }
    }

}
