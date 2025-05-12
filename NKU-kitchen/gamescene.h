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
    void updateCurrentOrderDisplay(const QString& currentOrderFullText, const QString& orderProductImageKey);
    void updateOrderTimerDisplay(int secondsLeft);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QPixmap m_backgroundImage;
    QGraphicsTextItem *m_scoreTextItem;
    QGraphicsTextItem *m_currentOrderTextItem;
    QGraphicsPixmapItem *m_currentOrderProductImage;
    QGraphicsTextItem *m_orderTimerTextItem;
};

#endif // GAMESCENE_H

