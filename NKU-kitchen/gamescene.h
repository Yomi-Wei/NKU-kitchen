#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>

class QPainter;

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);

    void updateScoreDisplay(int score);
    // 参数1: 完整的订单描述文本，例如 "当前订单: 沙拉" 或 "所有订单完成!"
    // 参数2: 用于获取订单产品图片的键名，例如 PRODUCT_SALAD
    void updateCurrentOrderDisplay(const QString& currentOrderFullText, const QString& orderProductImageKey);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QPixmap m_backgroundImage;
    QGraphicsTextItem *m_scoreTextItem;
    QGraphicsTextItem *m_currentOrderTextItem;      // 用于显示 "当前订单: XXX" 或 "所有订单完成!"
    QGraphicsPixmapItem *m_currentOrderProductImage; // 用于显示订单成品的图片
};

#endif
