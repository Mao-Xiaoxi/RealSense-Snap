#include <iostream>
#include <opencv2/opencv.hpp>
#include <yoloseg.h>

int main()
{
    // 打开默认摄像头（索引 0）
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "错误：无法打开摄像头！" << std::endl;
        return -1;
    }

    std::string model_path="/Users/maoxiaoxi/Documents/code/C++/Qt/RealSense_Snap/resources/models/yolo26n-seg.onnx";
    yoloSeg yolo(model_path);

    if(!yolo.isLoaded()){
        std::cout<<"模型加载失败"<<std::endl;
        return -1;
    }

    // 创建显示窗口
    cv::namedWindow("摄像头实时视频", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("人像掩码", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("抠图结果", cv::WINDOW_AUTOSIZE);

    cv::Mat frame;
    int frameCount = 0;
    while (true) {
        // 读取一帧
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "警告：读取帧失败，可能摄像头已断开。" << std::endl;
            break;
        }

        cv::Mat mask = yolo.Segmatation(frame);
        cv::Mat cutout = cv::Mat::zeros(frame.size(), frame.type());
        if (!mask.empty()) {
            frame.copyTo(cutout, mask);

            cv::imshow("人像掩码", mask);
            cv::imshow("抠图结果", cutout);
        } else {
            if (frameCount % 30 == 0) {
                std::cerr << "警告：模型没有返回有效 mask，请检查模型输出解析或画面中是否有人。" << std::endl;
            }
            cv::Mat emptyMask = cv::Mat::zeros(frame.size(), CV_8UC1);
            //cv::imshow("人像掩码", emptyMask);
            cv::imshow("抠图结果", cutout);
        }
        ++frameCount;

        // 显示当前帧，直接接在后边会看不出前面的帧
        //cv::imshow("摄像头实时视频", frame);

        // 等待按键，若按下 ESC (ASCII 27) 或 'q' 则退出
        char key = static_cast<char>(cv::waitKey(30));
        if (key == 27 || key == 'q' || key == 'Q') {
            break;
        }
    }

    // 释放摄像头资源并关闭窗口
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
