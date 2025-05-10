// item_definitions.h
#ifndef ITEM_DEFINITIONS_H
#define ITEM_DEFINITIONS_H

#include <QString>
#include <QtGlobal>

// ... (其他 ITEM_TYPE_ROLE, ITEM_NAME_ROLE, ITEM_IMAGE_PATH_ROLE 常量) ...
const int ITEM_TYPE_ROLE = Qt::UserRole + 1;
const int ITEM_NAME_ROLE = Qt::UserRole + 2;
const int ITEM_IMAGE_PATH_ROLE = Qt::UserRole + 3;


// ... (其他 TYPE_OBSTACLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE 等常量) ...
const QString TYPE_OBSTACLE = "obstacle";
const QString TYPE_COLLECTIBLE_VEGETABLE_SOURCE = "collectible_vegetable_source";
const QString TYPE_CHOPPING_STATION = "chopping_station";
const QString TYPE_SALAD_ASSEMBLY_STATION = "salad_assembly_station";
const QString TYPE_SERVING_HATCH = "serving_hatch"; // << 新增上菜口类型

// 成品名称
const QString PRODUCT_SALAD = "沙拉";

#endif // ITEM_DEFINITIONS_H
