#include "faciallandmark.h"
#include <iostream>  // 用于错误输出

facialLandmark::facialLandmark() {
    m_landmark_model_path = "path/to/your/lbfmodel.yaml";
    m_hear_model_path = "haarcascade_frontalface_alt2.xml";
    m_facemark = cv::face::FacemarkLBF::create();

    if (!loadLandmarkModel(m_landmark_model_path)) {
        std::cerr << "Warning: Failed to load landmark model!" << std::endl;
    }
    if (!loadHearModel(m_hear_model_path)) {
        std::cerr << "Warning: Failed to load Haar cascade!" << std::endl;
    }
}

facialLandmark::~facialLandmark() {
    m_faceDetector.release();
    // m_facemark 会自动释放
}

bool facialLandmark::loadLandmarkModel(const std::string& model_path) {
    try {
        m_facemark->loadModel(model_path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Load landmark model error: " << e.what() << std::endl;
        return false;
    }
}

bool facialLandmark::loadHearModel(const std::string& model_path) {
    if (!m_faceDetector.load(model_path)) {
        std::cerr << "Failed to load Haar cascade: " << model_path << std::endl;
        return false;
    }
    return true;
}

cv::Mat facialLandmark::LandmarkDetection(cv::Mat color) {
    if (color.empty()) {
        std::cerr << "Input image is empty!" << std::endl;
        return color;
    }

    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> faces;
    // 调优参数
    m_faceDetector.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(60, 60));

    if (faces.empty()) {
        std::cout << "No face detected!" << std::endl;
        return color;
    }

    std::vector<std::vector<cv::Point2f>> landmarks;
    bool success = m_facemark->fit(color, faces, landmarks);

    if (!success) {
        std::cout << "Facemark fitting failed!" << std::endl;
        return color;
    }

    // 绘制关键点
    for (const auto& face_landmarks : landmarks) {
        for (const auto& p : face_landmarks) {
            cv::circle(color, p, 2, cv::Scalar(0, 255, 0), -1);
        }
    }

    return color;
}