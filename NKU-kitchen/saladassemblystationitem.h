#ifndef SALADASSEMBLYSTATIONITEM_H
#define SALADASSEMBLYSTATIONITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include "item_definitions.h"

class SaladAssemblyStationItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit SaladAssemblyStationItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);

    bool tryAddIngredient(const QString& ingredientName); // 通用添加配料
    QList<QString> getProducibleItems() const; // 获取当前可生产的物品列表
    QString produceAndTakeItem(const QString& productNameToProduce); // 生产并拿走指定物品
    QMap<QString, int> getCurrentIngredientsOnBoard() const;

private:
    void initializeRecipes();
    void checkAvailableRecipes();

    QMap<QString, int> m_ingredientsOnBoard;
    // 外层Key: 成品名称 (e.g., PRODUCT_SALAD, PRODUCT_BURGER)
    // 内层Map: 配料名称 -> 所需数量
    QMap<QString, QMap<QString, int>> m_recipes;
    QList<QString> m_producibleProductNames;
    QList<QGraphicsPixmapItem*> m_visualIngredients;
    void updateVisualIngredients();
    void clearVisualIngredients();
};

#endif
