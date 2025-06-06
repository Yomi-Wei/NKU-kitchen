#ifndef ITEM_DEFINITIONS_H
#define ITEM_DEFINITIONS_H

#include <QString>
#include <QtGlobal> // For Qt::UserRole

// 用于 QGraphicsItem::setData() 的键 (Roles)
const int ITEM_TYPE_ROLE = Qt::UserRole + 1;
const int ITEM_NAME_ROLE = Qt::UserRole + 2;
const int ITEM_IMAGE_PATH_ROLE = Qt::UserRole + 3;

// 物品类型字符串常量
const QString TYPE_OBSTACLE = "obstacle";
const QString TYPE_COLLECTIBLE_VEGETABLE_SOURCE = "collectible_vegetable_source";
const QString TYPE_CHOPPING_STATION = "chopping_station";
const QString TYPE_SALAD_ASSEMBLY_STATION = "salad_assembly_station";
const QString TYPE_SERVING_HATCH = "serving_hatch";

// 具体物品名称常量
const QString RAW_TOMATO = "西红柿";
const QString RAW_LETTUCE = "生菜";
const QString CUT_TOMATO = RAW_TOMATO + "_已切";
const QString CUT_LETTUCE = RAW_LETTUCE + "_已切";
const QString PRODUCT_SALAD = "沙拉";

#endif
