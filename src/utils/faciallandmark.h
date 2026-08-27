#ifndef FACIALLANDMARK_H
#define FACIALLANDMARK_H

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>   // 新增！
#include <string>

/**
 * @brief 基于 OpenCV face 模块的人脸关键点检测工具。
 *
 * 该类使用 Haar 级联检测人脸区域，再使用 FacemarkLBF 拟合人脸关键点。
 * 当前主要用于 demo 验证，尚未接入主实时处理流程。
 */
class facialLandmark {
public:
    /**
     * @brief 创建人脸关键点检测器，并尝试加载默认模型。
     */
    facialLandmark();

    /**
     * @brief 释放人脸检测相关资源。
     */
    ~facialLandmark();

    /**
     * @brief 加载 LBF 人脸关键点模型。
     * @param model_path LBF 模型文件路径。
     * @return 加载成功返回 true，失败返回 false。
     */
    bool loadLandmarkModel(const std::string& model_path);  // 改为 bool

    /**
     * @brief 加载 Haar 人脸检测模型。
     * @param model_path Haar cascade XML 文件路径。
     * @return 加载成功返回 true，失败返回 false。
     */
    bool loadHearModel(const std::string& model_path);      // 改为 bool

    /**
     * @brief 检测图像中的人脸关键点并绘制到图像上。
     * @param color 输入 BGR 彩色图。
     * @return 绘制关键点后的图像；检测失败时返回原图。
     */
    cv::Mat LandmarkDetection(cv::Mat color);

private:
    // LBF 关键点拟合器。
    cv::Ptr<cv::face::FacemarkLBF> m_facemark;

    // Haar 人脸检测器，用于先定位人脸矩形区域。
    cv::CascadeClassifier m_faceDetector;

    // LBF 模型路径。
    std::string m_landmark_model_path;

    // Haar cascade 模型路径。
    std::string m_hear_model_path;
};

#endif // FACIALLANDMARK_H
