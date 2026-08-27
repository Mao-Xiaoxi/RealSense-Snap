#ifndef FACIALLANDMARK_H
#define FACIALLANDMARK_H

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>   // 新增！
#include <string>

class facialLandmark {
public:
    facialLandmark();
    ~facialLandmark();

    bool loadLandmarkModel(const std::string& model_path);  // 改为 bool
    bool loadHearModel(const std::string& model_path);      // 改为 bool

    cv::Mat LandmarkDetection(cv::Mat color);

private:
    cv::Ptr<cv::face::FacemarkLBF> m_facemark;
    cv::CascadeClassifier m_faceDetector;
    std::string m_landmark_model_path;
    std::string m_hear_model_path;
};

#endif // FACIALLANDMARK_H