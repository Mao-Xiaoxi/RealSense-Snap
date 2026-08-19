#ifndef IMAGEBACKGROUNDPROVIDER_H
#define IMAGEBACKGROUNDPROVIDER_H

#include <QString>
#include <opencv2/opencv.hpp>

#include "BackgroundProvider.h"

// 背景功能的启用和背景图片的选择。

class ImageBackgroundProvider : public BackgroundProvider
{
public:
    ImageBackgroundProvider();

    bool isReady() const override;
    cv::Mat backgroundForSize(const cv::Size &size) override;
    bool loadFromFile(const QString &path);

private:
    bool m_enable;

    cv::Mat m_cacheMat;
    cv::Mat m_resizedMat;
    cv::Size m_cachedSize;
};

#endif // IMAGEBACKGROUNDPROVIDER_H
