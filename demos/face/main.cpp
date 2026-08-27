#include <iostream>
#include <opencv2/opencv.hpp>
#include "faciallandmark.h"   // 你的封装类头文件

int main() {
    // 1. 创建面部关键点检测器对象
    facialLandmark landmarkDetector;

    // 2. 打开默认摄像头（索引 0）
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "ERROR: Cannot open camera!" << std::endl;
        return -1;
    }

    // 3. 设置摄像头分辨率（可选，降低分辨率可提升速度）
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame;
    std::cout << "Press 'ESC' or 'q' to exit." << std::endl;

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "WARNING: Frame is empty!" << std::endl;
            break;
        }

        // 4. 执行面部关键点检测（绘制结果直接返回）
        cv::Mat result = landmarkDetector.LandmarkDetection(frame);

        // 5. 显示结果
        cv::imshow("Face Landmark Detection", result);

        // 6. 按 ESC (27) 或 'q' 退出
        char key = static_cast<char>(cv::waitKey(30));
        if (key == 27 || key == 'q' || key == 'Q') {
            break;
        }
    }

    // 7. 释放资源
    cap.release();
    cv::destroyAllWindows();

    return 0;
}