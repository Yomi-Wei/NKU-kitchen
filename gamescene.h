#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPixmap> // << 添加 QPixmap 头文件

class QPainter; // 前向声明

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QPixmap m_backgroundImage; // << 用于存储背景图片的成员变量
};

#endif
