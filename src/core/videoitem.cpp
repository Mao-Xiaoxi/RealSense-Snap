#include "videoitem.h"
#include <QPainter>      // 提供绘图能力
#include <QDebug>        // 用于调试输出（可选）
#include <QSizeF>        // 用于宽高比计算

VideoItem::VideoItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)   // 必须显式调用基类构造函数，传递父对象
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);

    // 告诉 QML 引擎：该元素不透明，无需绘制其下方内容，节省渲染性能
    setOpaquePainting(true);

    // 启用抗锯齿，让图像边缘更平滑（但会增加性能开销，视情况开启）
    setAntialiasing(true);
}

void VideoItem::paint(QPainter *painter)
{
    if (m_image.isNull()) {
        return;
    }

    QRectF targetRect = boundingRect();
    if (targetRect.isEmpty()) {
        return;
    }

    QSizeF imageSize = m_image.size();
    qreal widthRatio = targetRect.width() / imageSize.width();
    qreal heightRatio = targetRect.height() / imageSize.height();
    qreal scale = qMin(widthRatio, heightRatio); // 取较小比例，保证图像完整显示

    qreal scaledWidth = imageSize.width() * scale;
    qreal scaledHeight = imageSize.height() * scale;
    QRectF drawRect(
        (targetRect.width() - scaledWidth) / 2,   // X 偏移
        (targetRect.height() - scaledHeight) / 2, // Y 偏移
        scaledWidth,
        scaledHeight
        );

    painter->drawImage(drawRect, m_image);
}

void VideoItem::setImage(const QImage &img)
{
    // 存储图像数据（深拷贝由 QImage 的拷贝构造自动完成）
    // 因为是值类型，直接赋值会共享内部数据（写时复制，COW），开销很小
    m_image = img;

    // 发射属性变更信号（如果 QML 绑定了 image 属性，会接收到通知）
    emit imageChanged();

    // 触发 QML 重新绘制（核心步骤）
    update();
}