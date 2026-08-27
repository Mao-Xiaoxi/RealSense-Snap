#ifndef BACKGROUNDPROVIDER_H
#define BACKGROUNDPROVIDER_H

#include <opencv2/opencv.hpp>

/**
 * @brief 背景图像提供器的抽象接口。
 *
 * 该接口用于隔离 CameraWorker 和具体背景来源。调用者只关心：
 * 背景是否可用，以及能否得到指定尺寸的背景图。
 */
class BackgroundProvider{
public:
    /**
     * @brief 虚析构函数，保证通过基类指针释放派生类时析构完整。
     */
    virtual ~BackgroundProvider() = default;

    /**
     * @brief 判断背景资源是否已经准备好。
     * @return 可提供背景图返回 true，否则返回 false。
     *
     * const 表示不修改成员变量；=0 表示纯虚函数，派生类必须自己实现。
     */
    virtual bool isReady() const = 0;

    /**
     * @brief 获取指定尺寸的背景图。
     * @param size 目标背景尺寸，通常与当前彩色帧一致。
     * @return 与 size 对应的背景图；资源不可用时返回空 Mat。
     */
    virtual cv::Mat backgroundForSize(const cv::Size &size) = 0;
};

#endif // BACKGROUNDPROVIDER_H
