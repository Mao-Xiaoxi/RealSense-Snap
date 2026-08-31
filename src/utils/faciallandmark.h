#ifndef FACIALLANDMARK_H
#define FACIALLANDMARK_H

#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

/**
 * @brief 基于 YOLOv8 face ONNX 模型的人脸关键点检测工具。
 *
 * 模型输出人脸检测框和 5 个关键点：左眼、右眼、鼻尖、左嘴角、右嘴角。
 */
class facialLandmark {
public:
    struct Face {
        cv::Rect rect;
        float confidence = 0.0f;
        std::vector<cv::Point> landmarks;
    };

    /**
     * @brief 创建检测器，并尝试加载 resources/models/yolov8n-face.onnx。
     */
    facialLandmark();

    /**
     * @brief 创建检测器，并加载指定 ONNX 模型。
     */
    explicit facialLandmark(const std::string &modelPath);

    ~facialLandmark();

    /**
     * @brief 加载或重新加载 YOLOv8 face ONNX 模型。
     * @param modelPath ONNX 模型文件路径。
     * @return 加载成功返回 true，失败返回 false。
     */
    bool loadModel(const std::string &modelPath);

    /**
     * @brief 兼容旧接口：加载 YOLOv8 face ONNX 模型。
     */
    bool loadLandmarkModel(const std::string &modelPath);

    /**
     * @brief 兼容旧接口：YOLOv8 face 不再需要 Haar 模型。
     */
    bool loadHearModel(const std::string &modelPath);

    /**
     * @brief 判断模型是否已经加载成功。
     */
    bool isLoaded() const;

    /**
     * @brief 检测图像中的人脸框和 5 个关键点。
     */
    std::vector<Face> detect(const cv::Mat &color);

    /**
     * @brief 检测图像中的人脸关键点并绘制到图像上。
     * @param color 输入 BGR/BGRA/灰度图。
     * @return 绘制检测结果后的图像；失败时返回原图。
     */
    cv::Mat LandmarkDetection(cv::Mat color);

private:
    struct LetterboxInfo {
        int resizedWidth = 0;
        int resizedHeight = 0;
        int padX = 0;
        int padY = 0;
        float ratioX = 1.0f;
        float ratioY = 1.0f;
    };

    cv::Mat resizeImage(const cv::Mat &src, LetterboxInfo &info) const;
    void generateProposal(
        const cv::Mat &output,
        std::vector<cv::Rect> &boxes,
        std::vector<float> &confidences,
        std::vector<std::vector<cv::Point>> &landmarks,
        const cv::Size &originalSize,
        const LetterboxInfo &info) const;
    static void softmax(const float *src, float *dst, int length);
    static float sigmoid(float value);
    static std::string defaultModelPath();

    cv::dnn::Net m_net;
    std::string m_modelPath;
    bool m_loaded = false;
    int m_inputWidth = 640;
    int m_inputHeight = 640;
    float m_confidenceThreshold = 0.45f;
    float m_nmsThreshold = 0.5f;
};

#endif // FACIALLANDMARK_H
