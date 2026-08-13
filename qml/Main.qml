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
    title: qsTr("Hello World")
    property bool lightMode: Application.styleHints.colorScheme === Qt.Light
    property color reallyDark: "#1f1f1f"
    property color dark: "#262626"
    property color reallyLight: "#e7e7e7"
    property color light: "#e0e0e0"



    GridLayout {
        id: grid
        columns: width < 400 ? 1 : 2
        rowSpacing: 0
        columnSpacing: 0
        anchors.fill: parent

        Rectangle {
           Layout.fillHeight: true
           Layout.fillWidth: true
           color: "black"

           VideoItem {
               id: liveView
               objectName: "liveView"
               anchors.fill: parent
           }
       }

        Rectangle {
            id: rectangle2
            color: window.lightMode ? window.light : window.dark
            Layout.fillHeight: true
            Layout.fillWidth: true

            Label {
                text: "Realsense Camera"
                anchors.centerIn: parent
                color: "white"
                font.pixelSize: 32
                style: Text.Outline
                styleColor: "black"
            }
            Slider {
                id: alphaSlider
                from: 0.0
                to: 1.0
                stepSize: 0.01
                value: cameraController.alpha   // 从 C++ 读取初始值
                onValueChanged: {
                    // 拖动时更新 C++ 的 alpha 值
                    cameraController.alpha = value
                }
            }
            Label {
                text: "Alpha: " + alphaSlider.value.toFixed(2)
            }
            ColumnLayout {
                anchors.fill: parent
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                Button {
                    id: button1
                    text: window.lightMode ? qsTr("\u263D  Dark mode")
                                           : qsTr("\u263C  Light mode")
                    Layout.bottomMargin: 16
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

                    contentItem: Text {
                        text: button1.text
                        color: window.lightMode ? window.light : window.dark
                        font: button1.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 36
                        radius: 8
                        color: window.lightMode ? window.dark : window.light
                    }

                    onClicked: window.lightMode = !window.lightMode
                }
            }
        }
    }

}
