// servinghatchitem.h
#ifndef SERVINGHATCHITEM_H
#define SERVINGHATCHITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include "item_definitions.h" // << 确保包含了这个，并且移除了下面可能存在的重复定义

// 移除下面这行，如果它存在于你的 servinghatchitem.h 中:
// const QString TYPE_SERVING_HATCH = "serving_hatch"; // << 删除或注释掉这行

class ServingHatchItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit ServingHatchItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);
    bool serveOrder(const QString& itemName);
signals:
    void orderServed(const QString& itemName, int points);
};

#endif // SERVINGHATCHITEM_H
