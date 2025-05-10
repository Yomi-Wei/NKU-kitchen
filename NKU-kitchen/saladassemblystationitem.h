#ifndef SALADASSEMBLYSTATIONITEM_H
#define SALADASSEMBLYSTATIONITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include <QMap>
#include "item_definitions.h" // << 包含共享常量定义

class SaladAssemblyStationItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit SaladAssemblyStationItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);

    bool tryAddCutIngredient(const QString& cutIngredientName);
    bool canProduceSalad() const;
    QString produceAndTakeSalad();
    QMap<QString, int> getCurrentIngredientsOnBoard() const;

private:
    void checkRecipeMet();

    QMap<QString, int> m_ingredientsOnBoard;
    QMap<QString, int> m_saladRecipe;
    bool m_recipeIsMet;
};

#endif // SALADASSEMBLYSTATIONITEM_H
