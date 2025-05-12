#ifndef FRYINGSTATIONITEM_H
#define FRYINGSTATIONITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include <QTimer>
#include "item_definitions.h"

class QGraphicsRectItem;

class FryingStationItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit FryingStationItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);
    ~FryingStationItem();

    // 放置物品到煎锅，只接受 CUT_MEAT
    bool placeItem(const QString& itemName, const QString& itemImagePath);
    QString getPlacedItemName() const;
    QString takeItem(); // 取走煎好或未煎的物品
    bool isEmpty() const;
    bool isFried() const;
    bool isFrying() const;

    // 开始煎制过程
    bool startFryingProcess(const QString& cutMeatName, const QString& friedMeatImagePath, int durationMs);

    QString getCurrentVisualNameOnBoard() const;

signals:
    // 当煎制动作（定时器结束）完成时发出
    void fryingCompleted(const QString& originalItemName, const QString& friedItemImagePathUsed);

private slots:
    void onFryingTimerTimeout();
    void updateFryingProgress();

private:
    QString m_placedItemName;
    QString m_placedItemImagePath;
    QGraphicsPixmapItem* m_visualItemOnBoard;
    bool m_isFried;
    bool m_isFrying;
    QTimer* m_fryingTimer;
    QString m_pendingFriedItemImagePath;

    //进度条相关成员
    QGraphicsRectItem* m_progressBarBackground;
    QGraphicsRectItem* m_progressBarForeground;
    QTimer* m_progressUpdateTimer;
    int m_totalFryingTimeMs;
    int m_elapsedFryingTimeMs;

    void setupProgressBar();
    void resetProgressBar();
    void showProgressBar();
};

#endif

