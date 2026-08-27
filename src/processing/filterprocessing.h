#ifndef FILTERPROCESSING_H
#define FILTERPROCESSING_H

#include <queue>

#include <librealsense2/rs.hpp>
#include <opencv2/core.hpp>

/**
 * @brief 深度图滤波与前景 mask 后处理工具。
 *
 * 该类封装 RealSense SDK 滤波器和 OpenCV 图像处理流程。
 * CameraWorker 每帧调用它，将原始深度帧转换为更稳定的前景深度图。
 */
class FilterProcessing
{
public:
    /**
     * @brief 创建滤波处理器，并设置默认 RealSense 滤波参数。
     */
    FilterProcessing();

    /**
     * @brief 对 RealSense 深度帧执行 SDK 级滤波。
     * @param depth 原始 RealSense 深度帧，格式应为 RS2_FORMAT_Z16。
     * @return 滤波后的 RealSense 深度帧；输入无效时返回原 depth。
     */
    rs2::depth_frame applyRsFilters(const rs2::depth_frame &depth);

    /**
     * @brief 对 OpenCV 深度图执行平滑、阈值和形态学处理。
     * @param depth 输入深度图，支持 CV_16UC1 或 CV_32FC1。
     * @return 只保留前景范围的深度图，类型与输入尽量保持一致。
     */
    cv::Mat applyOpenCVFilters(const cv::Mat &depth) const;

    /**
     * @brief 将 RealSense 深度帧转换为 cv::Mat 后执行 OpenCV 后处理。
     * @param depth RealSense 深度帧。
     * @return 处理后的深度图。
     */
    cv::Mat applyOpenCVFilters(const rs2::depth_frame &depth) const;

    /**
     * @brief 检测深度图中连续闪烁或异常变化的区域。
     * @param depth 当前帧 RealSense 深度图。
     * @return 单通道 mask，异常闪烁区域为 255，其余为 0。
     */
    cv::Mat flickerDetection(const rs2::depth_frame &depth);

    /**
     * @brief 设置前景深度范围。
     * @param minDepth 最小有效深度，单位通常为毫米。
     * @param maxDepth 最大有效深度，单位通常为毫米。
     */
    void setDepthRange(float minDepth, float maxDepth);

    /**
     * @brief 获取当前最小前景深度。
     * @return 最小深度阈值。
     */
    float minDepth() const;

    /**
     * @brief 获取当前最大前景深度。
     * @return 最大深度阈值。
     */
    float maxDepth() const;

    /**
     * @brief 设置 OpenCV 双边滤波参数。
     * @param diameter 邻域直径。
     * @param sigmaColor 颜色/深度值空间的 sigma。
     * @param sigmaSpace 坐标空间的 sigma。
     */
    void setBilateralFilter(int diameter, double sigmaColor, double sigmaSpace);

    /**
     * @brief 设置形态学操作核大小。
     * @param size 腐蚀核大小，闭运算核会在此基础上稍大一些。
     */
    void setMorphologyKernel(int size);

    /**
     * @brief 重置 RealSense temporal filter 等有状态滤波器。
     */
    void resetRsFilters();

private:
    /**
     * @brief 将 RealSense 深度帧拷贝为 OpenCV Mat。
     * @param depth RealSense 深度帧。
     * @return CV_16UC1 深度图；输入无效时返回空 Mat。
     */
    cv::Mat depthFrameToMat(const rs2::depth_frame &depth) const;

    /**
     * @brief 使用 K-Means 根据深度值估计前景区域。
     * @param smoothed 已平滑的深度图。
     * @return 单通道前景 mask，前景为 255。
     */
    cv::Mat maskByKmeans(cv::Mat &smoothed) const;

    // RealSense SDK 滤波器对象，其中 temporal filter 会保留跨帧状态。
    rs2::decimation_filter m_decimationFilter;
    rs2::disparity_transform m_depthToDisparity{true};
    rs2::spatial_filter m_spatialFilter;
    rs2::temporal_filter m_temporalFilter;
    rs2::hole_filling_filter m_holeFillingFilter;
    rs2::disparity_transform m_disparityToDepth{false};

    // 闪烁检测使用的上一帧深度图和累积分数。
    cv::Mat m_previousDepth;
    cv::Mat m_flickerScore;

    // 单像素深度变化超过该阈值时，认为该像素发生异常变化。
    int m_depthChangeThreshold = 50;

    // 闪烁分数超过该值时，将像素标记为异常区域。
    int m_flickerThreshold = 10;

    // 前景范围控制
    float m_minDepth = 200.0f;
    float m_maxDepth = 1200.0f;

    // 双边滤波参数，用于平滑深度图并尽量保留边缘。
    int m_bilateralDiameter = 5;
    double m_bilateralSigmaColor = 25.0;
    double m_bilateralSigmaSpace = 25.0;

    // 形态学处理核大小，分别用于腐蚀和闭运算。
    int m_morphologyKernelSize3 = 3;
    int m_morphologyKernelSize5 = 7;
};

#endif // FILTERPROCESSING_H
