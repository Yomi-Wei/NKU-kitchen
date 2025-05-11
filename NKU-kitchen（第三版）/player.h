#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QRectF>
#include <QtGlobal>
#include <QMap>
#include <QString>
#include <QList>
#include "item_definitions.h"

class ChoppingStationItem;
class SaladAssemblyStationItem;
class ServingHatchItem;

class Player : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT // 确保 Q_OBJECT 存在
public:
    Player(const QString &pixmapPath, qreal speed = 15.0, QGraphicsItem *parent = nullptr);

    void handleMovementKey(Qt::Key key, const QRectF &sceneBounds);
    void attemptPickupFromSource();
    void attemptInteractWithStation();

    const QMap<QString, int>& getInventory() const;
    static QString getImagePathForItem(const QString& itemName);
    bool isBusy() const;

public slots: // 将 onVegetableChopped 声明为 public slot
    void onVegetableChopped(const QString& originalVegetableName, const QString& cutImagePathUsedByStation);

private:
    void addVisualHeldItem(const QString& itemName);
    void updateHeldItemPositions();
    bool tryTakeFromInventory(const QString& itemName, int count = 1);
    void removeVisualHeldItem(const QString& itemName);

    qreal m_moveSpeed;
    QMap<QString, int> m_inventory;
    QList<QGraphicsPixmapItem*> m_visualHeldItems;
    int m_maxVisualHeldItems;
    bool m_isBusyWithStation; // 用于标记玩家是否在进行耗时操作
};

#endif
