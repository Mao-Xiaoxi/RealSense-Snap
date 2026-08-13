#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QQuickPaintedItem>
#include <QImage>

// C++默认是private继承，正确写法必须加public
class VideoItem : public QQuickPaintedItem{
    Q_OBJECT    // moc工具，扫描这个类，生成额外代码，实现信号槽、属性系统。
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)  // Qt属性系统，C++属性暴露给QML
public:
    explicit VideoItem(QQuickItem *parent = nullptr);   // 禁止单参数构造函数的隐式类型转换，若构造函数传入参数，则explicit十分重要
    void paint(QPainter *painter) override; //C++11特性，重写父类虚函数
    QImage image() const { return m_image; }    // const表示不会修改类成员变量

// Qt5之后普通的public函数也可以被QML调用，以下是较为清晰的传统写法。
public slots:   // 槽函数标记，可以连接信号-槽，可以被QML直接调用
    void setImage(const QImage &img);

signals:    // 信号段，不需要实现，moc自动实现
    void imageChanged();

private:
    QImage m_image;
};

#endif // VIDEOITEM_H
