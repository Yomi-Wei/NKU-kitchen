#include "choppingstationitem.h"
#include "player.h" // 为了共享 ITEM_TYPE_ROLE, ITEM_NAME_ROLE, TYPE_CHOPPING_STATION 常量
#include <QPixmap>
#include <QDebug>
#include <QGraphicsScene>


ChoppingStationItem::ChoppingStationItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent),
    m_visualVegetableOnBoard(nullptr), m_isCut(false)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_CHOPPING_STATION); // 使用 player.h 中定义的常量
    this->setData(ITEM_NAME_ROLE, "切菜板");
}

bool ChoppingStationItem::placeVegetable(const QString& vegetableName, const QString& vegetableImagePath)
{
    if (!isEmpty()) {
        qDebug() << "切菜板上已经有：" << getCurrentVisualNameOnBoard();
        return false;
    }

    QPixmap vegPixmap(vegetableImagePath);
    if (vegPixmap.isNull()) {
        qWarning() << "ChoppingStationItem Error: 无法为 '"<< vegetableName <<"' 加载图片: " << vegetableImagePath;
        return false;
    }

    // 确保之前的视觉物品被清理 (理论上 isEmpty() 已经保证了)
    if(m_visualVegetableOnBoard) {
        delete m_visualVegetableOnBoard;
        m_visualVegetableOnBoard = nullptr;
    }

    m_visualVegetableOnBoard = new QGraphicsPixmapItem(vegPixmap.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
    qreal boardWidth = this->boundingRect().width();
    qreal boardHeight = this->boundingRect().height();
    qreal vegWidth = m_visualVegetableOnBoard->boundingRect().width();
    qreal vegHeight = m_visualVegetableOnBoard->boundingRect().height();
    m_visualVegetableOnBoard->setPos( (boardWidth - vegWidth) / 2.0, (boardHeight - vegHeight) / 2.0 );

    m_placedVegetableName = vegetableName;
    m_placedVegetableImagePath = vegetableImagePath; // 保存原始图片路径
    m_isCut = false; // 新放置的物品是未切的
    qDebug() << "'" << vegetableName << "' 已放置在切菜板上 (未切)。";
    return true;
}

QString ChoppingStationItem::getPlacedVegetableName() const
{
    return m_placedVegetableName; // 总是返回原始名称
}

QString ChoppingStationItem::getCurrentVisualNameOnBoard() const {
    if (m_placedVegetableName.isEmpty()) return QString();
    return m_placedVegetableName + (m_isCut ? "_已切" : "");
}


QString ChoppingStationItem::takeVegetable()
{
    QString itemNameTaken = getCurrentVisualNameOnBoard();

    if (m_visualVegetableOnBoard) {
        delete m_visualVegetableOnBoard;
        m_visualVegetableOnBoard = nullptr;
    }
    m_placedVegetableName.clear();
    m_placedVegetableImagePath.clear();
    m_isCut = false; // 重置状态
    qDebug() << "从切菜板上取走了：" << itemNameTaken;
    return itemNameTaken;
}

bool ChoppingStationItem::isEmpty() const
{
    return m_placedVegetableName.isEmpty(); // 主要看逻辑上是否有物品
}

bool ChoppingStationItem::isCut() const
{
    return m_isCut;
}

bool ChoppingStationItem::chopVegetable(const QString& cutVegetableImagePath)
{
    if (isEmpty() || m_isCut) {
        qDebug() << "无法切菜：砧板为空或蔬菜已是切好状态。当前状态：" << getCurrentVisualNameOnBoard();
        return false;
    }

    QPixmap cutPixmap(cutVegetableImagePath);
    if (cutPixmap.isNull()) {
        qWarning() << "ChoppingStationItem Error: 无法加载已切蔬菜图片 '" << m_placedVegetableName << "': " << cutVegetableImagePath;
        return false;
    }

    if (m_visualVegetableOnBoard) {
        m_visualVegetableOnBoard->setPixmap(cutPixmap.scaled(35, 35, Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 切好的可以稍微调整大小
        // 可能需要重新居中，如果新图片尺寸变化较大
        qreal boardWidth = this->boundingRect().width();
        qreal boardHeight = this->boundingRect().height();
        qreal vegWidth = m_visualVegetableOnBoard->boundingRect().width();
        qreal vegHeight = m_visualVegetableOnBoard->boundingRect().height();
        m_visualVegetableOnBoard->setPos( (boardWidth - vegWidth) / 2.0, (boardHeight - vegHeight) / 2.0 );
        m_isCut = true;
        qDebug() << "'" << m_placedVegetableName << "' 已在砧板上切好！现在显示为：" << getCurrentVisualNameOnBoard();
        return true;
    } else {
        qWarning() << "ChoppingStationItem Error：砧板上没有视觉蔬菜可以更新为已切状态。";
        return false;
    }
}
