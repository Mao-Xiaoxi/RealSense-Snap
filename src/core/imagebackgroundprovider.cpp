#include "imagebackgroundprovider.h"
#include <QDebug>

ImageBackgroundProvider::ImageBackgroundProvider() {
}

bool ImageBackgroundProvider::isReady() const {
    if(!m_cacheMat.empty())
        return true;
    return false;
}

cv::Mat ImageBackgroundProvider::backgroundForSize(const cv::Size &size) {
    if (m_cacheMat.empty()) {
        return {};
    }

    if (m_cachedSize == size && !m_resizedMat.empty()) {
        return m_resizedMat;
    }

    cv::resize(m_cacheMat, m_resizedMat, size, 0, 0, cv::INTER_NEAREST);
    m_cachedSize = size;
    return m_resizedMat;
}

bool ImageBackgroundProvider::loadFromFile(const QString &path)
{
    std::string img_path = path.toStdString();
    m_cacheMat = cv::imread(img_path);
    if (m_cacheMat.empty()) {
        qCritical() << "Failed to load background:" << path;
        return false;
    }
    return true;
}
