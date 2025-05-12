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
const QString TYPE_FRYING_STATION = "frying_station";
const QString TYPE_SALAD_ASSEMBLY_STATION = "salad_assembly_station";
const QString TYPE_SERVING_HATCH = "serving_hatch";

// 具体物品名称常量
const QString RAW_TOMATO = "西红柿";
const QString RAW_LETTUCE = "生菜";
const QString RAW_MEAT = "生肉";
const QString BREAD_SLICE = "面包片";

const QString CUT_TOMATO = RAW_TOMATO + "_已切";
const QString CUT_LETTUCE = RAW_LETTUCE + "_已切";
const QString CUT_MEAT = RAW_MEAT + "_已切";

const QString FRIED_MEAT = CUT_MEAT + "_已煎";

const QString PRODUCT_SALAD = "沙拉";
const QString PRODUCT_BURGER = "汉堡";

const QString CHARACTER_A_ID = "player_a"; // 角色A的ID
const QString CHARACTER_B_ID = "player_b"; // 角色B的ID

// 角色对应的图片路径
const QString CHARACTER_A_SPRITE_PATH = ":/images/player_character.png"; // 现有角色图片
const QString CHARACTER_B_SPRITE_PATH = ":/images/player_b_character.png"; // 新角色图片

#endif
