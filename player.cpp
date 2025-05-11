#include "player.h"
#include "choppingstationitem.h"
#include "saladassemblystationitem.h"
#include "servinghatchitem.h"
#include <QPixmap>
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsScene>

QMap<QString, QString> Player_ItemImagePaths_map_cpp = {
    {RAW_TOMATO, ":/images/tomato_raw.png"}, {RAW_LETTUCE, ":/images/lettuce_raw.png"},
    {CUT_TOMATO, ":/images/tomato_cut.png"}, {CUT_LETTUCE, ":/images/lettuce_sliced.png"},
    {PRODUCT_SALAD, ":/images/salad_final.png"}
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
        qDebug() << "玩家正忙于切菜，无法移动。";
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
    if (m_isBusyWithStation) { qDebug() << "玩家正忙，无法拾取。"; return; }
    if (!scene()) return;
    QList<QGraphicsItem*> itemsUnderPlayer = scene()->collidingItems(this, Qt::IntersectsItemShape);
    for (QGraphicsItem *sourceItem : itemsUnderPlayer) {
        if (sourceItem->data(ITEM_TYPE_ROLE).toString() == TYPE_COLLECTIBLE_VEGETABLE_SOURCE && sourceItem->parentItem() == nullptr) {
            QString itemName = sourceItem->data(ITEM_NAME_ROLE).toString();
            m_inventory[itemName]++;
            addVisualHeldItem(itemName);
            break;
        }
    }
}

void Player::attemptInteractWithStation() {
    if (m_isBusyWithStation) { qDebug() << "玩家正忙(例如切菜中)，无法开始新的工作站交互。"; return; }
    if (!scene()) return;
    qDebug() << "尝试与工作站交互 (按 'E')...";

    QList<QGraphicsItem*> itemsUnderPlayer = scene()->collidingItems(this, Qt::IntersectsItemShape);
    QGraphicsItem* targetStationItem = nullptr;
    for (QGraphicsItem* item : itemsUnderPlayer) {
        QString itemType = item->data(ITEM_TYPE_ROLE).toString();
        if (itemType == TYPE_CHOPPING_STATION || itemType == TYPE_SALAD_ASSEMBLY_STATION || itemType == TYPE_SERVING_HATCH) {
            targetStationItem = item; break;
        }
    }
    if (!targetStationItem) { qDebug() << "附近没有可交互的工作站。"; return; }

    QString stationType = targetStationItem->data(ITEM_TYPE_ROLE).toString();

    if (stationType == TYPE_CHOPPING_STATION) {
        ChoppingStationItem* choppingStation = static_cast<ChoppingStationItem*>(targetStationItem);
        if (choppingStation->isChopping()) {
            qDebug() << "切菜板正在工作中，请稍候...";
            return;
        }

        if (choppingStation->isEmpty()) {
            QString itemToPlaceName;
            for (auto it = m_inventory.constBegin(); it != m_inventory.constEnd(); ++it) {
                if (it.value() > 0 && !it.key().endsWith("_已切") && it.key() != PRODUCT_SALAD) {
                    itemToPlaceName = it.key(); break;
                }
            }
            if (!itemToPlaceName.isEmpty()) {
                QString rawItemImagePath = Player::getImagePathForItem(itemToPlaceName);
                if (!rawItemImagePath.isEmpty() && choppingStation->placeVegetable(itemToPlaceName, rawItemImagePath)) {
                    tryTakeFromInventory(itemToPlaceName);
                }
            } else { qDebug() << "物品栏中没有可放置的【未切的】蔬菜。"; }
        } else if (!choppingStation->isCut()) { // 砧板上有未切蔬菜 -> 开始切菜过程
            QString rawVegetableName = choppingStation->getPlacedVegetableName();
            QString cutVegetableName = rawVegetableName + "_已切";
            QString cutVegetableImagePath = Player::getImagePathForItem(cutVegetableName);

            if (!cutVegetableImagePath.isEmpty()) {
                int choppingTimeMs = 3000; // 例如3秒
                if (choppingStation->startChoppingProcess(rawVegetableName, cutVegetableImagePath, choppingTimeMs)) {
                    m_isBusyWithStation = true; // 玩家开始忙碌
                    qDebug() << "玩家开始切 " << rawVegetableName << "，预计耗时 " << choppingTimeMs << " ms.";
                }
            } else { qWarning() << "切菜失败，没有 '" << rawVegetableName << "' 对应的已切图片 (" << cutVegetableName << ")。"; }
        } else { // 砧板上有已切蔬菜，取回
            QString cutVegName = choppingStation->takeVegetable();
            if (!cutVegName.isEmpty()) {
                m_inventory[cutVegName]++;
                addVisualHeldItem(cutVegName);
                qDebug() << "'" << cutVegName << "' 已从砧板取回到物品栏。";
            }
        }
    } else if (stationType == TYPE_SALAD_ASSEMBLY_STATION) {
        SaladAssemblyStationItem* saladStation = static_cast<SaladAssemblyStationItem*>(targetStationItem);
        if(saladStation->canProduceSalad()){QString p=saladStation->produceAndTakeSalad();if(!p.isEmpty()){m_inventory[p]++;addVisualHeldItem(p);}}
        else{QString citp;for(auto it=m_inventory.constBegin();it!=m_inventory.constEnd();++it){if(it.value()>0&&it.key().endsWith("_已切")){citp=it.key();break;}}
            if(!citp.isEmpty()){if(saladStation->tryAddCutIngredient(citp)){tryTakeFromInventory(citp);}}}
    } else if (stationType == TYPE_SERVING_HATCH) {
        ServingHatchItem* servingHatch = static_cast<ServingHatchItem*>(targetStationItem);
        if(m_inventory.contains(PRODUCT_SALAD)&&m_inventory.value(PRODUCT_SALAD)>0){if(servingHatch->serveOrder(PRODUCT_SALAD)){tryTakeFromInventory(PRODUCT_SALAD);}}
    }
}

// 槽函数实现
void Player::onVegetableChopped(const QString& originalVegetableName, const QString& /*cutImagePathUsedByStation*/) {
    qDebug() << "Player 收到切菜完成信号：" << originalVegetableName << " 已切好。";
    m_isBusyWithStation = false; // 玩家不再忙碌
    // 切菜完成后，物品仍在砧板上，状态已变为“已切”
    // 玩家需要再次按 'E' 与砧板交互才能取走切好的蔬菜
}

void Player::addVisualHeldItem(const QString& itemName) {
    QString itemImagePath = Player::getImagePathForItem(itemName);
    if(itemImagePath.isEmpty()){qWarning()<<"P:No img path for "<<itemName;return;} QPixmap p(itemImagePath); if(p.isNull()){qWarning()<<"P:Cannot load img "<<itemImagePath<<" for "<<itemName;return;}
    if(m_visualHeldItems.size()>=m_maxVisualHeldItems && !m_visualHeldItems.isEmpty()){delete m_visualHeldItems.takeFirst();}
    QGraphicsPixmapItem* v = new QGraphicsPixmapItem(p.scaled(25,25,Qt::KeepAspectRatio,Qt::SmoothTransformation),this);
    v->setData(ITEM_NAME_ROLE, itemName+"_visual_copy"); m_visualHeldItems.append(v); updateHeldItemPositions();
}
void Player::updateHeldItemPositions() {
    qreal cy = -(this->boundingRect().height()*0.6f); qreal sp = 1;
    for(int i=m_visualHeldItems.size()-1; i>=0; --i){
        QGraphicsPixmapItem* it=m_visualHeldItems.at(i); qreal ih=it->boundingRect().height(); qreal iw=it->boundingRect().width();
        it->setPos((this->boundingRect().width()-iw)/2.0, cy-ih); cy-=(ih+sp);
    }
}
bool Player::tryTakeFromInventory(const QString& itemName, int count) {
    if(m_inventory.contains(itemName) && m_inventory[itemName]>=count){
        m_inventory[itemName]-=count; if(m_inventory[itemName]==0){m_inventory.remove(itemName);}
        removeVisualHeldItem(itemName); return true;
    } return false;
}
void Player::removeVisualHeldItem(const QString& itemName) {
    QString tc=itemName+"_visual_copy"; for(int i=m_visualHeldItems.size()-1;i>=0;--i){
        if(m_visualHeldItems.at(i)->data(ITEM_NAME_ROLE).toString()==tc){
            delete m_visualHeldItems.takeAt(i); updateHeldItemPositions(); return;
        }
    }
}
const QMap<QString, int>& Player::getInventory() const { return m_inventory; }
