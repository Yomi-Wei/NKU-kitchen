#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QObject>
// ... 其他 includes ...
#include "item_definitions.h" // << 确保包含

class ChoppingStationItem;
class SaladAssemblyStationItem;
class ServingHatchItem; // << 前向声明上菜口

class Player : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    // ... (构造函数和现有方法声明不变) ...
    Player(const QString &pixmapPath, qreal speed = 10.0, QGraphicsItem *parent = nullptr);
    void handleMovementKey(Qt::Key key, const QRectF &sceneBounds);
    void attemptPickupFromSource();
    void attemptInteractWithStation();
    const QMap<QString, int>& getInventory() const;
    static QString getImagePathForItem(const QString& itemName);

private:
    // ... (私有方法和成员变量与之前相同) ...
    void addVisualHeldItem(const QString& itemName);
    void updateHeldItemPositions();
    bool tryTakeFromInventory(const QString& itemName, int count = 1);
    void removeVisualHeldItem(const QString& itemName);

    qreal m_moveSpeed;
    QMap<QString, int> m_inventory;
    QList<QGraphicsPixmapItem*> m_visualHeldItems;
    int m_maxVisualHeldItems;
};

#endif // PLAYER_H
