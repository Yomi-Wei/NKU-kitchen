#include "player.h"
#include "choppingstationitem.h"
#include "saladassemblystationitem.h"
#include "servinghatchitem.h"
#include "fryingstationitem.h"
#include "item_definitions.h"
#include <QPixmap>
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsScene>

QMap<QString, QString> Player_ItemImagePaths_map_cpp = {
    {RAW_TOMATO, ":/images/tomato_raw.png"},
    {RAW_LETTUCE, ":/images/lettuce_raw.png"},
    {RAW_MEAT, ":/images/meat_raw.png"},
    {BREAD_SLICE, ":/images/bread_slice.png"},

    {CUT_TOMATO, ":/images/tomato_cut.png"},
    {CUT_LETTUCE, ":/images/lettuce_sliced.png"},
    {CUT_MEAT, ":/images/meat_cut.png"},

    {FRIED_MEAT, ":/images/meat_fried.png"},

    {PRODUCT_SALAD, ":/images/salad_final.png"},
    {PRODUCT_BURGER, ":/images/burger_final.png"}
};

QString Player::getImagePathForItem(const QString& itemName) {
    return Player_ItemImagePaths_map_cpp.value(itemName, QString());
}

Player::Player(const QString &pixmapPath, qreal speed, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(parent), m_moveSpeed(speed),
    m_maxVisualHeldItems(3), m_isBusyWithStation(false)
{
    QPixmap originalPixmap(pixmapPath);
    if (originalPixmap.isNull()) {
        qWarning() << "错误：从路径加载玩家图片失败：" << pixmapPath;
        QPixmap fallbackPixmap(50, 50); fallbackPixmap.fill(Qt::red); setPixmap(fallbackPixmap);
    } else {
        QPixmap scaledPixmap = originalPixmap.scaled(100,100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setPixmap(scaledPixmap);
    }
}

bool Player::isBusy() const {
    return m_isBusyWithStation;
}

void Player::handleMovementKey(Qt::Key key, const QRectF &sceneBounds)
{
    if (m_isBusyWithStation) {
        return;
    }
    if (!scene()) { return; }
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
    // qDebug() << "Player::attemptPickupFromSource called. IsBusy:" << m_isBusyWithStation;
    if (m_isBusyWithStation) { qDebug() << "玩家正忙，无法拾取。"; return; }
    if (!scene()) {
        // qDebug() << "Player::attemptPickupFromSource - Scene is null.";
        return;
    }
    QList<QGraphicsItem*> itemsUnderPlayer = scene()->collidingItems(this, Qt::IntersectsItemShape);
    // qDebug() << "Player::attemptPickupFromSource - Items under player:" << itemsUnderPlayer.size();
    for (QGraphicsItem *sourceItem : itemsUnderPlayer) {
        QString itemType = sourceItem->data(ITEM_TYPE_ROLE).toString();
        // QString itemNameRole = sourceItem->data(ITEM_NAME_ROLE).toString();
        // qDebug() << "Player::attemptPickupFromSource - Checking item:" << itemNameRole << "Type:" << itemType;
        if (itemType == TYPE_COLLECTIBLE_VEGETABLE_SOURCE && sourceItem->parentItem() == nullptr) {
            QString itemName = sourceItem->data(ITEM_NAME_ROLE).toString();
            if (!itemName.isEmpty()) {
                m_inventory[itemName]++;
                addVisualHeldItem(itemName);
                qDebug() << "玩家拾取了: " << itemName << "。当前数量: " << m_inventory[itemName] << "库存详情:" << m_inventory;
                break;
            }
        }
    }
}


void Player::attemptInteractWithStation() {
    // qDebug() << "Player::attemptInteractWithStation called. IsBusy:" << m_isBusyWithStation;
    if (m_isBusyWithStation) {
        qDebug() << "玩家正忙(例如处理中)，无法开始新的工作站交互。";
        return;
    }
    if (!scene()) {
        //qDebug() << "Player::attemptInteractWithStation - Scene is null.";
        return;
    }
    //qDebug() << "尝试与工作站交互 (按 'E')... 玩家库存:" << m_inventory;

    QList<QGraphicsItem*> itemsUnderPlayer = scene()->collidingItems(this, Qt::IntersectsItemShape);
    QGraphicsItem* targetStationItem = nullptr;
    for (QGraphicsItem* item : itemsUnderPlayer) {
        QString itemType = item->data(ITEM_TYPE_ROLE).toString();
        // qDebug() << "Player::attemptInteractWithStation - Checking nearby item type:" << itemType << "Name:" << item->data(ITEM_NAME_ROLE).toString();
        if (itemType == TYPE_CHOPPING_STATION || itemType == TYPE_SALAD_ASSEMBLY_STATION ||
            itemType == TYPE_SERVING_HATCH || itemType == TYPE_FRYING_STATION) {
            targetStationItem = item;
            // qDebug() << "Player::attemptInteractWithStation - Found target station:" << item->data(ITEM_NAME_ROLE).toString() << "Type:" << itemType;
            break;
        }
    }

    QString stationType = targetStationItem->data(ITEM_TYPE_ROLE).toString();
    // qDebug() << "Player::attemptInteractWithStation - Interacting with station type:" << stationType;

    if (stationType == TYPE_CHOPPING_STATION) {
        ChoppingStationItem* choppingStation = static_cast<ChoppingStationItem*>(targetStationItem);
        if (choppingStation->isChopping()) { return; }
        if (choppingStation->isEmpty()) {
            QString itemToPlaceName;
            for (auto it = m_inventory.constBegin(); it != m_inventory.constEnd(); ++it) {
                if (it.value() > 0 && !it.key().endsWith("_已切") && !it.key().endsWith("_已煎") && it.key() != PRODUCT_SALAD && it.key() != PRODUCT_BURGER) {
                    if (it.key() == RAW_TOMATO || it.key() == RAW_LETTUCE || it.key() == RAW_MEAT) {
                        itemToPlaceName = it.key(); break;
                    }
                }
            }
            if (!itemToPlaceName.isEmpty()) {
                QString rawItemImagePath = Player::getImagePathForItem(itemToPlaceName);
                if (!rawItemImagePath.isEmpty() && choppingStation->placeVegetable(itemToPlaceName, rawItemImagePath)) {
                    tryTakeFromInventory(itemToPlaceName);
                }
            }
        } else if (!choppingStation->isCut()) {
            QString rawItemNameOnBoard = choppingStation->getPlacedVegetableName();
            QString cutItemName = rawItemNameOnBoard + "_已切";
            QString cutItemImagePath = Player::getImagePathForItem(cutItemName);
            if (!cutItemImagePath.isEmpty()) {
                int choppingTimeMs = 3000;
                if (choppingStation->startChoppingProcess(rawItemNameOnBoard, cutItemImagePath, choppingTimeMs)) {
                    m_isBusyWithStation = true;
                }
            }
        } else {
            QString cutItemName = choppingStation->takeVegetable();
            if (!cutItemName.isEmpty()) {
                m_inventory[cutItemName]++; addVisualHeldItem(cutItemName);
            }
        }
    } else if (stationType == TYPE_FRYING_STATION) {
        FryingStationItem* fryingStation = static_cast<FryingStationItem*>(targetStationItem);
        if (fryingStation->isFrying()) { return; }
        if (fryingStation->isEmpty()) {
            if (m_inventory.contains(CUT_MEAT) && m_inventory.value(CUT_MEAT) > 0) {
                QString cutMeatImagePath = Player::getImagePathForItem(CUT_MEAT);
                if (!cutMeatImagePath.isEmpty() && fryingStation->placeItem(CUT_MEAT, cutMeatImagePath)) {
                    tryTakeFromInventory(CUT_MEAT);
                }
            }
        } else if (!fryingStation->isFried()) {
            QString itemOnBoard = fryingStation->getPlacedItemName();
            if (itemOnBoard == CUT_MEAT) {
                QString friedMeatImagePath = Player::getImagePathForItem(FRIED_MEAT);
                if (!friedMeatImagePath.isEmpty()) {
                    int fryingTimeMs = 3000;
                    if (fryingStation->startFryingProcess(CUT_MEAT, friedMeatImagePath, fryingTimeMs)) {
                        m_isBusyWithStation = true;
                    }
                }
            }
        } else {
            QString friedItemName = fryingStation->takeItem();
            if (!friedItemName.isEmpty()) {
                m_inventory[friedItemName]++; addVisualHeldItem(friedItemName);
            }
        }
    } else if (stationType == TYPE_SALAD_ASSEMBLY_STATION) {
        SaladAssemblyStationItem* assemblyStation = static_cast<SaladAssemblyStationItem*>(targetStationItem);
        QList<QString> producible = assemblyStation->getProducibleItems();
        qDebug() << "Player::attemptInteractWithStation - ASSEMBLY STATION. Producible items from station:" << producible;
        qDebug() << "Player::attemptInteractWithStation - Current ingredients on assembly board:" << assemblyStation->getCurrentIngredientsOnBoard();

        if (!producible.isEmpty()) {
            qDebug() << "Player: Assembly station CAN produce something. Producible list:" << producible;
            qDebug() << "Player: Checking for PRODUCT_BURGER ('" << PRODUCT_BURGER << "') in producible list.";
            qDebug() << "Player: Checking for PRODUCT_SALAD ('" << PRODUCT_SALAD << "') in producible list.";

            QString itemToProduce = "";

            bool canMakeBurger = producible.contains(PRODUCT_BURGER);
            bool canMakeSalad = producible.contains(PRODUCT_SALAD);

            qDebug() << "Player: Can make Burger?" << canMakeBurger << "(Value of PRODUCT_BURGER: " << PRODUCT_BURGER << ")";
            qDebug() << "Player: Can make Salad?" << canMakeSalad << "(Value of PRODUCT_SALAD: " << PRODUCT_SALAD << ")";

            if (canMakeBurger) {
                itemToProduce = PRODUCT_BURGER;
                qDebug() << "Player: Prioritizing Burger. itemToProduce set to:" << itemToProduce;
            } else if (canMakeSalad) {
                itemToProduce = PRODUCT_SALAD;
                qDebug() << "Player: Burger not producible (or not found in producible list), prioritizing Salad. itemToProduce set to:" << itemToProduce;
            } else if (!producible.isEmpty()) {
                itemToProduce = producible.first();
                qDebug() << "Player: Fallback - neither Burger nor Salad specifically selected. Taking first producible:" << itemToProduce;
            }

            qDebug() << "Player: Final item selected to produce:" << itemToProduce;

            if (!itemToProduce.isEmpty()) {
                QString producedItem = assemblyStation->produceAndTakeItem(itemToProduce);
                if (!producedItem.isEmpty()) {
                    m_inventory[producedItem]++;
                    addVisualHeldItem(producedItem);
                    qDebug() << "玩家从组装台获得了: " << producedItem << "库存详情:" << m_inventory;
                    return;
                } else {
                    qDebug() << "ProduceAndTakeItem for" << itemToProduce << "returned empty. Station state might have changed or item not truly producible by station logic.";
                }
            } else {
                qDebug() << "No item was selected to produce from the producible list, or list was empty after checks (itemToProduce is empty).";
            }
        } else {
            qDebug() << "Player: Assembly station CANNOT produce anything. Trying to place ingredient.";
            QString ingredientToPlace;
            QStringList potentialIngredients;

            potentialIngredients << PRODUCT_SALAD << BREAD_SLICE << FRIED_MEAT << CUT_TOMATO << CUT_LETTUCE;


            for (const QString& invItem : m_inventory.keys()) {
                if (m_inventory.value(invItem) > 0 && potentialIngredients.contains(invItem)) {
                    ingredientToPlace = invItem;
                    qDebug() << "Player: Found potential ingredient in inventory to place:" << ingredientToPlace;
                    break;
                }
            }
            qDebug() << "Player inventory check for assembly: ingredientToPlace =" << ingredientToPlace;
            if (!ingredientToPlace.isEmpty()) {
                if (assemblyStation->tryAddIngredient(ingredientToPlace)) {
                    tryTakeFromInventory(ingredientToPlace);
                    qDebug() << "玩家将 " << ingredientToPlace << " 放入组装台。";
                } else {
                    qDebug() << "无法将 " << ingredientToPlace << " 放入组装台 (tryAddIngredient returned false).";
                }
            } else {
                qDebug() << "玩家物品栏中没有可放入组装台的配料，或者组装台不需要。";
            }
        }
    } else if (stationType == TYPE_SERVING_HATCH) {
        qDebug() << "Player Attempt: Interacting with serving hatch.";
        ServingHatchItem* servingHatch = static_cast<ServingHatchItem*>(targetStationItem);
        QString itemToServe;
        if (m_inventory.contains(PRODUCT_BURGER) && m_inventory.value(PRODUCT_BURGER) > 0) {
            itemToServe = PRODUCT_BURGER;
        } else if (m_inventory.contains(PRODUCT_SALAD) && m_inventory.value(PRODUCT_SALAD) > 0) {
            itemToServe = PRODUCT_SALAD;
        }
        qDebug() << "Player: Item to serve:" << itemToServe;
        if (!itemToServe.isEmpty()) {
            if (servingHatch->serveOrder(itemToServe)) {
                tryTakeFromInventory(itemToServe);
                qDebug() << "玩家提交了 " << itemToServe;
            } else {
                qDebug() << "玩家提交 " << itemToServe << " 失败 (serveOrder returned false).";
            }
        } else {
            qDebug() << "玩家物品栏中没有可上菜的成品。";
        }
    }
}


void Player::onVegetableChopped(const QString& originalItemName, const QString& /*cutImagePathUsedByStation*/) {
    qDebug() << "Player::onVegetableChopped called for" << originalItemName << ". Setting m_isBusyWithStation to false.";
    m_isBusyWithStation = false;
}

void Player::onMeatFried(const QString& originalCutMeatName, const QString& /*friedMeatImagePathUsed*/) {
    qDebug() << "Player::onMeatFried called for" << originalCutMeatName << ". Setting m_isBusyWithStation to false.";
    m_isBusyWithStation = false;
}

void Player::addVisualHeldItem(const QString& itemName) {
    QString itemImagePath = Player::getImagePathForItem(itemName);
    if(itemImagePath.isEmpty()){qWarning()<<"P:No img path for "<<itemName;return;}
    QPixmap p(itemImagePath);
    if(p.isNull()){qWarning()<<"P:Cannot load img "<<itemImagePath<<" for "<<itemName;return;}

    if(m_visualHeldItems.size() >= m_maxVisualHeldItems && !m_visualHeldItems.isEmpty()){
        QGraphicsPixmapItem* toDelete = m_visualHeldItems.takeFirst();
        if (toDelete) {
            delete toDelete;
        }
    }
    QGraphicsPixmapItem* v = new QGraphicsPixmapItem(p.scaled(25,25,Qt::KeepAspectRatio,Qt::SmoothTransformation), this);
    v->setData(ITEM_NAME_ROLE, itemName+"_visual_copy");
    m_visualHeldItems.append(v);
    updateHeldItemPositions();
}

void Player::updateHeldItemPositions() {
    qreal currentY = -(this->boundingRect().height() * 0.2f);
    const qreal spacing = 2.0;
    const qreal itemHeight = 25.0;

    for(int i = 0; i < m_visualHeldItems.size(); ++i){
        QGraphicsPixmapItem* item = m_visualHeldItems.at(i);
        qreal itemWidth = item->boundingRect().width();
        item->setPos((this->boundingRect().width() - itemWidth) / 2.0, currentY - (itemHeight + spacing) * i - itemHeight);
    }
}

bool Player::tryTakeFromInventory(const QString& itemName, int count) {
    if(m_inventory.contains(itemName) && m_inventory[itemName] >= count){
        m_inventory[itemName] -= count;
        if(m_inventory[itemName] == 0){
            m_inventory.remove(itemName);
        }
        removeVisualHeldItem(itemName);
        qDebug() << "Player: Took" << itemName << "from inventory. Remaining:" << m_inventory.value(itemName, 0) << "Full inv:" << m_inventory;
        return true;
    }
    qDebug() << "Player: Failed to take" << itemName << "from inventory. Has:" << m_inventory.value(itemName,0) << "Needs:" << count;
    return false;
}

void Player::removeVisualHeldItem(const QString& itemName) {
    QString visualCopyName = itemName + "_visual_copy";
    for(int i = m_visualHeldItems.size() - 1; i >= 0; --i){
        if(m_visualHeldItems.at(i)->data(ITEM_NAME_ROLE).toString() == visualCopyName){
            QGraphicsPixmapItem* toDelete = m_visualHeldItems.takeAt(i);
            if (toDelete) {
                delete toDelete;
            }
            updateHeldItemPositions();
            return;
        }
    }
}
const QMap<QString, int>& Player::getInventory() const { return m_inventory; }
