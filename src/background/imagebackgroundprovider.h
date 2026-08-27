#ifndef IMAGEBACKGROUNDPROVIDER_H
#define IMAGEBACKGROUNDPROVIDER_H

#include <QString>
#include <opencv2/opencv.hpp>

#include "BackgroundProvider.h"

/**
 * @brief 基于本地图片文件的背景提供器。
 *
 * 该类负责加载一张背景图片，并根据视频帧尺寸缓存 resize 后的背景图。
 * CameraWorker 每帧请求背景时，如果尺寸不变，会复用缓存结果。
 */
class ImageBackgroundProvider : public BackgroundProvider
{
public:
    /**
     * @brief 创建未加载图片的背景提供器。
     */
    ImageBackgroundProvider();

    /**
     * @brief 判断当前是否已经加载有效背景图。
     * @return 背景图片可用返回 true，否则返回 false。
     */
    bool isReady() const override;

    /**
     * @brief 获取指定尺寸的背景图。
     * @param size 目标尺寸。
     * @return resize 后的背景图；未加载图片时返回空 Mat。
     */
    cv::Mat backgroundForSize(const cv::Size &size) override;

    /**
     * @brief 从文件加载新的背景图片。
     * @param path 图片文件路径。
     * @return 加载成功返回 true，失败返回 false。
     *
     * 加载成功后会清空旧的 resize 缓存，下一次 backgroundForSize()
     * 会根据新的背景图片重新生成缓存。
     */
    bool loadFromFile(const QString &path);

private:
    // 背景功能是否启用；只有成功加载图片后才会置为 true。
    bool m_enable;

    // 原始背景图缓存。
    cv::Mat m_cacheMat;

    // 按当前视频尺寸 resize 后的背景图缓存。
    cv::Mat m_resizedMat;

    // m_resizedMat 对应的尺寸，用于判断是否可以复用缓存。
    cv::Size m_cachedSize;
};

#endif // IMAGEBACKGROUNDPROVIDER_H
