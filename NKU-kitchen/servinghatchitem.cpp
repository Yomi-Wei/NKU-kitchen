#include "servinghatchitem.h"
#include "item_definitions.h"
#include <QDebug>

ServingHatchItem::ServingHatchItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_SERVING_HATCH);
    this->setData(ITEM_NAME_ROLE, "上菜口");
}

bool ServingHatchItem::serveOrder(const QString& itemName)
{
    // 检查是否是沙拉或汉堡
    if (itemName == PRODUCT_SALAD) {
        qDebug() << "上菜口：收到一份 '" << itemName << "'!";
        emit orderServed(itemName, 20); // 沙拉20分
        return true;
    } else if (itemName == PRODUCT_BURGER) {
        qDebug() << "上菜口：收到一份 '" << itemName << "'!";
        emit orderServed(itemName, 50); // 汉堡可以给更多分，例如50分
        return true;
    } else {
        qDebug() << "上菜口：不接受 '" << itemName << "'。只接受沙拉或汉堡。";
        return false;
    }
}

