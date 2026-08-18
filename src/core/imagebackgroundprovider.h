#ifndef IMAGEBACKGROUNDPROVIDER_H
#define IMAGEBACKGROUNDPROVIDER_H

#include <QString>
#include <opencv2/opencv.hpp>

#include "BackgroundProvider.h"

class ImageBackgroundProvider : public BackgroundProvider
{
public:
    ImageBackgroundProvider();

    bool isReady() const override;
    cv::Mat backgroundForSize(const cv::Size &size) override;
    bool loadFromFile(const QString &path);

private:
    cv::Mat m_cacheMat;
    cv::Mat m_resizedMat;
    cv::Size m_cachedSize;
};

#endif // IMAGEBACKGROUNDPROVIDER_H
