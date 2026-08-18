#ifndef BACKGROUNDPROVIDER_H
#define BACKGROUNDPROVIDER_H

#include <opencv2/opencv.hpp>

// 虚函数，实现CameraWorker类与背景类的对接

class BackgroundProvider{
public:
    // = default 表示若无实现，编译器自动生成
    virtual ~BackgroundProvider() = default;
    // const 表示不修改成员变量；=0表示不作实现，继承的子类自己实现它。
    virtual bool isReady() const = 0;
    virtual cv::Mat backgroundForSize(const cv::Size &size) = 0;
};

#endif // BACKGROUNDPROVIDER_H
