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
class FryingStationItem;

class Player : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    Player(const QString &pixmapPath, qreal speed = 15.0, QGraphicsItem *parent = nullptr);

    void handleMovementKey(Qt::Key key, const QRectF &sceneBounds);
    void attemptPickupFromSource();
    void attemptInteractWithStation();

    const QMap<QString, int>& getInventory() const;
    static QString getImagePathForItem(const QString& itemName);
    bool isBusy() const;

public slots:
    void onVegetableChopped(const QString& originalVegetableName, const QString& cutImagePathUsedByStation);
    void onMeatFried(const QString& originalCutMeatName, const QString& friedMeatImagePathUsed);

private:
    void addVisualHeldItem(const QString& itemName);
    void updateHeldItemPositions();
    bool tryTakeFromInventory(const QString& itemName, int count = 1);
    void removeVisualHeldItem(const QString& itemName);

    qreal m_moveSpeed;
    QMap<QString, int> m_inventory;
    QList<QGraphicsPixmapItem*> m_visualHeldItems;
    int m_maxVisualHeldItems;
    bool m_isBusyWithStation;
};

#endif
