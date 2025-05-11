#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsTextItem> // << 新增：用于显示分数

class QPainter; // 前向声明

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);

    void updateScoreDisplay(int score); // << 新增：更新分数显示的方法

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QPixmap m_backgroundImage; // 用于存储背景图片的成员变量
    QGraphicsTextItem *m_scoreTextItem; // << 新增：用于显示分数的文本对象
};

#endif // GAMESCENE_H
