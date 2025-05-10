// player.cpp
#include "player.h"
#include "choppingstationitem.h"      // 确保这些类的前向声明在 player.h 中，完整包含在这里
#include "saladassemblystationitem.h" // 确保这些类的前向声明在 player.h 中，完整包含在这里
#include "servinghatchitem.h"         // 确保这些类的前向声明在 player.h 中，完整包含在这里
#include <QPixmap>
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsScene>

// --- 图片路径映射表 ---
// (确保这些路径与你的 .qrc 文件中的资源路径一致)
// (确保 item_definitions.h 中的 PRODUCT_SALAD 常量已定义，例如 const QString PRODUCT_SALAD = "沙拉";)
QMap<QString, QString> Player_ItemBaseImagePaths_map_cpp = {
    {"西红柿", ":/images/tomato_raw.png"},
    {"生菜", ":/images/lettuce_raw.png"},
    {PRODUCT_SALAD, ":/images/salad_final.png"} // 使用常量
};
QMap<QString, QString> Player_ItemCutImagePaths_map_cpp = {
    {"西红柿", ":/images/tomato_cut.png"},
    {"生菜", ":/images/lettuce_sliced.png"}
};

// --- Player 类的静态成员函数定义 ---
QString Player::getImagePathForItem(const QString& itemName) {
    QString baseName = itemName;
    bool isCut = false;
    if (itemName.endsWith("_已切")) {
        baseName.chop(3); // 移除 "_已切" 后缀
        isCut = true;
    }

    if (isCut) {
        return Player_ItemCutImagePaths_map_cpp.value(baseName, QString());
    } else {
        return Player_ItemBaseImagePaths_map_cpp.value(baseName, QString());
    }
}

// --- Player 类的构造函数定义 ---
Player::Player(const QString &pixmapPath, qreal speed, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(parent), m_moveSpeed(speed), m_maxVisualHeldItems(3)
{
    QPixmap originalPixmap(pixmapPath);
    if (originalPixmap.isNull()) {
        qWarning() << "错误：从路径加载玩家图片失败：" << pixmapPath;
        QPixmap fallbackPixmap(50, 50); // 你之前截图的 player.h 默认 speed=15, 这里是 50x50
        fallbackPixmap.fill(Qt::red);
        setPixmap(fallbackPixmap);
        qWarning() << "已使用备用红色方块作为玩家。";
    } else {
        // 你之前上传的 player.cpp 中图片缩放是 100,100。
        // 我之前的示例是 50,50。请根据你的期望调整。
        QPixmap scaledPixmap = originalPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setPixmap(scaledPixmap);
        qDebug() << "玩家图片已加载并缩放。尺寸：" << pixmap().size();
    }
}

// --- Player 类的其他成员函数定义 ---
void Player::handleMovementKey(Qt::Key key, const QRectF &sceneBounds)
{
    if (!scene()) { qWarning() << "玩家不在场景中。无法移动。"; return; }
    QRectF playerLocalBounds = this->boundingRect();
    QPointF currentPos = this->pos();
    qreal tentativeNewX = currentPos.x(), tentativeNewY = currentPos.y();

    switch (key) {
    case Qt::Key_A: case Qt::Key_Left: tentativeNewX -= m_moveSpeed; break;
    case Qt::Key_D: case Qt::Key_Right: tentativeNewX += m_moveSpeed; break;
    case Qt::Key_W: case Qt::Key_Up: tentativeNewY -= m_moveSpeed; break;
    case Qt::Key_S: case Qt::Key_Down: tentativeNewY += m_moveSpeed; break;
    default: return;
    }

    QRectF nextPotentialRect(QPointF(tentativeNewX, tentativeNewY), playerLocalBounds.size());
    bool collisionWithObstacle = false;
    QList<QGraphicsItem*> potentialCollisions = scene()->items(nextPotentialRect, Qt::IntersectsItemShape);
    for (QGraphicsItem *item : potentialCollisions) {
        if (item == this) continue;
        if (item->data(ITEM_TYPE_ROLE).toString() == TYPE_OBSTACLE && item->parentItem() != this) {
            collisionWithObstacle = true;
            qDebug() << "检测到与障碍物碰撞：" << item->data(ITEM_NAME_ROLE).toString();
            break;
        }
    }
    if (!collisionWithObstacle) {
        if (tentativeNewX < sceneBounds.left()) tentativeNewX = sceneBounds.left();
        if (tentativeNewY < sceneBounds.top()) tentativeNewY = sceneBounds.top();
        if (tentativeNewX + playerLocalBounds.width() > sceneBounds.right()) tentativeNewX = sceneBounds.right() - playerLocalBounds.width();
        if (tentativeNewY + playerLocalBounds.height() > sceneBounds.bottom()) tentativeNewY = sceneBounds.bottom() - playerLocalBounds.height();
        setPos(tentativeNewX, tentativeNewY);
    }
}

void Player::attemptPickupFromSource()
{
    if (!scene()) return;
    qDebug() << "尝试通过空格键从源拾取物品...";
    QList<QGraphicsItem*> itemsUnderPlayer = scene()->collidingItems(this, Qt::IntersectsItemShape);
    for (QGraphicsItem *sourceItem : itemsUnderPlayer) {
        if (sourceItem->data(ITEM_TYPE_ROLE).toString() == TYPE_COLLECTIBLE_VEGETABLE_SOURCE && sourceItem->parentItem() == nullptr) {
            QString itemName = sourceItem->data(ITEM_NAME_ROLE).toString();
            m_inventory[itemName]++;
            qDebug() << "从源拾取了 '" << itemName << "'。物品栏中 '" << itemName << "' 数量: " << m_inventory[itemName];
            addVisualHeldItem(itemName);
            break;
        }
    }
}

void Player::attemptInteractWithStation() {
    if (!scene()) return;
    qDebug() << "尝试与工作站交互 (按 'E')...";

    QList<QGraphicsItem*> itemsUnderPlayer = scene()->collidingItems(this, Qt::IntersectsItemShape);
    QGraphicsItem* targetStationItem = nullptr;

    for (QGraphicsItem* item : itemsUnderPlayer) {
        QString itemType = item->data(ITEM_TYPE_ROLE).toString();
        if (itemType == TYPE_CHOPPING_STATION ||
            itemType == TYPE_SALAD_ASSEMBLY_STATION ||
            itemType == TYPE_SERVING_HATCH) {
            targetStationItem = item;
            break;
        }
    }

    if (!targetStationItem) {
        qDebug() << "附近没有可交互的工作站。";
        return;
    }

    QString stationType = targetStationItem->data(ITEM_TYPE_ROLE).toString();

    if (stationType == TYPE_CHOPPING_STATION) {
        ChoppingStationItem* choppingStation = static_cast<ChoppingStationItem*>(targetStationItem);
        if (choppingStation->isEmpty()) {
            QString itemToPlaceName;
            for (auto it = m_inventory.constBegin(); it != m_inventory.constEnd(); ++it) {
                if (it.value() > 0 && !it.key().endsWith("_已切")) { itemToPlaceName = it.key(); break; }
            }
            if (!itemToPlaceName.isEmpty()) {
                QString rawItemImagePath = Player::getImagePathForItem(itemToPlaceName);
                if (!rawItemImagePath.isEmpty() && choppingStation->placeVegetable(itemToPlaceName, rawItemImagePath)) {
                    qDebug() << "玩家将 '" << itemToPlaceName << "' 放置到切菜板。";
                    tryTakeFromInventory(itemToPlaceName);
                } else { qDebug() << "无法放置 '" << itemToPlaceName << "' 到切菜板。"; }
            } else { qDebug() << "物品栏中没有可放置的【未切的】蔬菜。"; }
        } else if (!choppingStation->isCut()) {
            QString rawVegetableName = choppingStation->getPlacedVegetableName();
            QString cutItemName = rawVegetableName + "_已切"; // 构造切好的物品名
            QString cutVegetableImagePath = Player::getImagePathForItem(cutItemName); // 获取切好的图片路径
            if (!cutVegetableImagePath.isEmpty() && choppingStation->chopVegetable(cutVegetableImagePath)) {
                qDebug() << "'" << rawVegetableName << "' 已在砧板上切好！现在是 '" << choppingStation->getCurrentVisualNameOnBoard() << "'";
            } else { qWarning() << "切菜失败，可能没有 '" << rawVegetableName << "' 对应的已切图片。"; }
        } else { // 砧板上有已切蔬菜，取回
            QString cutVegName = choppingStation->takeVegetable(); // 从砧板取走，返回 "XX_已切"
            if (!cutVegName.isEmpty()) {
                m_inventory[cutVegName]++;
                addVisualHeldItem(cutVegName);
                qDebug() << "'" << cutVegName << "' 已从砧板取回到物品栏。";
            }
        }
    } else if (stationType == TYPE_SALAD_ASSEMBLY_STATION) {
        SaladAssemblyStationItem* saladStation = static_cast<SaladAssemblyStationItem*>(targetStationItem);
        if (saladStation->canProduceSalad()) {
            QString saladProduct = saladStation->produceAndTakeSalad();
            if (!saladProduct.isEmpty()) {
                m_inventory[saladProduct]++;
                addVisualHeldItem(saladProduct);
                qDebug() << "成功制作并获得了 '" << saladProduct << "'!";
            }
        } else {
            QString cutItemToPlace;
            for (auto it = m_inventory.constBegin(); it != m_inventory.constEnd(); ++it) {
                if (it.value() > 0 && it.key().endsWith("_已切")) {
                    cutItemToPlace = it.key(); break;
                }
            }
            if (!cutItemToPlace.isEmpty()) {
                if (saladStation->tryAddCutIngredient(cutItemToPlace)) {
                    qDebug() << "玩家将 '" << cutItemToPlace << "' 放置到沙拉组装台。";
                    tryTakeFromInventory(cutItemToPlace);
                } else { qDebug() << "无法将 '" << cutItemToPlace << "' 放置到沙拉组装台。"; }
            } else { qDebug() << "物品栏中没有可放置到沙拉组装台的【已切的】蔬菜。"; }
        }
    } else if (stationType == TYPE_SERVING_HATCH) {
        ServingHatchItem* servingHatch = static_cast<ServingHatchItem*>(targetStationItem);
        qDebug() << "与上菜口交互...";
        if (m_inventory.contains(PRODUCT_SALAD) && m_inventory.value(PRODUCT_SALAD) > 0) {
            if (servingHatch->serveOrder(PRODUCT_SALAD)) {
                qDebug() << "成功将一份 '" << PRODUCT_SALAD << "' 上菜！";
                tryTakeFromInventory(PRODUCT_SALAD);
            } else { qDebug() << "上菜失败。"; }
        } else { qDebug() << "物品栏中没有 '" << PRODUCT_SALAD << "' 可以上菜。"; }
    }
}

void Player::addVisualHeldItem(const QString& itemName)
{
    QString itemImagePath = Player::getImagePathForItem(itemName);
    if (itemImagePath.isEmpty()) {
        qWarning() << "Player Error: 为视觉物品 '" << itemName << "' 获取图片路径失败。";
        return;
    }
    QPixmap itemPixmap(itemImagePath);
    if (itemPixmap.isNull()) {
        qWarning() << "Player Error: 加载视觉物品图片失败: " << itemImagePath << " for " << itemName;
        return;
    }

    if (m_visualHeldItems.size() >= m_maxVisualHeldItems && !m_visualHeldItems.isEmpty()) {
        QGraphicsPixmapItem* oldestItem = m_visualHeldItems.takeFirst();
        delete oldestItem; // 从场景中删除并释放内存
    }

    QGraphicsPixmapItem* visualCopy = new QGraphicsPixmapItem(itemPixmap.scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation), this); // 父项是Player
    visualCopy->setData(ITEM_NAME_ROLE, itemName + "_visual_copy"); // 标记视觉副本
    m_visualHeldItems.append(visualCopy);
    updateHeldItemPositions();
    qDebug() << "添加了视觉物品：" << itemName + "_visual_copy";
}

void Player::updateHeldItemPositions()
{
    qreal currentYOffset = - (this->boundingRect().height() * 0.6f); // 调整起始Y偏移，使其更靠上
    qreal spacing = 1; // 物品之间的垂直间距
    // 从后往前迭代列表 (最新添加的物品)，让它们堆叠在最上方 (离角色头最近)
    for (int i = m_visualHeldItems.size() - 1; i >= 0; --i) {
        QGraphicsPixmapItem* item = m_visualHeldItems.at(i);
        qreal itemHeight = item->boundingRect().height();
        qreal itemWidth = item->boundingRect().width();
        // 水平居中于玩家
        item->setPos((this->boundingRect().width() - itemWidth) / 2.0, currentYOffset - itemHeight);
        currentYOffset -= (itemHeight + spacing); // 为下一个（更早添加的）物品准备更高的位置
    }
}

bool Player::tryTakeFromInventory(const QString& itemName, int count) {
    if (m_inventory.contains(itemName) && m_inventory[itemName] >= count) {
        m_inventory[itemName] -= count;
        if (m_inventory[itemName] == 0) {
            m_inventory.remove(itemName);
        }
        // 消耗了物品，对应地移除一个视觉表示
        removeVisualHeldItem(itemName);
        qDebug() << "'" << itemName << "' (数量: " << count << ") 已从物品栏消耗。剩余: " << m_inventory.value(itemName, 0);
        return true;
    }
    qDebug() << "物品栏中 '" << itemName << "' 数量不足 (需要: " << count << ", 拥有: " << m_inventory.value(itemName,0) << ")";
    return false;
}

void Player::removeVisualHeldItem(const QString& itemName) {
    QString targetVisualCopyName = itemName + "_visual_copy";
    for (int i = m_visualHeldItems.size() - 1; i >= 0; --i) {
        // 为了精确匹配，视觉物品也应该存储原始物品名，或者有一个更可靠的ID
        // 当前简单地通过构造的"_visual_copy"名称来匹配
        if (m_visualHeldItems.at(i)->data(ITEM_NAME_ROLE).toString() == targetVisualCopyName) {
            QGraphicsPixmapItem* visualItem = m_visualHeldItems.takeAt(i); // 从列表中移除
            // 因为 visualItem 是 Player 的子项，当 Player 被删除时，它会被自动删除。
            // 或者，如果希望立即从场景中消失并不再跟随，可以：
            // visualItem->setParentItem(nullptr); // 解除父子关系
            // scene()->removeItem(visualItem); // 如果解除了父子关系，需要从场景中移除
            delete visualItem; // 直接删除 (如果父项是 this, Qt会处理好)
            updateHeldItemPositions(); // 更新剩余物品的位置
            qDebug() << "移除了视觉物品：" << targetVisualCopyName;
            return; // 通常一次只移除一个匹配的
        }
    }
    // 如果没有精确匹配到带 "_visual_copy" 后缀的，
    // 可能是因为 addVisualHeldItem 之前没有正确设置这个data，
    // 或者要移除的 itemName 已经是 "沙拉" 这种没有原始/已切状态区分的成品。
    // 对于这种情况，可以尝试不加后缀查找，或者移除最后一个作为通用策略。
    // 但最稳妥的是确保 addVisualHeldItem 时设置的 ITEM_NAME_ROLE 与这里查找的逻辑一致。
    qDebug() << "警告：尝试移除视觉物品 '" << targetVisualCopyName << "' 但未在列表中精确找到。可能是因为名称不匹配或列表已空。";
}

const QMap<QString, int>& Player::getInventory() const {
    return m_inventory;
}
