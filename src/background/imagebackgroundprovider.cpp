#include "imagebackgroundprovider.h"
#include <QDebug>

ImageBackgroundProvider::ImageBackgroundProvider() {
    m_enable = false;
}

bool ImageBackgroundProvider::isReady() const {
    if(m_enable){
        if(!m_cacheMat.empty())
            return true;
        return false;
    }
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
    cv::Mat image = cv::imread(path.toStdString());
    if(image.empty()){
        qCritical()<<"Failed to load background:"<<path;
        m_enable = false;
        return false;
    }
    m_cacheMat = image;
    m_resizedMat.release();
    m_cachedSize=cv::Size();
    m_enable = true;
    return true;
}
