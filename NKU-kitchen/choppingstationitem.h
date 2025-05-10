#ifndef CHOPPINGSTATIONITEM_H
#define CHOPPINGSTATIONITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include "item_definitions.h" // << 包含共享常量定义

class ChoppingStationItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit ChoppingStationItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);

    bool placeVegetable(const QString& vegetableName, const QString& vegetableImagePath);
    QString getPlacedVegetableName() const;
    QString takeVegetable();
    bool isEmpty() const;
    bool isCut() const;
    bool chopVegetable(const QString& cutVegetableImagePath);
    QString getCurrentVisualNameOnBoard() const;

private:
    QString m_placedVegetableName;
    QString m_placedVegetableImagePath;
    QGraphicsPixmapItem* m_visualVegetableOnBoard;
    bool m_isCut;
};

#endif // CHOPPINGSTATIONITEM_H
