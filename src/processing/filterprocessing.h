#ifndef FILTERPROCESSING_H
#define FILTERPROCESSING_H

#include <queue>

#include <librealsense2/rs.hpp>
#include <opencv2/core.hpp>

class FilterProcessing
{
public:
    FilterProcessing();

    rs2::depth_frame applyRsFilters(const rs2::depth_frame &depth);
    cv::Mat applyOpenCVFilters(const cv::Mat &depth) const;
    cv::Mat applyOpenCVFilters(const rs2::depth_frame &depth) const;
    cv::Mat flickerDetection(const rs2::depth_frame &depth);

    void setDepthRange(float minDepth, float maxDepth);
    float minDepth() const;
    float maxDepth() const;
    void setBilateralFilter(int diameter, double sigmaColor, double sigmaSpace);
    void setMorphologyKernel(int size);
    void resetRsFilters();

private:
    cv::Mat depthFrameToMat(const rs2::depth_frame &depth) const;
    // 闪烁图像的判断

    cv::Mat maskByKmeans(cv::Mat &smoothed) const;

    rs2::decimation_filter m_decimationFilter;
    rs2::disparity_transform m_depthToDisparity{true};
    rs2::spatial_filter m_spatialFilter;
    rs2::temporal_filter m_temporalFilter;
    rs2::hole_filling_filter m_holeFillingFilter;
    rs2::disparity_transform m_disparityToDepth{false};

    cv::Mat m_previousDepth;
    cv::Mat m_flickerScore;
    int m_depthChangeThreshold = 50;
    int m_flickerThreshold = 10;

    // 前景范围控制
    float m_minDepth = 200.0f;
    float m_maxDepth = 1200.0f;

    int m_bilateralDiameter = 5;
    double m_bilateralSigmaColor = 25.0;
    double m_bilateralSigmaSpace = 25.0;
    int m_morphologyKernelSize3 = 3;
    int m_morphologyKernelSize5 = 5;
};

#endif // FILTERPROCESSING_H
