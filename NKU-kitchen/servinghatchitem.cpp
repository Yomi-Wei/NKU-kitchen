#include "servinghatchitem.h"
#include "item_definitions.h" // 包含常量定义
#include <QDebug>

ServingHatchItem::ServingHatchItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_SERVING_HATCH);
    this->setData(ITEM_NAME_ROLE, "上菜口");
}

bool ServingHatchItem::serveOrder(const QString& itemName)
{
    if (itemName == PRODUCT_SALAD) { // 假设 PRODUCT_SALAD = "沙拉" 在 item_definitions.h 中定义
        qDebug() << "上菜口：收到一份 '" << itemName << "'!";
        // 实际游戏中，这里可能还会有更复杂的订单匹配逻辑
        emit orderServed(itemName, 20); // 发出信号，增加20分
        return true;
    } else {
        qDebug() << "上菜口：不接受 '" << itemName << "'。只接受沙拉。";
        return false;
    }
}
