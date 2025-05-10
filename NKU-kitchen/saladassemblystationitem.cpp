#include "saladassemblystationitem.h"
#include "player.h" // For ITEM_TYPE_ROLE etc.
#include <QDebug>

SaladAssemblyStationItem::SaladAssemblyStationItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent), m_recipeIsMet(false)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_SALAD_ASSEMBLY_STATION);
    this->setData(ITEM_NAME_ROLE, "沙拉组装台");

    m_saladRecipe["西红柿_已切"] = 1; // 需要1份
    m_saladRecipe["生菜_已切"] = 1;   // 需要1份
}

bool SaladAssemblyStationItem::tryAddCutIngredient(const QString& cutIngredientName)
{
    if (m_recipeIsMet) { // 如果配方已满足，等待玩家取走沙拉，不再接受新材料
        qDebug() << "SaladAssemblyStation: 配方已满足，请先制作/取走沙拉。";
        return false;
    }
    if (!cutIngredientName.endsWith("_已切")) {
        qWarning() << "SaladAssemblyStation Error: 只能添加已切好的食材，收到：" << cutIngredientName;
        return false;
    }
    if (!m_saladRecipe.contains(cutIngredientName)) {
        qDebug() << "SaladAssemblyStation: '" << cutIngredientName << "' 不是当前沙拉配方所需。";
        return false;
    }
    if (m_ingredientsOnBoard.value(cutIngredientName, 0) >= m_saladRecipe.value(cutIngredientName)) {
        qDebug() << "SaladAssemblyStation: '" << cutIngredientName << "' 已达到配方所需数量。";
        return false; // 已经够了
    }

    m_ingredientsOnBoard[cutIngredientName]++;
    qDebug() << "SaladAssemblyStation: '" << cutIngredientName << "' 添加到组装台。当前台面：" << m_ingredientsOnBoard;
    checkRecipeMet();
    return true;
}

void SaladAssemblyStationItem::checkRecipeMet()
{
    bool met = true;
    for (auto it = m_saladRecipe.constBegin(); it != m_saladRecipe.constEnd(); ++it) {
        if (m_ingredientsOnBoard.value(it.key(), 0) < it.value()) {
            met = false;
            break;
        }
    }
    m_recipeIsMet = met;
    if (m_recipeIsMet) {
        qDebug() << "SaladAssemblyStation: 沙拉配方已满足！可以制作了。";
    }
}

bool SaladAssemblyStationItem::canProduceSalad() const
{
    return m_recipeIsMet;
}

QString SaladAssemblyStationItem::produceAndTakeSalad()
{
    if (!m_recipeIsMet) {
        qDebug() << "SaladAssemblyStation: 配方未满足，无法制作沙拉。";
        return QString();
    }

    qDebug() << "SaladAssemblyStation: 正在制作沙拉...";
    // 消耗食材 (直接清空，因为配方满足了就会用掉所有)
    m_ingredientsOnBoard.clear();
    m_recipeIsMet = false; // 重置状态，可以开始新的沙拉制作

    // TODO 可选: 短暂改变组装台的图片为完整沙拉，然后变回
    // e.g., setPixmap(QPixmap(":/images/salad_complete_on_station.png"));
    // QTimer::singleShot(1000, this, [this](){ this->setPixmap(QPixmap(":/images/salad_assembly_station_empty.png")); });

    return "沙拉"; // 成品名称
}

QMap<QString, int> SaladAssemblyStationItem::getCurrentIngredientsOnBoard() const
{
    return m_ingredientsOnBoard;
}
