import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Dialogs

import CustomComponents 1.0
import QtQuick.Controls 2.15


ApplicationWindow {
    id: window
    width: 1180
    height: 760
    minimumWidth: 860
    minimumHeight: 560
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
    property color accentHover: "#256fc9"
    property color accentPressed: "#1f5fae"
    property int sidePanelWidth: 340

    // Component.onCompleted: {
    //     cameraController.refreshDevices()
    // }

    color: window.lightMode ? "#e8eaee" : "#191b1f"

    FileDialog {
        id: backgroundDialog
        title: "选择背景图片"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.bmp)"]

        onAccepted: {
            cameraController.setBackgroundImage(selectedFile.toString().replace("file://", ""))
        }
    }

    GridLayout {
        id: grid
        columns: width < 920 ? 1 : 2
        rowSpacing: 0
        columnSpacing: 0
        anchors.fill: parent
        opacity: 0.0
        transform: Translate {
            id: enterOffset
            y: 10
        }

        Component.onCompleted: {
            enterAnimation.start()
        }

        ParallelAnimation {
            id: enterAnimation
            NumberAnimation {
                target: grid
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 360
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: enterOffset
                property: "y"
                from: 10
                to: 0
                duration: 360
                easing.type: Easing.OutCubic
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 820
            Layout.preferredHeight: 620
            color: window.lightMode ? "#111318" : "#07080a"

            VideoItem {
                id: liveView
                objectName: "liveView"
                anchors.fill: parent
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: window.lightMode ? "#00000022" : "#ffffff18"
                border.width: 1
            }
        }

        Rectangle {
            id: rectangle2
            color: window.panelBackground
            Layout.fillHeight: true
            Layout.preferredWidth: grid.columns === 1 ? grid.width : window.sidePanelWidth
            Layout.minimumWidth: 320
            Layout.maximumWidth: grid.columns === 1 ? grid.width : 390

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 26
                spacing: 20

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 7

                    Label {
                        text: "RealSense Snap"
                        color: window.primaryText
                        font.pixelSize: 28
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Label {
                        text: cameraController.selectedCameraSerial.length > 0
                              ? "当前设备: " + cameraController.selectedCameraSerial
                              : "请选择可用摄像头"
                        color: window.secondaryText
                        font.pixelSize: 13
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: cameraSection.implicitHeight + 32
                    radius: 8
                    color: window.panelSection
                    border.color: window.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: cameraSection
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 16
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: "摄像头"
                                color: window.primaryText
                                font.pixelSize: 16
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            Button {
                                id: refreshButton
                                text: "刷新"
                                implicitWidth: 72
                                implicitHeight: 34
                                scale: pressed ? 0.98 : 1.0
                                onClicked: cameraController.refreshDevices()

                                Behavior on scale {
                                    NumberAnimation {
                                        duration: 90
                                        easing.type: Easing.OutCubic
                                    }
                                }

                                contentItem: Text {
                                    text: refreshButton.text
                                    color: window.primaryText
                                    font.pixelSize: 13
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                background: Rectangle {
                                    radius: 8
                                    color: refreshButton.pressed
                                           ? (window.lightMode ? "#dfe4ec" : "#3c4047")
                                           : refreshButton.hovered
                                             ? (window.lightMode ? "#eef2f7" : "#373a40")
                                             : window.panelSection
                                    border.color: window.borderColor
                                    border.width: 1

                                    Behavior on color {
                                        ColorAnimation { duration: 120 }
                                    }
                                }
                            }
                        }

                        ComboBox {
                            id: cameraCombo
                            Layout.fillWidth: true
                            implicitHeight: 40
                            model: cameraController.cameras
                            textRole: "name"

                            delegate: ItemDelegate {
                                width: cameraCombo.width
                                text: modelData.name + (modelData.valid ? "" : " - " + modelData.reason)
                                enabled: modelData.valid
                            }

                            onActivated: function(index) {
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
                            font.pixelSize: 13
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true

                            Behavior on color {
                                ColorAnimation { duration: 160 }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: alphaSection.implicitHeight + 32
                    radius: 8
                    color: window.panelSection
                    border.color: window.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: alphaSection
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 16
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "深度叠加强度"
                                color: window.primaryText
                                font.pixelSize: 16
                                font.bold: true
                                Layout.fillWidth: true
                            }

                            Label {
                                text: alphaSlider.value.toFixed(2)
                                color: window.secondaryText
                                font.pixelSize: 14
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

                            Behavior on value {
                                NumberAnimation {
                                    duration: 120
                                    easing.type: Easing.OutCubic
                                }
                            }
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

                Button {
                    id: backgroundButton
                    text: "切换背景"
                    Layout.fillWidth: true
                    implicitHeight: 42
                    scale: pressed ? 0.985 : 1.0
                    onClicked: backgroundDialog.open()

                    Behavior on scale {
                        NumberAnimation {
                            duration: 90
                            easing.type: Easing.OutCubic
                        }
                    }

                    contentItem: Text {
                        text: backgroundButton.text
                        color: window.primaryText
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        radius: 8
                        color: backgroundButton.pressed
                               ? (window.lightMode ? "#dfe4ec" : "#3c4047")
                               : backgroundButton.hovered
                                 ? (window.lightMode ? "#eef2f7" : "#373a40")
                                 : window.panelSection
                        border.color: window.borderColor
                        border.width: 1

                        Behavior on color {
                            ColorAnimation { duration: 120 }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                Button {
                    id: button1
                    text: "拍照"
                    Layout.fillWidth: true
                    implicitHeight: 46
                    scale: pressed ? 0.97 : 1.0
                    opacity: pressed ? 0.82 : 1.0

                    Behavior on scale {
                        NumberAnimation {
                            duration: 90
                            easing.type: Easing.OutCubic
                        }
                    }

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 90
                            easing.type: Easing.OutCubic
                        }
                    }

                    contentItem: Text {
                        text: button1.text
                        color: window.lightMode ? window.light : window.dark
                        font.pixelSize: 15
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        radius: 8
                        color: button1.pressed
                               ? (window.lightMode ? "#4b5563" : "#d1d5db")
                               : (window.lightMode ? window.dark : window.light)

                        Behavior on color {
                            ColorAnimation { duration: 120 }
                        }
                    }

                    onClicked: cameraController.capturePhoto();
                }
            }
        }
    }

}
