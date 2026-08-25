#ifndef YOLOSEG_H
#define YOLOSEG_H

#include <memory>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <onnxruntime_cxx_api.h>

/**
 * @brief 基于 YOLO segmentation ONNX 模型的人像分割工具。
 *
 * 该类负责加载 ONNX 模型，将 OpenCV 图像预处理为模型输入，
 * 执行推理，并将模型输出后处理为原图尺寸的二值人像掩码。
 */
class yoloSeg
{
public:
    /**
     * @brief 创建未加载模型的分割器。
     *
     * 调用该构造函数后，需要再调用 loadModel() 加载模型，
     * 否则 Segmatation() 会返回空的 cv::Mat。
     */
    yoloSeg();

    /**
     * @brief 创建分割器并加载指定 ONNX 模型。
     * @param modelPath ONNX 模型文件路径。
     *
     * 如果模型加载失败，isLoaded() 会返回 false。
     */
    explicit yoloSeg(const std::string &modelPath);

    /**
     * @brief 加载或重新加载 ONNX segmentation 模型。
     * @param modelPath ONNX 模型文件路径。
     * @return 加载成功返回 true，加载失败返回 false。
     *
     * 成功后会缓存模型输入/输出名称和输入尺寸信息。
     * 失败时会清空当前 session。
     */
    bool loadModel(const std::string &modelPath);

    /**
     * @brief 判断模型是否已经成功加载。
     * @return 已加载可用模型返回 true，否则返回 false。
     */
    bool isLoaded() const;

    /**
     * @brief 对输入图像执行人像分割推理。
     * @param color 输入图像，支持 BGR、BGRA 或单通道灰度 cv::Mat。
     * @return 单通道二值人像掩码，类型为 CV_8UC1，尺寸与输入图像一致；
     *         人像区域为 255，背景区域为 0。输入为空、模型未加载或推理失败时返回空 cv::Mat。
     *
     * 函数内部会完成 letterbox 缩放、RGB 归一化、ONNX Runtime 推理、
     * person 类筛选、NMS 和 mask prototype 还原。
     */
    cv::Mat Segmatation(const cv::Mat &color);

private:
    /**
     * @brief 记录 letterbox 预处理时的缩放与补边信息。
     *
     * 后处理阶段会使用这些信息把模型坐标和 mask 映射回原图尺寸。
     */
    struct LetterboxInfo {
        float scale = 1.0f;
        int padX = 0;
        int padY = 0;
        int resizedWidth = 0;
        int resizedHeight = 0;
    };

    /**
     * @brief 将输入图像转换为 ONNX 模型输入张量数据。
     * @param color 输入图像，支持 BGR、BGRA 或单通道灰度 cv::Mat。
     * @param info 输出参数，保存缩放比例、补边大小和缩放后尺寸。
     * @return 连续内存的 CHW 格式 float blob，数值范围为 [0, 1]。
     *
     * 预处理流程包括通道转换、等比例缩放、letterbox 补边、
     * BGR 到 RGB 转换、归一化和 HWC 到 CHW 排列。
     */
    cv::Mat preprocess(const cv::Mat &color, LetterboxInfo &info) const;

    /**
     * @brief 将 ONNX 模型输出转换为原图尺寸的人像二值掩码。
     * @param outputs ONNX Runtime 推理输出，通常包含检测输出和 mask prototype 输出。
     * @param info preprocess() 生成的 letterbox 信息。
     * @param originalSize 原始输入图像尺寸。
     * @return 单通道二值人像掩码，类型为 CV_8UC1，尺寸为 originalSize；
     *         无检测结果时返回全黑 mask，输出格式不匹配时返回空 cv::Mat。
     *
     * 后处理流程包括 person 类过滤、置信度筛选、NMS、
     * mask 系数组合、sigmoid、去除 letterbox 补边、缩放回原图和形态学闭运算。
     */
    cv::Mat postprocess(
        const std::vector<Ort::Value> &outputs,
        const LetterboxInfo &info,
        const cv::Size &originalSize) const;

    /**
     * @brief 获取检测置信度阈值。
     * @return 低于该阈值的检测框会被过滤。
     */
    float confidenceThreshold() const;

    /**
     * @brief 获取 mask 二值化阈值。
     * @return mask 概率大于等于该阈值的像素会被视为人像区域。
     */
    float maskThreshold() const;

    /**
     * @brief 获取非极大值抑制阈值。
     * @return NMS 阶段用于过滤重叠检测框的 IoU 阈值。
     */
    float nmsThreshold() const;

    std::string m_modelPath;
    Ort::Env m_env;
    Ort::SessionOptions m_sessionOptions;
    std::unique_ptr<Ort::Session> m_session;
    std::vector<std::string> m_inputNames;
    std::vector<std::string> m_outputNames;
    std::vector<int64_t> m_inputShape;
    int m_inputWidth = 640;
    int m_inputHeight = 640;
};

#endif // YOLOSEG_H
