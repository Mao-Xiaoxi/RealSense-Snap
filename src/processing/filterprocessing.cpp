#include "filterprocessing.h"

#include <opencv2/imgproc.hpp>

FilterProcessing::FilterProcessing()
{
    m_spatialFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, 3.0f);
    m_spatialFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.3f);
    m_spatialFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20.0f);
    m_spatialFilter.set_option(RS2_OPTION_HOLES_FILL, 0.0f);    // 启用内部空洞填充会引发错误

    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.4f);
    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 80.0f);
    m_temporalFilter.set_option(RS2_OPTION_HOLES_FILL, 3.0f);
}

rs2::depth_frame FilterProcessing::applyRsFilters(const rs2::depth_frame &depth)
{
    if (!depth || depth.get_profile().format() != RS2_FORMAT_Z16)
        return depth;

    rs2::frame filtered = depth;


    //filtered = m_decimationFilter.process(filtered);
    filtered = m_depthToDisparity.process(filtered);
    filtered = m_spatialFilter.process(filtered);
    filtered = m_temporalFilter.process(filtered);
    filtered = m_disparityToDepth.process(filtered);
    filtered = m_holeFillingFilter.process(filtered);

    return filtered.as<rs2::depth_frame>();
}

cv::Mat FilterProcessing::applyOpenCVFilters(const cv::Mat &depth) const
{
    if (depth.empty())
        return {};

    cv::Mat depthFloat;
    if (depth.type() == CV_16UC1)
        depth.convertTo(depthFloat, CV_32FC1);
    else if (depth.type() == CV_32FC1)
        depthFloat = depth;
    else
        return {};

    cv::Mat smoothed;
    cv::bilateralFilter(depthFloat,
                        smoothed,
                        m_bilateralDiameter,
                        m_bilateralSigmaColor,
                        m_bilateralSigmaSpace);

    cv::Mat invalidMask;
    cv::compare(depthFloat, 0.0f, invalidMask, cv::CMP_LE);
    smoothed.setTo(0.0f, invalidMask);

    cv::Mat validMask;
    cv::inRange(smoothed, m_minDepth, m_maxDepth, validMask);

    if (m_morphologyKernelSize > 1) {
        const cv::Mat kernel = cv::getStructuringElement(
            cv::MORPH_ELLIPSE,
            cv::Size(m_morphologyKernelSize, m_morphologyKernelSize));
        cv::morphologyEx(validMask, validMask, cv::MORPH_CLOSE, kernel);
    }

    cv::Mat result = cv::Mat::zeros(depthFloat.size(), CV_32FC1);
    depthFloat.copyTo(result, validMask);

    if (depth.type() == CV_16UC1) {
        cv::Mat result16;
        result.convertTo(result16, CV_16UC1);
        return result16;
    }

    return result;
}

cv::Mat FilterProcessing::applyOpenCVFilters(const rs2::depth_frame &depth) const
{
    return applyOpenCVFilters(depthFrameToMat(depth));
}

void FilterProcessing::setDepthRange(float minDepth, float maxDepth)
{
    if (minDepth >= 0.0f && maxDepth > minDepth) {
        m_minDepth = minDepth;
        m_maxDepth = maxDepth;
    }
}

float FilterProcessing::minDepth() const
{
    return m_minDepth;
}

float FilterProcessing::maxDepth() const
{
    return m_maxDepth;
}

void FilterProcessing::setBilateralFilter(int diameter,
                                          double sigmaColor,
                                          double sigmaSpace)
{
    if (diameter > 0 && sigmaColor > 0.0 && sigmaSpace > 0.0) {
        m_bilateralDiameter = diameter;
        m_bilateralSigmaColor = sigmaColor;
        m_bilateralSigmaSpace = sigmaSpace;
    }
}

void FilterProcessing::setMorphologyKernel(int size)
{
    if (size > 0)
        m_morphologyKernelSize = size;
}

void FilterProcessing::resetRsFilters()
{
    m_temporalFilter = rs2::temporal_filter();
    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.2f);
    m_temporalFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 80.0f);
    m_temporalFilter.set_option(RS2_OPTION_HOLES_FILL, 3.0f);
}

cv::Mat FilterProcessing::depthFrameToMat(const rs2::depth_frame &depth) const
{
    if (!depth || depth.get_profile().format() != RS2_FORMAT_Z16
        || depth.get_width() <= 0 || depth.get_height() <= 0)
        return {};

    return cv::Mat(depth.get_height(),
                   depth.get_width(),
                   CV_16UC1,
                   const_cast<void *>(depth.get_data()),
                   depth.get_stride_in_bytes()).clone();
}

cv::Mat FilterProcessing::flickerDetection(const rs2::depth_frame &depth){
    if(!depth || depth.get_profile().format() != RS2_FORMAT_Z16){
        return {};
    }

    const int width = depth.get_width();
    const int height = depth.get_height();

    cv::Mat currentDepth(
        height,
        width,
        CV_16UC1,
        const_cast<void *>(depth.get_data()),
        depth.get_stride_in_bytes());

    if (m_previousDepth.empty() ||
        m_previousDepth.size() != currentDepth.size()) {
        m_previousDepth = currentDepth.clone();
        m_flickerScore = cv::Mat::zeros(height, width, CV_8UC1);
        return cv::Mat::zeros(height, width, CV_8UC1);
    }

    cv::Mat flickerMask = cv::Mat::zeros(height, width, CV_8UC1);
    rs2::depth_frame depth_detected=depth;

#pragma omp parallel for
    for (int y = 0; y < height; ++y) {
        const uint16_t *currentRow = currentDepth.ptr<uint16_t>(y);

        const uint16_t *previousRow = m_previousDepth.ptr<uint16_t>(y);

        uint8_t *scoreRow = m_flickerScore.ptr<uint8_t>(y);

        uint8_t *maskRow = flickerMask.ptr<uint8_t>(y);

        for (int x = 0; x < width; ++x) {
            const uint16_t current = currentRow[x];
            const uint16_t previous = previousRow[x];

            const bool currentValid = current > 0;
            const bool previousValid = previous > 0;

            bool changed = false;

            // 有效/无效状态发生切换
            if (currentValid != previousValid) {
                changed = true;
            }
            // 两帧都有效，但深度变化过大
            else if (currentValid &&
                     std::abs(
                         static_cast<int>(current) -
                         static_cast<int>(previous))
                         > m_depthChangeThreshold) {
                changed = true;
            }

            if (changed) {
                if (scoreRow[x] < 255) {
                    ++scoreRow[x];
                }
            } else {
                if (scoreRow[x] > 0) {
                    --scoreRow[x];
                }
            }

            if (scoreRow[x] >= m_flickerThreshold) {
                maskRow[x] = 255;
            }
        }
    }

    m_previousDepth = currentDepth.clone();

    return flickerMask;
}