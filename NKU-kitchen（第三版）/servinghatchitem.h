// servinghatchitem.h
#ifndef SERVINGHATCHITEM_H
#define SERVINGHATCHITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include "item_definitions.h"


class ServingHatchItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit ServingHatchItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);
    bool serveOrder(const QString& itemName);
signals:
    void orderServed(const QString& itemName, int points);
};

#endif
